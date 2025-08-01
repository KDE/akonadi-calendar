/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectioncalendar.h"
using namespace Qt::Literals::StringLiterals;

#include "akonadicalendar_debug.h"
#include "calendarbase_p.h"

#include <Akonadi/CalendarUtils>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Monitor>

#include <QAbstractProxyModel>

using namespace KCalendarCore;

namespace Akonadi
{

namespace
{

Akonadi::EntityTreeModel *findETM(QAbstractItemModel *model)
{
    while (model) {
        if (auto etm = qobject_cast<Akonadi::EntityTreeModel *>(model); etm != nullptr) {
            return etm;
        }
        if (auto proxy = qobject_cast<QAbstractProxyModel *>(model); proxy != nullptr) {
            model = proxy->sourceModel();
        } else {
            break;
        }
    }

    Q_ASSERT_X(false, "CollectionCalendar", "Model is not ETM or a proxy on top of an ETM!");
    return nullptr;
}

}

class CollectionCalendarPrivate : public CalendarBasePrivate
{
    Q_OBJECT
public:
    CollectionCalendarPrivate(QAbstractItemModel *model, CollectionCalendar *qq)
        : CalendarBasePrivate(qq)
        , m_model(model)
        , m_etm(findETM(model))
        , q(qq)
    {
    }

    void setCollection(const Collection &col)
    {
        if (!col.isValid()) {
            return;
        }

        Q_ASSERT(!m_collection.isValid());
        m_collection = col;

        if (m_monitor) {
            m_monitor->setCollectionMonitored(m_collection);
        }

        init();
    }

    Collection m_collection;
    QAbstractItemModel *m_model = nullptr;
    Akonadi::EntityTreeModel *m_etm = nullptr;
    Monitor *m_monitor = nullptr;
    bool m_populatedFromEtm = false;

