/*
 * This file is part of the KDE Baloo Project
 * Copyright (C) 2013  Vishesh Handa <me@vhanda.in>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FILEMAPPING_H
#define FILEMAPPING_H

#include <QString>
#include <QSqlDatabase>
#include "file_export.h"

namespace Baloo {

class BALOO_FILE_EXPORT FileMapping
{
public:
    FileMapping();
    explicit FileMapping(const QString& url);
    explicit FileMapping(uint id);

    uint id() const;
    QString url() const;

    void setUrl(const QString& url);
    void setId(uint id);

    bool fetched();
    bool empty() const;

    void clear();

    /**
     * Fetch the corresponding url or Id depending on what is not
     * available.
     *
     * Returns true if fetching was successful
     */
    bool fetch(QSqlDatabase db);

    /**
     * Creates the corresponding url <-> id mapping
     */
    bool create(QSqlDatabase db);

    bool remove(QSqlDatabase db);

    bool operator ==(const FileMapping& rhs) const;

private:
    QString m_url;
    uint m_id;
};

}
#endif // FILEMAPPING_H
