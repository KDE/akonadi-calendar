/*
   SPDX-FileCopyrightText: 2011 Sérgio Martins <sergio.martins@kdab.com>
   SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <KCalendarCore/Incidence>
#include <KCalendarCore/MemoryCalendar>

#include <memory>

namespace Akonadi
{
class CalendarBasePrivate;
class IncidenceChanger;

/*!
 * \class Akonadi::CalendarBase
 * \inmodule AkonadiCalendar
 * \inheaderfile Akonadi/CalendarBase
 *
 * \brief The base class for all akonadi aware calendars.
 *
 * Because it inherits KCalendarCore::Calendar, it provides seamless integration
 * with KCalendarCore and KCalUtils libraries eliminating any need for adapter
 * ( akonadi<->KCalendarCore ) classes.
 *
 * \sa ETMCalendar
 * \sa FetchJobCalendar
 *
 * \author Sérgio Martins <sergio.martins@kdab.com>
 * \since 4.11
 */
class AKONADI_CALENDAR_EXPORT CalendarBase : public KCalendarCore::MemoryCalendar
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CalendarBase>;

    /*!
     * Constructs a CalendarBase object.
     */
    explicit CalendarBase(QObject *parent = nullptr);

    /*!
     * Destroys the calendar.
     */
    ~CalendarBase() override;

    /*!
     * Returns the Item containing the incidence with uid \a uid or an invalid Item
     * if the incidence isn't found.
     * \sa Use item(Incidence::Ptr) instead where possible. This function doesn't take exceptions (recurrenceId) into account (and thus always returns the main
     * event).
     */
    [[nodiscard]] Akonadi::Item item(const QString &uid) const;

    /*!
     * Returns the Item containing \a incidence or an invalid Item if the incidence isn't found.
     */
    [[nodiscard]] Akonadi::Item item(const KCalendarCore::Incidence::Ptr &incidence) const;

    /*!
     * Returns the Item with \a id or an invalid Item if not found.
     */
    [[nodiscard]] Akonadi::Item item(Akonadi::Item::Id) const;

    /*!
     * Returns the list of items contained in this calendar that belong to the specified collection.
     * \sa incidences()
     * \since 4.12
     */
    [[nodiscard]] Akonadi::Item::List items(Akonadi::Collection::Id = -1) const;

    /*!
     * Returns the item list that corresponds to the \a incidenceList.
     */
    [[nodiscard]] Akonadi::Item::List itemList(const KCalendarCore::Incidence::List &incidenceList) const;

    /*!
     * Returns the child incidences of the parent identified by \a parentUid.
     * Only the direct children are returned
     * \a parentUid identifier of the parent incidence
     *///TODO: unit-test
    [[nodiscard]] KCalendarCore::Incidence::List childIncidences(const QString &parentUid) const;

    /*!
     * Returns the child incidences of the parent identified by \a parentId.
     * Only the direct children are returned
     * \a parentId identifier of the parent item
     */
    [[nodiscard]] KCalendarCore::Incidence::List childIncidences(Item::Id parentId) const;

    /*!
     * Returns the child items of the parent identified by \a parentUid.
     * Only the direct children are returned
     * \a parentUid identifier of the parent incidence
     */
    [[nodiscard]] Akonadi::Item::List childItems(const QString &parentUid) const;

    /*!
     * Returns the child items of the parent identified by \a parentId.
     * Only the direct children are returned
     * \a parentId identifier of the parent item
     */
    [[nodiscard]] Akonadi::Item::List childItems(Item::Id parentId) const;

    /*!
     * Adds an Event to the calendar.
     * It's added to akonadi in the background \sa createFinished().
     * \a event the event to be added
     */
    bool addEvent(const KCalendarCore::Event::Ptr &event) override;

    /*!
     * Deletes an Event from the calendar.
     * It's removed from akonadi in the background \sa deleteFinished().
     * \a event the event to be deleted
     */
    bool deleteEvent(const KCalendarCore::Event::Ptr &event) override;

    /*!
     * Adds a Todo to the calendar.
     * It's added to akonadi in the background \sa createFinished().
     * \a todo the todo to add
     */
    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;

    /*!
     * Deletes a Todo from the calendar.
     * It's removed from akonadi in the background \sa deleteFinished().
     * \a todo the todo to delete
     */
    bool deleteTodo(const KCalendarCore::Todo::Ptr &todo) override;

    /*!
     * Adds a Journal to the calendar.
     * It's added to akonadi in the background \sa createFinished().
     * \a journal the journal to add
     */
    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;

    /*!
     * Deletes a Journal from the calendar.
     * It's removed from akonadi in the background \sa deleteFinished().
     * \a journal the journal to delete
     */
    bool deleteJournal(const KCalendarCore::Journal::Ptr &journal) override;

    /*!
     * Adds an incidence to the calendar.
     * It's added to akonadi in the background \sa createFinished().
     * \a incidence the incidence to add
     */
    bool addIncidence(const KCalendarCore::Incidence::Ptr &incidence) override;

    /*!
     * Deletes an incidence from the calendar.
     * It's removed from akonadi in the background \sa deleteFinished().
     * \a incidence the incidence to delete
     */
    bool deleteIncidence(const KCalendarCore::Incidence::Ptr &incidence) override;

    /*!
        Call this to tell the calendar that you're adding a batch of incidences.
        So it doesn't, for example, ask the destination for each incidence.

        \sa endBatchAdding()
    */
    void startBatchAdding() override;

    /*!
     * Tells the Calendar that you stopped adding a batch of incidences.
     * \sa startBatchAdding()
     */
    void endBatchAdding() override;

    /*!
     * Returns the IncidenceChanger used by this calendar to make changes in akonadi.
     * Use this if you need the defaults used by CalendarBase.
     */
    [[nodiscard]] Akonadi::IncidenceChanger *incidenceChanger() const;

    /*!
     * Modifies an incidence.
     * The incidence with the same uid as \a newIncidence will be updated with the contents of
     * \a newIncidence the incidence to modify
     */
    bool modifyIncidence(const KCalendarCore::Incidence::Ptr &newIncidence);

Q_SIGNALS:
    /*!
     * This signal is emitted when an incidence is created in akonadi through
     * add{Incidence,Event,Todo,Journal}
     * \a success the success of the operation
     * \a errorMessage if \a success is false, contains the error message
     */
    void createFinished(bool success, const QString &errorMessage);

    /*!
     * This signal is emitted when an incidence is deleted in akonadi through
     * delete{Incidence,Event,Todo,Journal}
     * \a success the success of the operation
     * \a errorMessage if \a success is false, contains the error message
     */
    void deleteFinished(bool success, const QString &errorMessage);

    /*!
     * This signal is emitted when an incidence is modified in akonadi through
     * modifyIncidence().
     * \a success the success of the operation
     * \a errorMessage if \a success is false, contains the error message
     */
    void modifyFinished(bool success, const QString &errorMessage);

protected:
    Q_DECLARE_PRIVATE(CalendarBase)
    std::unique_ptr<CalendarBasePrivate> const d_ptr;
    CalendarBase(CalendarBasePrivate *const d, QObject *parent);

    friend class Scheduler;
};
}
