/*
  SPDX-FileCopyrightText: 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mailclient_p.h"

#include "akonadi-calendar-version.h"

#include <collection.h>

#include <KCalUtils/IncidenceFormatter>
#include <KCalendarCore/Attendee>
#include <KCalendarCore/Incidence>
#include <KEmailAddress>
#include <identity.h>

#include <messagequeuejob.h>
#include <transport.h>
#include <transportmanager.h>

#include "akonadicalendar_debug.h"
#include <KJob>
#include <KLocalizedString>
#include <KProtocolManager>

using namespace Akonadi;

MailClient::MailClient(ITIPHandlerComponentFactory *factory, QObject *parent)
    : QObject(parent)
    , mFactory(factory ? factory : new ITIPHandlerComponentFactory(this))
{
}

MailClient::~MailClient()
{
}

void MailClient::mailAttendees(const KCalendarCore::IncidenceBase::Ptr &incidence,
                               const KIdentityManagement::Identity &identity,
                               bool bccMe,
                               const QString &attachment,
                               const QString &mailTransport)
{
    Q_ASSERT(incidence);
    KCalendarCore::Attendee::List attendees = incidence->attendees();
    if (attendees.isEmpty()) {
        qCWarning(AKONADICALENDAR_LOG) << "There are no attendees to e-mail";
        Q_EMIT finished(ResultNoAttendees, i18n("There are no attendees to e-mail"));
        return;
    }

    const QString from = incidence->organizer().fullName();
    const QString organizerEmail = incidence->organizer().email();

    QStringList toList;
    QStringList ccList;
    const int numberOfAttendees = attendees.count();
    for (int i = 0; i < numberOfAttendees; ++i) {
        const KCalendarCore::Attendee a = attendees.at(i);

        const QString email = a.email();
        if (email.isEmpty()) {
            continue;
        }

        // In case we (as one of our identities) are the organizer we are sending
        // this mail. We could also have added ourselves as an attendee, in which
        // case we don't want to send ourselves a notification mail.
        if (organizerEmail == email) {
            continue;
        }

        // Optional Participants and Non-Participants are copied on the email
        if (a.role() == KCalendarCore::Attendee::OptParticipant || a.role() == KCalendarCore::Attendee::NonParticipant) {
            ccList << a.email();
        } else {
            toList << a.email();
        }
    }
    if (toList.isEmpty() && ccList.isEmpty()) {
        // Not really to be called a groupware meeting, eh
        qCWarning(AKONADICALENDAR_LOG) << "There are really no attendees to e-mail";
        Q_EMIT finished(ResultReallyNoAttendees, i18n("There are no attendees to e-mail"));
        return;
    }
    QString to;
    if (!toList.isEmpty()) {
        to = toList.join(QLatin1String(", "));
    }
    QString cc;
    if (!ccList.isEmpty()) {
        cc = ccList.join(QLatin1String(", "));
    }

    QString subject;
    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        subject = inc->summary();
    } else {
        subject = i18n("Free Busy Object");
    }

    const QString body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);

    send(incidence, identity, from, to, cc, subject, body, false, bccMe, attachment, mailTransport);
}

void MailClient::mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &incidence,
                               const KIdentityManagement::Identity &identity,
                               const QString &from,
                               bool bccMe,
                               const QString &attachment,
                               const QString &sub,
                               const QString &mailTransport)
{
    const QString to = incidence->organizer().fullName();
    QString subject = sub;

    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        if (subject.isEmpty()) {
            subject = inc->summary();
        }
    } else {
        subject = i18n("Free Busy Message");
    }

    const QString body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);

    send(incidence, identity, from, to, QString(), subject, body, false, bccMe, attachment, mailTransport);
}

void MailClient::mailTo(const KCalendarCore::IncidenceBase::Ptr &incidence,
                        const KIdentityManagement::Identity &identity,
                        const QString &from,
                        bool bccMe,
                        const QString &recipients,
                        const QString &attachment,
                        const QString &mailTransport)
{
    QString subject;

    if (incidence->type() != KCalendarCore::Incidence::TypeFreeBusy) {
        KCalendarCore::Incidence::Ptr inc = incidence.staticCast<KCalendarCore::Incidence>();
        subject = inc->summary();
    } else {
        subject = i18n("Free Busy Message");
    }

    const QString body = KCalUtils::IncidenceFormatter::mailBodyStr(incidence);

    send(incidence, identity, from, recipients, QString(), subject, body, false, bccMe, attachment, mailTransport);
}

static QStringList extractEmailAndNormalize(const QString &email)
{
    const QStringList splittedEmail = KEmailAddress::splitAddressList(email);
    QStringList normalizedEmail;
    normalizedEmail.reserve(splittedEmail.count());
    for (const QString &email : splittedEmail) {
        const QString str = KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(email));
        normalizedEmail << str;
    }
    return normalizedEmail;
}

void MailClient::send(const KCalendarCore::IncidenceBase::Ptr &incidence,
                      const KIdentityManagement::Identity &identity,
                      const QString &from,
                      const QString &_to,
                      const QString &cc,
                      const QString &subject,
                      const QString &body,
                      bool hidden,
                      bool bccMe,
                      const QString &attachment,
                      const QString &mailTransport)
{
    Q_UNUSED(hidden)
    if (!MailTransport::TransportManager::self()->showTransportCreationDialog(nullptr, MailTransport::TransportManager::IfNoTransportExists)) {
        qCritical() << "Error while creating transport";
        Q_EMIT finished(ResultErrorCreatingTransport, i18n("Error while creating transport"));
        return;
    }

    // We must have a recipients list for most MUAs. Thus, if the 'to' list
    // is empty simply use the 'from' address as the recipient.
    QString to = _to;
    if (to.isEmpty()) {
        to = from;
    }
    qCDebug(AKONADICALENDAR_LOG) << "\nFrom:" << from << "\nTo:" << to << "\nCC:" << cc << "\nSubject:" << subject << "\nBody: \n"
                                 << body << "\nAttachment:\n"
                                 << attachment << "\nmailTransport: " << mailTransport;

    MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportByName(mailTransport);

    if (!transport) {
        transport = MailTransport::TransportManager::self()->transportByName(MailTransport::TransportManager::self()->defaultTransportName());
    }

    if (!transport) {
        qCritical() << "Error fetching transport; mailTransport" << mailTransport << MailTransport::TransportManager::self()->defaultTransportName();
        Q_EMIT finished(ResultErrorFetchingTransport, i18n("Error fetching transport. Unable to send invitations"));
        return;
    }

    const int transportId = transport->id();

    // gather config values
    KConfig config(QStringLiteral("kmail2rc"));

    KConfigGroup configGroup(&config, QStringLiteral("Invitations"));
    const bool outlookConformInvitation = configGroup.readEntry("LegacyBodyInvites",
#ifdef KDEPIM_ENTERPRISE_BUILD
                                                                true
#else
                                                                false
#endif
    );

    // Now build the message we like to send. The message KMime::Message::Ptr instance
    // will be the root message that has 2 additional message. The body itself and
    // the attached cal.ics calendar file.
    KMime::Message::Ptr message = KMime::Message::Ptr(new KMime::Message);
    message->contentTransferEncoding()->clear(); // 7Bit, decoded.

    // Set the headers

    message->userAgent()->fromUnicodeString(KProtocolManager::userAgentForApplication(QStringLiteral("KOrganizer"), QStringLiteral(AKONADI_CALENDAR_VERSION)),
                                            "utf-8");

    message->from()->fromUnicodeString(from, "utf-8");
    message->to()->fromUnicodeString(to, "utf-8");
    message->cc()->fromUnicodeString(cc, "utf-8");
    if (bccMe) {
        message->bcc()->fromUnicodeString(from, "utf-8"); // from==me, right?
    }
    message->date()->setDateTime(QDateTime::currentDateTime());
    message->subject()->fromUnicodeString(subject, "utf-8");

    if (outlookConformInvitation) {
        message->contentType()->setMimeType("text/calendar");
        message->contentType()->setCharset("utf-8");
        message->contentType()->setName(QStringLiteral("cal.ics"), "utf-8");
        message->contentType()->setParameter(QStringLiteral("method"), QStringLiteral("request"));

        if (!attachment.isEmpty()) {
            auto disposition = new KMime::Headers::ContentDisposition;
            disposition->setDisposition(KMime::Headers::CDinline);
            message->setHeader(disposition);
            message->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
            message->setBody(KMime::CRLFtoLF(attachment.toUtf8()));
        }
    } else {
        // We need to set following 4 lines by hand else KMime::Content::addContent
        // will create a new Content instance for us to attach the main message
        // what we don't need cause we already have the main message instance where
        // 2 additional messages are attached.
        KMime::Headers::ContentType *ct = message->contentType();
        ct->setMimeType("multipart/mixed");
        ct->setBoundary(KMime::multiPartBoundary());
        ct->setCategory(KMime::Headers::CCcontainer);

        // Set the first multipart, the body message.
        auto bodyMessage = new KMime::Content;
        auto bodyDisposition = new KMime::Headers::ContentDisposition;
        bodyDisposition->setDisposition(KMime::Headers::CDinline);
        bodyMessage->contentType()->setMimeType("text/plain");
        bodyMessage->contentType()->setCharset("utf-8");
        bodyMessage->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
        bodyMessage->setBody(KMime::CRLFtoLF(body.toUtf8()));
        bodyMessage->setHeader(bodyDisposition);
        message->addContent(bodyMessage);

        // Set the second multipart, the attachment.
        if (!attachment.isEmpty()) {
            auto attachMessage = new KMime::Content;
            auto attachDisposition = new KMime::Headers::ContentDisposition;
            attachDisposition->setDisposition(KMime::Headers::CDattachment);
            attachMessage->contentType()->setMimeType("text/calendar");
            attachMessage->contentType()->setCharset("utf-8");
            attachMessage->contentType()->setName(QStringLiteral("cal.ics"), "utf-8");
            attachMessage->contentType()->setParameter(QStringLiteral("method"), QStringLiteral("request"));
            attachMessage->setHeader(attachDisposition);
            attachMessage->contentTransferEncoding()->setEncoding(KMime::Headers::CEquPr);
            attachMessage->setBody(KMime::CRLFtoLF(attachment.toUtf8()));
            message->addContent(attachMessage);
        }
    }

    // Job done, attach the both multiparts and assemble the message.
    message->assemble();

    // Put the newly created item in the MessageQueueJob.
    MailTransport::MessageQueueJob *qjob = mFactory->createMessageQueueJob(incidence, identity, this);

    if (identity.disabledFcc()) {
        qjob->sentBehaviourAttribute().setSentBehaviour(MailTransport::SentBehaviourAttribute::Delete);
    } else {
        const Akonadi::Collection sentCollection(identity.fcc().toLongLong());
        if (sentCollection.isValid()) {
            qjob->sentBehaviourAttribute().setSentBehaviour(MailTransport::SentBehaviourAttribute::MoveToCollection);
            qjob->sentBehaviourAttribute().setMoveToCollection(sentCollection);
        } else {
            qjob->sentBehaviourAttribute().setSentBehaviour(MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection);
        }
    }
    qjob->transportAttribute().setTransportId(transportId);

    const QString unormalizedFrom = (transport && transport->specifySenderOverwriteAddress()) ? transport->senderOverwriteAddress() : from;

    const QString normalizedFrom = KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(unormalizedFrom));

    const QString finalFrom = KEmailAddress::extractEmailAddress(normalizedFrom);
    qjob->addressAttribute().setFrom(finalFrom);

    QStringList toStringList;
    if (!to.isEmpty()) {
        toStringList = extractEmailAndNormalize(to);
        qjob->addressAttribute().setTo(toStringList);
    }

    QStringList ccStringList;
    if (!cc.isEmpty()) {
        ccStringList = extractEmailAndNormalize(cc);
        qjob->addressAttribute().setCc(ccStringList);
    }

    QStringList bccStringList;
    if (bccMe) {
        bccStringList = extractEmailAndNormalize(from);
        qjob->addressAttribute().setBcc(bccStringList);
    }

    qjob->setMessage(message);
    connect(qjob, &KJob::finished, this, &MailClient::handleQueueJobFinished);
    qjob->start();
}

void MailClient::handleQueueJobFinished(KJob *job)
{
    if (job->error()) {
        qCritical() << "Error queueing message:" << job->errorText();
        Q_EMIT finished(ResultQueueJobError, i18n("Error queuing message in outbox: %1", job->errorText()));
    } else {
        Q_EMIT finished(ResultSuccess, QString());
    }
}