    QHash<Item::Id, Item> m_itemById;

private:
    void init()
    {
        if (!m_collection.isValid()) {
            return;
        }

        if (!m_model) {
            m_model = createEtm();
        }

        connect(m_model, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last) {
            if (!isMatchingCollection(parent)) {
                return;
            }

            handleRowsInsertedUnchecked(parent, first, last);
        });
        connect(m_model,
                &QAbstractItemModel::rowsAboutToBeMoved,
                this,
                [this](const QModelIndex &parent, int start, int end, const QModelIndex &newParent, int row) {
                    Q_UNUSED(newParent);
                    Q_UNUSED(row);
                    // If rows are about to be moved from collection we monitor, it's like removal from our point of view.
                    // Rows being moved into the collection we monitor is handled in rowsMoved signal handler.
                    if (!isMatchingCollection(parent)) {
                        return;
                    }

                    handleRowsRemovedUnchecked(parent, start, end);
                });
        connect(m_model, &QAbstractItemModel::rowsMoved, this, [this](const QModelIndex &parent, int start, int end, const QModelIndex &newParent, int row) {
            Q_UNUSED(parent);
            // If rows were moved into the collection we monitor, it's like they were added.
            // Rows being moved from the collection we monitor is handled in rowsAboutToBeRemoved signal handler.
            if (!isMatchingCollection(newParent)) {
                return;
            }

            handleRowsInsertedUnchecked(newParent, row, row + (end - start));
        });
        connect(m_model, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex &parent, int first, int last) {
            if (!isMatchingCollection(parent)) {
                return;
            }

            handleRowsRemovedUnchecked(parent, first, last);
        });
        connect(m_model, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
            if (!isMatchingCollection(topLeft.parent())) {
                return;
            }

            auto index = topLeft;
            for (int row = topLeft.row(); row <= bottomRight.row(); ++row) {
                index = index.sibling(row, 0);
                const auto item = m_model->data(index, EntityTreeModel::ItemRole).value<Item>();
                if (item.isValid() || item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                    updateItem(item);
                }
            }
        });
        connect(m_model, &QAbstractItemModel::modelReset, this, [this]() {
            const auto itemList = q->items();
            for (const auto &item : itemList) {
                internalRemove(item);
            }
            m_populatedFromEtm = false;
            populateFromETM();
        });
        connect(m_model, &QAbstractItemModel::layoutChanged, this, &CollectionCalendarPrivate::populateFromETM);

        populateFromETM();
    }

    void handleRowsRemovedUnchecked(const QModelIndex &parent, int first, int last)
    {
        for (int row = first; row <= last; ++row) {
            const auto index = m_model->index(row, 0, parent);
            const auto item = m_model->data(index, EntityTreeModel::ItemRole).value<Item>();
            if (item.isValid() && item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                m_itemById.remove(item.id());
                internalRemove(item);
            }
        }
    }

    void handleRowsInsertedUnchecked(const QModelIndex &parent, int first, int last)
    {
        for (int row = first; row <= last; ++row) {
            const auto index = m_model->index(row, 0, parent);
            const auto item = m_model->data(index, EntityTreeModel::ItemRole).value<Item>();
            if (item.isValid() && item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                m_itemById.insert(item.id(), item);
                internalInsert(item);
            }
        }
    }

    bool isMatchingCollection(const QModelIndex &index) const
    {
        const auto colId = m_model->data(index, EntityTreeModel::CollectionIdRole).toLongLong();
        return colId == m_collection.id();
    }

    void updateItem(const Item &item)
    {
        Incidence::Ptr const newIncidence = CalendarUtils::incidence(item);
        newIncidence->setCustomProperty("VOLATILE", "AKONADI-ID", QString::number(item.id()));
        IncidenceBase::Ptr const existingIncidence = q->incidence(newIncidence->uid(), newIncidence->recurrenceId());

        auto oldItem = m_itemById.value(item.id()); // if not found, seenItem will be invalid

        if (!existingIncidence && !oldItem.isValid()) {
            // We don't know about this one because it was discarded, for example because of not having DTSTART
            return;
        }

        if (existingIncidence) {
            // We set the payload so that the internal incidence pointer and the one in m_itemById stay the same
            auto updatedItem = item;
            updatedItem.setPayload(existingIncidence.staticCast<KCalendarCore::Incidence>());
            m_itemById.insert(item.id(), updatedItem);

            (*existingIncidence.data()) = *(newIncidence.data());
        } else { // seenItem must be valid
            m_itemById.insert(item.id(), item);
            // The item changed it's UID, update our maps, the Google resource changes the UID when we create incidences.
            handleUidChange(oldItem, item, newIncidence->instanceIdentifier());
        }
    }

    void populateFromETM()
    {
        if (m_populatedFromEtm) {
            qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar not populating from ETM - already populated";
            return;
        }
        m_populatedFromEtm = true;

        if (!m_etm->isCollectionTreeFetched()) {
            qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar not populating from ETM - collection tree not fetched";
            return;
        }

        if (!m_etm->isCollectionPopulated(m_collection.id())) {
            qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar not populating from ETM - target collection not populated yet";
            return;
        }

        qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar populating from ETM";

        q->setIsLoading(true);
        const auto colIdx = EntityTreeModel::modelIndexForCollection(m_model, m_collection);
        Q_ASSERT(colIdx.isValid());
        if (!colIdx.isValid()) {
            qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar failed to populate from ETM - couldn't find model index for our Collection"
                                         << m_collection.id();
            return;
        }

        q->startBatchAdding();
        auto idx = m_model->index(0, 0, colIdx);
        std::size_t itemCount = 0;
        while (idx.isValid()) {
            const auto item = m_model->data(idx, EntityTreeModel::ItemRole).value<Item>();
            if (item.isValid() && item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                internalInsert(item);
                ++itemCount;
            }
            idx = idx.siblingAtRow(idx.row() + 1);
        }
        q->endBatchAdding();

        q->setIsLoading(false);

        qCDebug(AKONADICALENDAR_LOG) << "CollectionCalendar for Collection" << m_collection.id() << "populated from ETM with" << itemCount << "incidences";
    }

    EntityTreeModel *createEtm()
    {
        m_monitor = new Monitor(this);
        m_monitor->setCollectionMonitored(m_collection);
        m_monitor->itemFetchScope().fetchFullPayload();
        m_monitor->itemFetchScope().setCacheOnly(true);
        m_monitor->itemFetchScope().setAncestorRetrieval(ItemFetchScope::AncestorRetrieval::Parent);
        const auto mimeTypeList = KCalendarCore::Incidence::mimeTypes();
        for (const auto &mt : mimeTypeList) {
            m_monitor->setMimeTypeMonitored(mt, true);
        }

        return new EntityTreeModel(m_monitor, this);
    }

    CollectionCalendar *const q;
};

} // namespace Akonadi

