/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_CALFILTERPARTSTATUSPROXYMODEL_P_H
#define AKONADI_CALFILTERPARTSTATUSPROXYMODEL_P_H

#include <KCalendarCore/Attendee>
#include <QSortFilterProxyModel>

namespace Akonadi {
class CalFilterPartStatusProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CalFilterPartStatusProxyModel(QObject *parent = nullptr);
    ~CalFilterPartStatusProxyModel() override;

    void setFilterVirtual(bool filterVirtual);
    Q_REQUIRED_RESULT bool filterVirtual() const;

    void setBlockedStatusList(const QList<KCalendarCore::Attendee::PartStat> &blockStatusList);
    const QList<KCalendarCore::Attendee::PartStat> &blockedStatusList() const;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

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
