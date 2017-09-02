/*
    Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

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

#include "helper.h"

#include <itemfetchjob.h>
#include <collectionfetchjob.h>
#include <collectionfetchscope.h>

#include <QStringList>

using namespace Akonadi;

bool Helper::confirmExists(const Akonadi::Item &item)
{
    ItemFetchJob *job = new ItemFetchJob(item);
    return job->exec() != 0;
}

bool Helper::confirmDoesntExist(const Akonadi::Item &item)
{
    ItemFetchJob *job = new ItemFetchJob(item);
    return job->exec() == 0;
}

Akonadi::Collection Helper::fetchCollection()
{
    CollectionFetchJob *job = new CollectionFetchJob(Collection::root(),
                                                     CollectionFetchJob::Recursive);
    // Get list of collections
    job->fetchScope().setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.calendar.event"));
    const bool ret = job->exec();
    Q_ASSERT(ret);
    Q_UNUSED(ret);

    // Find our collection
    const Collection::List collections = job->collections();
    Q_ASSERT(!collections.isEmpty());
    Collection collection = collections.first();

    Q_ASSERT(collection.isValid());

    return collection;
}
