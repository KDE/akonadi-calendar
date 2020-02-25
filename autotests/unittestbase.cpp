/*
    Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "unittestbase.h"
#include "helper.h"
#include "../src/fetchjobcalendar.h"
#include "mailclient_p.h"

#include <kcalendarcore/event.h>
#include <kcalendarcore/icalformat.h>
#include <item.h>
#include <itemcreatejob.h>
#include <incidencechanger.h>
#include <itiphandler.h>

#include <QString>
#include <QFile>
#include <QByteArray>
#include <QTestEventLoop>
#include <QTest>

using namespace Akonadi;
using namespace KCalendarCore;

UnitTestBase::UnitTestBase()
{
    qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    qRegisterMetaType<QList<Akonadi::IncidenceChanger::ChangeType> >("QList<Akonadi::IncidenceChanger::ChangeType>");
    qRegisterMetaType<QVector<Akonadi::Item::Id> >("QVector<Akonadi::Item::Id>");
    qRegisterMetaType<Akonadi::MailClient::Result>("Akonadi::MailClient::Result");

    mChanger = new IncidenceChanger(this);
    mChanger->setShowDialogsOnError(false);
    mChanger->setHistoryEnabled(true);

    mCollection = Helper::fetchCollection();
    Q_ASSERT(mCollection.isValid());
    mChanger->setDefaultCollection(mCollection);
}

void UnitTestBase::waitForIt()
{
    QTestEventLoop::instance().enterLoop(10);
    QVERIFY(!QTestEventLoop::instance().timeout());
}

void UnitTestBase::stopWaiting()
{
    QTestEventLoop::instance().exitLoop();
}

void UnitTestBase::createIncidence(const QString &uid)
{
    Item item = generateIncidence(uid);
    createIncidence(item);
}

void UnitTestBase::createIncidence(const Item &item)
{
    QVERIFY(mCollection.isValid());
    ItemCreateJob *job = new ItemCreateJob(item, mCollection, this);
    QVERIFY(job->exec());
}

void UnitTestBase::verifyExists(const QString &uid, bool exists)
{
    FetchJobCalendar *calendar = new FetchJobCalendar();
    connect(calendar, &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();
    calendar->deleteLater();

    QCOMPARE(calendar->incidence(uid) != nullptr, exists);
}

Akonadi::Item::List UnitTestBase::calendarItems()
{
    FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr(new FetchJobCalendar());
    connect(calendar.data(), &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();
    KCalendarCore::ICalFormat format;
    QString dump = format.toString(calendar.staticCast<KCalendarCore::Calendar>());
    qDebug() << dump;
    calendar->deleteLater();
    return calendar->items();
}

void UnitTestBase::onLoadFinished(bool success, const QString &)
{
    QVERIFY(success);
    stopWaiting();
}

void UnitTestBase::compareCalendars(const KCalendarCore::Calendar::Ptr &expectedCalendar)
{
    FetchJobCalendar::Ptr calendar = FetchJobCalendar::Ptr(new FetchJobCalendar());
    connect(calendar.data(), &FetchJobCalendar::loadFinished, this, &UnitTestBase::onLoadFinished);
    waitForIt();

    // Now compare the expected calendar to the calendar we got.
    Incidence::List incidences = calendar->incidences();
    const Incidence::List expectedIncidences = expectedCalendar->incidences();

    // First, replace the randomly generated UIDs with the UID that came in the invitation e-mail...
    for (const KCalendarCore::Incidence::Ptr &incidence : qAsConst(incidences)) {
        incidence->setUid(incidence->schedulingID());
        qDebug() << "We have incidece with uid=" << incidence->uid()
                 << "; instanceidentifier=" << incidence->instanceIdentifier();
        auto attendees = incidence->attendees();
        for (auto &attendee : attendees) {
            attendee.setUid(attendee.email());
        }
        incidence->setAttendees(attendees);
    }

    // ... so we can compare them
    for (const KCalendarCore::Incidence::Ptr &incidence : expectedIncidences) {
        incidence->setUid(incidence->schedulingID());
        qDebug() << "We expect incidece with uid=" << incidence->uid()
                 << "; instanceidentifier=" << incidence->instanceIdentifier();
        auto attendees = incidence->attendees();
        for (auto &attendee : attendees) {
            attendee.setUid(attendee.email());
        }
        incidence->setAttendees(attendees);
    }

    QCOMPARE(incidences.count(), expectedIncidences.count());

    for (const KCalendarCore::Incidence::Ptr &expectedIncidence : expectedIncidences) {
        KCalendarCore::Incidence::Ptr incidence;
        for (int i = 0; i < incidences.count(); i++) {
            if (incidences.at(i)->instanceIdentifier() == expectedIncidence->instanceIdentifier()) {
                incidence = incidences.at(i);
                incidences.remove(i);
                break;
            }
        }
        QVERIFY(incidence);
        // Don't fail on creation times, which are obviously different
        expectedIncidence->setCreated(incidence->created());
        incidence->removeCustomProperty(QByteArray("LIBKCAL"), QByteArray("ID"));

        if (*expectedIncidence != *incidence) {
            ICalFormat format;
            QString expectedData = format.toString(expectedIncidence);
            QString gotData = format.toString(incidence);
            qDebug() << "Test failed, expected:\n" << expectedData << "\nbut got " << gotData;
            QVERIFY(false);
        }
    }
}

/** static */
QByteArray UnitTestBase::readFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "File could not be opened for reading:" << filename;
        return QByteArray();
    }

    return file.readAll();
}

Item UnitTestBase::generateIncidence(const QString &uid, const QString &organizer)
{
    Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    KCalendarCore::Incidence::Ptr incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());

    if (!uid.isEmpty()) {
        incidence->setUid(uid);
    }

    const QDateTime now = QDateTime::currentDateTimeUtc();
    incidence->setDtStart(now);
    incidence->setDateTime(now.addSecs(3600), Incidence::RoleEnd);
    incidence->setSummary(QStringLiteral("summary"));
    item.setPayload<KCalendarCore::Incidence::Ptr>(incidence);

    if (!organizer.isEmpty()) {
        incidence->setOrganizer(organizer);
    }

    return item;
}
