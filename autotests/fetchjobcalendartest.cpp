/*
    Copyright (c) 2011 Sérgio Martins <iamsergio@gmail.com>

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

#include "../src/fetchjobcalendar.h"
#include <itemfetchjob.h>
#include <itemcreatejob.h>
#include <collectionfetchjob.h>
#include <collectionfetchscope.h>
#include <qtest_akonadi.h>

using namespace Akonadi;
using namespace KCalCore;

class FetchJobCalendarTest : public QObject
{
    Q_OBJECT
    Collection mCollection;

    void createIncidence(const QString &uid)
    {
        Item item;
        item.setMimeType(Event::eventMimeType());
        Incidence::Ptr incidence(new Event());
        incidence->setUid(uid);
        incidence->setSummary(QStringLiteral("summary"));
        incidence->setDtStart(KDateTime::currentDateTime(KDateTime::UTC));
        item.setPayload<KCalCore::Incidence::Ptr>(incidence);
        ItemCreateJob *job = new ItemCreateJob(item, mCollection, this);
        AKVERIFYEXEC(job);
    }

    void fetchCollection()
    {
        CollectionFetchJob *job = new CollectionFetchJob(Collection::root(),
                                                         CollectionFetchJob::Recursive,
                                                         this);
        // Get list of collections
        job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
        AKVERIFYEXEC(job);

        // Find our collection
        Collection::List collections = job->collections();
        QVERIFY(!collections.isEmpty());
        mCollection = collections.first();

        QVERIFY(mCollection.isValid());
    }
private Q_SLOTS:
    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();

        fetchCollection();
        qRegisterMetaType<Akonadi::Item>("Akonadi::Item");
    }

    void testFetching()
    {
        createIncidence(QStringLiteral("a"));
        createIncidence(QStringLiteral("b"));
        createIncidence(QStringLiteral("c"));
        createIncidence(QStringLiteral("d"));
        createIncidence(QStringLiteral("e"));
        createIncidence(QStringLiteral("f"));

        FetchJobCalendar calendar;
        QSignalSpy spy(&calendar, &FetchJobCalendar::loadFinished);
        QVERIFY(spy.wait(1000));
        QVERIFY2(spy.at(0).at(0).toBool(), qPrintable(spy.at(0).at(1).toString()));

        const Incidence::List incidences = calendar.incidences();
        QCOMPARE(incidences.count(), 6);
        QVERIFY(calendar.item(QStringLiteral("a")).isValid());
        QVERIFY(calendar.item(QStringLiteral("b")).isValid());
        QVERIFY(calendar.item(QStringLiteral("c")).isValid());
        QVERIFY(calendar.item(QStringLiteral("d")).isValid());
        QVERIFY(calendar.item(QStringLiteral("e")).isValid());
        QVERIFY(calendar.item(QStringLiteral("f")).isValid());
    }
};

QTEST_AKONADIMAIN(FetchJobCalendarTest)

#include "fetchjobcalendartest.moc"
