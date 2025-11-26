/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calfilterpartstatusproxymodel.h"
#include "akonadicalendar_debug.h"
#include "calendarutils.h"
#include "utils_p.h"

#include <Akonadi/Collection>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>

#include <KCalendarCore/Incidence>
// #include <email.h>

#include <KIdentityManagementCore/IdentityManager>

using namespace Akonadi;

class Akonadi::CalFilterPartStatusProxyModelPrivate
{
public:
    explicit CalFilterPartStatusProxyModelPrivate()
        : mIdentityManager(KIdentityManagementCore::IdentityManager::self())
    {
    }

    QList<KCalendarCore::Attendee::PartStat> mBlockedStatusList;
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    bool mFilterVirtual = false;
};

void CalFilterPartStatusProxyModel::slotIdentitiesChanged()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateRowsFilter();
#endif
}

CalFilterPartStatusProxyModel::CalFilterPartStatusProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new CalFilterPartStatusProxyModelPrivate())
{
    QObject::connect(d->mIdentityManager, qOverload<>(&KIdentityManagementCore::IdentityManager::changed), this, [this]() {
        slotIdentitiesChanged();
    });
}

CalFilterPartStatusProxyModel::~CalFilterPartStatusProxyModel() = default;

const QList<KCalendarCore::Attendee::PartStat> &CalFilterPartStatusProxyModel::blockedStatusList() const
{
    return d->mBlockedStatusList;
}

void CalFilterPartStatusProxyModel::setBlockedStatusList(const QList<KCalendarCore::Attendee::PartStat> &blockStatusList)
{
    if (d->mBlockedStatusList != blockStatusList) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        beginFilterChange();
#endif
        d->mBlockedStatusList = blockStatusList;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
        invalidateRowsFilter();
#endif
    }
}

bool CalFilterPartStatusProxyModel::filterVirtual() const
{
    return d->mFilterVirtual;
}

void CalFilterPartStatusProxyModel::setFilterVirtual(bool filterVirtual)
{
    if (d->mFilterVirtual != filterVirtual) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        beginFilterChange();
#endif
        d->mFilterVirtual = filterVirtual;
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
        endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
        invalidateRowsFilter();
#endif
    }
}

bool CalFilterPartStatusProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) {
        return false;
    }

    const auto item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    // If it's not an item, accept it: it's not our business to filter out collections...
    if (!item.isValid()) {
        return true;
    }

    const KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
    if (!incidence) {
        return false;
    }

    // Incidences from virtual collections are always ok
    const auto col = idx.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (!d->mFilterVirtual && col.isVirtual()) {
        return true;
    }

    // always show if we are the organizer
    if (CalendarUtils::thatIsMe(incidence->organizer().email())) {
        return true;
    }

    const auto attendees = incidence->attendees();
    for (const KCalendarCore::Attendee &attendee : attendees) {
        if (CalendarUtils::thatIsMe(attendee)) {
            return !d->mBlockedStatusList.contains(attendee.status());
        }
    }

    // We are not attendee, so we accept the incidence
    return true;
}

#include "moc_calfilterpartstatusproxymodel.cpp"
