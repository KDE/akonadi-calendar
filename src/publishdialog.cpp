/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Allen Winter <winter@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
*/

// TODO: validate hand-entered email addresses
// TODO: don't allow duplicates; at least remove dupes when passing back
// TODO: the list in PublishDialog::addresses()

#include "publishdialog_p.h"

#include <kcalendarcore/attendee.h>
#include <kcalendarcore/person.h>

#include <KCodecs/KEmailAddress>

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
    , d(new Private(this))
{
    setWindowTitle(i18nc("@title:window", "Select Addresses"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("PublishFreeBusy"));
    d->mUI.setupUi(widget);
    layout->addWidget(widget);
    d->mUI.mListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    d->mUI.mNameLineEdit->setEnabled(false);
    d->mUI.mEmailLineEdit->setEnabled(false);

    d->mUI.mNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    d->mUI.mRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    d->mUI.mRemove->setEnabled(false);
    d->mUI.mSelectAddressee->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-contacts")));
    connect(d->mUI.mListWidget, &QListWidget::itemSelectionChanged,
            d, &Private::updateInput);
    connect(d->mUI.mNew, &QAbstractButton::clicked,
            d, &Private::addItem);
    connect(d->mUI.mRemove, &QAbstractButton::clicked,
            d, &Private::removeItem);
    connect(d->mUI.mSelectAddressee, &QAbstractButton::clicked,
            d, &Private::openAddressbook);
    connect(d->mUI.mNameLineEdit, &QLineEdit::textChanged,
            d, &Private::updateItem);
    connect(d->mUI.mEmailLineEdit, &QLineEdit::textChanged,
            d, &Private::updateItem);

    QDialogButtonBox *buttonBox
        = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    layout->addWidget(buttonBox);

    okButton->setToolTip(i18n("Send email to these recipients"));
    okButton->setWhatsThis(i18n("Clicking the <b>Ok</b> button will cause "
                                "an email to be sent to the recipients you "
                                "have entered."));

    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton->setToolTip(i18n("Cancel recipient selection and the email"));
    cancelButton->setWhatsThis(i18n("Clicking the <b>Cancel</b> button will "
                                    "cause the email operation to be terminated."));

    QPushButton *helpButton = buttonBox->button(QDialogButtonBox::Help);
    helpButton->setWhatsThis(i18n("Click the <b>Help</b> button to read "
                                  "more information about Group Scheduling."));

    connect(buttonBox, &QDialogButtonBox::accepted, this, &PublishDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PublishDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, this, &PublishDialog::slotHelp);
}

PublishDialog::~PublishDialog()
{
    delete d;
}

void PublishDialog::slotHelp()
{
    const QUrl url = QUrl(QStringLiteral("help:/")).resolved(QUrl(QStringLiteral("korganizer/group-scheduling.html")));
    // launch khelpcenter, or a browser for URIs not handled by khelpcenter
    QDesktopServices::openUrl(url);
}

void PublishDialog::addAttendee(const Attendee &attendee)
{
    d->mUI.mNameLineEdit->setEnabled(true);
    d->mUI.mEmailLineEdit->setEnabled(true);
    QListWidgetItem *item = new QListWidgetItem(d->mUI.mListWidget);
    Person person(attendee.name(), attendee.email());
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
            toList << item->text();
        }
    }

    return toList.join(QLatin1Char(','));
}

void PublishDialog::accept()
{
    QString badAddress;
    const KEmailAddress::EmailParseResult addressOk
        = KEmailAddress::isValidAddressList(addresses(), badAddress);
    if (addressOk != KEmailAddress::AddressOk) {
        KMessageBox::sorry(this,
                           i18n("Unable to publish the calendar incidence due to an "
                                "invalid recipients string. %1",
                                emailParseResultToString(addressOk)),
                           i18n("Publishing Error"));
    } else {
        QDialog::accept();
    }
}
