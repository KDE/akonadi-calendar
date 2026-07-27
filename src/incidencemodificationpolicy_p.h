/*
  SPDX-FileCopyrightText: 2026 Dominique Michel <dominique.michel@enioka.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

namespace Akonadi
{
/*!
 * This enum adds context for modification changes.
 *
 * \internal
 */
enum class IncidenceModificationPolicy {
    Default = 0, ///< Default modification policy.
    Organizer = 1, ///< An update received from the organizer.
};

}
