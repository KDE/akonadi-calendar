/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/CalendarPlugin>

namespace Akonadi
{
class EntityTreeModel;
class Collection;
}

/** Akonadi platform calendar plugin for KCalendarCore. */
class AkonadiCalendarPlugin : public KCalendarCore::CalendarPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kcalendarcore.CalendarPlugin")
public:
    explicit AkonadiCalendarPlugin(QObject *parent = nullptr, const QVariantList &args = {});
    ~AkonadiCalendarPlugin() override;

    QVector<KCalendarCore::Calendar::Ptr> calendars() const override;

private:
    void addCalendar(const Akonadi::Collection &collection);
    void removeCalendar(const Akonadi::Collection &collection);
    void updateCalendar(const Akonadi::Collection &collection);

    QVector<KCalendarCore::Calendar::Ptr> m_calendars;
    Akonadi::EntityTreeModel *m_etm = nullptr;
};
