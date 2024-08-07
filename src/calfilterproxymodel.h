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

class AKONADI_CALENDAR_EXPORT CalFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CalFilterProxyModel(QObject *parent = nullptr);
    ~CalFilterProxyModel() override;

    [[nodiscard]] KCalendarCore::CalFilter *filter() const;
    void setFilter(KCalendarCore::CalFilter *filter);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    std::unique_ptr<CalFilterProxyModelPrivate> const d;
};
}
