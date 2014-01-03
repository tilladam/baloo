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

#include "app.h"
#include "../util.h"
#include "../basicindexingjob.h"
#include "../database.h"

#include <KCmdLineArgs>
#include <KMimeType>
#include <KStandardDirs>
#include <KDebug>

#include <QTimer>
#include <QtCore/QFileInfo>
#include <QDBusMessage>
#include <QDBusConnection>

#include <iostream>

using namespace Baloo;

App::App(QObject* parent)
    : QObject(parent)
    , m_termCount(0)
{
    m_path = KStandardDirs::locateLocal("data", "baloo/file");

    m_db.setPath(m_path);
    m_db.init();

    const KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    m_bData = args->isSet("bdata");

    m_results.reserve(args->count());
    for (int i=0; i<args->count(); i++) {
        FileMapping mapping = FileMapping(args->arg(i).toUInt());
        if (mapping.fetch(m_db.sqlDatabase())) {
            // arg is an id
            if (QFile::exists(mapping.url())) {
                m_urls << mapping.url();
            } else {
                // id was looked up, but file deleted
                kDebug() << mapping.url() << "does not exist";
                deleteDocument(mapping.id());
            }
        } else {
            // arg is a url
            QString url = args->url(i).toLocalFile();
            if (!QFile::exists(url)) {
              // arg is a url but could not be found
              continue;
            }
            m_urls << args->url(i).toLocalFile();
        }
    }

    connect(this, SIGNAL(saved()), this, SLOT(processNextUrl()), Qt::QueuedConnection);

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

App::~App()
{
}

void App::processNextUrl()
{
    if (m_urls.isEmpty()) {
        if (m_results.isEmpty()) {
            QCoreApplication::instance()->exit(0);
        }
        else {
            saveChanges();
        }
        return;
    }

    const QString url = m_urls.takeFirst();
    const QString mimetype = KMimeType::findByUrl(KUrl::fromLocalFile(url))->name();

    FileMapping file(url);
    if (!m_bData) {
        if (!file.fetch(m_db.sqlDatabase())) {
            file.create(m_db.sqlDatabase());
        }
    }

    Xapian::Document doc;
    if (file.fetched()) {
        try {
            doc = m_db.xapainDatabase()->get_document(file.id());
        }
        catch (const Xapian::DocNotFoundError&) {
            BasicIndexingJob basicIndexer(&m_db.sqlDatabase(), file, mimetype);
            basicIndexer.index();

            file.setId(basicIndexer.id());
            doc = basicIndexer.document();
        }
    }

    Result result;
    result.setInputUrl(url);
    result.setInputMimetype(mimetype);
    result.setId(file.id());
    result.setDocument(doc);

    QList<KFileMetaData::ExtractorPlugin*> exList = m_manager.fetchExtractors(mimetype);

    Q_FOREACH (KFileMetaData::ExtractorPlugin* plugin, exList) {
        plugin->extract(&result);
    }
    m_results << result;
    m_termCount += result.document().termlist_count();

    // Documents with these many terms occupy about 10 mb
    if (m_termCount >= 10000) {
        saveChanges();
        return;
    }

    if (m_urls.isEmpty()) {
        if (m_bData) {
            QByteArray arr;
            QDataStream s(&arr, QIODevice::WriteOnly);

            Q_FOREACH (const Result& res, m_results)
                s << res.map();

            std::cout << arr.toBase64().constData();
        }
        else {
            saveChanges();
        }
        return;
    }

    QTimer::singleShot(0, this, SLOT(processNextUrl()));
}

void App::saveChanges()
{
    if (m_results.isEmpty())
        return;

    QList<QString> updatedFiles;

    try {
        Xapian::WritableDatabase db(m_path.toStdString(), Xapian::DB_CREATE_OR_OPEN);
        for (int i = 0; i<m_results.size(); i++) {
            Result& res = m_results[i];
            res.save(db);

            updateIndexingLevel(db, res.id(), 2);
            updatedFiles << res.inputUrl();
        }
        
        Q_FOREACH (Xapian::docid id, m_docsToRemove) {
            try {
                db.delete_document(id);
            }
            catch (const Xapian::DocNotFoundError&) {
            }
        }

        db.commit();
        m_results.clear();
        m_docsToRemove.clear();
        m_termCount = 0;

        Q_EMIT saved();
    }
    catch (const Xapian::DatabaseLockError& err) {
        kError() << "Cannot open database in write mode:" << err.get_error_string();
        QTimer::singleShot(100, this, SLOT(saveChanges()));
        return;
    }

    QDBusMessage message = QDBusMessage::createSignal(QLatin1String("/files"),
                                                      QLatin1String("org.kde"),
                                                      QLatin1String("changed"));

    QVariantList vl;
    vl.reserve(1);
    vl << QVariant(updatedFiles);
    message.setArguments(vl);

    QDBusConnection::sessionBus().send(message);
}

void App::deleteDocument(unsigned docid)
{
    m_docsToRemove << docid;
}

