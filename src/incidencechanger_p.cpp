/*
  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencechanger_p.h"
#include "akonadicalendar_debug.h"
#include "utils_p.h"
#include <KCalendarCore/Incidence>
#include <QDialog>
#include <QString>
#include <collectionfetchjob.h>
#include <collectionfetchscope.h>
#include <itemcreatejob.h>

using namespace Akonadi;

void Change::emitUserDialogClosedAfterChange(Akonadi::ITIPHandlerHelper::SendResult status)
{
    Q_EMIT dialogClosedAfterChange(id, status);
}

void Change::emitUserDialogClosedBeforeChange(Akonadi::ITIPHandlerHelper::SendResult status)
{
    Q_EMIT dialogClosedBeforeChange(id, status);
}

void IncidenceChanger::Private::loadCollections()
{
    if (isLoadingCollections()) {
        // Collections are already loading
        return;
    }

    m_collectionFetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive);

    m_collectionFetchJob->fetchScope().setContentMimeTypes(KCalendarCore::Incidence::mimeTypes());
    connect(m_collectionFetchJob, &KJob::result, this, &IncidenceChanger::Private::onCollectionsLoaded);
    m_collectionFetchJob->start();
}

Collection::List IncidenceChanger::Private::collectionsForMimeType(const QString &mimeType, const Collection::List &collections)
{
    Collection::List result;
    for (const Akonadi::Collection &collection : collections) {
        if (collection.contentMimeTypes().contains(mimeType)) {
            result << collection;
        }
    }

    return result;
}

void IncidenceChanger::Private::onCollectionsLoaded(KJob *job)
{
    Q_ASSERT(!mPendingCreations.isEmpty());
    if (job->error() != 0 || !m_collectionFetchJob) {
        qCritical() << "Error loading collections:" << job->errorString();
        return;
    }

    Q_ASSERT(job == m_collectionFetchJob);
    Akonadi::Collection::List allCollections;
    const auto collections = m_collectionFetchJob->collections();
    for (const Akonadi::Collection &collection : collections) {
        if (collection.rights() & Akonadi::Collection::CanCreateItem) {
            allCollections << collection;
        }
    }

    m_collectionFetchJob = nullptr;
    bool canceled = false;

    // These two will never be true, maybe even assert
    bool noAcl = false;
    bool invalidCollection = false;
    Collection collectionToUse;
    const auto lstPendingCreations = mPendingCreations;
    for (const Change::Ptr &change : lstPendingCreations) {
        mPendingCreations.removeAll(change);

        if (canceled) {
            change->resultCode = ResultCodeUserCanceled;
            continue;
        }

        if (noAcl) {
            change->resultCode = ResultCodePermissions;
            continue;
        }

        if (invalidCollection) {
            change->resultCode = ResultCodeInvalidUserCollection;
            continue;
        }

        if (collectionToUse.isValid()) {
            // We don't show the dialog multiple times
            step2CreateIncidence(change, collectionToUse);
            continue;
        }

        KCalendarCore::Incidence::Ptr incidence = CalendarUtils::incidence(change->newItem);
        Collection::List candidateCollections = collectionsForMimeType(incidence->mimeType(), allCollections);
        if (candidateCollections.count() == 1 && candidateCollections.first().isValid()) {
            // We only have 1 writable collection, don't bother the user with a dialog
            collectionToUse = candidateCollections.first();
            qCDebug(AKONADICALENDAR_LOG) << "Only one collection exists, will not show collection dialog: " << collectionToUse.displayName();
            step2CreateIncidence(change, collectionToUse);
            continue;
        }

        // Lets ask the user which collection to use:
        int dialogCode;
        QWidget *parent = change->parentWidget;

        const QStringList mimeTypes(incidence->mimeType());
        collectionToUse = CalendarUtils::selectCollection(parent, /*by-ref*/ dialogCode, mimeTypes, mDefaultCollection);
        if (dialogCode != QDialog::Accepted) {
            qCDebug(AKONADICALENDAR_LOG) << "User canceled collection choosing";
            change->resultCode = ResultCodeUserCanceled;
            canceled = true;
            cancelTransaction();
            continue;
        }

        if (collectionToUse.isValid() && !hasRights(collectionToUse, ChangeTypeCreate)) {
            qCWarning(AKONADICALENDAR_LOG) << "No ACLs for incidence creation";
            const QString errorMessage = showErrorDialog(ResultCodePermissions, parent);
            change->resultCode = ResultCodePermissions;
            change->errorString = errorMessage;
            noAcl = true;
            cancelTransaction();
            continue;
        }

        // TODO: add unit test for these two situations after reviewing API
        if (!collectionToUse.isValid()) {
            qCritical() << "Invalid collection selected. Can't create incidence.";
            change->resultCode = ResultCodeInvalidUserCollection;
            const QString errorString = showErrorDialog(ResultCodeInvalidUserCollection, parent);
            change->errorString = errorString;
            invalidCollection = true;
            cancelTransaction();
            continue;
        }

        step2CreateIncidence(change, collectionToUse);
    }
}

