/*
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <QSortFilterProxyModel>

#include <memory>

namespace KCalendarCore
{
class CalFilter;
}

namespace Akonadi
{
class CalFilterProxyModelPrivate;
/*!
 * \class Akonadi::CalFilterProxyModel
 * \inmodule AkonadiCalendar
 * \inheaderfile Akonadi/CalFilterProxyModel
 */
class AKONADI_CALENDAR_EXPORT CalFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /*!
     * Creates a new CalFilterProxyModel.
     */
    explicit CalFilterProxyModel(QObject *parent = nullptr);
    /*!
     * Destroys the CalFilterProxyModel.
     */
    ~CalFilterProxyModel() override;

    /*!
     * Returns the calendar filter.
     */
    [[nodiscard]] KCalendarCore::CalFilter *filter() const;
    /*!
     * Sets the calendar filter.
     */
    void setFilter(KCalendarCore::CalFilter *filter);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    std::unique_ptr<CalFilterProxyModelPrivate> const d;
};
}
