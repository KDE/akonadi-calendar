/*
  SPDX-FileCopyrightText: 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>
  SPDX-FileContributor: Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
  SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_CALENDARUTILS_H
#define AKONADI_CALENDARUTILS_H

#include "akonadi-calendar_export.h"

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

namespace Akonadi
{

class Collection;
class ETMCalendar;
class Item;

/** Utility methods for dealing with calendar content in Akonadi items.
 *  @since 5.20.42
 */
namespace CalendarUtils
{
/**
 * Returns the incidence from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Incidence::Ptr incidence(const Akonadi::Item &item);

/**
 * Returns the event from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Event::Ptr event(const Akonadi::Item &item);

/**
 * Returns the todo from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Todo::Ptr todo(const Akonadi::Item &item);

/**
 * Returns the journal from an Akonadi item, or a null pointer if the item has no such payload.
 */
AKONADI_CALENDAR_EXPORT KCalendarCore::Journal::Ptr journal(const Akonadi::Item &item);

/**
 * Returns a suitable display name for the calendar (or calendar folder) @p collection.
 * This takes backend-specific special cases into account.
 */
AKONADI_CALENDAR_EXPORT QString displayName(Akonadi::ETMCalendar *calendar, const Akonadi::Collection &collection);
}

}

#endif // AKONADI_CALENDARUTILS_H
