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

#ifndef _BALOO_SEARCHSTORE_H
#define _BALOO_SEARCHSTORE_H

#include <QString>
#include <QHash>
#include <QUrl>
#include <KService>

#include "core_export.h"

namespace Baloo {

class Query;

class BALOO_CORE_EXPORT SearchStore : public QObject
{
    Q_OBJECT
public:
    explicit SearchStore(QObject* parent = 0);
    virtual ~SearchStore();

    /**
     * Override search stores for testing
     */
    static void overrideSearchStores(const QList<SearchStore*> &overrideSearchStores);

    /**
     * Gives a list of available search stores. These stores must be managed and
     * deleted by the caller
     */
    static QList<SearchStore*> searchStores();

    /**
     * Returns a list of types which can be searched for
     * in this store
     */
    virtual QStringList types() = 0;

    /**
     * Executes the particular query synchronously.
     *
     * \return Returns a integer representating the integer
     */
    virtual int exec(const Query& query) = 0;
    virtual bool next(int queryId) = 0;
    virtual void close(int queryId) = 0;

    virtual QByteArray id(int queryId) = 0;

    virtual QUrl url(int queryId);
    virtual QString text(int queryId);
    virtual QString icon(int queryId);
    virtual QString property(int queryId, const QString& propName);
};


//
// Convenience functions
//
inline QByteArray serialize(const QByteArray& namespace_, int id) {
    return namespace_ + ':' + QByteArray::number(id);
}

inline int deserialize(const QByteArray& namespace_, const QByteArray& str) {
    // The +1 is for the ':'
    return str.mid(namespace_.size() + 1).toInt();
}

}

Q_DECLARE_INTERFACE(Baloo::SearchStore, "org.kde.Baloo.SearchStore")

// BALOO_NO_PLUGINS allows to compile all plugins into a single test binary
// (otherwise the qt_plugin_query_verification_data will be defined multiple times)
#ifndef BALOO_NO_PLUGINS
#define BALOO_EXPORT_SEARCHSTORE( classname, libname )    \
    Q_EXPORT_PLUGIN2(libname, classname)
#else
#define BALOO_EXPORT_SEARCHSTORE( classname, libname )
#endif

#endif // _BALOO_SEARCHSTORE_H
