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

#include "metadatamover.h"
#include "filewatch.h"
#include "filemapping.h"
#include "database.h"

#include <QTimer>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>

#include <KDebug>

using namespace Baloo;

MetadataMover::MetadataMover(Database* db, QObject* parent)
    : QObject(parent)
    , m_db(db)
{
}


MetadataMover::~MetadataMover()
{
}


void MetadataMover::moveFileMetadata(const QString& from, const QString& to)
{
//    kDebug() << from << to;
    Q_ASSERT(!from.isEmpty() && from != "/");
    Q_ASSERT(!to.isEmpty() && to != "/");

    // We do NOT get deleted messages for overwritten files! Thus, we
    // have to remove all metadata for overwritten files first.
    removeMetadata(to);

    // and finally update the old statements
    updateMetadata(from, to);
}

void MetadataMover::removeFileMetadata(const QString& file)
{
    Q_ASSERT(!file.isEmpty() && file != "/");
    removeFileMetadata(QStringList() << file);
}


void MetadataMover::removeFileMetadata(const QStringList& files)
{
    kDebug() << files;

    Q_FOREACH (const QString& file, files) {
        removeMetadata(file);
    }
}


void MetadataMover::removeMetadata(const QString& url)
{
    if (url.isEmpty()) {
        kDebug() << "empty path. Looks like a bug somewhere...";
        return;
    }

    FileMapping file(url);
    file.fetch(m_db->sqlDatabase());

    QSqlQuery query(m_db->sqlDatabase());
    query.prepare("delete from files where url = ?");
    query.addBindValue(url);
    if (!query.exec()) {
        kError() << query.lastError().text();
    }

    if (file.id())
        Q_EMIT fileRemoved(file.id());
}


void MetadataMover::updateMetadata(const QString& from, const QString& to)
{
    kDebug() << from << "->" << to;
    if (from.isEmpty() || to.isEmpty()) {
        kError() << "Paths Empty - File a bug" << from << to;
        return;
    }

    Q_ASSERT(from[from.size()-1] != '/');
    Q_ASSERT(to[to.size()-1] != '/');

    FileMapping fromFile(from);
    fromFile.fetch(m_db->sqlDatabase());

    if (fromFile.fetch(m_db->sqlDatabase())) {
        QSqlQuery q(m_db->sqlDatabase());
        q.prepare("update files set url = ? where id = ?");
        q.addBindValue(to);
        q.addBindValue(fromFile.id());
        if (!q.exec())
            kError() << q.lastError().text();
    }

    if (!fromFile.id()) {
        //
        // If we have no metadata yet we need to tell the file indexer (if running) so it can
        // create the metadata in case the target folder is configured to be indexed.
        //
        Q_EMIT movedWithoutData(to);
    }

    if (!QFileInfo(to).isDir()) {
        return;
    }

    QSqlQuery query(m_db->sqlDatabase());
    /*
    query.prepare("update files set url = ':t' || substr(url, :fs) "
                  "where url like ':f/%'");

    query.bindValue(":t", to);
    query.bindValue(":fs", from.size() + 1);
    query.bindValue(":f", from);
    */

    //
    // Temporary workaround because either sqlite3_prepare16_v2 seems to be buggy
    // or the qt sqlite driver is not calling it properly
    //
    QString queryStr("update files set url = '");
    queryStr.append(to);
    queryStr.append("' || substr(url, ");
    queryStr.append(QString::number(from.size() + 1));
    queryStr.append(") where url like '");
    queryStr.append(from);
    queryStr.append("/%'");

    if (!query.exec(queryStr)) {
        kError() << "Big query failed:" << query.lastError().text();
    }

    m_db->sqlDatabase().commit();
    m_db->sqlDatabase().transaction();
}

#include "metadatamover.moc"
