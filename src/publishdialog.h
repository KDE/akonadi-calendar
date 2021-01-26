/*
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef AKONADI_PUBLISHDIALOG_H
#define AKONADI_PUBLISHDIALOG_H

#include "akonadi-calendar_export.h"

#include <KCalendarCore/Attendee>
#include <QDialog>

// TODO: documentation
// Uses akonadi-contact, so don't move this class to KCalUtils.
namespace Akonadi
{
class AKONADI_CALENDAR_EXPORT PublishDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Creates a new PublishDialog
     * @param parent the dialog's parent
     */
    explicit PublishDialog(QWidget *parent = nullptr);

    /**
     * Destructor
     */
    ~PublishDialog() override;

    /**
     * Adds a new attendee to the dialog
     * @param attendee the attendee to add
     */
    void addAttendee(const KCalendarCore::Attendee &attendee);

    /**
     * Returns a list of e-mail addresses.
     * //TODO: This should be a QStringList, but KCalUtils::Scheduler::publish() accepts a QString.
     */
    Q_REQUIRED_RESULT QString addresses() const;

public Q_SLOTS:
    void accept() override;

private:
    void slotHelp();
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond
};
}

#endif
