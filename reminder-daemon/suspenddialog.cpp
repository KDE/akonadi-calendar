// SPDX-FileCopyrightText: 2000, 2003 Cornelius Schumacher <schumacher@kde.org>
// SPDX-FileCopyrightText: 2008-2020 Allen Winter <winter@kde.org>
// SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
// SPDX-FileCopyrightText: 2024 David Faure <faure@kde.org>

// SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0

#include "suspenddialog.h"

#include <KConfigGroup>
#include <KLocalizedString>

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QMetaEnum>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(SuspendDialog, SuspendUnit)

namespace
{
const int defSuspendVal = 5;
const SuspendDialog::SuspendUnit defSuspendUnit = SuspendDialog::SuspendInMinutes;

std::chrono::seconds delayFromGui(QSpinBox *spinBox, QComboBox *suspendUnitCombo)
{
    int unit = 1;
    switch (suspendUnitCombo->currentIndex()) {
    case SuspendDialog::SuspendInWeeks:
        unit *= 7;
        Q_FALLTHROUGH();
    case SuspendDialog::SuspendInDays:
        unit *= 24;
        Q_FALLTHROUGH();
    case SuspendDialog::SuspendInHours:
        unit *= 60;
        Q_FALLTHROUGH();
    case SuspendDialog::SuspendInMinutes:
        unit *= 60;
        Q_FALLTHROUGH();
    default:
        break;
    }

    return std::chrono::seconds(spinBox->value() * unit);
}
}

SuspendDialog::SuspendDialog(const KSharedConfig::Ptr &config, const QString &title, const QString &text, QWidget *parent)
    : QDialog(parent, Qt::WindowStaysOnTopHint)
    , m_config(config)
{
    KConfigGroup const configGroup(config, QStringLiteral("Suspend"));
    const int suspendVal = configGroup.readEntry(QStringLiteral("DefaultSuspendValue"), defSuspendVal);
    const SuspendUnit suspendUnit = configGroup.readEntry(QStringLiteral("DefaultSuspendUnit"), defSuspendUnit);

    auto mainLayout = new QVBoxLayout(this);

    auto titleLabel = new QLabel(title, this);
    mainLayout->addWidget(titleLabel);

    if (!text.isEmpty()) {
        auto textLabel = new QLabel(text, this);
        mainLayout->addWidget(textLabel);
    }

    auto suspendBox = new QWidget(this);
    auto suspendBoxHBoxLayout = new QHBoxLayout(suspendBox);
    mainLayout->addWidget(suspendBox);

    auto label = new QLabel(i18nc("@label:spinbox", "Suspend &duration:"), suspendBox);
    suspendBoxHBoxLayout->addWidget(label);

    auto suspendSpin = new QSpinBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(suspendSpin);
    suspendSpin->setRange(1, 9999);
    suspendSpin->setValue(suspendVal);
    suspendSpin->setToolTip(i18nc("@info:tooltip", "Suspend the reminders by this amount of time"));
    suspendSpin->setWhatsThis(i18nc("@info:whatsthis",
                                    "Each reminder for the selected incidences will be suspended "
                                    "by this number of time units. You can choose the time units "
                                    "(typically minutes) in the adjacent selector."));

    label->setBuddy(suspendSpin);

    auto suspendUnitCombo = new QComboBox(suspendBox);
    suspendBoxHBoxLayout->addWidget(suspendUnitCombo);
    suspendUnitCombo->addItem(i18nc("@item:inlistbox suspend in terms of minutes", "minute(s)"));
    suspendUnitCombo->addItem(i18nc("@item:inlistbox suspend in terms of hours", "hour(s)"));
    suspendUnitCombo->addItem(i18nc("@item:inlistbox suspend in terms of days", "day(s)"));
    suspendUnitCombo->addItem(i18nc("@item:inlistbox suspend in terms of weeks", "week(s)"));
    suspendUnitCombo->setToolTip(i18nc("@info:tooltip", "Suspend the reminders using this time unit"));
    suspendUnitCombo->setWhatsThis(i18nc("@info:whatsthis",
                                         "Each reminder for the selected incidences will be suspended "
                                         "using this time unit. You can set the number of time units "
                                         "in the adjacent number entry input."));
    suspendUnitCombo->setCurrentIndex(suspendUnit);

    // Button box

    auto buttonBox = new QDialogButtonBox(this);
    mainLayout->addWidget(buttonBox);

    auto suspendButton = new QPushButton(this);
    suspendButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    buttonBox->addButton(suspendButton, QDialogButtonBox::AcceptRole);
    suspendButton->setText(i18nc("@action:button", "Remind Later"));
    suspendButton->setToolTip(i18nc("@info:tooltip", "Remind me again after the specified interval"));
    suspendButton->setWhatsThis(i18nc("@info:whatsthis", "Press this button to be reminded again about this incidence after the specified amount of time."));

    auto cancelButton = new QPushButton(this);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);
    cancelButton->setText(i18nc("@action:button", "Cancel"));
    cancelButton->setToolTip(i18nc("@info:tooltip", "Cancel reminding later, show the notification again"));

    connect(suspendButton, &QPushButton::clicked, this, [this, suspendSpin, suspendUnitCombo]() {
        KConfigGroup configGroup(m_config, QStringLiteral("Suspend"));
        configGroup.writeEntry(QStringLiteral("DefaultSuspendValue"), suspendSpin->value());
        configGroup.writeEntry(QStringLiteral("DefaultSuspendUnit"), static_cast<SuspendUnit>(suspendUnitCombo->currentIndex()));

        Q_EMIT suspendRequested(delayFromGui(suspendSpin, suspendUnitCombo));
        close();
    });
    connect(cancelButton, &QPushButton::clicked, this, [this]() {
        Q_EMIT cancelRequested();
        close();
    });
}

#include "moc_suspenddialog.cpp"
