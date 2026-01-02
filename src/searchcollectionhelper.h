/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "akonadi-calendar_export.h"

#include <QObject>
#include <QString>

#include <Akonadi/Collection>
#include <KCalendarCore/Attendee>

class KJob;

namespace Akonadi
{
/// Helper class to initialise the search collections
class SearchCollectionHelperPrivate;
class AKONADI_CALENDAR_EXPORT SearchCollectionHelper : public QObject
{
    Q_OBJECT
public:
    /*!
     * \brief Construct a new Search Collection Helper object.
     *
     * Note that helper is disabled by default. Call setEnabled(true) to enable it.
     *
     * \a parent
     */
    explicit SearchCollectionHelper(QObject *parent = nullptr);
    ~SearchCollectionHelper() override;

    void setEnabled(bool enabled);
    [[nodiscard]] bool enabled() const;

private:
    AKONADI_CALENDAR_NO_EXPORT void onSearchCollectionsFetched(KJob *job);
    AKONADI_CALENDAR_NO_EXPORT void updateOpenInvitation();
    AKONADI_CALENDAR_NO_EXPORT void updateDeclinedInvitation();
    AKONADI_CALENDAR_NO_EXPORT void init();
    AKONADI_CALENDAR_NO_EXPORT void deinit();

    AKONADI_CALENDAR_NO_EXPORT void createSearchJobFinished(KJob *job);
    AKONADI_CALENDAR_NO_EXPORT void modifyResult(KJob *job);

    AKONADI_CALENDAR_NO_EXPORT void fetchSearchCollections();
    AKONADI_CALENDAR_NO_EXPORT void
    updateSearchCollection(Akonadi::Collection col, KCalendarCore::Attendee::PartStat status, const QString &name, const QString &displayName);
    AKONADI_CALENDAR_NO_EXPORT void removeSearchCollections();

    std::unique_ptr<SearchCollectionHelperPrivate> const d;
};
}
