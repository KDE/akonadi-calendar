/*
   SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TODOPURGER_P_H
#define TODOPURGER_P_H

#include "todopurger.h"
#include "incidencechanger.h"

#include <QString>
#include <QObject>

namespace Akonadi {
class IncidenceChanger;

class Q_DECL_HIDDEN TodoPurger::Private : public QObject
{
    Q_OBJECT
public:
    Private(TodoPurger *q);
    IncidenceChanger *m_changer = nullptr;
    QString m_lastError;
    CalendarBase::Ptr m_calendar;
    int m_currentChangeId = -1;
    int m_ignoredItems = 0;
    bool m_calendarOwnership = true; // If false it's not ours.

    void deleteTodos();
    Q_REQUIRED_RESULT bool treeIsDeletable(const KCalendarCore::Todo::Ptr &todo);

public Q_SLOTS:
    void onCalendarLoaded(bool success, const QString &message);
    void onItemsDeleted(int changeId, const QVector<Akonadi::Item::Id> &deletedItems,
                        Akonadi::IncidenceChanger::ResultCode,
                        const QString &message);
private:
    TodoPurger *const q;
};
}

#endif
