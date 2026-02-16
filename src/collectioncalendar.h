/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

class QAbstractItemModel;

namespace Akonadi
{
class Collection;
class EntityTreeModel;

class CollectionCalendarPrivate;

/*! Calendar representing a single Akonadi::Collection. */
class AKONADI_CALENDAR_EXPORT CollectionCalendar : public Akonadi::CalendarBase
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CollectionCalendar>;

    /*!
     * Creates a new CollectionCalendar for the given collection.
     */
    explicit CollectionCalendar(const Akonadi::Collection &col, QObject *parent = nullptr);
    /*!
     * Creates a new CollectionCalendar for the given collection using the specified model.
     */
    CollectionCalendar(QAbstractItemModel *model, const Akonadi::Collection &col, QObject *parent = nullptr);
    /*!
     * Destroys the CollectionCalendar.
     */
    ~CollectionCalendar() override;

    /*!
     * Returns the collection associated with this calendar.
     */
    [[nodiscard]] Akonadi::Collection collection() const;
    /*!
     * Sets the collection associated with this calendar.
     */
    void setCollection(const Akonadi::Collection &c);

    /*!
     * Returns the model associated with this calendar.
     */
    [[nodiscard]] Akonadi::EntityTreeModel *model() const;

    /*!
     * Adds an event to the calendar.
     */
    bool addEvent(const KCalendarCore::Event::Ptr &event) override;
    /*!
     * Adds a todo to the calendar.
     */
    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;
    /*!
     * Adds a journal to the calendar.
     */
    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;

    /*!
     * Returns whether the calendar has the given right.
     */
    [[nodiscard]] bool hasRight(Akonadi::Collection::Right right) const;

Q_SIGNALS:
    /*!
     * \brief Emitted whenever an incidence is added, removed or changed
     */
    void calendarChanged();

private:
    Q_DECLARE_PRIVATE(CollectionCalendar)
};

} // namespace Akonadi
