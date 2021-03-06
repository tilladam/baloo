/* This file is part of the KDE Project
   Copyright (c) 2009-2011 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _METADATA_MOVER_H_
#define _METADATA_MOVER_H_

#include <QObject>

class Database;

namespace Baloo
{

class MetadataMover : public QObject
{
    Q_OBJECT

public:
    MetadataMover(Database* db, QObject* parent = 0);
    ~MetadataMover();

public Q_SLOTS:
    void moveFileMetadata(const QString& from, const QString& to);
    void removeFileMetadata(const QString& file);
    void removeFileMetadata(const QStringList& files);

Q_SIGNALS:
    /**
     * Emitted for files (and folders) that have been moved but
     * do not have metadata to be moved. This allows the file indexer
     * service to pick them up in case they are of interest. The
     * typical example would be moving a file from a non-indexed into
     * an indexed folder.
     */
    void movedWithoutData(const QString& path);

    /**
     * Emitted when file with id \p id has been removed. This is emitted
     * so that the clients using this id can update their databases.
     */
    void fileRemoved(int id);

private:
    /**
     * Remove the metadata for file \p url
     */
    void removeMetadata(const QString& url);

    /**
     * Recursively update the nie:url and nie:isPartOf properties
     * of the resource describing \p from.
     */
    void updateMetadata(const QString& from, const QString& to);

    Database* m_db;
};
}

#endif
