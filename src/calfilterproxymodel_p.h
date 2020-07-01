/*
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_CALFILTERPROXYMODEL_P_H
#define AKONADI_CALFILTERPROXYMODEL_P_H

#include <QSortFilterProxyModel>

namespace KCalendarCore {
class CalFilter;
}

namespace Akonadi {
class CalFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CalFilterProxyModel(QObject *parent = nullptr);
    ~CalFilterProxyModel();

    KCalendarCore::CalFilter *filter() const;
    void setFilter(KCalendarCore::CalFilter *filter);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    class Private;
    Private *const d;
};
}

#endif
