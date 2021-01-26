/*
  SPDX-FileCopyrightText: 2011 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileContributor: Volker Krause <vkrause@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "incidencefetchjob_p.h"
#include <KCalendarCore/Event>
#include <KCalendarCore/Journal>
#include <collectionfetchjob.h>
#include <collectionfetchscope.h>
#include <itemfetchjob.h>
#include <itemfetchscope.h>
#include <kcalendarcore/todo.h>

using namespace Akonadi;

Akonadi::IncidenceFetchJob::IncidenceFetchJob(QObject *parent)
    : Job(parent)
{
    m_mimeTypeChecker.addWantedMimeType(QStringLiteral("text/calendar"));
}

Item::List Akonadi::IncidenceFetchJob::items() const
{
    return m_items;
}

void Akonadi::IncidenceFetchJob::doStart()
{
    auto job = new CollectionFetchJob(Collection::root(), CollectionFetchJob::Recursive, this);
    job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("text/calendar") << KCalendarCore::Event::eventMimeType()
                                                        << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType());
    connect(job, &CollectionFetchJob::result, this, &IncidenceFetchJob::collectionFetchResult);
}

void Akonadi::IncidenceFetchJob::collectionFetchResult(KJob *job)
{
    if (job->error()) { // handled in base class
        return;
    }
    auto fetch = qobject_cast<CollectionFetchJob *>(job);
    Q_ASSERT(fetch);

    if (fetch->collections().isEmpty()) {
        emitResult();
        return;
    }

    const auto collections = fetch->collections();
    for (const Collection &col : collections) {
        if (!m_mimeTypeChecker.isWantedCollection(col) || col.isVirtual()) {
            continue;
        }
        auto itemFetch = new ItemFetchJob(col, this);
        itemFetch->fetchScope().fetchFullPayload(true);
        connect(itemFetch, &ItemFetchJob::result, this, &IncidenceFetchJob::itemFetchResult);
        ++m_jobCount;
    }
}

void Akonadi::IncidenceFetchJob::itemFetchResult(KJob *job)
{
    if (job->error()) { // handled in base class
        return;
    }
    --m_jobCount;
    auto fetch = qobject_cast<ItemFetchJob *>(job);
    const auto items = fetch->items();
    for (const Item &item : items) {
        if (!m_mimeTypeChecker.isWantedItem(item)) {
            continue;
        }
        m_items.push_back(item);
    }

    if (m_jobCount <= 0) {
        emitResult();
    }
}
