/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Vishesh Handa <me@vhanda.in>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef FILEINDEXINGQUEUE_H
#define FILEINDEXINGQUEUE_H

#include "indexingqueue.h"
#include "filemapping.h"

#include <KJob>
#include <Soprano/QueryResultIterator>

class Database;

namespace Baloo
{

class FileIndexingQueue : public IndexingQueue
{
    Q_OBJECT
public:
    FileIndexingQueue(Database* db, QObject* parent = 0);
    virtual bool isEmpty();
    virtual void fillQueue();

    void clear();
    void clear(const QString& path);
    QString currentUrl();

public Q_SLOTS:
    /**
     * Fills up the queue and starts the indexing
     */
    void start();

    void enqueue(const Baloo::FileMapping& file);
Q_SIGNALS:
    void beginIndexingFile(const Baloo::FileMapping& file);
    void endIndexingFile(const Baloo::FileMapping& file);

protected:
    virtual void processNextIteration();

private Q_SLOTS:
    void slotFinishedIndexingFile(KJob* job);
    void slotConfigChanged();

private:
    void process(const FileMapping& file);

    QQueue<FileMapping> m_fileQueue;
    FileMapping m_currentFile;
    Database* m_db;
};
}

#endif // FILEINDEXINGQUEUE_H
