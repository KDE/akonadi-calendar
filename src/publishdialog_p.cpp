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

#include "publishdialog_p.h"

#include <Akonadi/Contact/EmailAddressSelectionDialog>
#include <Akonadi/Contact/AbstractEmailAddressSelectionDialog>
#include <KEmailAddress>
#include <kcalendarcore/person.h>


#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginLoader>
#include <QPointer>
#include <QTreeView>

using namespace KCalendarCore;
using namespace Akonadi;

PublishDialog::Private::Private(PublishDialog *qq) : QObject(), q(qq)
{
}

void PublishDialog::Private::addItem()
{
    mUI.mNameLineEdit->setEnabled(true);
    mUI.mEmailLineEdit->setEnabled(true);
    QListWidgetItem *item = mUI.mListWidget->currentItem();
    if (item) {
        if (!item->text().isEmpty()) {
            item = new QListWidgetItem(mUI.mListWidget);
            mUI.mListWidget->addItem(item);
        }
    } else {
        item = new QListWidgetItem(mUI.mListWidget);
        mUI.mListWidget->addItem(item);
    }

    item->setSelected(true);
    mUI.mNameLineEdit->setPlaceholderText(i18n("(EmptyName)"));
    mUI.mEmailLineEdit->setPlaceholderText(i18n("(EmptyEmail)"));
    mUI.mListWidget->setCurrentItem(item);
    mUI.mRemove->setEnabled(true);
}

void PublishDialog::Private::removeItem()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }
    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);

    int row = mUI.mListWidget->row(item);
    mUI.mListWidget->takeItem(row);

    if (!mUI.mListWidget->count()) {
        mUI.mNameLineEdit->setText(QString());
        mUI.mNameLineEdit->setEnabled(false);
        mUI.mEmailLineEdit->setText(QString());
        mUI.mEmailLineEdit->setEnabled(false);
        mUI.mRemove->setEnabled(false);
        return;
    }
    if (row > 0) {
        row--;
    }

    mUI.mListWidget->setCurrentRow(row);
}

void PublishDialog::Private::insertAddresses(const KContacts::Addressee::List &list)
{
    for (const KContacts::Addressee &contact : list) {
        mUI.mNameLineEdit->setEnabled(true);
        mUI.mEmailLineEdit->setEnabled(true);
        QListWidgetItem *item = new QListWidgetItem(mUI.mListWidget);
        item->setSelected(true);
        mUI.mNameLineEdit->setText(contact.name());
        mUI.mEmailLineEdit->setText(contact.preferredEmail());
        mUI.mListWidget->addItem(item);
    }
}


void PublishDialog::Private::openAddressbook()
{
    QPointer<Akonadi::AbstractEmailAddressSelectionDialog> dialog;
    KPluginLoader loader(QStringLiteral("akonadi/emailaddressselectionldapdialogplugin"));
    KPluginFactory *factory = loader.factory();
    if (factory) {
        dialog = factory->create<Akonadi::AbstractEmailAddressSelectionDialog>(q);
    } else {
        dialog = new Akonadi::EmailAddressSelectionDialog(q);
    }

    dialog->view()->view()->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(dialog.data(), &Akonadi::AbstractEmailAddressSelectionDialog::insertAddresses, this, &PublishDialog::Private::insertAddresses);
    if (dialog->exec() == QDialog::Accepted) {
        const Akonadi::EmailAddressSelection::List selections = dialog->selectedAddresses();
        if (!selections.isEmpty()) {
            for (const Akonadi::EmailAddressSelection &selection : selections) {
                mUI.mNameLineEdit->setEnabled(true);
                mUI.mEmailLineEdit->setEnabled(true);
                QListWidgetItem *item = new QListWidgetItem(mUI.mListWidget);
                item->setSelected(true);
                mUI.mNameLineEdit->setText(selection.name());
                mUI.mEmailLineEdit->setText(selection.email());
                mUI.mListWidget->addItem(item);
            }

            mUI.mRemove->setEnabled(true);
        }
    }
    delete dialog;
}

void PublishDialog::Private::updateItem()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }

    Person person(mUI.mNameLineEdit->text(), mUI.mEmailLineEdit->text());
    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);
    item->setText(person.fullName());
}

void PublishDialog::Private::updateInput()
{
    if (mUI.mListWidget->selectedItems().isEmpty()) {
        return;
    }

    mUI.mNameLineEdit->setEnabled(true);
    mUI.mEmailLineEdit->setEnabled(true);

    QListWidgetItem *item = mUI.mListWidget->selectedItems().at(0);
    QString mail, name;
    KEmailAddress::extractEmailAddressAndName(item->text(), mail, name);
    mUI.mNameLineEdit->setText(name);
    mUI.mEmailLineEdit->setText(mail);
}

