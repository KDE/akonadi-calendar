/*
    SPDX-FileCopyrightText: 2013 Sérgio Martins <iamsergio@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef HELPER_H_
#define HELPER_H_

#include <collection.h>
#include <item.h>

namespace Helper
{
bool confirmExists(const Akonadi::Item &item);
bool confirmDoesntExist(const Akonadi::Item &item);
Akonadi::Collection fetchCollection();
}

#endif
