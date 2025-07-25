/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "searchcollectionhelper.h"
using namespace Qt::Literals::StringLiterals;

#include "akonadicalendar_debug.h"

#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/PersistentSearchAttribute>
#include <Akonadi/SearchCreateJob>
#include <Akonadi/SearchQuery>

#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <KCalendarCore/Todo>

#include <KIdentityManagementCore/IdentityManager>

#include <KLocalizedString>

using namespace Akonadi;

namespace Akonadi
{

class SearchCollectionHelperPrivate
{
public:
    KIdentityManagementCore::IdentityManager *const identityManager = KIdentityManagementCore::IdentityManager::self();
    Akonadi::Collection openInvitationCollection;
    Akonadi::Collection declineInvitationCollection;
    bool enabled = false;
};

}

SearchCollectionHelper::SearchCollectionHelper(QObject *parent)
    : QObject(parent)
    , d(std::make_unique<SearchCollectionHelperPrivate>())
{
    fetchSearchCollections();
}

SearchCollectionHelper::~SearchCollectionHelper() = default;

void SearchCollectionHelper::setEnabled(bool enabled)
{
    if (d->enabled == enabled) {
        return;
    }

    d->enabled = enabled;
    if (d->enabled) {
        init();
    } else {
        deinit();
    }
}

bool SearchCollectionHelper::enabled() const
{
    return d->enabled;
}

void SearchCollectionHelper::init()
{
    fetchSearchCollections();
    connect(d->identityManager, qOverload<>(&KIdentityManagementCore::IdentityManager::changed), this, &SearchCollectionHelper::updateOpenInvitation);
    connect(d->identityManager, qOverload<>(&KIdentityManagementCore::IdentityManager::changed), this, &SearchCollectionHelper::updateDeclinedInvitation);
}

void SearchCollectionHelper::deinit()
{
    d->identityManager->disconnect(this);
    removeSearchCollections();
}

void SearchCollectionHelper::fetchSearchCollections()
{
    // Collection "Search", has always ID 1
    auto fetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection(1), Akonadi::CollectionFetchJob::FirstLevel);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &SearchCollectionHelper::onSearchCollectionsFetched);
}

void SearchCollectionHelper::onSearchCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADICALENDAR_LOG) << "Search failed: " << job->errorString();
    } else {
        auto fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
        const Akonadi::Collection::List lstCols = fetchJob->collections();
        for (const Akonadi::Collection &col : lstCols) {
            const QString collectionName = col.name();
            if (collectionName == "OpenInvitations"_L1) {
                d->openInvitationCollection = col;
            } else if (collectionName == "DeclinedInvitations"_L1) {
                d->declineInvitationCollection = col;
            }
        }
    }

    if (d->enabled) {
        updateOpenInvitation();
        updateDeclinedInvitation();
    } else {
        // When disabled, make sure the collections are not shown
        removeSearchCollections();
    }
}

void SearchCollectionHelper::updateSearchCollection(Akonadi::Collection col,
                                                    KCalendarCore::Attendee::PartStat status,
                                                    const QString &name,
                                                    const QString &displayName)
{
    // Update or create search collections

    Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
    const QStringList lstEmails = d->identityManager->allEmails();
    for (const QString &email : lstEmails) {
        if (!email.isEmpty()) {
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QString(email + QString::number(status))));
        }
    }

    if (!col.isValid()) {
        auto job = new Akonadi::SearchCreateJob(name, query);
        job->setRemoteSearchEnabled(false);
        job->setSearchMimeTypes({
            KCalendarCore::Event::eventMimeType(),
            KCalendarCore::Todo::todoMimeType(),
            KCalendarCore::Journal::journalMimeType(),
        });
        connect(job, &Akonadi::SearchCreateJob::result, this, &SearchCollectionHelper::createSearchJobFinished);
        qCDebug(AKONADICALENDAR_LOG) << "We have to create a " << name << " virtual Collection";
    } else {
        auto attribute = col.attribute<Akonadi::PersistentSearchAttribute>(Akonadi::Collection::AddIfMissing);
        auto displayname = col.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
        attribute->setQueryString(QString::fromLatin1(query.toJSON()));
        attribute->setRemoteSearchEnabled(false);
        attribute->setRecursive(true);
        displayname->setDisplayName(displayName);
        col.setEnabled(true);
        auto job = new Akonadi::CollectionModifyJob(col, this);
        connect(job, &Akonadi::CollectionModifyJob::result, this, &SearchCollectionHelper::modifyResult);
        qCDebug(AKONADICALENDAR_LOG) << "updating " << name << " (" << col.id() << ") virtual Collection";
        qCDebug(AKONADICALENDAR_LOG) << "query" << query.toJSON();
    }
}

void SearchCollectionHelper::removeSearchCollections()
{
    if (d->openInvitationCollection.isValid()) {
        new Akonadi::CollectionDeleteJob(d->openInvitationCollection, this);
        d->openInvitationCollection = {};
    }
    if (d->declineInvitationCollection.isValid()) {
        new Akonadi::CollectionDeleteJob(d->declineInvitationCollection, this);
        d->declineInvitationCollection = {};
    }
}

void SearchCollectionHelper::updateDeclinedInvitation()
{
    updateSearchCollection(d->declineInvitationCollection,
                           KCalendarCore::Attendee::Declined,
                           u"DeclinedInvitations"_s,
                           i18nc("A collection of all declined invidations.", "Declined Invitations"));
}

void SearchCollectionHelper::updateOpenInvitation()
{
    updateSearchCollection(d->openInvitationCollection,
                           KCalendarCore::Attendee::NeedsAction,
                           u"OpenInvitations"_s,
                           i18nc("A collection of all open invidations.", "Open Invitations"));
}

void SearchCollectionHelper::createSearchJobFinished(KJob *job)
{
    auto createJob = qobject_cast<Akonadi::SearchCreateJob *>(job);
    const Akonadi::Collection searchCollection = createJob->createdCollection();
    if (job->error()) {
        qCWarning(AKONADICALENDAR_LOG) << "Error occurred " << searchCollection.name() << job->errorString();
        return;
    }
    qCDebug(AKONADICALENDAR_LOG) << "Created search folder successfully " << searchCollection.name();

    const QString searchCollectionName = searchCollection.name();
    if (searchCollectionName == "OpenInvitations"_L1) {
        d->openInvitationCollection = searchCollection;
        updateOpenInvitation();
    } else if (searchCollectionName == "DeclinedInvitations"_L1) {
        d->declineInvitationCollection = searchCollection;
        updateDeclinedInvitation();
    }
}

void SearchCollectionHelper::modifyResult(KJob *job)
{
    if (job->error()) {
        qCWarning(AKONADICALENDAR_LOG) << "Error occurred " << job->errorString();
    } else {
        qCDebug(AKONADICALENDAR_LOG) << "modify was successful";
    }
}

#include "moc_searchcollectionhelper.cpp"
