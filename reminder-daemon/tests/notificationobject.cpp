/*
   SPDX-FileCopyrightText: 2023-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "notificationobject.h"
#include <QDebug>

NotificationObject::NotificationObject(QObject *parent)
    : QObject{parent}
{
}

NotificationObject::~NotificationObject() = default;

void NotificationObject::sendNotification(const QString &title, const QString &summary)
{
    const bool notificationExists = m_notification;
    if (!notificationExists) {
        m_notification = new KNotification(QStringLiteral("alarm"));
    }
    m_notification->setTitle(title);

    m_notification->setFlags(KNotification::Persistent);
    m_notification->setText(summary);

    if (!notificationExists) {
        (void)m_notification->addAction(QStringLiteral("Remind in 5 mins"));
        (void)m_notification->addAction(QStringLiteral("Remind in 1 hour"));

        (void)m_notification->addAction(QStringLiteral("Dismiss"));
        qDebug() << " send event ";
        m_notification->sendEvent();
    }
}

#include "moc_notificationobject.cpp"
