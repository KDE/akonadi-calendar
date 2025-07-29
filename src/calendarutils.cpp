/*
  SPDX-FileCopyrightText: 2009, 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>
  SPDX-FileContributor: Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
  SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarutils.h"
using namespace Qt::Literals::StringLiterals;

#include "etmcalendar.h"

#include <Akonadi/AgentManager>
#include <Akonadi/Collection>
#include <Akonadi/EntityTreeModel>

#include <KCalUtils/ICalDrag>

#include <KLocalizedString>

#include <QMimeData>

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
        if (incidence && incidence->type() == KCalendarCore::Incidence::TypeEvent) {
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
        if (incidence && incidence->type() == KCalendarCore::Incidence::TypeTodo) {
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
        if (incidence && incidence->type() == KCalendarCore::Incidence::TypeJournal) {
            return item.payload<KCalendarCore::Journal::Ptr>();
        }
    } catch (const Akonadi::PayloadException &) {
        return {};
    }
    return {};
}

static QString displayNameImpl(const Akonadi::EntityTreeModel *model, const Akonadi::Collection &c)
{
    Akonadi::Collection fullCollection = c;
    if (model) {
        if (const auto col = Akonadi::EntityTreeModel::updatedCollection(model, c); col.isValid()) {
            fullCollection = col;
        }
    }

    QString const cName = fullCollection.name();
    const QString resourceName = fullCollection.resource();

    // Kolab Groupware
    if (resourceName.contains("kolab"_L1)) {
        QString typeStr = cName; // contents type: "Calendar", "Tasks", etc
        QString ownerStr; // folder owner: "fred", "ethel", etc
        QString nameStr; // folder name: "Public", "Test", etc
        if (model) {
            Akonadi::Collection p = c.parentCollection();
            while (p != Akonadi::Collection::root()) {
                Akonadi::Collection const tCol = Akonadi::EntityTreeModel::updatedCollection(model, Collection{p.id()});
                const QString tName = tCol.name();
                if (tName.startsWith("shared.cal"_L1, Qt::CaseInsensitive)) {
                    ownerStr = u"Shared"_s;
                    nameStr = cName;
                    typeStr = i18n("Calendar");
                    break;
                } else if (tName.startsWith("shared.tasks"_L1, Qt::CaseInsensitive) || tName.startsWith("shared.todo"_L1, Qt::CaseInsensitive)) {
                    ownerStr = u"Shared"_s;
                    nameStr = cName;
                    typeStr = i18n("Tasks");
                    break;
                } else if (tName.startsWith("shared.journal"_L1, Qt::CaseInsensitive)) {
                    ownerStr = u"Shared"_s;
                    nameStr = cName;
                    typeStr = i18n("Journal");
                    break;
                } else if (tName.startsWith("shared.notes"_L1, Qt::CaseInsensitive)) {
                    ownerStr = u"Shared"_s;
                    nameStr = cName;
                    typeStr = i18n("Notes");
                    break;
                } else if (tName != i18n("Calendar") && tName != i18n("Tasks") && tName != i18n("Journal") && tName != i18n("Notes")) {
                    ownerStr = tName;
                    break;
                } else {
                    nameStr = typeStr;
                    typeStr = tName;
                }
                p = p.parentCollection();
            }
        }

        if (!ownerStr.isEmpty()) {
            if (!ownerStr.compare("INBOX"_L1, Qt::CaseInsensitive)) {
                return i18nc("%1 is folder contents", "My Kolab %1", typeStr);
            } else if (!ownerStr.compare("SHARED"_L1, Qt::CaseInsensitive) || !ownerStr.compare("CALENDAR"_L1, Qt::CaseInsensitive)
                       || !ownerStr.compare("RESOURCES"_L1, Qt::CaseInsensitive)) {
                return i18nc("%1 is folder name, %2 is folder contents", "Shared Kolab %1 %2", nameStr, typeStr);
            } else {
                if (nameStr.isEmpty()) {
                    return i18nc("%1 is folder owner name, %2 is folder contents", "%1's Kolab %2", ownerStr, typeStr);
                } else {
                    return i18nc("%1 is folder owner name, %2 is folder name, %3 is folder contents", "%1's %2 Kolab %3", ownerStr, nameStr, typeStr);
                }
            }
        } else {
            return i18nc("%1 is folder contents", "Kolab %1", typeStr);
        }
    } // end kolab section

    // Dav Groupware
    if (resourceName.contains("davgroupware"_L1)) {
        const QString resourceDisplayName = Akonadi::AgentManager::self()->instance(resourceName).name();
        return i18nc("%1 is the folder name", "%1 in %2", fullCollection.displayName(), resourceDisplayName);
    } // end caldav section

    // Google
    if (resourceName.contains("google"_L1)) {
        QString ownerStr; // folder owner: "user@gmail.com"
        if (model) {
            Akonadi::Collection const p = c.parentCollection();
            ownerStr = Akonadi::EntityTreeModel::updatedCollection(model, Collection{p.id()}).displayName();
        }

        const QString nameStr = c.displayName(); // folder name: can be anything

        QString typeStr;
        const QString mimeStr = c.contentMimeTypes().join(u',');
        if (mimeStr.contains(".event"_L1)) {
            typeStr = i18n("Calendar");
        } else if (mimeStr.contains(".todo"_L1)) {
            typeStr = i18n("Tasks");
        } else if (mimeStr.contains(".journal"_L1)) {
            typeStr = i18n("Journal");
        } else if (mimeStr.contains(".note"_L1)) {
            typeStr = i18n("Notes");
        } else {
            typeStr = mimeStr;
        }

        if (!ownerStr.isEmpty()) {
            const int atChar = ownerStr.lastIndexOf(u'@');
            if (atChar >= 0) {
                ownerStr.truncate(atChar);
            }
            if (nameStr.isEmpty()) {
                return i18nc("%1 is folder owner name, %2 is folder contents", "%1's Google %2", ownerStr, typeStr);
            } else {
                return i18nc("%1 is folder owner name, %2 is folder name", "%1's %2", ownerStr, nameStr);
            }
        } else {
            return i18nc("%1 is folder contents", "Google %1", typeStr);
        }
    } // end google section

    // Not groupware so the collection is "mine"
    const QString dName = fullCollection.displayName();

    if (!dName.isEmpty()) {
        return fullCollection.name().startsWith("akonadi_"_L1) ? i18n("My %1", dName) : dName;
    } else if (!fullCollection.name().isEmpty()) {
        return fullCollection.name();
    } else {
        return i18nc("unknown resource", "Unknown");
    }
}

QString CalendarUtils::displayName(Akonadi::ETMCalendar *calendar, const Akonadi::Collection &c)
{
    return displayNameImpl(calendar ? calendar->entityTreeModel() : nullptr, c);
}

QString CalendarUtils::displayName(const Akonadi::EntityTreeModel *model, const Akonadi::Collection &c)
{
    return displayNameImpl(model, c);
}

QString CalendarUtils::displayName(const Akonadi::Collection &c)
{
    return displayNameImpl(nullptr, c);
}

QMimeData *CalendarUtils::createMimeData(const Akonadi::Item::List &items)
{
    if (items.isEmpty()) {
        return nullptr;
    }

    KCalendarCore::MemoryCalendar::Ptr const cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));

    QList<QUrl> urls;
    int incidencesFound = 0;
    for (const Akonadi::Item &item : items) {
        const KCalendarCore::Incidence::Ptr incidence(CalendarUtils::incidence(item));
        if (!incidence) {
            continue;
        }
        ++incidencesFound;
        urls.push_back(item.url());
        KCalendarCore::Incidence::Ptr const i(incidence->clone());
        cal->addIncidence(i);
    }

    if (incidencesFound == 0) {
        return nullptr;
    }

    auto mimeData = std::make_unique<QMimeData>();
    mimeData->setUrls(urls);

    if (KCalUtils::ICalDrag::populateMimeData(mimeData.get(), cal)) {
        return mimeData.release();
    } else {
        return nullptr;
    }
}
