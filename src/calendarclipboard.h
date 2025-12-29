/*
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

#include <KCalendarCore/Incidence>
#include <QObject>

#include <memory>

namespace Akonadi
{
class IncidenceChanger;
class CalendarClipboardPrivate;

/*!
 * \class Akonadi::CalendarClipboard
 * \inmodule AkonadiCalendar
 * \inheaderfile Akonadi/CalendarClipboard
 *
 * \brief Class to copy or cut calendar incidences.
 *
 * \author Sérgio Martins <iamsergio@gmail.com>
 * \since 4.11
 */
class AKONADI_CALENDAR_EXPORT CalendarClipboard : public QObject
{
    Q_OBJECT
public:
    enum Mode {
        SingleMode = 0, ///< Only the specified incidence is cut/copied.
        RecursiveMode, ///< The specified incidence's children are also cut/copied
        AskMode ///< The user is asked if he wants children to be cut/copied too
    };

    /*!
     * Constructs a new CalendarClipboard.
     * \a calendar calendar containing incidences
     * \a changer incidence changer that will delete incidences while copying.
     *        If 0, an internal one will be created.
     * \a parent QObject parent
     */
    explicit CalendarClipboard(const Akonadi::CalendarBase::Ptr &calendar, Akonadi::IncidenceChanger *changer = nullptr, QObject *parent = nullptr);
    /*!
     * Destroys the CalendarClipboard instance.
     */
    ~CalendarClipboard() override;

    /*!
     * Copies the specified incidence into the clipboard and then deletes it from akonadi.
     * The incidence must be present in the calendar.
     * After it's deletion from akonadi, signal cutFinished() is emitted.
     * \a incidence to cut
     * \a mode how to treat child incidences. Defaults to #RecursiveMode
     * \sa cutFinished().
     */
    void cutIncidence(const KCalendarCore::Incidence::Ptr &incidence, CalendarClipboard::Mode mode = RecursiveMode);

    /*!
     * Copies the specified incidence into the clipboard.
     * \a incidence the incidence to copy
     * \a mode how to treat child incidences. Defaults to #RecursiveMode
     * Returns true on success
     */
    bool copyIncidence(const KCalendarCore::Incidence::Ptr &incidence, CalendarClipboard::Mode mode = RecursiveMode);

    /*!
     * Returns if there's any ical mime data available for pasting.
     */
    [[nodiscard]] bool pasteAvailable() const;

Q_SIGNALS:
    /*!
     * Emitted after cutIncidences() finishes.
     * \a success true if the cut was successful
     * \a errorMessage if \a success if false, contains the error message, empty otherwise.
     */
    void cutFinished(bool success, const QString &errorMessage);

private:
    std::unique_ptr<CalendarClipboardPrivate> const d;
};
}
