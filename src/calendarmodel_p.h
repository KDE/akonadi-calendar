/*
  SPDX-FileCopyrightText: 2009 KDAB
  SPDX-FileContributor: Frank Osterfeld <osterfeld@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_CALENDARMODEL_P_H
#define AKONADI_CALENDARMODEL_P_H

#include <entitytreemodel.h>
#include <monitor.h>
#include <QSharedPointer>
#include <QWeakPointer>

namespace Akonadi {
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
    ~CalendarModel() override;

    Q_REQUIRED_RESULT QWeakPointer<CalendarModel> weakPointer() const;
    void setWeakPointer(const QWeakPointer<CalendarModel> &weakPointer);

    Q_REQUIRED_RESULT QVariant entityData(const Akonadi::Item &item, int column, int role = Qt::DisplayRole) const override;

    Q_REQUIRED_RESULT QVariant entityData(const Akonadi::Collection &collection, int column, int role = Qt::DisplayRole) const override;

    Q_REQUIRED_RESULT int entityColumnCount(EntityTreeModel::HeaderGroup headerSet) const override;

    Q_REQUIRED_RESULT QVariant entityHeaderData(int section, Qt::Orientation orientation, int role, EntityTreeModel::HeaderGroup headerSet) const override;

private:
    explicit CalendarModel(Akonadi::Monitor *monitor);
    class Private;
    Private *const d;
};
}

#endif
