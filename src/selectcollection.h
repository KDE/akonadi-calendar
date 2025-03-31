/*
 * SPDX-FileCopyrightText: 2010-2012 Sérgio Martins <iamsergio@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "akonadi-calendar_export.h"

#include <Akonadi/Collection>

class QWidget;

namespace Akonadi
{
namespace CalendarUtils
{
[[nodiscard]] AKONADI_CALENDAR_EXPORT Akonadi::Collection
selectCollection(QWidget *parent, int &dialogCode, const QStringList &mimeTypes, const Akonadi::Collection &defaultCollection = Akonadi::Collection());
}
}
