/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

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

#ifndef AKONADI_CALFILTERPARTSTATUSPROXYMODEL_P_H
#define AKONADI_CALFILTERPARTSTATUSPROXYMODEL_P_H

#include <kcalcore/attendee.h>
#include <QSortFilterProxyModel>

namespace Akonadi
{

class CalFilterPartStatusProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CalFilterPartStatusProxyModel(QObject *parent = nullptr);
    ~CalFilterPartStatusProxyModel();

    void setFilterVirtual(bool filterVirtual);
    bool filterVirtual() const;

    void setBlockedStatusList(const QList<KCalCore::Attendee::PartStat> &blockStatusList);
    const QList<KCalCore::Attendee::PartStat> &blockedStatusList() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const Q_DECL_OVERRIDE;

private:
    //@cond PRIVATE
    class Private;
    Private *const d;

    //@endcond
private Q_SLOTS:
    void slotIdentitiesChanged();
};

}

#endif
