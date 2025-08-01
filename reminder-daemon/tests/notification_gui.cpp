/*
   SPDX-FileCopyrightText: 2023-2025 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "notificationobject.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication const app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    NotificationObject *obj = new NotificationObject;
    obj->sendNotification(QStringLiteral("ff"), QStringLiteral("ddsdfsf"));
    QTimer *timer = new QTimer;
    timer->setInterval(5000);
    QObject::connect(timer, &QTimer::timeout, obj, [obj, timer]() {
        static int value = 0;
        obj->sendNotification(QStringLiteral("title %1").arg(value), QStringLiteral("message %1").arg(value));
        timer->start();
        value++;
    });
    timer->start();
    const int returnValue = app.exec();
    delete timer;
    return returnValue;
}
