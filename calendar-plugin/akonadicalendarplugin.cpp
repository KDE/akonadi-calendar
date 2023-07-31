/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopytightText: 2023 Daniel Vrátil <dvratil@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "akonadicalendarplugin.h"
#include "akonadicalendarplugin_debug.h"
#include "collectioncalendar.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Monitor>
#include <Akonadi/ServerManager>

static bool filterCollection(const Akonadi::Collection &col)
{
    return !col.isVirtual();
}

AkonadiCalendarPlugin::AkonadiCalendarPlugin(QObject *parent, const QVariantList &args)
    : KCalendarCore::CalendarPlugin(parent, args)
{
    // don't automatically start Akonadi if that's explicitly forbidden
    // (useful in e.g. the CI environment)
    if (qEnvironmentVariableIsSet("AKONADI_CALENDAR_KCALENDARCORE_PLUGIN_NO_AUTO_LAUNCH") && !Akonadi::ServerManager::isRunning()) {
        qCWarning(AKONADICALENDARPLUGIN_LOG) << "Akonadi is not running, but auto-launch is disabled!";
        return;
    }

    auto monitor = new Akonadi::Monitor(this);
    monitor->collectionFetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    m_etm = new Akonadi::EntityTreeModel(monitor, this);
    connect(m_etm, &Akonadi::EntityTreeModel::collectionTreeFetched, this, [this](const Akonadi::Collection::List &collectionTree) {
        for (const auto &col : collectionTree) {
            addCalendar(col);
        }
        Q_EMIT calendarsChanged();
    });
    connect(monitor, &Akonadi::Monitor::collectionAdded, this, [this](const auto &collection) {
        addCalendar(collection);
        Q_EMIT calendarsChanged();
    });
    connect(monitor, &Akonadi::Monitor::collectionRemoved, this, [this](const auto &collection) {
        removeCalendar(collection);
        Q_EMIT calendarsChanged();
    });
    connect(monitor, qOverload<const Akonadi::Collection &>(&Akonadi::Monitor::collectionChanged), this, [this](const auto &collection) {
        updateCalendar(collection);
        Q_EMIT calendarsChanged();
    });
}

void AkonadiCalendarPlugin::addCalendar(const Akonadi::Collection &collection)
{
    if (!filterCollection(collection)) {
        return;
    }

    m_calendars.push_back(Akonadi::CollectionCalendar::Ptr::create(m_etm, collection));
}

namespace
{

auto matchByCollection(const Akonadi::Collection &collection)
{
    return [collection](const KCalendarCore::Calendar::Ptr &calendar) {
        return calendar.staticCast<Akonadi::CollectionCalendar>()->collection() == collection;
    };
}

}

void AkonadiCalendarPlugin::removeCalendar(const Akonadi::Collection &collection)
{
    m_calendars.erase(std::remove_if(m_calendars.begin(), m_calendars.end(), matchByCollection(collection)), m_calendars.end());
}

void AkonadiCalendarPlugin::updateCalendar(const Akonadi::Collection &collection)
{
    auto calendar = std::find_if(m_calendars.begin(), m_calendars.end(), matchByCollection(collection));
    if (calendar == m_calendars.end()) {
        return;
    }

    calendar->staticCast<Akonadi::CollectionCalendar>()->setCollection(collection);
}

AkonadiCalendarPlugin::~AkonadiCalendarPlugin() = default;

QVector<KCalendarCore::Calendar::Ptr> AkonadiCalendarPlugin::calendars() const
{
    return m_calendars;
}
