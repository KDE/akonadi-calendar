/*
  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2009-2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KCalendarCore/Incidence>

namespace Akonadi
{
namespace MailBodyFormatter
{
/*!
  Create a QString representation of an Incidence in format suitable for
  including inside a mail message.
  All dates and times are converted to local time for display.
  \param incidence a pointer to the Incidence to be formatted
  \return the formatted string representation of the incidence
*/
[[nodiscard]] QString mailBodyStr(const KCalendarCore::IncidenceBase::Ptr &incidence);

}
}
