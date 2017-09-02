/*
  Copyright (c) 2011-2012 Sérgio Martins <iamsergio@gmail.com>

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

#ifndef AKONADI_ETMCALENDAR_P_H
#define AKONADI_ETMCALENDAR_P_H

#include "etmcalendar.h"
#include "calendarbase_p.h"
#include "incidencechanger.h"
#include "calendarmodel_p.h"

#include <entitytreemodel.h>
#include <KCheckableProxyModel>

#include <QSet>
#include <QModelIndex>

class QAbstractItemModel;
class CheckableProxyModel;
class KSelectionProxyModel;

namespace Akonadi
{

class EntityTreeModel;
class EntityMimeTypeFilterModel;
class CollectionFilterProxyModel;
class CalFilterProxyModel;
class CalFilterPartStatusProxyModel;

static bool isStructuralCollection(const Akonadi::Collection &collection)
{
    const QStringList mimeTypes = QStringList() << QStringLiteral("text/calendar")
                                                << KCalCore::Event::eventMimeType()
                                                << KCalCore::Todo::todoMimeType()
                                                << KCalCore::Journal::journalMimeType();
    const QStringList collectionMimeTypes = collection.contentMimeTypes();
    for (const QString &mimeType : mimeTypes) {
        if (collectionMimeTypes.contains(mimeType)) {
            return false;
        }
    }

    return true;
}

class CheckableProxyModel : public KCheckableProxyModel
{
    Q_OBJECT
public:
    explicit CheckableProxyModel(QObject *parent = nullptr)
        : KCheckableProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::CheckStateRole) {
            // Don't show the checkbox if the collection can't contain incidences
            const Akonadi::Collection collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (isStructuralCollection(collection)) {
                return QVariant();
            }
        }
        return KCheckableProxyModel::data(index, role);
    }
};

class ETMCalendarPrivate : public CalendarBasePrivate
{
    Q_OBJECT
public:

    explicit ETMCalendarPrivate(ETMCalendar *qq);
    ~ETMCalendarPrivate();

    void init();
    void setupFilteredETM();
    void loadFromETM();

public Q_SLOTS:
    Akonadi::Item::List itemsFromModel(const QAbstractItemModel *model,
                                       const QModelIndex &parentIndex = QModelIndex(),
                                       int start = 0,
                                       int end = -1);

    Akonadi::Collection::List collectionsFromModel(const QAbstractItemModel *model,
            const QModelIndex &parentIndex = QModelIndex(),
            int start = 0,
            int end = -1);

    // KCalCore::CalFilter has changed.
    void onFilterChanged();

    void clear();
    void updateItem(const Akonadi::Item &item);
    Akonadi::Item itemFromIndex(const QModelIndex &idx);
    Akonadi::Collection collectionFromIndex(const QModelIndex &index);
    void itemsAdded(const Akonadi::Item::List &items);
    void itemsRemoved(const Akonadi::Item::List &items);

    void onRowsInserted(const QModelIndex &index, int start, int end);
    void onRowsRemoved(const QModelIndex &index, int start, int end);
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd,
                     const QModelIndex &destinationParent, int destinationRow);

    void onLayoutChangedInFilteredModel();
    void onModelResetInFilteredModel();
    void onDataChangedInFilteredModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onRowsInsertedInFilteredModel(const QModelIndex &index, int start, int end);
    void onRowsAboutToBeRemovedInFilteredModel(const QModelIndex &index, int start, int end);
    void onCollectionChanged(const Akonadi::Collection &collection, const QSet<QByteArray> &attributeNames);
    void onCollectionPopulated(Akonadi::Collection::Id);

public:
    Akonadi::CalendarModel::Ptr mETM;
    Akonadi::EntityMimeTypeFilterModel *mFilteredETM = nullptr;

    // akonadi id to collections
    QHash<Akonadi::Collection::Id, Akonadi::Collection> mCollectionMap;
    CheckableProxyModel *mCheckableProxyModel = nullptr;
    Akonadi::CollectionFilterProxyModel *mCollectionProxyModel = nullptr;
    Akonadi::CalFilterProxyModel *mCalFilterProxyModel = nullptr; //KCalCore::CalFilter stuff
    //filter out all invitations and declined events
    Akonadi::CalFilterPartStatusProxyModel *mCalFilterPartStatusProxyModel = nullptr;
    KSelectionProxyModel *mSelectionProxy = nullptr;
    bool mCollectionFilteringEnabled;
    QSet<Akonadi::Collection::Id> mPopulatedCollectionIds;
    QStringList mMimeTypes;
private:
    ETMCalendar *const q;
};

}

#endif
