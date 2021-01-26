/*
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_CALENDAR_UTILS_P_
#define AKONADI_CALENDAR_UTILS_P_

#include <KCalendarCore/Incidence>
#include <collection.h>
#include <item.h>

#include <QString>
#include <QStringList>

/**
 * Util functions that have no place to live.
 */

class QWidget;

namespace Akonadi
{
namespace CalendarUtils
{
Q_REQUIRED_RESULT QString fullName();
Q_REQUIRED_RESULT QString email();
Q_REQUIRED_RESULT bool thatIsMe(const QString &email);

// faster version, because we know that attendee->email() is only the email address
Q_REQUIRED_RESULT bool thatIsMe(const KCalendarCore::Attendee &attendee);

Q_REQUIRED_RESULT QStringList allEmails();

Q_REQUIRED_RESULT KCalendarCore::Incidence::Ptr incidence(const Akonadi::Item &item);

Q_REQUIRED_RESULT Akonadi::Collection
selectCollection(QWidget *parent, int &dialogCode, const QStringList &mimeTypes, const Akonadi::Collection &defaultCollection = Akonadi::Collection());
}
}

#endif
