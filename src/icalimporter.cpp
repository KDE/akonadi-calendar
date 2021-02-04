/**
  This file is part of the akonadi-calendar library.

  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icalimporter.h"
#include "akonadicalendar_debug.h"
#include "icalimporter_p.h"
#include "utils_p.h"

#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/AgentManager>
#include <AkonadiCore/ServerManager>

#include <KCalendarCore/FileStorage>

#include <KIO/Job>

#include <QDBusInterface>
#include <QTemporaryFile>
#include <QTimeZone>

using namespace KCalendarCore;
using namespace Akonadi;

ICalImporter::Private::Private(IncidenceChanger *changer, ICalImporter *qq)
    : QObject(qq)
    , q(qq)
    , m_changer(changer)
{
    if (!changer) {
        m_changer = new IncidenceChanger(q);
    }
    connect(m_changer, &IncidenceChanger::createFinished, this, &ICalImporter::Private::onIncidenceCreated);
}

ICalImporter::Private::~Private()
{
    delete m_temporaryFile;
}

void ICalImporter::Private::onIncidenceCreated(int changeId,
                                               const Akonadi::Item &item,
                                               Akonadi::IncidenceChanger::ResultCode resultCode,
                                               const QString &errorString)
{
    Q_UNUSED(item)

    if (!m_pendingRequests.contains(changeId)) {
        return; // Not ours
    }

    m_pendingRequests.removeAll(changeId);

    if (resultCode != IncidenceChanger::ResultCodeSuccess) {
        m_working = false;
        setErrorMessage(errorString);
        m_pendingRequests.clear();
        Q_EMIT q->importIntoExistingFinished(false, m_numIncidences);
    } else if (m_pendingRequests.isEmpty()) {
        m_working = false;
        Q_EMIT q->importIntoExistingFinished(true, m_numIncidences);
    }
}

void ICalImporter::Private::setErrorMessage(const QString &message)
{
    m_lastErrorMessage = message;
    qCritical() << message;
}

void ICalImporter::Private::resourceCreated(KJob *job)
{
    auto createjob = qobject_cast<Akonadi::AgentInstanceCreateJob *>(job);

    Q_ASSERT(createjob);
    m_working = false;
    if (createjob->error()) {
        setErrorMessage(i18n("Error creating ical resource: %1", createjob->errorString()));
        Q_EMIT q->importIntoNewFinished(false);
        return;
    }

    Akonadi::AgentInstance instance = createjob->instance();
    const QString service = Akonadi::ServerManager::agentServiceName(Akonadi::ServerManager::Resource, instance.identifier());

    QDBusInterface iface(service, QStringLiteral("/Settings"));
    if (!iface.isValid()) {
        setErrorMessage(i18n("Failed to obtain D-Bus interface for remote configuration."));
        Q_EMIT q->importIntoNewFinished(false);
        return;
    }

    const QString path = createjob->property("path").toString();
    Q_ASSERT(!path.isEmpty());

    iface.call(QStringLiteral("setPath"), path);
    instance.reconfigure();

    Q_EMIT q->importIntoNewFinished(true);
}

void ICalImporter::Private::remoteDownloadFinished(KIO::Job *job, const QByteArray &data)
{
    const bool success = job->error() == 0;
    m_working = false;
    if (success) {
        delete m_temporaryFile;
        m_temporaryFile = new QTemporaryFile();
        m_temporaryFile->write(data.constData(), data.count());
        q->importIntoExistingResource(QUrl(m_temporaryFile->fileName()), m_collection);
    } else {
        setErrorMessage(i18n("Could not download remote file."));
        Q_EMIT q->importIntoExistingFinished(false, 0);
    }
}

ICalImporter::ICalImporter(Akonadi::IncidenceChanger *changer, QObject *parent)
    : QObject(parent)
    , d(new Private(changer, this))
{
}

ICalImporter::~ICalImporter()
{
}

QString ICalImporter::errorMessage() const
{
    return d->m_lastErrorMessage;
}

bool ICalImporter::importIntoNewResource(const QString &filename)
{
    d->m_lastErrorMessage.clear();

    if (d->m_working) {
        d->setErrorMessage(i18n("An import task is already in progress."));
        return false;
    }

    d->m_working = true;

    Akonadi::AgentType type = Akonadi::AgentManager::self()->type(QStringLiteral("akonadi_ical_resource"));

    auto job = new Akonadi::AgentInstanceCreateJob(type, this);
    job->setProperty("path", filename);
    connect(job, &KJob::result, d, &Private::resourceCreated);
    job->start();

    return true;
}

bool ICalImporter::importIntoExistingResource(const QUrl &url, Akonadi::Collection collection)
{
    d->m_lastErrorMessage.clear();

    if (d->m_working) {
        d->setErrorMessage(i18n("An import task is already in progress."));
        return false;
    }

    if (url.isEmpty()) {
        d->setErrorMessage(i18n("Empty filename. Will not import ical file."));
        return false;
    }

    if (!url.isValid()) {
        d->setErrorMessage(i18n("Url to import is malformed."));
        return false;
    }

    if (url.isLocalFile()) {
        if (!QFile::exists(url.path())) {
            d->setErrorMessage(i18n("The specified file doesn't exist, aborting import."));
            return false;
        }
        MemoryCalendar::Ptr temporaryCalendar(new MemoryCalendar(QTimeZone::systemTimeZone()));
        FileStorage storage(temporaryCalendar);
        storage.setFileName(url.path());
        bool success = storage.load();
        if (!success) {
            d->setErrorMessage(i18n("Failed to load ical file, check permissions."));
            return false;
        }

        d->m_pendingRequests.clear();
        const Incidence::List incidences = temporaryCalendar->incidences();

        if (incidences.isEmpty()) {
            d->setErrorMessage(i18n("The ical file to merge is empty."));
            return false;
        }

        if (!collection.isValid()) {
            int dialogCode;
            const QStringList mimeTypes = QStringList()
                << KCalendarCore::Event::eventMimeType() << KCalendarCore::Todo::todoMimeType() << KCalendarCore::Journal::journalMimeType();
            collection = CalendarUtils::selectCollection(nullptr, dialogCode /*by-ref*/, mimeTypes);
        }

        if (!collection.isValid()) {
            // user canceled
            d->setErrorMessage(QString());
            return false;
        }

        const IncidenceChanger::DestinationPolicy policySaved = d->m_changer->destinationPolicy();
        d->m_changer->startAtomicOperation(i18n("Merge ical file into existing calendar."));
        d->m_changer->setDestinationPolicy(IncidenceChanger::DestinationPolicyNeverAsk);
        for (const Incidence::Ptr &incidence : qAsConst(incidences)) {
            Q_ASSERT(incidence);
            if (!incidence) {
                continue;
            }
            const int requestId = d->m_changer->createIncidence(incidence, collection);
            Q_ASSERT(requestId != -1); // -1 only happens with invalid incidences
            if (requestId != -1) {
                d->m_pendingRequests << requestId;
            }
        }
        d->m_changer->endAtomicOperation();

        d->m_changer->setDestinationPolicy(policySaved); // restore
        d->m_numIncidences = incidences.count();
    } else {
        d->m_collection = collection;
        KIO::StoredTransferJob *job = KIO::storedGet(url);
        connect(job, QOverload<KIO::Job *, const QByteArray &>::of(&KIO::TransferJob::data), d, [this](KIO::Job *job, const QByteArray &ba) {
            d->remoteDownloadFinished(job, ba);
        });
    }

    d->m_working = true;
    return true;
}
