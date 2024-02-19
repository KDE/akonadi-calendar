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

class AKONADI_CALENDAR_EXPORT CalFilterPartStatusProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CalFilterPartStatusProxyModel(QObject *parent = nullptr);
    ~CalFilterPartStatusProxyModel() override;

    void setFilterVirtual(bool filterVirtual);
    [[nodiscard]] bool filterVirtual() const;

    void setBlockedStatusList(const QList<KCalendarCore::Attendee::PartStat> &blockStatusList);
    const QList<KCalendarCore::Attendee::PartStat> &blockedStatusList() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    //@cond PRIVATE
    AKONADI_CALENDAR_NO_EXPORT void slotIdentitiesChanged();

    std::unique_ptr<CalFilterPartStatusProxyModelPrivate> const d;

    //@endcond
};
}
