/**
  This file is part of the akonadi-calendar library.

  SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "icalimporter.h"
#include "incidencechanger.h"

#include <collection.h>
#include <item.h>

#include <QList>
#include <QObject>
#include <QString>

class KJob;
class QTemporaryFile;
class QByteArray;
namespace KIO
{
class Job;
}

namespace Akonadi
{
class Q_DECL_HIDDEN ICalImporter::Private : public QObject
{
    Q_OBJECT
public:
    Private(Akonadi::IncidenceChanger *changer, ICalImporter *qq);
    ~Private();
    void setErrorMessage(const QString &message);

    ICalImporter *const q;
    Akonadi::IncidenceChanger *m_changer = nullptr;
    int m_numIncidences = 0;
    QList<int> m_pendingRequests;

    QString m_lastErrorMessage;
    bool m_working = false;
    QTemporaryFile *m_temporaryFile = nullptr;
    Akonadi::Collection m_collection;
public Q_SLOTS:
    void resourceCreated(KJob *job);
    void remoteDownloadFinished(KIO::Job *job, const QByteArray &data);
    void onIncidenceCreated(int changeId, const Akonadi::Item &item, Akonadi::IncidenceChanger::ResultCode resultCode, const QString &errorString);
};
}
