/* This file is part of the KDE Project
   Copyright (c) 2007-2010 Sebastian Trueg <trueg@kde.org>
   Copyright (c) 2012-2014 Vishesh Handa <me@vhanda.in>

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

#include "kcm.h"
#include "../fileexcludefilters.h"
#include "folderselectionwidget.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KSharedConfig>
#include <KDirWatch>
#include <KDebug>
#include <KStandardDirs>

#include <QPushButton>
#include <QDir>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

K_PLUGIN_FACTORY(BalooConfigModuleFactory, registerPlugin<Baloo::ServerConfigModule>();)
K_EXPORT_PLUGIN(BalooConfigModuleFactory("kcm_baloofile", "kcm_baloofile"))


namespace
{
QStringList defaultFolders()
{
    return QStringList() << QDir::homePath();
}

}

using namespace Baloo;

ServerConfigModule::ServerConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(BalooConfigModuleFactory::componentData(), parent, args)
{
    KAboutData* about = new KAboutData(
        "kcm_baloofile", "kcm_baloofile", ki18n("Configure Desktop Search"),
        KDE_VERSION_STRING, KLocalizedString(), KAboutData::License_GPL,
        ki18n("Copyright 2007-2010 Sebastian Trüg"));
    about->addAuthor(ki18n("Sebastian Trüg"), KLocalizedString(), "trueg@kde.org");
    about->addAuthor(ki18n("Vishesh Handa"), KLocalizedString(), "vhanda@kde.org");
    setAboutData(about);
    setButtons(Help | Apply | Default);

    setupUi(this);

    QPixmap pixmap = KIcon("baloo").pixmap(IconSize(KIconLoader::Desktop));
    m_pixmapLabel->setPixmap(pixmap);

    connect(m_folderSelectionWidget, SIGNAL(changed()),
            this, SLOT(changed()));
    connect(m_folderSelectionWidget, SIGNAL(changed()),
            this, SLOT(folderSelectionChanged()));
    connect(m_enableCheckbox, SIGNAL(stateChanged(int)),
            this, SLOT(changed()));
}


ServerConfigModule::~ServerConfigModule()
{
}


void ServerConfigModule::load()
{
    // File indexer settings
    KConfig config("baloofilerc");
    KConfigGroup group = config.group("General");

    KConfigGroup basicSettings = config.group("Basic Settings");
    m_previouslyEnabled = basicSettings.readEntry("Indexing-Enabled", true);
    m_enableCheckbox->setChecked(m_previouslyEnabled);

    QStringList includeFolders = group.readPathEntry("folders", defaultFolders());
    QStringList excludeFolders = group.readPathEntry("exclude folders", QStringList());
    m_folderSelectionWidget->setFolders(includeFolders, excludeFolders);

    // All values loaded -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::save()
{
    QStringList includeFolders = m_folderSelectionWidget->includeFolders();
    QStringList excludeFolders = m_folderSelectionWidget->excludeFolders();

    // Change the settings
    KConfig config("baloofilerc");
    KConfigGroup basicSettings = config.group("Basic Settings");

    bool mountPointsEx = m_folderSelectionWidget->allMountPointsExcluded();

    bool enabled = m_enableCheckbox->isChecked();
    if (mountPointsEx)
        enabled = false;

    basicSettings.writeEntry("Indexing-Enabled", enabled);

    // 2.2 Update normals paths
    config.group("General").writePathEntry("folders", includeFolders);
    config.group("General").writePathEntry("exclude folders", excludeFolders);

    if (m_previouslyEnabled != enabled) {
        config.group("General").deleteEntry("first run");
    }

    // Start Baloo
    if (enabled) {
        const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file"));
        QProcess::startDetached(exe);
    }
    else {
        QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.kde.baloo.file"),
                                                              QLatin1String("/indexer"),
                                                              QLatin1String("org.kde.baloo.file"),
                                                              QLatin1String("quit"));

        QDBusConnection::sessionBus().asyncCall(message);
    }

    // Start cleaner
    const QString exe = KStandardDirs::findExe(QLatin1String("baloo_file_cleaner"));
    QProcess::startDetached(exe);

    // all values saved -> no changes
    Q_EMIT changed(false);
}


void ServerConfigModule::defaults()
{
    m_folderSelectionWidget->setFolders(defaultFolders(), QStringList());
}

void ServerConfigModule::folderSelectionChanged()
{
    bool disabled = m_folderSelectionWidget->allMountPointsExcluded();
    m_enableCheckbox->setChecked(!disabled);
}

#include "kcm.moc"
