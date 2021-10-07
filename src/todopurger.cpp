/*
   SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "todopurger.h"
#include "fetchjobcalendar.h"
#include "todopurger_p.h"
#include "utils_p.h"

#include <KCalendarCore/Todo>

#include <KLocalizedString>
using namespace Akonadi;

TodoPurger::Private::Private(TodoPurger *q)
    : q(q)
{
}

void TodoPurger::Private::onCalendarLoaded(bool success, const QString &message)
{
    if (success) {
        deleteTodos();
    } else {
        m_lastError = message;
        if (m_calendarOwnership) {
            m_calendar.clear();
        }
        Q_EMIT q->todosPurged(false, 0, 0);
    }
}

void TodoPurger::Private::onItemsDeleted(int changeId, const QVector<Item::Id> &deletedItems, IncidenceChanger::ResultCode result, const QString &message)
{
    if (changeId != m_currentChangeId) {
        return; // Not ours.
    }

    m_lastError = message;
    if (m_calendarOwnership) {
        m_calendar.clear();
    }
    Q_EMIT q->todosPurged(result == IncidenceChanger::ResultCodeSuccess, deletedItems.count(), m_ignoredItems);
}

void TodoPurger::Private::deleteTodos()
{
    if (!m_changer) {
        q->setIncidenceChager(new IncidenceChanger(this));
        m_changer->setShowDialogsOnError(false);
        m_changer->setHistoryEnabled(false);
    }

    const bool oldShowdialogs = m_changer->showDialogsOnError();
    const bool oldGroupware = m_changer->groupwareCommunication();
    m_changer->setShowDialogsOnError(false);
    m_changer->setGroupwareCommunication(false);

    m_changer->startAtomicOperation(i18n("Purging completed to-dos"));
    const Akonadi::Item::List items = m_calendar->items();
    Akonadi::Item::List toDelete;
    m_ignoredItems = 0;
    for (const Akonadi::Item &item : items) {
        KCalendarCore::Todo::Ptr todo = CalendarUtils::incidence(item).dynamicCast<KCalendarCore::Todo>();

        if (!todo || !todo->isCompleted()) {
            continue;
        }

        if (treeIsDeletable(todo)) {
            toDelete.append(item);
        } else {
            m_ignoredItems++;
        }
    }

    if (toDelete.isEmpty()) {
        if (m_calendarOwnership) {
            m_calendar.clear();
        }
        Q_EMIT q->todosPurged(true, 0, m_ignoredItems);
    } else {
        m_currentChangeId = m_changer->deleteIncidences(toDelete);
        Q_ASSERT(m_currentChangeId > 0);
    }

    m_changer->endAtomicOperation();

    m_changer->setShowDialogsOnError(oldShowdialogs);
    m_changer->setGroupwareCommunication(oldGroupware);
}

bool TodoPurger::Private::treeIsDeletable(const KCalendarCore::Todo::Ptr &todo)
{
    Q_ASSERT(todo);

    if (!todo->isCompleted() || todo->isReadOnly()) {
        return false;
    }

    const KCalendarCore::Incidence::List children = m_calendar->childIncidences(todo->uid());
    if (children.isEmpty()) {
        return true;
    }

    for (const KCalendarCore::Incidence::Ptr &child : children) {
        KCalendarCore::Todo::Ptr childTodo = child.dynamicCast<KCalendarCore::Todo>();

        if (!childTodo) {
            return false; // This never happens
        }

        if (!treeIsDeletable(childTodo)) {
            return false;
        }
    }

    return true;
}

TodoPurger::TodoPurger(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

TodoPurger::~TodoPurger() = default;

void TodoPurger::setIncidenceChager(IncidenceChanger *changer)
{
    d->m_changer = changer;
    d->m_currentChangeId = -1;
    if (changer) {
        connect(changer, &IncidenceChanger::deleteFinished, d.get(), &Private::onItemsDeleted);
    }
}

void TodoPurger::setCalendar(const CalendarBase::Ptr &calendar)
{
    d->m_calendar = calendar;
    d->m_calendarOwnership = calendar.isNull();
}

void TodoPurger::purgeCompletedTodos()
{
    d->m_lastError.clear();

    if (d->m_calendar) {
        d->deleteTodos();
    } else {
        d->m_calendar = FetchJobCalendar::Ptr(new FetchJobCalendar(this));
        // clang-format off
        connect(d->m_calendar.data(), SIGNAL(loadFinished(bool,QString)), d.get(), SLOT(onCalendarLoaded(bool,QString)));
        // clang-format on
    }
}

QString TodoPurger::lastError() const
{
    return d->m_lastError;
}
