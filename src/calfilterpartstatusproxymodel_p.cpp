/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calfilterpartstatusproxymodel_p.h"
#include "utils_p.h"

#include <collection.h>
#include <entitytreemodel.h>
#include <item.h>

#include <KCalendarCore/Incidence>
// #include <email.h>

#include <KEMailSettings>
#include <identitymanager.h>

using namespace Akonadi;

class Q_DECL_HIDDEN CalFilterPartStatusProxyModel::Private
{
public:
    explicit Private()
        : mIdentityManager(KIdentityManagement::IdentityManager::self())
    {
    }

    QList<KCalendarCore::Attendee::PartStat> mBlockedStatusList;
    KIdentityManagement::IdentityManager *const mIdentityManager;
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
    QObject::connect(d->mIdentityManager,
                     QOverload<>::of(&KIdentityManagement::IdentityManager::changed),
                     this,
                     &CalFilterPartStatusProxyModel::slotIdentitiesChanged);
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

    const auto item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
    if (!item.isValid()) {
        return false;
    }

    const KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(item);
    if (!incidence) {
        return false;
    }

    // Incidences from virtual collections are always ok
    const auto col = idx.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
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
