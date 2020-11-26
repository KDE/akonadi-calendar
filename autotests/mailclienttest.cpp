/*
    SPDX-FileCopyrightText: 2011 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

// mailclient_p.cpp isn't exported so we include it directly.

#include "mailclient_p.h"

#include <KCalendarCore/Incidence>
#include <KCalendarCore/FreeBusy>
#include <MailTransportAkonadi/MessageQueueJob>
#include <KIdentityManagement/Identity>

#include <qtest_akonadi.h>

#include <QTestEventLoop>
#include <QObject>

static const char *s_ourEmail = "unittests@dev.nul"; // change also in kdepimlibs/akonadi/calendar/tests/unittestenv/kdehome/share/config

using namespace Akonadi;

Q_DECLARE_METATYPE(KIdentityManagement::Identity)
Q_DECLARE_METATYPE(KCalendarCore::Incidence::Ptr)

class FakeMessageQueueJob : public MailTransport::MessageQueueJob
{
public:
    explicit FakeMessageQueueJob(QObject *parent = nullptr)
        : MailTransport::MessageQueueJob(parent)
    {
    }

    void start() override
    {
        UnitTestResult unitTestResult;
        unitTestResult.message = message();
        unitTestResult.from = addressAttribute().from();
        unitTestResult.to = addressAttribute().to();
        unitTestResult.cc = addressAttribute().cc();
        unitTestResult.bcc = addressAttribute().bcc();
        unitTestResult.transportId = transportAttribute().transportId();
        FakeMessageQueueJob::sUnitTestResults << unitTestResult;

        setError(Akonadi::MailClient::ResultSuccess);
        setErrorText(QString());

        emitResult();
    }

    static UnitTestResult::List sUnitTestResults;
};

UnitTestResult::List FakeMessageQueueJob::sUnitTestResults;

class FakeITIPHandlerComponentFactory : public ITIPHandlerComponentFactory
{
public:
    explicit FakeITIPHandlerComponentFactory(QObject *parent = nullptr)
        : ITIPHandlerComponentFactory(parent)
    {
    }

    MailTransport::MessageQueueJob *createMessageQueueJob(const KCalendarCore::IncidenceBase::Ptr &incidence, const KIdentityManagement::Identity &identity, QObject *parent = nullptr) override
    {
        Q_UNUSED(incidence)
        Q_UNUSED(identity)
        return new FakeMessageQueueJob(parent);
    }
};

class MailClientTest : public QObject
{
    Q_OBJECT

private:
    MailClient *mMailClient = nullptr;
    int mPendingSignals;
    MailClient::Result mLastResult;
    QString mLastErrorMessage;

private Q_SLOTS:

    void initTestCase()
    {
        AkonadiTest::checkTestIsIsolated();

        mPendingSignals = 0;
        mMailClient = new MailClient(new FakeITIPHandlerComponentFactory(this), this);
        mLastResult = MailClient::ResultSuccess;
        connect(mMailClient, &MailClient::finished,
                this, &MailClientTest::handleFinished);
    }

    void cleanupTestCase()
    {
    }

    void testMailAttendees_data()
    {
        QTest::addColumn<KCalendarCore::Incidence::Ptr>("incidence");
        QTest::addColumn<KIdentityManagement::Identity>("identity");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedCcList");
        QTest::addColumn<QStringList>("expectedBccList");

        KCalendarCore::Incidence::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagement::Identity identity;
        bool bccMe;
        QString attachment;
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultNoAttendees;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        const QString expectedFrom = QStringLiteral("unittests@dev.nul");   // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), QStringLiteral("unittests@dev.nul"));

        QStringList toList;
        QStringList toCcList;
        QStringList toBccList;
        //----------------------------------------------------------------------------------------------
        QTest::newRow("No attendees") << incidence << identity << bccMe << attachment << transport
                                      << expectedResult << -1 << QString()
                                      << toList << toCcList << toBccList;
        //----------------------------------------------------------------------------------------------
        // One attendee, but without e-mail
        KCalendarCore::Attendee attendee(QStringLiteral("name1"), QString());
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->addAttendee(attendee);
        expectedResult = MailClient::ResultReallyNoAttendees;
        QTest::newRow("No attendees with email") << incidence << identity << bccMe << attachment << transport
                                                 << expectedResult << -1 << QString()
                                                 << toList << toCcList << toBccList;
        //----------------------------------------------------------------------------------------------
        // One valid attendee
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->addAttendee(attendee);
        incidence->setOrganizer(organizer);
        expectedResult = MailClient::ResultSuccess;
        toList << QStringLiteral("test@foo.org");
        QTest::newRow("One attendee") << incidence << identity << bccMe << attachment << transport
                                      << expectedResult << expectedTransportId << expectedFrom
                                      << toList << toCcList << toBccList;
        //----------------------------------------------------------------------------------------------
        // One valid attendee
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        QString invalidTransport = QStringLiteral("foo");
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        QTest::newRow("Invalid transport") << incidence << identity << bccMe << attachment
                                           << invalidTransport  << expectedResult
                                           << expectedTransportId << expectedFrom
                                           << toList << toCcList << toBccList;
        //----------------------------------------------------------------------------------------------
        // One valid attendee, and bcc me
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        toBccList.clear();
        toBccList << QStringLiteral("unittests@dev.nul");
        QTest::newRow("Test bcc") << incidence << identity << /*bccMe*/ true << attachment
                                  << transport  << expectedResult
                                  << expectedTransportId << expectedFrom
                                  << toList << toCcList << toBccList;
        //----------------------------------------------------------------------------------------------
        // Test CC list
        attendee = KCalendarCore::Attendee(QStringLiteral("name1"), QStringLiteral("test@foo.org"));
        KCalendarCore::Attendee optionalAttendee(QStringLiteral("opt"), QStringLiteral("optional@foo.org"));
        KCalendarCore::Attendee nonParticipant(QStringLiteral("non"), QStringLiteral("non@foo.org"));
        optionalAttendee.setRole(KCalendarCore::Attendee::OptParticipant);
        nonParticipant.setRole(KCalendarCore::Attendee::NonParticipant);
        incidence = KCalendarCore::Incidence::Ptr(new KCalendarCore::Event());
        incidence->setOrganizer(organizer);
        incidence->addAttendee(attendee);
        incidence->addAttendee(optionalAttendee);
        incidence->addAttendee(nonParticipant);
        expectedResult = MailClient::ResultSuccess;
        // Should default to the default transport
        toBccList.clear();
        toBccList << QStringLiteral("unittests@dev.nul");

        toCcList.clear();
        toCcList << QStringLiteral("optional@foo.org")
                 << QStringLiteral("non@foo.org");
        QTest::newRow("Test cc") << incidence << identity << /*bccMe*/ true << attachment
                                 << transport  << expectedResult
                                 << expectedTransportId << expectedFrom
                                 << toList << toCcList << toBccList;
    }

    void testMailAttendees()
    {
        QFETCH(KCalendarCore::Incidence::Ptr, incidence);
        QFETCH(KIdentityManagement::Identity, identity);
        QFETCH(bool, bccMe);
        QFETCH(QString, attachment);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedCcList);
        QFETCH(QStringList, expectedBccList);

        FakeMessageQueueJob::sUnitTestResults.clear();

        mPendingSignals = 1;
        mMailClient->mailAttendees(incidence, identity, bccMe, attachment, transport);
        waitForSignals();

        if (mLastResult != expectedResult) {
            qDebug() << "Fail1: last=" << mLastResult << "; expected=" << expectedResult
                     << "; error=" << mLastErrorMessage;
            QVERIFY(false);
        }

        UnitTestResult unitTestResult;
        if (FakeMessageQueueJob::sUnitTestResults.isEmpty()) {
            qDebug() << "mail results are empty";
        } else {
            unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
        }

        if (expectedTransportId != -1 && unitTestResult.transportId != expectedTransportId) {
            qDebug() << "got " << unitTestResult.transportId
                     << "; expected=" << expectedTransportId;
            QVERIFY(false);
        }

        QCOMPARE(unitTestResult.from, expectedFrom);
        QCOMPARE(unitTestResult.to, expectedToList);
        QCOMPARE(unitTestResult.cc, expectedCcList);
        QCOMPARE(unitTestResult.bcc, expectedBccList);
    }

    void testMailOrganizer_data()
    {
        QTest::addColumn<KCalendarCore::IncidenceBase::Ptr>("incidence");
        QTest::addColumn<KIdentityManagement::Identity>("identity");
        QTest::addColumn<QString>("from");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("subject");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedBccList");
        QTest::addColumn<QString>("expectedSubject");

        KCalendarCore::IncidenceBase::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagement::Identity identity;
        const QString from = QLatin1String(s_ourEmail);
        bool bccMe;
        QString attachment;
        QString subject = QStringLiteral("subject1");
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultSuccess;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        QString expectedFrom = from; // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), QStringLiteral("unittests@dev.nul"));
        incidence->setOrganizer(organizer);

        QStringList toList;
        toList << QStringLiteral("unittests@dev.nul");
        QStringList toBccList;
        QString expectedSubject;
        //----------------------------------------------------------------------------------------------
        expectedSubject = subject;
        QTest::newRow("test1") << incidence << identity << from << bccMe << attachment << subject
                               << transport << expectedResult << expectedTransportId << expectedFrom
                               << toList << toBccList << expectedSubject;
        //----------------------------------------------------------------------------------------------
        expectedSubject = QStringLiteral("Free Busy Message");
        incidence = KCalendarCore::IncidenceBase::Ptr(new KCalendarCore::FreeBusy());
        incidence->setOrganizer(organizer);
        QTest::newRow("FreeBusy") << incidence << identity << from << bccMe << attachment << subject
                                  << transport << expectedResult << expectedTransportId << expectedFrom
                                  << toList << toBccList << expectedSubject;
    }

    void testMailOrganizer()
    {
        QFETCH(KCalendarCore::IncidenceBase::Ptr, incidence);
        QFETCH(KIdentityManagement::Identity, identity);
        QFETCH(QString, from);
        QFETCH(bool, bccMe);
        QFETCH(QString, attachment);
        QFETCH(QString, subject);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedBccList);
        QFETCH(QString, expectedSubject);
        FakeMessageQueueJob::sUnitTestResults.clear();

        mPendingSignals = 1;
        mMailClient->mailOrganizer(incidence, identity, from, bccMe, attachment, subject, transport);
        waitForSignals();
        QCOMPARE(mLastResult, expectedResult);

        UnitTestResult unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
        if (expectedTransportId != -1) {
            QCOMPARE(unitTestResult.transportId, expectedTransportId);
        }

        QCOMPARE(unitTestResult.from, expectedFrom);
        QCOMPARE(unitTestResult.to, expectedToList);
        QCOMPARE(unitTestResult.bcc, expectedBccList);
        QCOMPARE(unitTestResult.message->subject()->asUnicodeString(), expectedSubject);
    }

    void testMailTo_data()
    {
        QTest::addColumn<KCalendarCore::IncidenceBase::Ptr>("incidence");
        QTest::addColumn<KIdentityManagement::Identity>("identity");
        QTest::addColumn<QString>("from");
        QTest::addColumn<bool>("bccMe");
        QTest::addColumn<QString>("recipients");
        QTest::addColumn<QString>("attachment");
        QTest::addColumn<QString>("transport");
        QTest::addColumn<MailClient::Result>("expectedResult");
        QTest::addColumn<int>("expectedTransportId");
        QTest::addColumn<QString>("expectedFrom");
        QTest::addColumn<QStringList>("expectedToList");
        QTest::addColumn<QStringList>("expectedBccList");

        KCalendarCore::IncidenceBase::Ptr incidence(new KCalendarCore::Event());
        KIdentityManagement::Identity identity;
        const QString from = QLatin1String(s_ourEmail);
        bool bccMe;
        const QString recipients = QStringLiteral("unittests@dev.nul");
        QString attachment;
        QString transport;
        MailClient::Result expectedResult = MailClient::ResultSuccess;
        const int expectedTransportId = 69372773; // from tests/unittestenv/kdehome/share/config/mailtransports
        QString expectedFrom = from; // from tests/unittestenv/kdehome/share/config/emailidentities
        KCalendarCore::Person organizer(QStringLiteral("Organizer"), QStringLiteral("unittests@dev.nul"));
        QStringList toList;
        toList << QLatin1String(s_ourEmail);
        QStringList toBccList;
        //----------------------------------------------------------------------------------------------
        QTest::newRow("test1") << incidence << identity << from << bccMe << recipients << attachment
                               << transport << expectedResult << expectedTransportId << expectedFrom
                               << toList << toBccList;
    }

    void testMailTo()
    {
        QFETCH(KCalendarCore::IncidenceBase::Ptr, incidence);
        QFETCH(KIdentityManagement::Identity, identity);
        QFETCH(QString, from);
        QFETCH(bool, bccMe);
        QFETCH(QString, recipients);
        QFETCH(QString, attachment);
        QFETCH(QString, transport);
        QFETCH(MailClient::Result, expectedResult);
        QFETCH(int, expectedTransportId);
        QFETCH(QString, expectedFrom);
        QFETCH(QStringList, expectedToList);
        QFETCH(QStringList, expectedBccList);
        FakeMessageQueueJob::sUnitTestResults.clear();

        mPendingSignals = 1;
        mMailClient->mailTo(incidence, identity, from, bccMe, recipients, attachment, transport);
        waitForSignals();
        QCOMPARE(mLastResult, expectedResult);
        UnitTestResult unitTestResult = FakeMessageQueueJob::sUnitTestResults.first();
        if (expectedTransportId != -1) {
            QCOMPARE(unitTestResult.transportId, expectedTransportId);
        }

        QCOMPARE(unitTestResult.from, expectedFrom);
        QCOMPARE(unitTestResult.to, expectedToList);
        QCOMPARE(unitTestResult.bcc, expectedBccList);
    }

    void handleFinished(Akonadi::MailClient::Result result, const QString &errorMessage)
    {
        qDebug() << "handleFinished: " << result << errorMessage;
        mLastResult = result;
        mLastErrorMessage = errorMessage;
        --mPendingSignals;
        QTestEventLoop::instance().exitLoop();
    }

    void waitForSignals()
    {
        if (mPendingSignals > 0) {
            QTestEventLoop::instance().enterLoop(5);   // 5 seconds is enough
            QVERIFY(!QTestEventLoop::instance().timeout());
        }
    }
};

QTEST_AKONADIMAIN(MailClientTest)

#include "mailclienttest.moc"
