/*
   Copyright (C) 2011 Sérgio Martins <sergio.martins@kdab.com>
   Copyright (C) 2012 Sérgio Martins <iamsergio@gmail.com>

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

#include "fetchjobcalendar.h"
#include "fetchjobcalendar_p.h"
#include "incidencefetchjob_p.h"
#include "akonadicalendar_debug.h"
#include <item.h>
#include <collection.h>

using namespace Akonadi;
using namespace KCalendarCore;

FetchJobCalendarPrivate::FetchJobCalendarPrivate(FetchJobCalendar *qq)
    : CalendarBasePrivate(qq)
    , m_isLoaded(false)
    , q(qq)
{
    IncidenceFetchJob *job = new IncidenceFetchJob();
    connect(job, &KJob::result,
            this, &FetchJobCalendarPrivate::slotSearchJobFinished);
    connect(this, &CalendarBasePrivate::fetchFinished,
            this, &FetchJobCalendarPrivate::slotFetchJobFinished);
}

FetchJobCalendarPrivate::~FetchJobCalendarPrivate()
{
}

void FetchJobCalendarPrivate::slotSearchJobFinished(KJob *job)
{
    IncidenceFetchJob *searchJob = static_cast<Akonadi::IncidenceFetchJob *>(job);
    m_success = true;
    m_errorMessage = QString();
    if (searchJob->error()) {
        m_success = false;
        m_errorMessage = searchJob->errorText();
        qCWarning(AKONADICALENDAR_LOG) << "Unable to fetch incidences:" << searchJob->errorText();
    } else {
        const Akonadi::Item::List lstItem = searchJob->items();
        for (const Akonadi::Item &item : lstItem) {
            if (!item.isValid() || !item.hasPayload<KCalendarCore::Incidence::Ptr>()) {
                m_success = false;
                m_errorMessage = QStringLiteral("Invalid item or payload: %1").arg(item.id());
                qCWarning(AKONADICALENDAR_LOG) << "Unable to fetch incidences:" << m_errorMessage;
                continue;
            }
            internalInsert(item);
        }
    }

    if (mCollectionJobs.isEmpty()) {
        slotFetchJobFinished();
    }
}

void FetchJobCalendarPrivate::slotFetchJobFinished()
{
    m_isLoaded = true;
    // Q_EMIT loadFinished() in a delayed manner, due to freezes because of execs.
    QMetaObject::invokeMethod(q, "loadFinished", Qt::QueuedConnection,
                              Q_ARG(bool, m_success), Q_ARG(QString, m_errorMessage));
}

FetchJobCalendar::FetchJobCalendar(QObject *parent)
    : CalendarBase(new FetchJobCalendarPrivate(this), parent)
{
}

FetchJobCalendar::~FetchJobCalendar()
{
}

bool FetchJobCalendar::isLoaded() const
{
    FetchJobCalendarPrivate *d = static_cast<FetchJobCalendarPrivate *>(d_ptr.data());
    return d->m_isLoaded;
}

#include "moc_fetchjobcalendar.cpp"
#include "moc_fetchjobcalendar_p.cpp"