using namespace Akonadi;

CollectionCalendar::CollectionCalendar(const Akonadi::Collection &col, QObject *parent)
    : Akonadi::CalendarBase(new CollectionCalendarPrivate(nullptr, this), parent)
{
    setCollection(col);

    incidenceChanger()->setDefaultCollection(col);
    incidenceChanger()->setGroupwareCommunication(false);
    incidenceChanger()->setDestinationPolicy(Akonadi::IncidenceChanger::DestinationPolicyNeverAsk);
}

CollectionCalendar::CollectionCalendar(QAbstractItemModel *model, const Akonadi::Collection &col, QObject *parent)
    : Akonadi::CalendarBase(new CollectionCalendarPrivate(model, this), parent)
{
    setCollection(col);
}

CollectionCalendar::~CollectionCalendar() = default;

Akonadi::Collection CollectionCalendar::collection() const
{
    Q_D(const CollectionCalendar);
    return d->m_collection;
}

void CollectionCalendar::setCollection(const Akonadi::Collection &c)
{
    Q_D(CollectionCalendar);

    if (c.id() == d->m_collection.id()) {
        return;
    }

    Q_ASSERT(!d->m_collection.isValid());
    if (d->m_collection.isValid()) {
        qCWarning(AKONADICALENDAR_LOG) << "Cannot change collection of CollectionCalendar at runtime yet, sorry.";
        return;
    }

    setName(Akonadi::CalendarUtils::displayName(d->m_etm, c));
    setAccessMode((c.rights() & (Akonadi::Collection::CanCreateItem | Akonadi::Collection::CanChangeItem)) ? KCalendarCore::ReadWrite
                                                                                                           : KCalendarCore::ReadOnly);
    d->setCollection(c);
}

Akonadi::EntityTreeModel *CollectionCalendar::model() const
{
    Q_D(const CollectionCalendar);
    return d->m_etm;
}

bool CollectionCalendar::addEvent(const KCalendarCore::Event::Ptr &event)
{
    Q_D(CollectionCalendar);

    if (d->m_collection.contentMimeTypes().contains(event->mimeType()) || d->m_collection.contentMimeTypes().contains("text/calendar"_L1)) {
        return CalendarBase::addEvent(event);
    }
    return false;
}

bool CollectionCalendar::addTodo(const KCalendarCore::Todo::Ptr &todo)
{
    Q_D(CollectionCalendar);

    if (d->m_collection.contentMimeTypes().contains(todo->mimeType()) || d->m_collection.contentMimeTypes().contains("text/calendar"_L1)) {
        return CalendarBase::addTodo(todo);
    }
    return false;
}

bool CollectionCalendar::addJournal(const KCalendarCore::Journal::Ptr &journal)
{
    Q_D(CollectionCalendar);

    if (d->m_collection.contentMimeTypes().contains(journal->mimeType()) || d->m_collection.contentMimeTypes().contains("text/calendar"_L1)) {
        return CalendarBase::addJournal(journal);
    }
    return false;
}

bool CollectionCalendar::hasRight(Akonadi::Collection::Right right) const
{
    Q_D(const CollectionCalendar);
    const auto fullCollection = Akonadi::EntityTreeModel::updatedCollection(d->m_model, d->m_collection);
    if (!fullCollection.isValid()) {
        return false;
    }

    return (fullCollection.rights() & right) == right;
}

#include "collectioncalendar.moc"
#include "moc_collectioncalendar.cpp"
