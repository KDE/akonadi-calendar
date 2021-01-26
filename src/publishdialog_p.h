/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef AKONADI_PUBLISHDIALOG_P_H
#define AKONADI_PUBLISHDIALOG_P_H

#include "publishdialog.h"
#include "ui_publishdialog_base.h"
#include <KContacts/Addressee>

namespace Akonadi
{
class Q_DECL_HIDDEN PublishDialog::Private : public QObject
{
    Q_OBJECT
public:
    Private(PublishDialog *q);
    void insertAddresses(const KContacts::Addressee::List &list);

public Q_SLOTS:
    void addItem();
    void removeItem();
    void openAddressbook();
    void updateItem();
    void updateInput();

public:
    Ui::PublishDialog_base mUI;
    PublishDialog *q = nullptr;
};
}
#endif
