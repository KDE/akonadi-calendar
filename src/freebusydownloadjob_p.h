/*
  SPDX-FileCopyrightText: 2010 Bertjan Broeksema <broeksema@kde.org>
  SPDX-FileCopyrightText: 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_FREEBUSYDOWNLOADJOB_P_H
#define AKONADI_FREEBUSYDOWNLOADJOB_P_H

#include <KJob>
#include <QUrl>

namespace KIO {
class Job;
}

namespace Akonadi {
class FreeBusyDownloadJob : public KJob
{
    Q_OBJECT
public:
    explicit FreeBusyDownloadJob(const QUrl &url, QWidget *parentWidget = nullptr);
    ~FreeBusyDownloadJob();

    void start() override;

    QUrl url() const;
    QByteArray rawFreeBusyData() const;

private Q_SLOTS:
    void slotData(KIO::Job *, const QByteArray &data);
    void slotResult(KJob *job);

private:
    QUrl mUrl;
    QByteArray mFreeBusyData;
    QWidget *const mParent;
};
}

#endif // FREEBUSYMANAGER_P_H
