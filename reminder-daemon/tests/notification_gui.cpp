/*
   SPDX-FileCopyrightText: 2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "notificationobject.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    NotificationObject *obj = new NotificationObject;
    obj->sendNotification(QStringLiteral("ff"), QStringLiteral("ddsdfsf"));
    QTimer *timer = new QTimer;
    timer->setInterval(5000);
    QObject::connect(timer, &QTimer::timeout, [obj, timer]() {
        obj->sendNotification(QStringLiteral("ff"), QStringLiteral("ddsdfsf"));
        timer->start();
    });
    timer->start();

    // TODO
    return app.exec();
}
