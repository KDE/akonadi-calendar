/*
  SPDX-FileCopyrightText: 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>
  SPDX-FileContributor: Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
  SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarutils.h"

#include <Akonadi/Item>

using namespace Akonadi;

KCalendarCore::Incidence::Ptr CalendarUtils::incidence(const Akonadi::Item &item)
{
    // With this try-catch block, we get a 2x performance improvement in retrieving the payload
    // since we don't call hasPayload()
    try {
        return item.payload<KCalendarCore::Incidence::Ptr>();
    } catch (const Akonadi::PayloadException &) {
        return {};
    }
}

KCalendarCore::Event::Ptr CalendarUtils::event(const Akonadi::Item &item)
{
    try {
        auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
        if (incidence && incidence->type() == KCalCore::Incidence::TypeEvent) {
            return item.payload<KCalendarCore::Event::Ptr>();
        }
    } catch (const Akonadi::PayloadException &) {
        return {};
    }
    return {};
}

KCalendarCore::Todo::Ptr CalendarUtils::todo(const Akonadi::Item &item)
{
    try {
        auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
        if (incidence && incidence->type() == KCalCore::Incidence::TypeTodo) {
            return item.payload<KCalendarCore::Todo::Ptr>();
        }
    } catch (const Akonadi::PayloadException &) {
        return {};
    }
    return {};
}

KCalendarCore::Journal::Ptr CalendarUtils::journal(const Akonadi::Item &item)
{
    try {
        auto incidence = item.payload<KCalendarCore::Incidence::Ptr>();
        if (incidence && incidence->type() == KCalCore::Incidence::TypeJournal) {
            return item.payload<KCalendarCore::Journal::Ptr>();
        }
    } catch (const Akonadi::PayloadException &) {
        return {};
    }
    return {};
}
