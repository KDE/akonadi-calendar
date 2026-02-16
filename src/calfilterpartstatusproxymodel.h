/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <KCalendarCore/Attendee>
#include <QSortFilterProxyModel>

#include <memory>

namespace Akonadi
{
class CalFilterPartStatusProxyModelPrivate;
/*!
 * \class Akonadi::CalFilterPartStatusProxyModel
 * \inmodule AkonadiCalendar
 * \inheaderfile Akonadi/CalFilterPartStatusProxyModel
 */
class AKONADI_CALENDAR_EXPORT CalFilterPartStatusProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /*!
     * Creates a new CalFilterPartStatusProxyModel.
     */
    explicit CalFilterPartStatusProxyModel(QObject *parent = nullptr);
    /*!
     * Destroys the CalFilterPartStatusProxyModel.
     */
    ~CalFilterPartStatusProxyModel() override;

    /*!
     * Sets whether to filter virtual attendees.
     */
    void setFilterVirtual(bool filterVirtual);
    /*!
     * Returns whether virtual attendees are filtered.
     */
    [[nodiscard]] bool filterVirtual() const;

    /*!
     * Sets the list of participation statuses to filter.
     */
    void setBlockedStatusList(const QList<KCalendarCore::Attendee::PartStat> &blockStatusList);
    /*!
     * Returns the list of participation statuses that are filtered.
     */
    const QList<KCalendarCore::Attendee::PartStat> &blockedStatusList() const;

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    AKONADI_CALENDAR_NO_EXPORT void slotIdentitiesChanged();

    std::unique_ptr<CalFilterPartStatusProxyModelPrivate> const d;
};
}
