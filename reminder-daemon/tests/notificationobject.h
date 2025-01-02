/*
   SPDX-FileCopyrightText: 2023-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KNotification>
#include <QObject>
#include <QPointer>

class NotificationObject : public QObject
{
    Q_OBJECT
public:
    explicit NotificationObject(QObject *parent = nullptr);
    ~NotificationObject() override;

    void sendNotification(const QString &title, const QString &summary);

private:
    QPointer<KNotification> m_notification;
};
