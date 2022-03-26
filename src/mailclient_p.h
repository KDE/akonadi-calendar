/*
  SPDX-FileCopyrightText: 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_tests_export.h"
#include "itiphandler.h"
#include <KCalendarCore/IncidenceBase>
#include <KMime/KMimeMessage>
#include <QObject>

struct UnitTestResult {
    using List = QVector<UnitTestResult>;
    QString from;
    QStringList to;
    QStringList cc;
    QStringList bcc;
    int transportId;
    KMime::Message::Ptr message;
    UnitTestResult()
        : transportId(-1)
    {
    }
};

namespace KIdentityManagement
{
class Identity;
}

class KJob;

namespace Akonadi
{
class ITIPHandlerComponentFactory;

class AKONADI_CALENDAR_TESTS_EXPORT MailClient : public QObject
{
    Q_OBJECT
public:
    enum Result { ResultSuccess, ResultNoAttendees, ResultReallyNoAttendees, ResultErrorCreatingTransport, ResultErrorFetchingTransport, ResultQueueJobError };

    explicit MailClient(ITIPHandlerComponentFactory *factory, QObject *parent = nullptr);
    ~MailClient() override;

    void mailAttendees(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const KIdentityManagement::Identity &identity,
                       bool bccMe,
                       const QString &attachment = QString(),
                       const QString &mailTransport = QString());

    void mailOrganizer(const KCalendarCore::IncidenceBase::Ptr &incidence,
                       const KIdentityManagement::Identity &identity,
                       const QString &from,
                       bool bccMe,
                       const QString &attachment = QString(),
                       const QString &sub = QString(),
                       const QString &mailTransport = QString());

    void mailTo(const KCalendarCore::IncidenceBase::Ptr &incidence,
                const KIdentityManagement::Identity &identity,
                const QString &from,
                bool bccMe,
                const QString &recipients,
                const QString &attachment = QString(),
                const QString &mailTransport = QString());

    /**
      Sends mail with specified from, to and subject field and body as text.
      If bcc is set, send a blind carbon copy to the sender

      @param incidence is the incidence, that is sended
      @param identity is the Identity of the sender
      @param from is the address of the sender of the message
      @param to a list of addresses to receive the message
      @param cc a list of addresses to receive message carbon copies
      @param subject is the subject of the message
      @param body is the boody of the message
      @param hidden if true and using KMail as the mailer, send the message
      without opening a composer window.
      @param bcc if true, send a blind carbon copy to the message sender
      @param attachment optional attachment (raw data)
      @param mailTransport defines the mail transport method. See here the
      kdepimlibs/mailtransport library.
    */
    void send(const KCalendarCore::IncidenceBase::Ptr &incidence,
              const KIdentityManagement::Identity &identity,
              const QString &from,
              const QString &to,
              const QString &cc,
              const QString &subject,
              const QString &body,
              bool hidden = false,
              bool bccMe = false,
              const QString &attachment = QString(),
              const QString &mailTransport = QString());

private:
    void handleQueueJobFinished(KJob *job);

Q_SIGNALS:
    void finished(Akonadi::MailClient::Result result, const QString &errorString);

public:
    // For unit-test usage, since we can't depend on kdepim-runtime on jenkins
    ITIPHandlerComponentFactory *mFactory = nullptr;
};
}

Q_DECLARE_METATYPE(Akonadi::MailClient::Result)
