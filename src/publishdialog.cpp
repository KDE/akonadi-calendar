/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

// TODO: validate hand-entered email addresses
// TODO: don't allow duplicates; at least remove dupes when passing back
// TODO: the list in PublishDialog::addresses()

#include "publishdialog_p.h"
using namespace Qt::Literals::StringLiterals;

#include <KCalendarCore/Person>

#include <KEmailAddress>

#include <KLocalizedString>
#include <KMessageBox>

#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QUrl>

using namespace KCalendarCore;
using namespace Akonadi;

PublishDialog::PublishDialog(QWidget *parent)
    : QDialog(parent)
    , d(new PublishDialogPrivate(this))
{
    setWindowTitle(i18nc("@title:window", "Select Addresses"));
    auto layout = new QVBoxLayout(this);
    auto widget = new QWidget(this);
    widget->setObjectName("PublishFreeBusy"_L1);
    d->mUI.setupUi(widget);
    layout->addWidget(widget);
    d->mUI.mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mUI.mNameLineEdit->setEnabled(false);
    d->mUI.mEmailLineEdit->setEnabled(false);

    d->mUI.mNew->setIcon(QIcon::fromTheme(u"list-add"_s));
    d->mUI.mRemove->setIcon(QIcon::fromTheme(u"list-remove"_s));
    d->mUI.mRemove->setEnabled(false);
    d->mUI.mSelectAddressee->setIcon(QIcon::fromTheme(u"view-pim-contacts"_s));
    connect(d->mUI.mListWidget, &QListWidget::itemSelectionChanged, d.get(), &PublishDialogPrivate::updateInput);
    connect(d->mUI.mNew, &QAbstractButton::clicked, d.get(), &PublishDialogPrivate::addItem);
    connect(d->mUI.mRemove, &QAbstractButton::clicked, d.get(), &PublishDialogPrivate::removeItem);
    connect(d->mUI.mSelectAddressee, &QAbstractButton::clicked, d.get(), &PublishDialogPrivate::openAddressbook);
    connect(d->mUI.mNameLineEdit, &QLineEdit::textChanged, d.get(), &PublishDialogPrivate::updateItem);
    connect(d->mUI.mEmailLineEdit, &QLineEdit::textChanged, d.get(), &PublishDialogPrivate::updateItem);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    layout->addWidget(buttonBox);

    okButton->setToolTip(i18nc("@info:tooltip", "Send email to these recipients"));
    okButton->setWhatsThis(
        i18n("Clicking the <b>Ok</b> button will cause "
             "an email to be sent to the recipients you "
             "have entered."));

    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton->setToolTip(i18nc("@info:tooltip", "Cancel recipient selection and the email"));
    cancelButton->setWhatsThis(
        i18n("Clicking the <b>Cancel</b> button will "
             "cause the email operation to be terminated."));

    QPushButton *helpButton = buttonBox->button(QDialogButtonBox::Help);
    helpButton->setWhatsThis(
        i18n("Click the <b>Help</b> button to read "
             "more information about Group Scheduling."));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PublishDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PublishDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &PublishDialog::slotHelp);
}

PublishDialog::~PublishDialog() = default;

void PublishDialog::slotHelp()
{
    const QUrl url = QUrl(u"help:/"_s).resolved(QUrl(u"korganizer/group-scheduling.html"_s));
    // launch khelpcenter, or a browser for URIs not handled by khelpcenter
    QDesktopServices::openUrl(url);
}

void PublishDialog::addAttendee(const Attendee &attendee)
{
    d->mUI.mNameLineEdit->setEnabled(true);
    d->mUI.mEmailLineEdit->setEnabled(true);
    auto item = new QListWidgetItem(d->mUI.mListWidget);
    Person const person(attendee.name(), attendee.email());
    item->setText(person.fullName());
    d->mUI.mListWidget->addItem(item);
    d->mUI.mRemove->setEnabled(!d->mUI.mListWidget->selectedItems().isEmpty());
}

QString PublishDialog::addresses() const
{
    QStringList toList;
    const int count = d->mUI.mListWidget->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = d->mUI.mListWidget->item(i);
        if (!item->text().isEmpty()) {
            toList << KEmailAddress::extractEmailAddress(KEmailAddress::normalizeAddressesAndEncodeIdn(item->text()));
        }
    }

    return toList.join(u',');
}

void PublishDialog::accept()
{
    QString badAddress;
    const KEmailAddress::EmailParseResult addressOk = KEmailAddress::isValidAddressList(addresses(), badAddress);
    if (addressOk != KEmailAddress::AddressOk) {
        auto errorMessage = emailParseResultToString(addressOk);
        if (d->mUI.mEmailLineEdit->text().isEmpty()) {
            errorMessage = i18nc("@info", "Please provide at least 1 valid email address");
        }
        KMessageBox::error(this,
                           i18nc("@info",
                                 "Unable to publish the calendar incidence due to an "
                                 "invalid recipients string.\n%1",
                                 errorMessage),
                           i18nc("@title:window", "Publishing Error"));
    } else {
        QDialog::accept();
    }
}

#include "moc_publishdialog.cpp"
