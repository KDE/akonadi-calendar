// SPDX-FileCopyrightText: 2024 David Faure <faure@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0

#pragma once

#include <QDialog>
#include <chrono>

#include <KSharedConfig>

class SuspendDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SuspendDialog(const KSharedConfig::Ptr &config, const QString &title, const QString &text, QWidget *parent = nullptr);

    enum SuspendUnit {
        SuspendInMinutes = 0, ///< Suspend time is in minutes
        SuspendInHours = 1, ///< Suspend time is in hours
        SuspendInDays = 2, ///< Suspend time is in days
        SuspendInWeeks = 3 ///< Suspend time is in weeks
    };
    Q_ENUM(SuspendUnit)

Q_SIGNALS:
    void suspendRequested(std::chrono::seconds seconds);
    void cancelRequested();

private:
    KSharedConfig::Ptr m_config;
};

Q_DECLARE_METATYPE(SuspendDialog::SuspendUnit)