bool IncidenceChanger::Private::isLoadingCollections() const
{
    return m_collectionFetchJob != nullptr;
}

void IncidenceChanger::Private::step1DetermineDestinationCollection(const Change::Ptr &change, const Akonadi::Collection &collection)
{
    QWidget *parent = change->parentWidget.data();
    if (collection.isValid() && hasRights(collection, ChangeTypeCreate)) {
        // The collection passed always has priority
        step2CreateIncidence(change, collection);
    } else {
        switch (mDestinationPolicy) {
        case DestinationPolicyDefault:
            if (mDefaultCollection.isValid() && hasRights(mDefaultCollection, ChangeTypeCreate)) {
                step2CreateIncidence(change, mDefaultCollection);
                break;
            }
            qCWarning(AKONADICALENDAR_LOG) << "Destination policy is to use the default collection."
                                           << "But it's invalid or doesn't have proper ACLs."
                                           << "isValid = " << mDefaultCollection.isValid() << "has ACLs = " << hasRights(mDefaultCollection, ChangeTypeCreate);
            // else fallthrough, and ask the user.
            Q_FALLTHROUGH();
        case DestinationPolicyAsk:
            mPendingCreations << change;
            loadCollections(); // Now we wait, collections are being loaded async
            break;
        case DestinationPolicyNeverAsk: {
            const bool hasRights = this->hasRights(mDefaultCollection, ChangeTypeCreate);
            if (mDefaultCollection.isValid() && hasRights) {
                step2CreateIncidence(change, mDefaultCollection);
            } else {
                const QString errorString = showErrorDialog(ResultCodeInvalidDefaultCollection, parent);
                qCritical() << errorString << "; rights are " << hasRights;
                change->resultCode = hasRights ? ResultCodeInvalidDefaultCollection : ResultCodePermissions;
                change->errorString = errorString;
                cancelTransaction();
            }
            break;
        }
        default:
            // Never happens
            Q_ASSERT_X(false, "createIncidence()", "unknown destination policy");
            cancelTransaction();
        }
    }
}

void IncidenceChanger::Private::step2CreateIncidence(const Change::Ptr &change, const Akonadi::Collection &collection)
{
    Q_ASSERT(change);

    mLastCollectionUsed = collection;

    auto createJob = new ItemCreateJob(change->newItem, collection, parentJob(change));
    mChangeForJob.insert(createJob, change);

    if (mBatchOperationInProgress) {
        AtomicOperation *atomic = mAtomicOperations[mLatestAtomicOperationId];
        Q_ASSERT(atomic);
        atomic->addChange(change);
    }

    // QueuedConnection because of possible sync exec calls.
    connect(createJob, &KJob::result, this, &IncidenceChanger::Private::handleCreateJobResult, Qt::QueuedConnection);

    mChangeById.insert(change->id, change);
}
