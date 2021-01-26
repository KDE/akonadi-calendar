/*
  SPDX-FileCopyrightText: 2008 Bruno Virlet <bvirlet@kdemail.net>
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarmodel_p.h"

#include "utils_p.h"
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <itemfetchscope.h>
#include <kcalendarcore/todo.h>
#include <monitor.h>

#include <KLocalizedString>

#include <QIcon>

using namespace Akonadi;

static KCalendarCore::Todo::Ptr todo(const Akonadi::Item &item)
{
    return item.hasPayload<KCalendarCore::Todo::Ptr>() ? item.payload<KCalendarCore::Todo::Ptr>() : KCalendarCore::Todo::Ptr();
}

class Q_DECL_HIDDEN CalendarModel::Private
{
public:
    Private()
    {
    }

    QWeakPointer<CalendarModel> m_weakPointer;
};

CalendarModel::CalendarModel(Akonadi::Monitor *monitor)
    : EntityTreeModel(monitor)
    , d(new Private())
{
    monitor->itemFetchScope().fetchAllAttributes(true);
}

CalendarModel::Ptr CalendarModel::create(Monitor *monitor)
{
    auto model = new CalendarModel(monitor);
    CalendarModel::Ptr modelPtr = CalendarModel::Ptr(model);
    model->setWeakPointer(modelPtr.toWeakRef());
    return modelPtr;
}

CalendarModel::~CalendarModel()
{
    delete d;
}

QWeakPointer<CalendarModel> CalendarModel::weakPointer() const
{
    return d->m_weakPointer;
}

void CalendarModel::setWeakPointer(const QWeakPointer<CalendarModel> &weakPointer)
{
    d->m_weakPointer = weakPointer;
}

QVariant CalendarModel::entityData(const Akonadi::Item &item, int column, int role) const
{
    const KCalendarCore::Incidence::Ptr inc = CalendarUtils::incidence(item);
    if (!inc) {
        return QVariant();
    }

    switch (role) {
    case Qt::DecorationRole:
        if (column != Summary) {
            return QVariant();
        }
        if (inc->type() == KCalendarCore::IncidenceBase::TypeTodo) {
            return QIcon::fromTheme(QStringLiteral("view-pim-tasks"));
        } else if (inc->type() == KCalendarCore::IncidenceBase::TypeJournal) {
            return QIcon::fromTheme(QStringLiteral("view-pim-journal"));
        } else if (inc->type() == KCalendarCore::IncidenceBase::TypeEvent) {
            return QIcon::fromTheme(QStringLiteral("view-calendar"));
        }
        return QIcon::fromTheme(QStringLiteral("network-wired"));

    case Qt::DisplayRole:
        switch (column) {
        case Summary:
            return inc->summary();

        case DateTimeStart:
            return inc->dtStart().toString();

        case DateTimeEnd:
            return inc->dateTime(KCalendarCore::Incidence::RoleEndTimeZone).toString();

        case DateTimeDue:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->dtDue().toString();
            } else {
                return QVariant();
            }

        case Priority:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->priority();
            } else {
                return QVariant();
            }

        case PercentComplete:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->percentComplete();
            } else {
                return QVariant();
            }

        case Type:
            return inc->typeStr();
        default:
            break;
        }

        return QVariant();
    case SortRole:
        switch (column) {
        case Summary:
            return inc->summary();

        case DateTimeStart:
            return inc->dtStart().toUTC();

        case DateTimeEnd:
            return inc->dateTime(KCalendarCore::Incidence::RoleEndTimeZone).toUTC();

        case DateTimeDue:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->dtDue().toUTC();
            } else {
                return QVariant();
            }

        case Priority:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->priority();
            } else {
                return QVariant();
            }

        case PercentComplete:
            if (KCalendarCore::Todo::Ptr t = todo(item)) {
                return t->percentComplete();
            } else {
                return QVariant();
            }

        case Type:
            return inc->type();

        default:
            break;
        }

        return QVariant();

    case RecursRole:
        return inc->recurs();

    default:
        return QVariant();
    }
}

QVariant CalendarModel::entityData(const Akonadi::Collection &collection, int column, int role) const
{
    return EntityTreeModel::entityData(collection, column, role);
}

int CalendarModel::entityColumnCount(EntityTreeModel::HeaderGroup headerSet) const
{
    if (headerSet == EntityTreeModel::ItemListHeaders) {
        return ItemColumnCount;
    } else {
        return CollectionColumnCount;
    }
}

QVariant CalendarModel::entityHeaderData(int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (headerSet == EntityTreeModel::ItemListHeaders) {
        switch (section) {
        case Summary:
            return i18nc("@title:column calendar event summary", "Summary");
        case DateTimeStart:
            return i18nc("@title:column calendar event start date and time", "Start Date and Time");
        case DateTimeEnd:
            return i18nc("@title:column calendar event end date and time", "End Date and Time");
        case Type:
            return i18nc("@title:column calendar event type", "Type");
        case DateTimeDue:
            return i18nc("@title:column todo item due date and time", "Due Date and Time");
        case Priority:
            return i18nc("@title:column todo item priority", "Priority");
        case PercentComplete:
            return i18nc("@title:column todo item completion in percent", "Complete");
        default:
            return QVariant();
        }
    }

    if (headerSet == EntityTreeModel::CollectionTreeHeaders) {
        switch (section) {
        case CollectionTitle:
            return i18nc("@title:column calendar title", "Calendar");
        default:
            return QVariant();
        }
    }
    return QVariant();
}
