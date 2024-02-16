/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <QObject>
#include <QString>

#include <Akonadi/Collection>
#include <KCalendarCore/Attendee>
#include <KIdentityManagementCore/IdentityManager>

class KJob;

namespace Akonadi
{
/// Helper class to initialise the search collections
class AKONADI_CALENDAR_EXPORT SearchCollectionHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Construct a new Search Collection Helper object.
     *
     * Note that helper is disabled by default. Call setEnabled(true) to enable it.
     *
     * @param parent
     */
    explicit SearchCollectionHelper(QObject *parent = nullptr);

    void setEnabled(bool enabled);
    bool enabled() const;

private:
    void onSearchCollectionsFetched(KJob *job);
    void updateOpenInvitation();
    void updateDeclinedInvitation();
    void init();
    void deinit();

    void createSearchJobFinished(KJob *job);
    void modifyResult(KJob *job);

    void fetchSearchCollections();
    void updateSearchCollection(Akonadi::Collection col, KCalendarCore::Attendee::PartStat status, const QString &name, const QString &displayName);
    void removeSearchCollections();

private:
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    Akonadi::Collection mOpenInvitationCollection;
    Akonadi::Collection mDeclineCollection;
    bool mEnabled = false;
};
}
