/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mailbodyformatter_p.h"

#include <KCalendarCore/Event>
#include <KCalendarCore/FreeBusy>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>
#include <KCalendarCore/Visitor>
using namespace KCalendarCore;

#include <KLocalizedString>

#include <QTextDocumentFragment>

using namespace Qt::Literals;

namespace
{
QString cleanHtml(const QString &html)
{
    return QTextDocumentFragment::fromHtml(html).toPlainText();
}

QString mailBodyIncidence(const Incidence::Ptr &incidence)
{
    QString body;
    if (!incidence->summary().trimmed().isEmpty()) {
        body += i18n("Summary: %1\n", incidence->richSummary());
    }
    if (!incidence->organizer().isEmpty()) {
        body += i18n("Organizer: %1\n", incidence->organizer().fullName());
    }
    if (!incidence->location().trimmed().isEmpty()) {
        body += i18n("Location: %1\n", incidence->richLocation());
    }
    return body;
}

class MailBodyVisitor : public Visitor
{
public:
    bool act(const IncidenceBase::Ptr &incidence)
    {
        mResult = QLatin1StringView("");
        return incidence ? incidence->accept(*this, incidence) : false;
    }

    [[nodiscard]] const QString &result() const
    {
        return mResult;
    }

protected:
    bool visit(const Event::Ptr &event) override;
    bool visit(const Todo::Ptr &todo) override;
    bool visit(const Journal::Ptr &journal) override;
    bool visit(const FreeBusy::Ptr &) override
    {
        mResult = i18n("This is a Free Busy Object");
        return true;
    }

protected:
    QString mResult;
};

bool MailBodyVisitor::visit(const Event::Ptr &event)
{
    /* cppcheck-suppress variableScope */
    QString const recurrence[] = {i18nc("no recurrence", "None"),
                                  i18nc("event recurs by minutes", "Minutely"),
                                  i18nc("event recurs by hours", "Hourly"),
                                  i18nc("event recurs by days", "Daily"),
                                  i18nc("event recurs by weeks", "Weekly"),
                                  i18nc("event recurs same position (e.g. first monday) each month", "Monthly Same Position"),
                                  i18nc("event recurs same day each month", "Monthly Same Day"),
                                  i18nc("event recurs same month each year", "Yearly Same Month"),
                                  i18nc("event recurs same day each year", "Yearly Same Day"),
                                  i18nc("event recurs same position (e.g. first monday) each year", "Yearly Same Position")};

    mResult = mailBodyIncidence(event);
    mResult += i18n("Start Date: %1\n", QLocale().toString(event->dtStart().toLocalTime().date(), QLocale::ShortFormat));
    if (!event->allDay()) {
        mResult += i18n("Start Time: %1\n", QLocale().toString(event->dtStart().toLocalTime().time(), QLocale::ShortFormat));
    }
    if (event->dtStart() != event->dtEnd()) {
        mResult += i18n("End Date: %1\n", QLocale().toString(event->dtEnd().toLocalTime().date(), QLocale::ShortFormat));
    }
    if (!event->allDay()) {
        mResult += i18n("End Time: %1\n", QLocale().toString(event->dtEnd().toLocalTime().time(), QLocale::ShortFormat));
    }
    if (event->recurs()) {
        Recurrence const *recur = event->recurrence();
        // TODO: Merge these two to one of the form "Recurs every 3 days"
        mResult += i18n("Recurs: %1\n", recurrence[recur->recurrenceType()]);
        mResult += i18n("Frequency: %1\n", event->recurrence()->frequency());

        if (recur->duration() > 0) {
            mResult += i18np("Repeats once", "Repeats %1 times", recur->duration());
            mResult += u'\n';
        } else {
            if (recur->duration() != -1) {
                // TODO_Recurrence: What to do with all-day
                QString endstr;
                if (event->allDay()) {
                    endstr = QLocale().toString(recur->endDate());
                } else {
                    endstr = QLocale().toString(recur->endDateTime(), QLocale::ShortFormat);
                }
                mResult += i18n("Repeat until: %1\n", endstr);
            } else {
                mResult += i18n("Repeats forever\n");
            }
        }
    }

    if (!event->description().isEmpty()) {
        QString descStr;
        if (event->descriptionIsRich() || event->description().startsWith(QLatin1StringView("<!DOCTYPE HTML"))) {
            descStr = cleanHtml(event->description());
        } else {
            descStr = event->description();
        }
        if (!descStr.isEmpty()) {
            mResult += i18n("Details:\n%1\n", descStr);
        }
    }
    return !mResult.isEmpty();
}

bool MailBodyVisitor::visit(const Todo::Ptr &todo)
{
    mResult = mailBodyIncidence(todo);

    if (todo->hasStartDate() && todo->dtStart().isValid()) {
        mResult += i18n("Start Date: %1\n", QLocale().toString(todo->dtStart(false).toLocalTime().date(), QLocale::ShortFormat));
        if (!todo->allDay()) {
            mResult += i18n("Start Time: %1\n", QLocale().toString(todo->dtStart(false).toLocalTime().time(), QLocale::ShortFormat));
        }
    }
    if (todo->hasDueDate() && todo->dtDue().isValid()) {
        mResult += i18n("Due Date: %1\n", QLocale().toString(todo->dtDue().toLocalTime().date(), QLocale::ShortFormat));
        if (!todo->allDay()) {
            mResult += i18n("Due Time: %1\n", QLocale().toString(todo->dtDue().toLocalTime().time(), QLocale::ShortFormat));
        }
    }
    QString const details = todo->richDescription();
    if (!details.isEmpty()) {
        mResult += i18n("Details:\n%1\n", details);
    }
    return !mResult.isEmpty();
}

bool MailBodyVisitor::visit(const Journal::Ptr &journal)
{
    mResult = mailBodyIncidence(journal);
    mResult += i18n("Date: %1\n", QLocale().toString(journal->dtStart().toLocalTime().date(), QLocale::ShortFormat));
    if (!journal->allDay()) {
        mResult += i18n("Time: %1\n", QLocale().toString(journal->dtStart().toLocalTime().time(), QLocale::ShortFormat));
    }
    if (!journal->description().isEmpty()) {
        mResult += i18n("Text of the journal:\n%1\n", journal->richDescription());
    }
    return true;
}
}

QString Akonadi::MailBodyFormatter::mailBodyStr(const IncidenceBase::Ptr &incidence)
{
    if (!incidence) {
        return QString();
    }

    MailBodyVisitor v;
    if (v.act(incidence)) {
        return v.result();
    }
    return QString();
}
