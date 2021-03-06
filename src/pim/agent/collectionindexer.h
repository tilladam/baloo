/*
 * Copyright 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COLLECTIONINDEXER_H
#define COLLECTIONINDEXER_H

#include <akonadi/collection.h>
#include <QString>

namespace Xapian {
    class WritableDatabase;
}

class CollectionIndexer : public QObject
{
public:
    CollectionIndexer(const QString& path);
    ~CollectionIndexer();

    void index(const Akonadi::Collection& collection);
    void change(const Akonadi::Collection& collection);
    void remove(const Akonadi::Collection& col);
    void move(const Akonadi::Collection& collection,
            const Akonadi::Collection& from,
            const Akonadi::Collection& to);
    void commit();

private:
    void adjustPath(const Akonadi::Collection& col, int level, const QString &name);
    Xapian::WritableDatabase* m_db;
};

#endif
