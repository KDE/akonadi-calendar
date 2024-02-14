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
    explicit SearchCollectionHelper(QObject *parent = nullptr);

private:
    AKONADI_CALENDAR_NO_EXPORT void onSearchCollectionsFetched(KJob *job);
    AKONADI_CALENDAR_NO_EXPORT void updateOpenInvitation();
    AKONADI_CALENDAR_NO_EXPORT void updateDeclinedInvitation();

    AKONADI_CALENDAR_NO_EXPORT void createSearchJobFinished(KJob *job);
    AKONADI_CALENDAR_NO_EXPORT void modifyResult(KJob *job);

    AKONADI_CALENDAR_NO_EXPORT void setupSearchCollections();
    AKONADI_CALENDAR_NO_EXPORT void
    updateSearchCollection(Akonadi::Collection col, KCalendarCore::Attendee::PartStat status, const QString &name, const QString &displayName);

private:
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    Akonadi::Collection mOpenInvitationCollection;
    Akonadi::Collection mDeclineCollection;
};
}
