// SPDX-FileCopyrightText: 2021 Claudio Cambra <claudio.cambra@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "kalendaralarmclient.h"
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>

#include "akonadi-calendar_version.h"

int main(int argc, char **argv)
{
    QApplication const app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setQuitLockEnabled(false);

    KAboutData aboutData(
        // The program name used internally.
        QStringLiteral("kalendarac"),
        // A displayable program name string.
        i18nc("@title", "Reminders"),
        QStringLiteral(AKONADICALENDAR_VERSION_STRING),
        // Short description of what the app does.
        i18n("Calendar Reminder Service"),
        // The license this code is released under.
        KAboutLicense::GPL,
        // Copyright Statement.
        i18n("(c) KDE Community 2021-2024"));
    aboutData.addAuthor(i18nc("@info:credit", "Carl Schwan"),
                        i18nc("@info:credit", "Maintainer"),
                        QStringLiteral("carl@carlschwan.eu"),
                        QStringLiteral("https://carlschwan.eu"));
    aboutData.addAuthor(i18nc("@info:credit", "Clau Cambra"),
                        i18nc("@info:credit", "Maintainer"),
                        QStringLiteral("claudio.cambra@gmail.com"),
                        QStringLiteral("https://claudiocambra.com"));
    aboutData.setProductName("Reminder Daemon/general"); // Bugzilla product/component name
    KAboutData::setApplicationData(aboutData);

    KCrash::initialize();

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService const service(KDBusService::Unique);
    KalendarAlarmClient const client;

    return app.exec();
}
