/*
 * SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "selectcollection.h"
#include "akonadicalendar_debug.h"

#include <Akonadi/CollectionDialog>

#include <QPointer>

Akonadi::Collection
Akonadi::CalendarUtils::selectCollection(QWidget *parent, int &dialogCode, const QStringList &mimeTypes, const Akonadi::Collection &defaultCollection)
{
    QPointer<Akonadi::CollectionDialog> dlg(new Akonadi::CollectionDialog(parent));

    qCDebug(AKONADICALENDAR_LOG) << "selecting collections with mimeType in " << mimeTypes;

    dlg->changeCollectionDialogOptions(Akonadi::CollectionDialog::KeepTreeExpanded);
    dlg->setMimeTypeFilter(mimeTypes);
    dlg->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    if (defaultCollection.isValid()) {
        dlg->setDefaultCollection(defaultCollection);
    }
    Akonadi::Collection collection;

    // FIXME: don't use exec.
    dialogCode = dlg->exec();
    if (dialogCode == QDialog::Accepted) {
        collection = dlg->selectedCollection();

        if (!collection.isValid()) {
            qCWarning(AKONADICALENDAR_LOG) << "An invalid collection was selected!";
        }
    }
    delete dlg;

    return collection;
}
