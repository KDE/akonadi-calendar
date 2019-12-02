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

#include "calfilterpartstatusproxymodel_p.h"
#include "utils_p.h"

#include <collection.h>
#include <item.h>
#include <entitytreemodel.h>

#include <kcalendarcore/incidence.h>
#include <kcalendarcore/attendee.h>
// #include <email.h>

#include <identitymanager.h>
#include <KEMailSettings>

using namespace Akonadi;

class Q_DECL_HIDDEN CalFilterPartStatusProxyModel::Private
{
public:
    explicit Private()
        : mIdentityManager(KIdentityManagement::IdentityManager::self()),
          mFilterVirtual(false)
    {
    }

    QList<KCalendarCore::Attendee::PartStat> mBlockedStatusList;
    KIdentityManagement::IdentityManager *mIdentityManager = nullptr;
    bool mFilterVirtual = false;
};

void CalFilterPartStatusProxyModel::slotIdentitiesChanged()
{
    invalidate();
}

CalFilterPartStatusProxyModel::CalFilterPartStatusProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , d(new Private())
{

    QObject::connect(d->mIdentityManager, QOverload<>::of(&KIdentityManagement::IdentityManager::changed), this, &CalFilterPartStatusProxyModel::slotIdentitiesChanged);
}

CalFilterPartStatusProxyModel::~CalFilterPartStatusProxyModel()
{
    delete d;
}

const QList<KCalendarCore::Attendee::PartStat> &CalFilterPartStatusProxyModel::blockedStatusList() const
{
    return d->mBlockedStatusList;
}

void CalFilterPartStatusProxyModel::setBlockedStatusList(const QList<KCalendarCore::Attendee::PartStat> &blockStatusList)
{
    d->mBlockedStatusList = blockStatusList;
}

bool CalFilterPartStatusProxyModel::filterVirtual() const
{
    return d->mFilterVirtual;
}

void CalFilterPartStatusProxyModel::setFilterVirtual(bool filterVirtual)
{
    d->mFilterVirtual = filterVirtual;
}

bool CalFilterPartStatusProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid()) {
        return false;
    }

    const Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        return false;
    }

    const KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
    if (!incidence) {
        return false;
    }

    // Incidences from virtual collections are always ok
    const Akonadi::Collection col = idx.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (!d->mFilterVirtual && col.isVirtual()) {
        return true;
    }

    // always show if we are the organizer
    if (CalendarUtils::thatIsMe(incidence->organizer().email())) {
        return true;
    }

    const auto attendees = incidence->attendees();
    for (const KCalendarCore::Attendee &attendee : attendees) {
        if (CalendarUtils::thatIsMe(attendee)) {
            return !d->mBlockedStatusList.contains(attendee.status());
        }
    }

    // We are not attendee, so we accept the incidence
    return true;
}
