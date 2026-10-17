/*
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "collectioncalendartest.h"
#include "collectioncalendar.h"

#include <QSignalSpy>
#include <QTest>
#include <QUuid>

#include <akonadi/qtest_akonadi.h>

#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemMoveJob>
#include <Akonadi/Monitor>
#include <private/dbus_p.h>

#include <KCalendarCore/Event>

namespace
{

class EphemeralItem
{
public:
    EphemeralItem(const Akonadi::Item &item)
        : mItem(item)
    {
    }
    EphemeralItem &operator=(const Akonadi::Item &item)
    {
        deleteItem();
        mItem = item;
        return *this;
    }

    EphemeralItem(const EphemeralItem &) = delete;
    EphemeralItem(EphemeralItem &&) = delete;
    EphemeralItem &operator=(const EphemeralItem &) = delete;
    EphemeralItem &operator=(EphemeralItem &&) = delete;

    ~EphemeralItem()
    {
        deleteItem();
    }

    const Akonadi::Item *operator->() const
    {
        return &mItem;
    }

    const Akonadi::Item &operator*() const
    {
        return mItem;
    }

    Akonadi::Item &operator*()
    {
        return mItem;
    }

private:
    void deleteItem()
    {
        if (mItem.isValid()) {
            auto job = new Akonadi::ItemDeleteJob(mItem);
            job->exec();
            mItem = {};
        }
    }
    Akonadi::Item mItem;
};

class Observer : public QObject, public KCalendarCore::Calendar::CalendarObserver
{
    Q_OBJECT
public:
    explicit Observer(Akonadi::CollectionCalendar &calendar)
        : mCalendar(calendar)
    {
        mCalendar.registerObserver(this);
    }

    ~Observer()
    {
        mCalendar.unregisterObserver(this);
    }

    QSignalSpy incidenceAddedSpy{this, &Observer::incidenceAdded};
    QSignalSpy incidenceChangedSpy{this, &Observer::incidenceChanged};
    QSignalSpy incidenceRemovedSpy{this, &Observer::incidenceRemoved};

Q_SIGNALS:
    // NOLINTBEGIN(readability-inconsistent-declaration-parameter-name)
    void incidenceAdded(const Akonadi::Item &item);
    void incidenceChanged(const Akonadi::Item &item);
    void incidenceRemoved(const Akonadi::Item &item);
    // NOLINTEND(readability-inconsistent-declaration-parameter-name)

private:
    void calendarIncidenceAdded(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceAdded(mCalendar.item(incidence));
    }

    void calendarIncidenceChanged(const KCalendarCore::Incidence::Ptr &incidence) override
    {
        Q_EMIT incidenceChanged(mCalendar.item(incidence));
    }

    void calendarIncidenceDeleted(const KCalendarCore::Incidence::Ptr &incidence, const KCalendarCore::Calendar *) override
    {
        Akonadi::Item item(incidence->customProperty("VOLATILE", "AKONADI-ID").toLongLong());
        item.setParentCollection(Akonadi::Collection(incidence->customProperty("VOLATILE", "COLLECTION-ID").toLongLong()));

        Q_EMIT incidenceRemoved(item);
    }

    Akonadi::CollectionCalendar &mCalendar;
};

Akonadi::Collection findResourceCollection(const QString &resource)
{
    auto fetch = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);
    fetch->fetchScope().setResource(resource);
    if (!fetch->exec()) {
        qWarning() << "Failed to fetch collection tree";
        return {};
    }
    const auto collectionList = fetch->collections();
    for (const auto &col : collectionList) {
        if (col.resource() == resource) {
            return col;
        }
    }

    qWarning() << "Failed to find a suitable parent collection for our test";
    return {};
}

Akonadi::Item createIncidence(const Akonadi::Collection &parent, const QString &summary, const QString &uid = {})
{
    auto event = KCalendarCore::Event::Ptr::create();
    event->setSummary(summary);
    event->setDtStart(QDateTime(QDate::currentDate(), QTime(10, 0, 0), QTimeZone::utc()));
    event->setDtEnd(QDateTime(QDate::currentDate(), QTime(11, 0, 0), QTimeZone::utc()));
    event->setUid(uid.isEmpty() ? QUuid::createUuid().toString() : uid);

    Akonadi::Item item;
    item.setMimeType(event->mimeType());
    item.setParentCollection(parent);
    item.setPayload(event);
    item.setGid(event->uid());

    auto job = new Akonadi::ItemCreateJob(item, parent);
    if (!job->exec()) {
        qWarning() << "Failed to create test incidence" << summary << ":" << job->errorString();
        return {};
    }
    return job->item();
}

std::unique_ptr<Akonadi::Monitor> createMonitor()
{
    auto monitor = std::make_unique<Akonadi::Monitor>();
    monitor->setAllMonitored(true);
    monitor->itemFetchScope().fetchFullPayload(true);
    monitor->itemFetchScope().setCacheOnly(true);

    return monitor;
}

bool waitForEtmToPopulate(Akonadi::EntityTreeModel *etm)
{
    return QTest::qWaitFor([etm]() {
        return etm->isFullyPopulated();
    });
}

// Returns the ETM's version of the item, invalid if the ETM doesn't have it
Akonadi::Item etmItem(Akonadi::EntityTreeModel *etm, const Akonadi::Item &item)
{
    const auto indexes = Akonadi::EntityTreeModel::modelIndexesForItem(etm, item);
    return indexes.isEmpty() ? Akonadi::Item() : indexes.first().data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
}

// Waits until the ETM holds the given revision of the item
bool waitForItemRevision(Akonadi::EntityTreeModel *etm, const Akonadi::Item &item)
{
    return QTest::qWaitFor([etm, &item]() {
        const auto current = etmItem(etm, item);
        return current.isValid() && current.revision() >= item.revision();
    });
}

bool populateCollection(const Akonadi::Collection &collection, int count)
{
    for (int i = 0; i < count; ++i) {
        const auto item = createIncidence(collection, QStringLiteral("Test Incidence %1").arg(i));
        if (!item.isValid()) {
            return false;
        }
    }

    return true;
}

void configureResource(const Akonadi::AgentInstance &instance, const QString &file)
{
    QDBusInterface iface(Akonadi::DBus::agentServiceName(instance.identifier(), Akonadi::DBus::Resource),
                         QStringLiteral("/Settings"),
                         QStringLiteral("org.kde.Akonadi.ICal.Settings"));
    iface.call(QStringLiteral("setPath"), file);
    iface.call(QStringLiteral("save"));
    instance.reconfigure();
}
}

CollectionCalendarTest::CollectionCalendarTest(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Akonadi::Collection::Id>();
}

void CollectionCalendarTest::initTestCase()
{
    AkonadiTest::checkTestIsIsolated();

    if (!otherResourceConfig.open()) {
        qWarning() << "Cannot open resource:" << otherResourceConfig.fileName();
        return;
    }

    // We need two agents to test that the calendar properly ignores events in other collections
    // However we can't just add it to unittestenv, as having multiple resources breaks other
    // more complex tests that expect exactly one calendar agent.
    auto agentJob = new Akonadi::AgentInstanceCreateJob(QStringLiteral("akonadi_ical_resource"));
    QVERIFY(agentJob->exec());
    configureResource(agentJob->instance(), otherResourceConfig.fileName());

    // Populate the ETM
    testCollection = findResourceCollection(QStringLiteral("akonadi_ical_resource_0"));
    QVERIFY(testCollection.isValid());
    QVERIFY(populateCollection(testCollection, 10));

    otherCollection = findResourceCollection(agentJob->instance().identifier());
    QVERIFY(otherCollection.isValid());
    QVERIFY(populateCollection(otherCollection, 5));
}

void CollectionCalendarTest::testPopulateFromReadyETM()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar const calendar(&etm, testCollection);

    // Should populate right away
    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testPopulateWhenETMLoads()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    Akonadi::CollectionCalendar const calendar(&etm, testCollection);
    QVERIFY(!etm.isFullyPopulated());
    QVERIFY(!etm.isCollectionPopulated(testCollection.id()));
    QVERIFY(calendar.incidences().empty());

    QVERIFY(waitForEtmToPopulate(&etm));

    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testItemAdded()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    EphemeralItem const item = createIncidence(testCollection, QStringLiteral("New Test Item"));
    QVERIFY(observer.incidenceAddedSpy.wait());

    const auto newItem = std::as_const(observer.incidenceAddedSpy)[0][0].value<Akonadi::Item>();
    QCOMPARE(newItem.id(), item->id());
    QVERIFY(newItem.hasPayload<KCalendarCore::Event::Ptr>());
    QCOMPARE(*newItem.payload<KCalendarCore::Event::Ptr>(), *item->payload<KCalendarCore::Event::Ptr>());

    // Also make sure that the incidence is actually obtainable from the calendar
    auto newEvent = calendar.event(item->gid());
    QVERIFY(newEvent);
    QCOMPARE(*newItem.payload<KCalendarCore::Event::Ptr>(), *newEvent);

    QCOMPARE(calendar.incidences().size(), 11);
}

void CollectionCalendarTest::testItemRemoved()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    Akonadi::Item newItem;
    {
        EphemeralItem item = createIncidence(testCollection, QStringLiteral("Will be deleted"));
        newItem = *item;
        QVERIFY(observer.incidenceAddedSpy.wait());
        QCOMPARE(calendar.incidences().size(), 11);
        // EphemeralItem will delete the Item here
    }

    QVERIFY(observer.incidenceRemovedSpy.wait());
    const auto removedItem = std::as_const(observer.incidenceRemovedSpy)[0][0].value<Akonadi::Item>();
    QCOMPARE(removedItem.id(), newItem.id());

    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testItemChanged()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    QCOMPARE(calendar.incidences().size(), 10);

    {
        EphemeralItem item = createIncidence(testCollection, QStringLiteral("Will be changed"));
        QVERIFY(observer.incidenceAddedSpy.wait());
        QCOMPARE(calendar.incidences().size(), 11);

        KCalendarCore::Event::Ptr const copy(item->payload<KCalendarCore::Event::Ptr>()->clone());
        copy->setSummary(QStringLiteral("Changed"));
        Akonadi::Item newItem = *item;
        newItem.setPayload(copy);

        auto modifyJob = new Akonadi::ItemModifyJob(newItem);
        QVERIFY(modifyJob->exec());

        QVERIFY(observer.incidenceChangedSpy.wait());
        const auto changedItem = std::as_const(observer.incidenceChangedSpy)[0][0].value<Akonadi::Item>();
        QCOMPARE(changedItem.id(), newItem.id());
        QVERIFY(changedItem.hasPayload<KCalendarCore::Event::Ptr>());
        QCOMPARE(*changedItem.payload<KCalendarCore::Event::Ptr>(), *copy);
    }
}

void CollectionCalendarTest::testPopulatedItemChanged()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    // The calendar loads this item from the ETM when it is created
    QSignalSpy itemChangedSpy(monitor.get(), &Akonadi::Monitor::itemChanged);
    EphemeralItem item = createIncidence(testCollection, QStringLiteral("Populated"));
    // Wait for the resource to set the remote id, so that the calendar doesn't see that change
    QTRY_VERIFY(std::any_of(itemChangedSpy.cbegin(), itemChangedSpy.cend(), [&item](const QList<QVariant> &args) {
        return args[0].value<Akonadi::Item>().id() == item->id();
    }));
    *item = etmItem(&etm, *item);
    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    const QString oldUid = item->payload<KCalendarCore::Event::Ptr>()->uid();
    QVERIFY(calendar.event(oldUid));

    const QString newUid = QUuid::createUuid().toString();
    KCalendarCore::Event::Ptr const renamed(item->payload<KCalendarCore::Event::Ptr>()->clone());
    renamed->setUid(newUid);
    (*item).setPayload(renamed);
    auto modifyJob = new Akonadi::ItemModifyJob(*item);
    QVERIFY(modifyJob->exec());
    *item = modifyJob->item();
    QVERIFY(waitForItemRevision(&etm, *item));
    QVERIFY(!calendar.event(oldUid));
    QVERIFY(calendar.event(newUid));
    QCOMPARE(calendar.item(newUid).id(), item->id());
    QCOMPARE(calendar.item(item->id()).revision(), item->revision());

    KCalendarCore::Event::Ptr const changed(item->payload<KCalendarCore::Event::Ptr>()->clone());
    changed->setSummary(QStringLiteral("Changed"));
    (*item).setPayload(changed);
    modifyJob = new Akonadi::ItemModifyJob(*item);
    QVERIFY(modifyJob->exec());
    *item = modifyJob->item();
    QVERIFY(waitForItemRevision(&etm, *item));
    QCOMPARE(calendar.event(newUid)->summary(), QStringLiteral("Changed"));
    QCOMPARE(calendar.item(item->id()).revision(), item->revision());
}

void CollectionCalendarTest::testUnrelatedItemIsNotSeen()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));
    QSignalSpy etmAddedSpy(&etm, &Akonadi::EntityTreeModel::rowsInserted);
    QSignalSpy etmChangedSpy(&etm, &Akonadi::EntityTreeModel::dataChanged);
    QSignalSpy etmRemovedSpy(&etm, &Akonadi::EntityTreeModel::rowsRemoved);

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer const observer(calendar);

    {
        // Create new incidence in the other collection
        EphemeralItem item = createIncidence(otherCollection, QStringLiteral("Invisible"));
        // Wait for it to appear in the ETM
        QVERIFY(etmAddedSpy.wait());

        // Our calendar should remain unaffected
        QVERIFY(observer.incidenceAddedSpy.empty());
        QVERIFY(observer.incidenceChangedSpy.empty());
        QVERIFY(observer.incidenceRemovedSpy.empty());
        QCOMPARE(calendar.incidences().size(), 10);

        // Now we modify the incidence in the other collection
        item->payload<KCalendarCore::Event::Ptr>()->setSummary(QStringLiteral("Invisible and Changed"));
        auto modifyJob = new Akonadi::ItemModifyJob(*item);
        QVERIFY(modifyJob->exec());
        // Wait for the change to appear in the ETM
        QVERIFY(etmChangedSpy.wait());

        // Our calendar should still remain unaffected
        QVERIFY(observer.incidenceAddedSpy.empty());
        QVERIFY(observer.incidenceChangedSpy.empty());
        QVERIFY(observer.incidenceRemovedSpy.empty());
        QCOMPARE(calendar.incidences().size(), 10);

        // The item will be deleted when EphemeralItem leaves this scope
    }

    // Wait for ETM to notice the Item has been removed
    QVERIFY(etmRemovedSpy.wait());

    // Our calendar still remains unaffected
    QVERIFY(observer.incidenceAddedSpy.empty());
    QVERIFY(observer.incidenceChangedSpy.empty());
    QVERIFY(observer.incidenceRemovedSpy.empty());
    QCOMPARE(calendar.incidences().size(), 10);
}

void CollectionCalendarTest::testDuplicateUid()
{
    auto monitor = createMonitor();
    Akonadi::EntityTreeModel etm(monitor.get());
    QVERIFY(waitForEtmToPopulate(&etm));

    Akonadi::CollectionCalendar calendar(&etm, testCollection);
    Observer observer(calendar);

    EphemeralItem kept = createIncidence(testCollection, QStringLiteral("Kept"));
    QVERIFY(observer.incidenceAddedSpy.wait());
    const QString uid = kept->gid();
    QCOMPARE(calendar.item(uid).id(), kept->id());

    // The calendar discards a second item with the same UID
    EphemeralItem duplicate = createIncidence(testCollection, QStringLiteral("Duplicate"), uid);
    QVERIFY(duplicate->isValid());
    QTRY_VERIFY(!Akonadi::EntityTreeModel::modelIndexesForItem(&etm, *duplicate).isEmpty());

    // Changing the duplicate must not change the incidence of the kept item
    KCalendarCore::Event::Ptr const changed(duplicate->payload<KCalendarCore::Event::Ptr>()->clone());
    changed->setSummary(QStringLiteral("Changed duplicate"));
    (*duplicate).setPayload(changed);
    auto modifyJob = new Akonadi::ItemModifyJob(*duplicate);
    QVERIFY(modifyJob->exec());
    *duplicate = modifyJob->item();
    QVERIFY(waitForItemRevision(&etm, *duplicate));
    QVERIFY(calendar.event(uid));
    QCOMPARE(calendar.event(uid)->summary(), QStringLiteral("Kept"));

    // Deleting the duplicate must not remove the incidence of the kept item
    const Akonadi::Item deleted = *duplicate;
    duplicate = Akonadi::Item();
    QTRY_VERIFY(Akonadi::EntityTreeModel::modelIndexesForItem(&etm, deleted).isEmpty());
    QVERIFY(calendar.event(uid));
    QCOMPARE(calendar.item(uid).id(), kept->id());

    // Changing the UID of a duplicate must not remove the incidence of the kept item
    EphemeralItem duplicate2 = createIncidence(testCollection, QStringLiteral("Duplicate 2"), uid);
    QVERIFY(duplicate2->isValid());
    QTRY_VERIFY(!Akonadi::EntityTreeModel::modelIndexesForItem(&etm, *duplicate2).isEmpty());
    KCalendarCore::Event::Ptr const newUid(duplicate2->payload<KCalendarCore::Event::Ptr>()->clone());
    newUid->setUid(QUuid::createUuid().toString());
    (*duplicate2).setPayload(newUid);
    modifyJob = new Akonadi::ItemModifyJob(*duplicate2);
    QVERIFY(modifyJob->exec());
    *duplicate2 = modifyJob->item();
    QVERIFY(waitForItemRevision(&etm, *duplicate2));
    QVERIFY(calendar.event(uid));
    QCOMPARE(calendar.item(uid).id(), kept->id());
    QVERIFY(observer.incidenceRemovedSpy.empty());
}

QTEST_AKONADIMAIN(CollectionCalendarTest)

#include "collectioncalendartest.moc"
#include "moc_collectioncalendartest.cpp"
