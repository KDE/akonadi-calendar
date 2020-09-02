/*
  SPDX-FileCopyrightText: 2011-2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_FETCHJOBCALENDAR_P_H
#define AKONADI_FETCHJOBCALENDAR_P_H

#include "fetchjobcalendar.h"
#include "calendarbase_p.h"

class KJob;

namespace Akonadi {
class FetchJobCalendarPrivate : public CalendarBasePrivate
{
    Q_OBJECT
public:

    explicit FetchJobCalendarPrivate(FetchJobCalendar *qq);
    ~FetchJobCalendarPrivate();

public Q_SLOTS:
    void slotSearchJobFinished(KJob *job);
    void slotFetchJobFinished();

public:
    bool m_isLoaded = false;

private:
    FetchJobCalendar *const q;
    QString m_errorMessage;
    bool m_success;
};
}

#endif
