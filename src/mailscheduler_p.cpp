/*
  SPDX-FileCopyrightText: 2001, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2010, 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mailscheduler_p.h"
#include "calendarbase.h"
#include "calendarsettings.h"
#include "utils_p.h"

#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/ScheduleMessage>
#include <KIdentityManagement/Identity>
#include <KIdentityManagement/IdentityManager>

#include <KLocalizedString>
#include <QStandardPaths>

using namespace Akonadi;
using namespace KIdentityManagement;

class Akonadi::MailSchedulerPrivate
{
public:
    Q_REQUIRED_RESULT KIdentityManagement::Identity identityForIncidence(const KCalendarCore::IncidenceBase::Ptr &incidence) const;

    KIdentityManagement::IdentityManager *m_identityManager = nullptr;
    MailClient *m_mailer = nullptr;
};

MailScheduler::MailScheduler(ITIPHandlerComponentFactory *factory, QObject *parent)
    : Scheduler(parent)
    , d(new MailSchedulerPrivate())
{
    d->m_identityManager = KIdentityManagement::IdentityManager::self();
    d->m_mailer = new MailClient(factory, parent);

    connect(d->m_mailer, &MailClient::finished, this, &MailScheduler::onMailerFinished);
}

MailScheduler::~MailScheduler()
{
    delete d->m_mailer;
}

KIdentityManagement::Identity MailSchedulerPrivate::identityForIncidence(const KCalendarCore::IncidenceBase::Ptr &incidence) const
{
    const auto organizer = incidence->organizer();
    const QString organizerEmail = !organizer.isEmpty() ? organizer.email() : CalendarUtils::email();
    return m_identityManager->identityForAddress(organizerEmail);
}

void MailScheduler::publish(const KCalendarCore::IncidenceBase::Ptr &incidence, const QString &recipients)
{
    Q_ASSERT(incidence);
    if (!incidence) {
        return;
    }

    const QString messageText = mFormat->createScheduleMessage(incidence, KCalendarCore::iTIPPublish);
    d->m_mailer->mailTo(incidence,
                        d->identityForIncidence(incidence),
                        CalendarUtils::email(),
                        CalendarSettings::self()->bcc(),
                        recipients,
                        messageText,
                        CalendarSettings::self()->mailTransport());
}

void MailScheduler::performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::iTIPMethod method, const QString &recipients)
{
    Q_ASSERT(incidence);
    if (!incidence) {
        return;
    }
    const QString messageText = mFormat->createScheduleMessage(incidence, method);

    d->m_mailer->mailTo(incidence,
                        d->identityForIncidence(incidence),
                        Akonadi::CalendarUtils::email(),
                        CalendarSettings::self()->bcc(),
                        recipients,
                        messageText,
                        CalendarSettings::self()->mailTransport());
}

void MailScheduler::performTransaction(const KCalendarCore::IncidenceBase::Ptr &incidence, KCalendarCore::iTIPMethod method)
{
    Q_ASSERT(incidence);
    if (!incidence) {
        return;
    }

    const QString messageText = mFormat->createScheduleMessage(incidence, method);

    if (method == KCalendarCore::iTIPRequest || method == KCalendarCore::iTIPCancel || method == KCalendarCore::iTIPAdd
        || method == KCalendarCore::iTIPDeclineCounter) {
        d->m_mailer->mailAttendees(incidence,
                                   d->identityForIncidence(incidence),
                                   CalendarSettings::self()->bcc(),
                                   messageText,
                                   CalendarSettings::self()->mailTransport());
    } else {
        QString subject;
        KCalendarCore::Incidence::Ptr inc = incidence.dynamicCast<KCalendarCore::Incidence>();
        if (inc && method == KCalendarCore::iTIPCounter) {
            subject = i18n("Counter proposal: %1", inc->summary());
        }

        d->m_mailer->mailOrganizer(incidence,
                                   d->identityForIncidence(incidence),
                                   CalendarUtils::email(),
                                   CalendarSettings::self()->bcc(),
                                   messageText,
                                   subject,
                                   CalendarSettings::self()->mailTransport());
    }
}

QString MailScheduler::freeBusyDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/korganizer/freebusy");
}

// TODO: AKONADI_PORT review following code
void MailScheduler::acceptCounterProposal(const KCalendarCore::Incidence::Ptr &incidence, const Akonadi::CalendarBase::Ptr &calendar)
{
    Q_ASSERT(incidence);
    Q_ASSERT(calendar);
    if (!incidence || !calendar) {
        return;
    }

    Akonadi::Item exInc = calendar->item(incidence);
    if (!exInc.isValid()) {
        KCalendarCore::Incidence::Ptr exIncidence = calendar->incidenceFromSchedulingID(incidence->uid());
        if (exIncidence) {
            exInc = calendar->item(exIncidence);
        }
        // exInc = exIncItem.isValid() && exIncItem.hasPayload<KCalendarCore::Incidence::Ptr>() ?
        //        exIncItem.payload<KCalendarCore::Incidence::Ptr>() : KCalendarCore::Incidence::Ptr();
    }

    incidence->setRevision(incidence->revision() + 1);
    Result result = ResultSuccess;

    if (exInc.isValid() && exInc.hasPayload<KCalendarCore::Incidence::Ptr>()) {
        auto exIncPtr = exInc.payload<KCalendarCore::Incidence::Ptr>();
        incidence->setRevision(qMax(incidence->revision(), exIncPtr->revision() + 1));
        // some stuff we don't want to change, just to be safe
        incidence->setSchedulingID(exIncPtr->schedulingID());
        incidence->setUid(exIncPtr->uid());

        Q_ASSERT(exIncPtr && incidence);

        KCalendarCore::IncidenceBase::Ptr i1 = exIncPtr;
        KCalendarCore::IncidenceBase::Ptr i2 = incidence;

        if (i1->type() == i2->type()) {
            *i1 = *i2;
        }

        exIncPtr->updated();

        if (!calendar->modifyIncidence(exIncPtr)) {
            result = ResultModifyingError;
        }
    } else {
        if (!calendar->addIncidence(KCalendarCore::Incidence::Ptr(incidence->clone()))) {
            result = ResultCreatingError;
        }
    }

    if (result != ResultSuccess) {
        Q_EMIT transactionFinished(result, QStringLiteral("Error creating job"));
    } else {
        // Nothing to do here. Signal will be emitted when we hear back from the calendar.
    }
}

void MailScheduler::onMailerFinished(Akonadi::MailClient::Result result, const QString &errorMsg)
{
    if (result == MailClient::ResultSuccess) {
        Q_EMIT transactionFinished(ResultSuccess, QString());
    } else {
        const QString message = i18n("Error sending e-mail: ") + errorMsg;
        Q_EMIT transactionFinished(ResultGenericError, message);
    }
}
