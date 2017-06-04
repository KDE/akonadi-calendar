/*
  Copyright (c) 2009 KDAB
  Author: Frank Osterfeld <osterfeld@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

#ifndef AKONADI_CALENDARMODEL_P_H
#define AKONADI_CALENDARMODEL_P_H

#include <entitytreemodel.h>
#include <monitor.h>
#include <QSharedPointer>
#include <QWeakPointer>

namespace Akonadi
{

class CalendarModel : public Akonadi::EntityTreeModel
{
    Q_OBJECT
public:
    typedef QSharedPointer<CalendarModel> Ptr;
    enum ItemColumn {
        Summary = 0,
        Type,
        DateTimeStart,
        DateTimeEnd,
        DateTimeDue,
        Priority,
        PercentComplete,
        ItemColumnCount
    };

    enum CollectionColumn {
        CollectionTitle = 0,
        CollectionColumnCount
    };

    enum Role {
        SortRole = Akonadi::EntityTreeModel::UserRole,
        RecursRole
    };

    static Akonadi::CalendarModel::Ptr create(Akonadi::Monitor *monitor);
    ~CalendarModel();

    QWeakPointer<CalendarModel> weakPointer() const;
    void setWeakPointer(const QWeakPointer<CalendarModel> &weakPointer);

    QVariant entityData(const Akonadi::Item &item, int column, int role = Qt::DisplayRole) const override;

    QVariant entityData(const Akonadi::Collection &collection, int column,
                        int role = Qt::DisplayRole) const override;

    int entityColumnCount(EntityTreeModel::HeaderGroup headerSet) const override;

    QVariant entityHeaderData(int section, Qt::Orientation orientation, int role,
                              EntityTreeModel::HeaderGroup headerSet) const override;

private:
    explicit CalendarModel(Akonadi::Monitor *monitor);
    class Private;
    Private *const d;
};

}

#endif
