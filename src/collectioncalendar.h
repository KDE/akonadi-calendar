/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"
#include "calendarbase.h"

namespace Akonadi
{
class Collection;
class EntityTreeModel;

class CollectionCalendarPrivate;

/** Calendar representing a single Akonadi::Collection. */
class AKONADI_CALENDAR_EXPORT CollectionCalendar : public Akonadi::CalendarBase
{
    Q_OBJECT
public:
    using Ptr = QSharedPointer<CollectionCalendar>;

    /**
     * @brief Construct a new Collection Calendar for given \c collection.
     *
     * Internally, the Calendar will create a new EntityTreeModel instance. Since ETMs can be very heavy,
     * if you already have an ETM with calendars in your application, it's recommended to use the other
     * constructor and pass your ETM to the calendar.
     */
    explicit CollectionCalendar(const Akonadi::Collection &collection, QObject *parent = nullptr);
    /**
     * @brief Construct a new Collection Calendar for given \c collection on top of the given \c model
     *
     * This is the preferred constructor as it allows to reuse the same ETM (and thus the same underlying data
     * storage) for multiple calendars, saving memory resources, as ETMs can be rather heavy.
     */
    CollectionCalendar(Akonadi::EntityTreeModel *model, const Akonadi::Collection &col, QObject *parent = nullptr);
    ~CollectionCalendar() override;

    Akonadi::Collection collection() const;

    /**
     * @brief Set or update the displayed collection
     *
     * Note that once Collection is set, it is only allowed to call setCollection() again
     * with a \c collection that has the same ID as the currently displayed Collection.
     * This can be used to update the name and access rights of the calendar if the Collection's
     * name or ACLs change.
     */
    void setCollection(const Akonadi::Collection &collection);

    /**
     * @brief Returns the ETM used by the model.
     *
     * If you did no pass a custom ETM in the constructor, then the ETM is owned by the Calendar.
     */
    Akonadi::EntityTreeModel *model() const;

    bool addEvent(const KCalendarCore::Event::Ptr &event) override;
    bool addTodo(const KCalendarCore::Todo::Ptr &todo) override;
    bool addJournal(const KCalendarCore::Journal::Ptr &journal) override;

    bool hasRight(Akonadi::Collection::Right right) const;

Q_SIGNALS:
    /**
     * @brief Emitted whenever an incidence is added, removed or changed
     */
    void calendarChanged();

private:
    Q_DECLARE_PRIVATE(CollectionCalendar)
};

} // namespace Akonadi