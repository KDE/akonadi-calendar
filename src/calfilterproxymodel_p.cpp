/*
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calfilterproxymodel_p.h"
#include "utils_p.h"

#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>

#include <KCalendarCore/CalFilter>
#include <KCalendarCore/Incidence>

using namespace Akonadi;

class Q_DECL_HIDDEN CalFilterProxyModel::Private
{
public:
    explicit Private()
    {
    }

    KCalendarCore::CalFilter *filter = nullptr;
};

CalFilterProxyModel::CalFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private)
{
    setFilterKeyColumn(0);
}

CalFilterProxyModel::~CalFilterProxyModel()
{
    delete d;
}

KCalendarCore::CalFilter *CalFilterProxyModel::filter() const
{
    return d->filter;
}

void CalFilterProxyModel::setFilter(KCalendarCore::CalFilter *filter)
{
    if (filter == d->filter) {
        return;
    }

    d->filter = filter;
    invalidateFilter();
}

bool CalFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (!d->filter) {
        return true;
    }

    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) {
        return false;
    }

    const auto item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        return false;
    }

    const KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
    if (!incidence) {
        return false;
    }

    return d->filter->filterIncidence(incidence);
}
