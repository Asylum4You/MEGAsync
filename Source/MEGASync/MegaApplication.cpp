#include "MegaApplication.h"
#include "gui/CrashReportDialog.h"
#include "gui/MegaProxyStyle.h"
#include "control/Utilities.h"
#include "control/CrashHandler.h"
#include "control/ExportProcessor.h"
#include "platform/Platform.h"
#include "qtlockedfile/qtlockedfile.h"

#include <QTranslator>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFontDatabase>
#include <QNetworkProxy>

#ifndef WIN32
//sleep
#include <unistd.h>
#endif

using namespace mega;

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();
QString MegaApplication::dataPath = QString();

int main(int argc, char *argv[])
{
    MegaApplication app(argc, argv);
    app.setStyle(new MegaProxyStyle());

#ifdef Q_OS_MACX
    if ( QSysInfo::MacintoshVersion > QSysInfo::MV_10_8 )
    {
        // fix Mac OS X 10.9 (mavericks) font issue
        // https://bugreports.qt-project.org/browse/QTBUG-32789
        QFont::insertSubstitution(QString::fromUtf8(".Lucida Grande UI"), QString::fromUtf8("Lucida Grande"));
    }

    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QString crashPath = QDir::current().filePath(QString::fromAscii("crashDumps"));
    QString appLockPath = QDir::current().filePath(QString::fromAscii("megasync.lock"));
    QDir crashDir(crashPath);
    if(!crashDir.exists()) crashDir.mkpath(QString::fromAscii("."));

    CrashHandler::instance()->Init(QDir::toNativeSeparators(crashPath));
    if((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        Preferences *preferences = Preferences::instance();
        if(!preferences->error())
        {
            if(preferences->logged())
                preferences->unlink();

            for(int i=0; i<preferences->getNumUsers(); i++)
            {
                preferences->enterUser(i);
                for(int j=0; j<preferences->getNumSyncedFolders(); j++)
                {
                    Platform::syncFolderRemoved(preferences->getLocalFolder(j), preferences->getSyncName(j));
                    Utilities::removeRecursively(preferences->getLocalFolder(j) +
                                                 QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
                }
                preferences->leaveUser();
            }
        }

        QDirIterator di(MegaApplication::applicationDataPath(), QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext()) {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if(fi.fileName().endsWith(QString::fromAscii(".db")) || !fi.fileName().compare(QString::fromUtf8("MEGAsync.cfg")))
                QFile::remove(di.filePath());
        }

        //QDir dataDir(dataPath);
        //Utilities::removeRecursively(dataDir);
        return 0;
    }

    SharedTools::QtLockedFile singleInstanceChecker(appLockPath);
    bool alreadyStarted = true;
    for(int i=0; i<10; i++)
    {
        singleInstanceChecker.open(SharedTools::QtLockedFile::ReadWrite);
        if(singleInstanceChecker.lock(SharedTools::QtLockedFile::WriteLock, false))
        {
            alreadyStarted = false;
            break;
        }

        #ifdef WIN32
            Sleep(1000);
        #else
            sleep(1);
        #endif
    }
    if(alreadyStarted)
    {
        LOG("Already started");
        return 0;
    }

    Platform::initialize(argc, argv);

#ifndef WIN32
#ifndef __APPLE__
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Semibold.ttf"));

    QFont font(QString::fromAscii("Open Sans"), 8);
    app.setFont(font);
#endif
#endif

    //QDate betaLimit(2014, 1, 21);
    //long long now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //long long betaLimitTime = QDateTime(betaLimit).toMSecsSinceEpoch();
    //if(now > betaLimitTime)
    //{
    //    QMessageBox::information(NULL, QCoreApplication::translate("MegaApplication", "MEGAsync BETA"),
    //           QCoreApplication::translate("MegaApplication", "Thank you for testing MEGAsync.<br>"
    //       "This beta version is no longer current and has expired.<br>"
    //       "Please follow <a href=\"https://twitter.com/MEGAprivacy\">@MEGAprivacy</a> on Twitter for updates."));
    //    return 0;
    //}

    app.initialize();
    app.start();
    return app.exec();

#if 0 //Strings for the translation system. These lines don't need to be built
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&No");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Cancel");
    QT_TRANSLATE_NOOP("Installer", "Choose Users");
    QT_TRANSLATE_NOOP("Installer", "Choose for which users you want to install $(^NameDA).");
    QT_TRANSLATE_NOOP("Installer", "Select whether you want to install $(^NameDA) for yourself only or for all users of this computer. $(^ClickNext)");
    QT_TRANSLATE_NOOP("Installer", "Install for anyone using this computer");
    QT_TRANSLATE_NOOP("Installer", "Install just for me");

    QT_TRANSLATE_NOOP("MegaError", "No error");
    QT_TRANSLATE_NOOP("MegaError", "Internal error");
    QT_TRANSLATE_NOOP("MegaError", "Invalid argument");
    QT_TRANSLATE_NOOP("MegaError", "Request failed, retrying");
    QT_TRANSLATE_NOOP("MegaError", "Rate limit exceeded");
    QT_TRANSLATE_NOOP("MegaError", "Failed permanently");
    QT_TRANSLATE_NOOP("MegaError", "Too many concurrent connections or transfers");
    QT_TRANSLATE_NOOP("MegaError", "Out of range");
    QT_TRANSLATE_NOOP("MegaError", "Expired");
    QT_TRANSLATE_NOOP("MegaError", "Not found");
    QT_TRANSLATE_NOOP("MegaError", "Circular linkage detected");
    QT_TRANSLATE_NOOP("MegaError", "Access denied");
    QT_TRANSLATE_NOOP("MegaError", "Already exists");
    QT_TRANSLATE_NOOP("MegaError", "Incomplete");
    QT_TRANSLATE_NOOP("MegaError", "Invalid key/Decryption error");
    QT_TRANSLATE_NOOP("MegaError", "Bad session ID");
    QT_TRANSLATE_NOOP("MegaError", "Blocked");
    QT_TRANSLATE_NOOP("MegaError", "Over quota");
    QT_TRANSLATE_NOOP("MegaError", "Temporarily not available");
    QT_TRANSLATE_NOOP("MegaError", "Connection overflow");
    QT_TRANSLATE_NOOP("MegaError", "Write error");
    QT_TRANSLATE_NOOP("MegaError", "Read error");
    QT_TRANSLATE_NOOP("MegaError", "Invalid application key");
    QT_TRANSLATE_NOOP("MegaError", "Unknown error");
#endif

}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    // set log level

/*TODO: Enable logs

#if DEBUG
    mega::SimpleLogger::setLogLevel(mega::logDebug);
    mega::SimpleLogger::setOutputSettings(mega::logDebug, true, true, true);
#else
    mega::SimpleLogger::setLogLevel(mega::logInfo);
#endif

    // set output to stdout
    mega::SimpleLogger::setAllOutputs(&std::cout);
*/

    //Set QApplication fields
    setOrganizationName(QString::fromAscii("Mega Limited"));
    setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    setApplicationName(QString::fromAscii("MEGAsync"));
    setApplicationVersion(QString::number(Preferences::VERSION_CODE));
    appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    //Set the working directory
#if QT_VERSION < 0x050000
    dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    dataPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

    QDir currentDir(dataPath);
    if(!currentDir.exists()) currentDir.mkpath(QString::fromAscii("."));
    QDir::setCurrent(dataPath);

    finished = false;
    updateAvailable = false;
    lastStartedDownload = 0;
    lastStartedUpload = 0;
    trayIcon = NULL;
    trayMenu = NULL;
    megaApi = NULL;
    httpServer = NULL;
    totalDownloadSize = totalUploadSize = 0;
    totalDownloadedSize = totalUploadedSize = 0;
    uploadSpeed = downloadSpeed = 0;
    exportOps = 0;
    infoDialog = NULL;
    setupWizard = NULL;
    settingsDialog = NULL;
    reboot = false;
    translator = NULL;
    exitAction = NULL;
    aboutAction = NULL;
    settingsAction = NULL;
    importLinksAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    trayMenu = NULL;
    waiting = false;
    updated = false;
    updateAction = NULL;
    showStatusAction = NULL;
    pasteMegaLinksDialog = NULL;
    importDialog = NULL;
    uploadFolderSelector = NULL;
    downloadFolderSelector = NULL;
    updateBlocked = false;
    updateThread = NULL;
    updateTask = NULL;
    multiUploadFileDialog = NULL;
    exitDialog = NULL;
    notificator = NULL;
}

MegaApplication::~MegaApplication()
{

}

void MegaApplication::initialize()
{
    if(megaApi != NULL) return;

    paused = false;
    indexing = false;
    setQuitOnLastWindowClosed(false);

#ifdef Q_OS_LINUX
    isLinux = true;
#else
    isLinux = false;
#endif

    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
    qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    preferences = Preferences::instance();
    if(preferences->error())
        QMessageBox::critical(NULL, QString::fromAscii("MEGAsync"), tr("Your config is corrupt, please start over"));

    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    QString basePath = QDir::toNativeSeparators(QDir::currentPath()+QString::fromAscii("/"));

#ifdef WIN32
    //Backwards compatibility code
    QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
    while (di.hasNext()) {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        if(fi.fileName().startsWith(QString::fromAscii(".tmp.")))
            QFile::remove(di.filePath());
    }
#endif

#ifndef __APPLE__
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT);
#else
    megaApi = new MegaApi(Preferences::CLIENT_KEY, basePath.toUtf8().constData(), Preferences::USER_AGENT, MacXPlatform::fd);
#endif

    delegateListener = new MEGASyncDelegateListener(megaApi, this);
    megaApi->addListener(delegateListener);
    uploader = new MegaUploader(megaApi);
    downloader = new MegaDownloader(megaApi);
    scanningTimer = new QTimer();
    scanningTimer->setSingleShot(false);
    scanningTimer->setInterval(500);
    scanningAnimationIndex = 1;
    connect(scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));

    connect(uploader, SIGNAL(dupplicateUpload(QString, QString, mega::MegaHandle)), this, SLOT(onDupplicateUpload(QString, QString, mega::MegaHandle)));

    if(preferences->isCrashed())
    {
        preferences->setCrashed(false);
        QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if(fi.fileName().endsWith(QString::fromAscii(".db")) ||
               fi.fileName().endsWith(QString::fromAscii(".db-wal")) ||
               fi.fileName().endsWith(QString::fromAscii(".db-shm")))
            {
                QFile::remove(di.filePath());
            }
        }

        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if(reports.size())
        {
            CrashReportDialog crashDialog(reports.join(QString::fromAscii("------------------------------\n")));
            if(crashDialog.exec() == QDialog::Accepted)
            {
                applyProxySettings();
                CrashHandler::instance()->sendPendingCrashReports(crashDialog.getUserMessage());
                #ifndef __APPLE__
                    QMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("Thank you for your collaboration!"));
                #endif
            }
        }
    }

    //Create GUI elements
#ifdef __APPLE__
    trayIcon = new MegaSystemTrayIcon();
#else
    trayIcon = new QSystemTrayIcon();
#endif

    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(onMessageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    initialMenu = new QMenu();
    QString language = preferences->language();
    changeLanguage(language);

#ifdef __APPLE__
    notificator = new Notificator(applicationName(), NULL, NULL);
#else
    notificator = new Notificator(applicationName(), trayIcon, NULL);
#endif
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(changeProxy()));
    initialExitAction = new QAction(tr("Exit"), this);
    connect(initialExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    initialMenu->addAction(changeProxyAction);
    initialMenu->addAction(initialExitAction);

    refreshTimer = new QTimer();
    refreshTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTrayIcon()));

    infoDialogTimer = new QTimer();
    infoDialogTimer->setSingleShot(true);
    connect(infoDialogTimer, SIGNAL(timeout()), this, SLOT(showInfoDialog()));

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));
}

QString MegaApplication::applicationFilePath()
{
    return appPath;
}

QString MegaApplication::applicationDirPath()
{
    return appDirPath;
}

QString MegaApplication::applicationDataPath()
{
    return dataPath;
}

void MegaApplication::changeLanguage(QString languageCode)
{
    if(translator)
    {
        removeTranslator(translator);
        delete translator;
        translator = NULL;
    }

    QTranslator *newTranslator = new QTranslator();
    if(newTranslator->load(Preferences::TRANSLATION_FOLDER + Preferences::TRANSLATION_PREFIX + languageCode))
    {
        installTranslator(newTranslator);
        translator = newTranslator;
    }
    else delete newTranslator;

    if(notificator)
        createTrayIcon();
}

void MegaApplication::updateTrayIcon()
{
    if(!trayIcon) return;
    if(!infoDialog)
    {
        LOG("STATE: Logging in...");
        #ifndef __APPLE__
            #ifdef _WIN32
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/login_ico.ico")));
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_logging.png")));
            #endif
        #else
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_logging_mac.png")));
            if(scanningTimer->isActive())
                scanningTimer->stop();
        #endif
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    }
    else if(paused)
    {
        LOG("STATE: Setting the \"pause\" tray icon. The pause flag is active");
        QString tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Paused");
        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_paused.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_paused_mac.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_paused.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_paused_mac.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else if(indexing || waiting || megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads())
    {
        LOG("STATE: Setting the \"syncing\" tray icon");
        LOG("Reason:");
        if(indexing) LOG("Indexing");
        if(waiting) LOG("Waiting");
        if(megaApi->getNumPendingUploads())
        {
            LOG("Pending uploads");
            LOG(QString::number(megaApi->getNumPendingUploads()));
        }

        if(megaApi->getNumPendingDownloads())
        {
            LOG("Pending downloads");
            LOG(QString::number(megaApi->getNumPendingDownloads()));
        }

        QString tooltip;
        if(indexing) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Scanning");
        else if(waiting) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Waiting");
        else tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Syncing");

        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")));
                if(!scanningTimer->isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer->start();
                }
            #endif
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")));
                if(!scanningTimer->isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer->start();
                }
            #endif
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else
    {
        LOG("STATE: Setting the \"synced\" tray icon (default).");
        if(!updateAvailable)
        {
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_uptodate.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_synced_mac.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Up to date"));
        }
        else
        {
            //TODO: Change icon
            #ifndef __APPLE__
                #ifdef _WIN32
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
                #else
                    trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_uptodate.png")));
                #endif
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_synced_mac.png")));
                if(scanningTimer->isActive())
                    scanningTimer->stop();
            #endif
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Update available!"));
        }

        if(reboot)
            rebootApplication();
    }
}

void MegaApplication::start()
{
    paused = false;
    indexing = false;
    delete multiUploadFileDialog;
    multiUploadFileDialog = NULL;

#ifndef __APPLE__
    #ifdef _WIN32
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/login_ico.ico")));
    #else
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_logging.png")));
    #endif
#else
    trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_logging_mac.png")));
#endif
    trayIcon->setContextMenu(initialMenu);
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    trayIcon->show();
    if(!preferences->lastExecutionTime())
        Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());

    if(updated)
        showInfoMessage(tr("MEGAsync has been updated"));

    applyProxySettings();

    //Start the initial setup wizard if needed
    if(!preferences->logged())
    {
        updated = false;
        setupWizard = new SetupWizard(this);
        setupWizard->setModal(false);
        connect(setupWizard, SIGNAL(finished(int)), this, SLOT(setupWizardFinished()));
        setupWizard->show();
        return;
    }
	else
	{
        QStringList exclusions = preferences->getExcludedSyncNames();
        vector<string> vExclusions;
        for(int i=0; i<exclusions.size(); i++)
            vExclusions.push_back(exclusions[i].toUtf8().constData());
        megaApi->setExcludedNames(&vExclusions);

        //Otherwise, login in the account
        if(preferences->getSession().size())
        {
            megaApi->fastLogin(preferences->getSession().toUtf8().constData());
        }
        else
        {
            megaApi->fastLogin(preferences->email().toUtf8().constData(),
                       preferences->emailHash().toUtf8().constData(),
                       preferences->privatePw().toUtf8().constData());
        }
    }
}

void MegaApplication::loggedIn()
{
    if(infoDialog)
    {
        infoDialog->init();
        return;
    }

    if(settingsDialog)
        settingsDialog->setProxyOnly(false);

    // Apply the "Start on startup" configuration, make sure configuration has the actual value
    // get the requested value
    bool startOnStartup = preferences->startOnStartup();
    // try to enable / disable startup (e.g. copy or delete desktop file)
    if (!Platform::startOnStartup(startOnStartup)) {
        // in case of failure - make sure configuration keeps the right value
        //LOG_debug << "Failed to " << (startOnStartup ? "enable" : "disable") << " MEGASync on startup.";
        preferences->setStartOnStartup(!startOnStartup);
    }

    startUpdateTask();

    QString language = preferences->language();
    changeLanguage(language);

#ifdef WIN32
    if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click here to open the status window."));
    else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click here to open the status window."));
#else
    #ifdef __APPLE__
        if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
        else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click the menu bar icon to open the status window."));
    #else
        if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
        else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click the system tray icon to open the status window."));
    #endif
#endif

    preferences->setLastExecutionTime(QDateTime::currentDateTime().toMSecsSinceEpoch());

    infoDialog = new InfoDialog(this);

    //Set the upload limit
    setUploadLimit(preferences->uploadLimitKB());
    Platform::startShellDispatcher(this);

    //Start the HTTP server
    //httpServer = new HTTPServer(2973, NULL);
    updateUserStats();
}

void MegaApplication::startSyncs()
{
    //Ensure that there is no active syncs
    if(megaApi->getNumActiveSyncs() != 0) stopSyncs();

    //Start syncs
    MegaNode *rubbishNode =  megaApi->getRubbishNode();
	for(int i=0; i<preferences->getNumSyncedFolders(); i++)
	{
        if(!preferences->isFolderActive(i))
            continue;

        MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
        if(!node)
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                "because the remote folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            continue;
        }

        QString localFolder = preferences->getLocalFolder(i);
        if(!QFileInfo(localFolder).isDir())
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                "because the local folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->setSyncState(i, false);
            continue;
        }

        LOG(QString::fromAscii("Sync  %1 added.").arg(i));
        megaApi->syncFolder(localFolder.toUtf8().constData(), node);
        delete node;
	}
    delete rubbishNode;
}

void MegaApplication::stopSyncs()
{
    //Stop syncs
    megaApi->stopSyncs();
}

//This function is called to upload all files in the uploadQueue field
//to the Mega node that is passed as parameter
void MegaApplication::processUploadQueue(mega::MegaHandle nodeHandle)
{
    MegaNode *node = megaApi->getNodeByHandle(nodeHandle);

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if(!node || node->isFile())
	{
		uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        delete node;
        return;
	}

    //Process the upload queue using the MegaUploader object
	while(!uploadQueue.isEmpty())
	{
		QString filePath = uploadQueue.dequeue();
        uploader->upload(filePath, node);
    }
    delete node;
}

void MegaApplication::processDownloadQueue(QString path)
{
    QDir dir(path);
    if(!dir.exists() && !dir.mkpath(QString::fromAscii(".")))
    {
        downloadQueue.clear();
        return;
    }

    while(!downloadQueue.isEmpty())
    {
        mega::MegaHandle nodeHandle = downloadQueue.dequeue();
        MegaNode *node = megaApi->getNodeByHandle(nodeHandle);
        downloader->download(node, path + QDir::separator());
    }
}

void MegaApplication::unityFix()
{
    static QMenu *dummyMenu = NULL;
    if(!dummyMenu)
    {
        dummyMenu = new QMenu();
        connect(this, SIGNAL(unityFixSignal()), dummyMenu, SLOT(close()), Qt::QueuedConnection);
    }

    emit unityFixSignal();
    dummyMenu->exec();
}

void MegaApplication::rebootApplication(bool update)
{
    reboot = true;
    if(update && (megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads() || megaApi->isWaiting()))
    {
        if(!updateBlocked)
        {
            updateBlocked = true;
            showInfoMessage(tr("An update will be applied during the next application restart"));
        }
        return;
    }

    trayIcon->hide();
    if(setupWizard && setupWizard->isVisible())
        setupWizard->hide();
    if(settingsDialog && settingsDialog->isVisible())
        settingsDialog->hide();
    if(infoDialog && infoDialog->isVisible())
        infoDialog->hide();
    if(uploadFolderSelector && uploadFolderSelector->isVisible())
        uploadFolderSelector->hide();
    if(downloadFolderSelector && downloadFolderSelector->isVisible())
        downloadFolderSelector->hide();
    if(multiUploadFileDialog && multiUploadFileDialog->isVisible())
        multiUploadFileDialog->hide();
    if(pasteMegaLinksDialog && pasteMegaLinksDialog->isVisible())
        pasteMegaLinksDialog->hide();
    if(importDialog && importDialog->isVisible())
        importDialog->hide();

#ifdef __APPLE__
    cleanAll();
    ::exit(0);
#endif

    QApplication::exit();
}

void MegaApplication::exitApplication()
{
#ifndef __APPLE__
    if(!megaApi->isLoggedIn())
    {
#endif
        reboot = false;
        trayIcon->hide();
        if(setupWizard && setupWizard->isVisible())
            setupWizard->hide();
        if(settingsDialog && settingsDialog->isVisible())
            settingsDialog->hide();
        if(infoDialog && infoDialog->isVisible())
            infoDialog->hide();
        if(uploadFolderSelector && uploadFolderSelector->isVisible())
            uploadFolderSelector->hide();
        if(downloadFolderSelector && downloadFolderSelector->isVisible())
            downloadFolderSelector->hide();
        if(multiUploadFileDialog && multiUploadFileDialog->isVisible())
            multiUploadFileDialog->hide();
        if(pasteMegaLinksDialog && pasteMegaLinksDialog->isVisible())
            pasteMegaLinksDialog->hide();
        if(importDialog && importDialog->isVisible())
            importDialog->hide();

        #ifdef __APPLE__
            cleanAll();
            ::exit(0);
        #endif

        QApplication::exit();
        return;
#ifndef __APPLE__
    }
#endif

    if(!exitDialog)
    {
        exitDialog = new QMessageBox(QMessageBox::Question, tr("MEGAsync"),
                                     tr("Synchronization will stop.\n\nExit anyway?"), QMessageBox::Yes|QMessageBox::No);
        int button = exitDialog->exec();
        exitDialog->deleteLater();
        exitDialog = NULL;
        if(button == QMessageBox::Yes)
        {
            reboot = false;
            trayIcon->hide();
            if(setupWizard && setupWizard->isVisible())
                setupWizard->hide();
            if(settingsDialog && settingsDialog->isVisible())
                settingsDialog->hide();
            if(infoDialog && infoDialog->isVisible())
                infoDialog->hide();
            if(uploadFolderSelector && uploadFolderSelector->isVisible())
                uploadFolderSelector->hide();
            if(downloadFolderSelector && downloadFolderSelector->isVisible())
                downloadFolderSelector->hide();
            if(multiUploadFileDialog && multiUploadFileDialog->isVisible())
                multiUploadFileDialog->hide();
            if(pasteMegaLinksDialog && pasteMegaLinksDialog->isVisible())
                pasteMegaLinksDialog->hide();
            if(importDialog && importDialog->isVisible())
                importDialog->hide();

            #ifdef __APPLE__
                cleanAll();
                ::exit(0);
            #endif

            QApplication::exit();
        }
    }
    else
    {
        exitDialog->raise();
        exitDialog->activateWindow();
    }
}

void MegaApplication::pauseTransfers(bool pause)
{
    megaApi->pauseTransfers(pause);
}

void MegaApplication::aboutDialog()
{
    QMessageBox::about(NULL, tr("About MEGAsync"), tr("MEGAsync version code %1").arg(this->applicationVersion()));
}

void MegaApplication::refreshTrayIcon()
{
    static int counter = 0;
    if(megaApi)
    {
        LOG("STATE: Refreshing state");
        if(!(++counter % 6))
            megaApi->update();

        megaApi->updateStatics();
        onSyncStateChanged(megaApi);
        if(isLinux) updateTrayIcon();
    }
    if(trayIcon) trayIcon->show();
}

void MegaApplication::cleanAll()
{
    LOG("Cleaning resources");
    finished = true;
    refreshTimer->stop();
    stopSyncs();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        Platform::notifyItemChange(preferences->getLocalFolder(i));

    delete uploader;
    delete pasteMegaLinksDialog;
    delete importDialog;
    delete uploadFolderSelector;
    delete downloadFolderSelector;
    delete delegateListener;
    delete megaApi;

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());
    trayIcon->deleteLater();

    if(reboot)
    {
    #ifndef __APPLE__
        QString app = MegaApplication::applicationFilePath();
        QProcess::startDetached(app);
    #else
        QString app = MegaApplication::applicationDirPath();
        QString launchCommand = QString::fromUtf8("open");
        QStringList args = QStringList();

        QDir appPath(app);
        appPath.cdUp();
        appPath.cdUp();

        args.append(QString::fromAscii("-n"));
        args.append(appPath.absolutePath());
        QProcess::startDetached(launchCommand, args);
    #endif

    #ifdef WIN32
        Sleep(2000);
    #else
        sleep(2);
    #endif
    }

    //QFontDatabase::removeAllApplicationFonts();
}

void MegaApplication::onDupplicateLink(QString, QString name, MegaHandle handle)
{
    addRecentFile(name, handle);
}

void MegaApplication::onDupplicateUpload(QString localPath, QString name, MegaHandle handle)
{
    addRecentFile(name, handle, localPath);
}

void MegaApplication::onInstallUpdateClicked()
{
    showInfoMessage(tr("Installing update..."));
    emit installUpdate();
}

void MegaApplication::showInfoDialog()
{
    if(infoDialog)
    {
        if(!infoDialog->isVisible())
        {
            int posx, posy;
            QPoint position;
            QRect screenGeometry;

            #ifdef __APPLE__
                position = trayIcon->getPosition();
                screenGeometry = QApplication::desktop()->availableGeometry();
            #else
                position = QCursor::pos();
                QDesktopWidget *desktop = QApplication::desktop();
                int screenIndex = desktop->screenNumber(position);
                screenGeometry = desktop->availableGeometry(screenIndex);
            #endif

            #ifdef __APPLE__
                posx = position.x() + trayIcon->geometry().width()/2 - infoDialog->width()/2 - 1;
                posy = screenGeometry.top();
            #else
                #ifdef WIN32
                    QRect totalGeometry = QApplication::desktop()->screenGeometry();
                    if(totalGeometry == screenGeometry)
                    {
                        APPBARDATA pabd;
                        pabd.cbSize = sizeof(APPBARDATA);
                        pabd.hWnd = FindWindow(L"Shell_TrayWnd", NULL);
                        if(pabd.hWnd && SHAppBarMessage(ABM_GETTASKBARPOS, &pabd))
                        {
                            switch (pabd.uEdge)
                            {
                                case ABE_LEFT:
                                    screenGeometry.setLeft(pabd.rc.right+1);
                                    break;
                                case ABE_RIGHT:
                                    screenGeometry.setRight(pabd.rc.left-1);
                                    break;
                                case ABE_TOP:
                                    screenGeometry.setTop(pabd.rc.bottom+1);
                                    break;
                                case ABE_BOTTOM:
                                    screenGeometry.setBottom(pabd.rc.top-1);
                                    break;
                            }
                        }
                    }
                #endif

                if(position.x() > (screenGeometry.right()/2))
                    posx = screenGeometry.right() - infoDialog->width() - 2;
                else
                    posx = screenGeometry.left() + 2;

                if(position.y() > (screenGeometry.bottom()/2))
                    posy = screenGeometry.bottom() - infoDialog->height() - 2;
                else
                    posy = screenGeometry.top() + 2;
            #endif

            if(isLinux) unityFix();

            infoDialog->move(posx, posy);
            infoDialog->show();
            infoDialog->setFocus();
            infoDialog->raise();
            infoDialog->activateWindow();
            infoDialog->updateTransfers();
        }
        else
        {
            infoDialog->closeSyncsMenu();
            if(trayMenu->isVisible())
                trayMenu->close();
            infoDialog->hide();
        }
    }
}

bool MegaApplication::anUpdateIsAvailable()
{
    return updateAvailable;
}

void MegaApplication::triggerInstallUpdate()
{
    emit installUpdate();
}

void MegaApplication::scanningAnimationStep()
{
    scanningAnimationIndex = scanningAnimationIndex%4;
    scanningAnimationIndex++;
    trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac") +
                            QString::number(scanningAnimationIndex) + QString::fromAscii(".png")));
}

void MegaApplication::setupWizardFinished()
{
    if(!preferences->logged())
        ::exit(0);
    setupWizard->deleteLater();
    setupWizard = NULL;

    QStringList exclusions = preferences->getExcludedSyncNames();
    vector<string> vExclusions;
    for(int i=0; i<exclusions.size(); i++)
        vExclusions.push_back(exclusions[i].toUtf8().constData());
    megaApi->setExcludedNames(&vExclusions);

    loggedIn();
    startSyncs();
}

void MegaApplication::unlink()
{
    //Reset fields that will be initialized again upon login
    //delete httpServer;
    //httpServer = NULL;
    stopSyncs();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    megaApi->logout();
}

void MegaApplication::showInfoMessage(QString message, QString title)
{
    if(notificator)
    {
        #ifdef __APPLE__
            if(infoDialog && infoDialog->isVisible()) infoDialog->hide();
        #endif
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                                    QIcon(QString::fromUtf8("://images/app_512.png")));
    }
    else QMessageBox::information(NULL, title, message);
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Warning, title, message,
                                    QIcon(QString::fromUtf8("://images/app_512.png")));
    }
    else QMessageBox::warning(NULL, title, message);
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    if(notificator)
    {
        #ifdef __APPLE__
            if(infoDialog && infoDialog->isVisible()) infoDialog->hide();
        #endif
        notificator->notify(Notificator::Critical, title, message,
        QIcon(QString::fromUtf8("://images/app_512.png")));
    }
    else QMessageBox::critical(NULL, title, message);
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(notificator)
    {
        lastTrayMessage = message;
        notificator->notify(Notificator::Information, title, message,
                                    QIcon(QString::fromUtf8("://images/app_512.png")));
    }
}

//KB/s
void MegaApplication::setUploadLimit(int limit)
{
    if(limit<0) megaApi->setUploadLimit(-1);
    else megaApi->setUploadLimit(limit*1024);
}

void MegaApplication::startUpdateTask()
{

#if defined(WIN32) || defined(__APPLE__)

    if(!updateThread && preferences->canUpdate())
    {
        updateThread = new QThread();
        updateTask = new UpdateTask();
        updateTask->moveToThread(updateThread);

        connect(this, SIGNAL(startUpdaterThread()), updateTask, SLOT(startUpdateThread()), Qt::UniqueConnection);
        connect(this, SIGNAL(tryUpdate()), updateTask, SLOT(checkForUpdates()), Qt::UniqueConnection);
        connect(this, SIGNAL(installUpdate()), updateTask, SLOT(installUpdate()), Qt::UniqueConnection);

        connect(updateTask, SIGNAL(updateCompleted()), this, SLOT(onUpdateCompleted()), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateAvailable(bool)), this, SLOT(onUpdateAvailable(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(installingUpdate(bool)), this, SLOT(onInstallingUpdate(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateNotFound(bool)), this, SLOT(onUpdateNotFound(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateError()), this, SLOT(onUpdateError()), Qt::UniqueConnection);

        connect(updateThread, SIGNAL(finished()), updateTask, SLOT(deleteLater()), Qt::UniqueConnection);
        connect(updateThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()), Qt::UniqueConnection);

        updateThread->start();
        emit startUpdaterThread();
    }
#endif
}

void MegaApplication::stopUpdateTask()
{
    if(updateThread)
    {
        updateThread->quit();
        updateThread = NULL;
        updateTask = NULL;
    }
}

void MegaApplication::applyProxySettings()
{
    QNetworkProxy proxy(QNetworkProxy::NoProxy);
    MegaProxy *proxySettings = new MegaProxy();
    proxySettings->setProxyType(preferences->proxyType());

    if(preferences->proxyType() == MegaProxy::PROXY_CUSTOM)
    {
        QString proxyString = preferences->proxyHostAndPort();
        proxySettings->setProxyURL(proxyString.toUtf8().constData());

        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(preferences->proxyServer());
        proxy.setPort(preferences->proxyPort());
        if(preferences->proxyRequiresAuth())
        {
            QString username = preferences->getProxyUsername();
            QString password = preferences->getProxyPassword();
            proxySettings->setCredentials(username.toUtf8().constData(), password.toUtf8().constData());

            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
    }
    else if(preferences->proxyType() == MegaProxy::PROXY_AUTO)
    {
        MegaProxy* autoProxy = megaApi->getAutoProxySettings();
        delete proxySettings;
        proxySettings = autoProxy;

        if(proxySettings->getProxyType()==MegaProxy::PROXY_CUSTOM)
        {
            string sProxyURL = proxySettings->getProxyURL();
            QString proxyURL = QString::fromUtf8(sProxyURL.data());

            QStringList arguments = proxyURL.split(QString::fromAscii(":"));
            if(arguments.size() == 2)
            {
                proxy.setType(QNetworkProxy::HttpProxy);
                proxy.setHostName(arguments[0]);
                proxy.setPort(arguments[1].toInt());
            }
        }
    }

    megaApi->setProxySettings(proxySettings);
    delete proxySettings;
    QNetworkProxy::setApplicationProxy(proxy);
}

void MegaApplication::showUpdatedMessage()
{
    updated = true;
}

void MegaApplication::updateUserStats()
{
    long long lastRequest = preferences->lastStatsRequest();
    if((QDateTime::currentMSecsSinceEpoch()-lastRequest) > Preferences::MIN_UPDATE_STATS_INTERVAL)
    {
        preferences->setLastStatsRequest(QDateTime::currentMSecsSinceEpoch());
        megaApi->getAccountDetails();
    }
}

void MegaApplication::addRecentFile(QString fileName, long long fileHandle, QString localPath, QString nodeKey)
{
    if(infoDialog)
        infoDialog->addRecentFile(fileName, fileHandle, localPath, nodeKey);
}

void MegaApplication::checkForUpdates()
{
    this->showInfoMessage(tr("Checking for updates..."));
    emit tryUpdate();
}

void MegaApplication::showTrayMenu(QPoint *point)
{
    if(trayMenu)
    {
        if(trayMenu->isVisible())
            trayMenu->close();
        QPoint p = point ? (*point)-QPoint(trayMenu->sizeHint().width(), 0) : QCursor::pos();
        #ifdef __APPLE__
            trayMenu->exec(p);
        #else
            trayMenu->popup(p);
        #endif
    }
}

void MegaApplication::pauseSync()
{
    pauseTransfers(true);
}

void MegaApplication::resumeSync()
{
    pauseTransfers(false);
}

//Called when the "Import links" menu item is clicked
void MegaApplication::importLinks()
{
    if(pasteMegaLinksDialog)
    {
        pasteMegaLinksDialog->setVisible(true);
        pasteMegaLinksDialog->activateWindow();
        pasteMegaLinksDialog->raise();
        pasteMegaLinksDialog->setFocus();
        return;
    }

    if(importDialog)
    {
        importDialog->setVisible(true);
        importDialog->activateWindow();
        importDialog->raise();
        importDialog->setFocus();
        return;
    }

    //Show the dialog to paste public links
    pasteMegaLinksDialog = new PasteMegaLinksDialog();
    pasteMegaLinksDialog->exec();

    //If the dialog isn't accepted, return
    if(pasteMegaLinksDialog->result()!=QDialog::Accepted)
    {
        delete pasteMegaLinksDialog;
        pasteMegaLinksDialog = NULL;
        return;
    }

    //Get the list of links from the dialog
    QStringList linkList = pasteMegaLinksDialog->getLinks();
    delete pasteMegaLinksDialog;
    pasteMegaLinksDialog = NULL;

    //Send links to the link processor
	LinkProcessor *linkProcessor = new LinkProcessor(megaApi, linkList);

    //Open the import dialog
    importDialog = new ImportMegaLinksDialog(megaApi, preferences, linkProcessor);
    importDialog->exec();
    if(importDialog->result()!=QDialog::Accepted)
    {
        delete importDialog;
        importDialog = NULL;
        return;
    }

    //If the user wants to download some links, do it
    if(importDialog->shouldDownload())
	{
        preferences->setDownloadFolder(importDialog->getDownloadPath());
        linkProcessor->downloadLinks(importDialog->getDownloadPath());
	}

    //If the user wants to import some links, do it
    if(importDialog->shouldImport())
	{
		connect(linkProcessor, SIGNAL(onLinkImportFinish()), this, SLOT(onLinkImportFinished()));
        connect(linkProcessor, SIGNAL(onDupplicateLink(QString, QString, long long)),
                this, SLOT(onDupplicateLink(QString, QString, long long)));
        linkProcessor->importLinks(importDialog->getImportPath());
	}
    //If importing links isn't needed, we can delete the link processor
    //It doesn't track transfers, only the importation of links
    else delete linkProcessor;

    delete importDialog;
    importDialog = NULL;
}

void MegaApplication::uploadActionClicked()
{
    #ifdef __APPLE__
                infoDialog->hide();
                QApplication::processEvents();
                QStringList files = MacXPlatform::multipleUpload(QCoreApplication::translate("ShellExtension", "Upload to MEGA"));
                if(files.size())
                {
                    QQueue<QString> qFiles;
                    foreach(QString file, files)
                    {
                        qFiles.append(file);
                    }

                    shellUpload(qFiles);
                }
         return;
    #endif

    if(multiUploadFileDialog)
    {
        #ifdef WIN32
            multiUploadFileDialog->showMinimized();
            multiUploadFileDialog->setWindowState(Qt::WindowActive);
            multiUploadFileDialog->showNormal();
        #endif

        multiUploadFileDialog->raise();
        multiUploadFileDialog->activateWindow();
        return;
    }

#if QT_VERSION < 0x050000
    QString defaultFolderPath = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
#else
    QString defaultFolderPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
#endif

    multiUploadFileDialog = new MultiQFileDialog(NULL,
           QCoreApplication::translate("ShellExtension", "Upload to MEGA"),
           defaultFolderPath);

    if(multiUploadFileDialog->exec() == QDialog::Accepted)
    {
        QStringList files = multiUploadFileDialog->selectedFiles();
        if(files.size())
        {
            QQueue<QString> qFiles;
            foreach(QString file, files)
                qFiles.append(file);
            shellUpload(qFiles);
        }
    }

    delete multiUploadFileDialog;
    multiUploadFileDialog = NULL;
}

void MegaApplication::downloadActionClicked()
{
    if(finished) return;

    NodeSelector *nodeSelector = new NodeSelector(megaApi, true, true, 0,true);
    nodeSelector->nodesReady();
    int result = nodeSelector->exec();
    if(result != QDialog::Accepted)
    {
        delete nodeSelector;
        return;
    }

    long long selectedMegaFolderHandle = nodeSelector->getSelectedFolderHandle();
    MegaNode *selectedFolder = megaApi->getNodeByHandle(selectedMegaFolderHandle);
    if(!selectedFolder)
    {
        selectedMegaFolderHandle = mega::INVALID_HANDLE;
        delete nodeSelector;
        return;
    }

    downloadQueue.append(selectedMegaFolderHandle);
    if(downloadFolderSelector)
    {
        downloadFolderSelector->setVisible(true);
        #ifdef WIN32
            downloadFolderSelector->showMinimized();
            downloadFolderSelector->setWindowState(Qt::WindowActive);
            downloadFolderSelector->showNormal();
        #endif
        downloadFolderSelector->raise();
        downloadFolderSelector->activateWindow();
        downloadFolderSelector->setFocus();
        return;
    }

    QString defaultPath = preferences->downloadFolder();
    if(QFile(defaultPath).exists())
    {
        processDownloadQueue(defaultPath);
        return;
    }

    downloadFolderSelector = new DownloadFromMegaDialog();
    #ifdef WIN32
        downloadFolderSelector->showMinimized();
        downloadFolderSelector->setWindowState(Qt::WindowActive);
        downloadFolderSelector->showNormal();
    #endif
    downloadFolderSelector->raise();
    downloadFolderSelector->activateWindow();
    downloadFolderSelector->setFocus();
    downloadFolderSelector->exec();
    if(downloadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        QString path = downloadFolderSelector->getPath();
        if(downloadFolderSelector->isDefaultFolder())
            preferences->setDownloadFolder(path);
        processDownloadQueue(path);
    }
    //If the dialog is rejected, cancel uploads
    else downloadQueue.clear();

    delete downloadFolderSelector;
    downloadFolderSelector = NULL;
    return;
}

//Called when the user wants to generate the public link for a node
void MegaApplication::copyFileLink(MegaHandle fileHandle, QString nodeKey)
{
    if(nodeKey.size())
    {
        //Public node
        const char* base64Handle = MegaApi::handleToBase64(fileHandle);
        QString handle = QString::fromUtf8(base64Handle);
        QString linkForClipboard = QString::fromUtf8("https://mega.co.nz/#!%1!%2").arg(handle).arg(nodeKey);
        delete [] base64Handle;
        QApplication::clipboard()->setText(linkForClipboard);
        showInfoMessage(tr("The link has been copied to the clipboard"));
        return;
    }

    //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
    if(infoDialog) infoDialog->disableGetLink(true);
	megaApi->exportNode(megaApi->getNodeByHandle(fileHandle));
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{
    if(finished) return;

    //Append the list of files to the upload queue
	uploadQueue.append(newUploadQueue);

    //If the dialog to select the upload folder is active, return.
    //Files will be uploaded when the user selects the upload folder
    if(uploadFolderSelector)
    {
        uploadFolderSelector->setVisible(true);
        #ifdef WIN32
            uploadFolderSelector->showMinimized();
            uploadFolderSelector->setWindowState(Qt::WindowActive);
            uploadFolderSelector->showNormal();
        #endif
        uploadFolderSelector->raise();
        uploadFolderSelector->activateWindow();
        uploadFolderSelector->setFocus();
        return;
    }

    //If there is a default upload folder in the preferences
    MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
	if(node)
	{
        //use it to upload the list of files
        processUploadQueue(node->getHandle());
        delete node;
		return;
	}

    uploadFolderSelector = new UploadToMegaDialog(megaApi);
    #ifdef WIN32
        uploadFolderSelector->showMinimized();
        uploadFolderSelector->setWindowState(Qt::WindowActive);
        uploadFolderSelector->showNormal();
    #endif
    uploadFolderSelector->raise();
    uploadFolderSelector->activateWindow();
    uploadFolderSelector->setFocus();
    uploadFolderSelector->exec();
    if(uploadFolderSelector->result()==QDialog::Accepted)
    {
        //If the dialog is accepted, get the destination node
        MegaHandle nodeHandle = uploadFolderSelector->getSelectedHandle();
        if(uploadFolderSelector->isDefaultFolder())
            preferences->setUploadFolder(nodeHandle);
        processUploadQueue(nodeHandle);
    }
    //If the dialog is rejected, cancel uploads
    else uploadQueue.clear();

    delete uploadFolderSelector;
    uploadFolderSelector = NULL;
    return;
}

void MegaApplication::shellExport(QQueue<QString> newExportQueue)
{
    if(finished) return;

    ExportProcessor *processor = new ExportProcessor(megaApi, newExportQueue);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

//Called when the link import finishes
void MegaApplication::onLinkImportFinished()
{
	LinkProcessor *linkProcessor = ((LinkProcessor *)QObject::sender());
	preferences->setImportFolder(linkProcessor->getImportParentFolder());
    linkProcessor->deleteLater();
}

void MegaApplication::onRequestLinksFinished()
{
    ExportProcessor *exportProcessor = ((ExportProcessor *)QObject::sender());
    QStringList links = exportProcessor->getValidLinks();
    if(!links.size()) return;
    QString linkForClipboard(links.join(QChar::fromAscii('\n')));
    QApplication::clipboard()->setText(linkForClipboard);
    if(links.size()==1) showInfoMessage(tr("The link has been copied to the clipboard"));
    else showInfoMessage(tr("The links have been copied to the clipboard"));
    exportProcessor->deleteLater();
    exportOps--;
}

void MegaApplication::onUpdateCompleted()
{
    LOG("Update completed. Initializing a silent reboot...");
    if(trayMenu)
    {
        updateAction->setText(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING);
        updateAction->setEnabled(false);
    }
    updateAvailable = false;
    rebootApplication();
}

void MegaApplication::onUpdateAvailable(bool requested)
{
    if(infoDialog)
    {
        updateAvailable = true;
        updateAction->setText(tr("Install update"));
        updateAction->setEnabled(true);
    }

    if(settingsDialog)
        settingsDialog->setUpdateAvailable(true);

    if(requested)
    {
#ifdef WIN32
        showInfoMessage(tr("A new version of MEGAsync is available! Click on this message to install it"));
#else
        showInfoMessage(tr("A new version of MEGAsync is available!"));
#endif
    }
}

void MegaApplication::onInstallingUpdate(bool requested)
{
    if(requested)
        showInfoMessage(tr("Update available. Downloading..."));
}

void MegaApplication::onUpdateNotFound(bool requested)
{
    if(requested)
    {
        if(!updateAvailable)
            showInfoMessage(tr("No update available at this time"));
        else
            showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync"));
    }
}

void MegaApplication::onUpdateError()
{
    showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync"));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    LOG("Tray icon clicked");
    megaApi->retryPendingConnections();

    if(reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Context)
    {
        LOG("Event QSystemTrayIcon::Trigger");

        if(!infoDialog)
        {
            LOG("NULL information dialog");
            if(setupWizard)
            {
                LOG("Showing setup wizard");
                setupWizard->setVisible(true);
                setupWizard->raise();
                setupWizard->activateWindow();
            }
            else if(reason == QSystemTrayIcon::Trigger)
                showInfoMessage(tr("Logging in..."));

            return;
        }

        LOG("Information dialog available");
#ifndef __APPLE__
        infoDialogTimer->start(200);
#else
        showInfoDialog();
#endif
    }
#ifndef __APPLE__
    else if(reason == QSystemTrayIcon::DoubleClick)
    {
        LOG("Event QSystemTrayIcon::DoubleClick");

        if(!infoDialog)
        {
            LOG("NULL information dialog");
            if(setupWizard)
            {
                LOG("Showing setup wizard");
                setupWizard->setVisible(true);
                setupWizard->raise();
                setupWizard->activateWindow();
            }
            else showInfoMessage(tr("Logging in..."));

            return;
        }

        int i;
        for(i = 0; i < preferences->getNumSyncedFolders(); i++)
        {
            if(preferences->isFolderActive(i))
                break;
        }
        if(i == preferences->getNumSyncedFolders())
            return;

        infoDialogTimer->stop();
        infoDialog->hide();
        QString localFolderPath = preferences->getLocalFolder(i);
        if(!localFolderPath.isEmpty())
            QDesktopServices::openUrl(QUrl::fromLocalFile(localFolderPath));
    }
    else if(reason == QSystemTrayIcon::MiddleClick)
    {
        showTrayMenu();
    }
#endif
}

void MegaApplication::onMessageClicked()
{
    LOG("onMessageClicked");
    if(lastTrayMessage == tr("A new version of MEGAsync is available! Click on this message to install it"))
        triggerInstallUpdate();
    else
        trayIconActivated(QSystemTrayIcon::Trigger);
}

//Called when the user wants to open the settings dialog
void MegaApplication::openSettings()
{
    if(settingsDialog)
    {
        //If the dialog is active
		if(settingsDialog->isVisible())
		{
            //and visible -> show it
            settingsDialog->raise();
			settingsDialog->activateWindow();
			return;
		}

        //Otherwise, delete it
        delete settingsDialog;
	}

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this);
    settingsDialog->setUpdateAvailable(updateAvailable);
    settingsDialog->setModal(false);
    settingsDialog->show();
}

void MegaApplication::changeProxy()
{
    if(settingsDialog)
    {
        settingsDialog->setProxyOnly(true);

        //If the dialog is active
        if(settingsDialog->isVisible())
        {
            //and visible -> show it
            settingsDialog->raise();
            settingsDialog->activateWindow();
            return;
        }

        //Otherwise, delete it
        delete settingsDialog;
    }

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this, true);
    settingsDialog->setModal(false);
    settingsDialog->show();
}

//This function creates the tray icon
void MegaApplication::createTrayIcon()
{
    if(!trayMenu)
    {
        trayMenu = new QMenu();
        #ifndef __APPLE__
            trayMenu->setStyleSheet(QString::fromAscii(
                    "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
                    "QMenu::item {background-color: white; color: black;} "
                    "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
        #endif
    }
    else
    {
        QList<QAction *> actions = trayMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            trayMenu->removeAction(actions[i]);
        }
    }

    if(exitAction) delete exitAction;
#ifndef __APPLE__
    exitAction = new QAction(tr("Exit"), this);
#else
    exitAction = new QAction(tr("Quit"), this);
#endif
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if(aboutAction) delete aboutAction;
    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    if(settingsAction) delete settingsAction;
#ifndef __APPLE__
    settingsAction = new QAction(tr("Settings"), this);
#else
    settingsAction = new QAction(tr("Preferences"), this);
#endif
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    if(importLinksAction) delete importLinksAction;
    importLinksAction = new QAction(tr("Import links"), this);
    connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));

    if(uploadAction) delete uploadAction;
    uploadAction = new QAction(tr("Upload to MEGA"), this);
    connect(uploadAction, SIGNAL(triggered()), this, SLOT(uploadActionClicked()));

    if(downloadAction) delete downloadAction;
    downloadAction = new QAction(tr("Download from MEGA"), this);
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(downloadActionClicked()));

    if(updateAction) delete updateAction;
    if(updateAvailable)
    {
        updateAction = new QAction(tr("Install update"), this);
        updateAction->setEnabled(true);
    }
    else
    {
        updateAction = new QAction(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING, this);
#ifndef __APPLE__
        updateAction->setIcon(QIcon(QString::fromAscii("://images/check_mega_version.png")));
        updateAction->setIconVisibleInMenu(true);
#endif
        updateAction->setEnabled(false);
    }
    connect(updateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));

    trayMenu->addAction(updateAction);
    trayMenu->addSeparator();
	trayMenu->addAction(importLinksAction);
    trayMenu->addAction(uploadAction);
    trayMenu->addAction(downloadAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAction);

    if (isLinux)
    {
        if(showStatusAction) delete showStatusAction;
        showStatusAction = new QAction(tr("Show status"), this);
        connect(showStatusAction, SIGNAL(triggered()), this, SLOT(showInfoDialog()));

        trayMenu->insertAction(importLinksAction, showStatusAction);
        trayIcon->setContextMenu(trayMenu);
    }
    else
    {
        #if defined(Q_OS_LINUX) && !defined(Q_OS_MAC)
            trayIcon->setContextMenu(NULL);
        #else
            trayIcon->setContextMenu(&emptyMenu);
        #endif

        #ifndef __APPLE__
            #ifdef _WIN32
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
            #else
                trayIcon->setIcon(QIcon(QString::fromAscii("://images/22_synching.png")));
            #endif
        #else
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/icon_syncing_mac.png")));
            if(!scanningTimer->isActive())
            {
                scanningAnimationIndex = 1;
                scanningTimer->start();
            }
        #endif
    }
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + Preferences::VERSION_STRING + QString::fromAscii("\n") + tr("Starting"));
}

//Called when a request is about to start
void MegaApplication::onRequestStart(MegaApi* , MegaRequest *)
{

}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi*, MegaRequest *request, MegaError* e)
{
    switch (request->getType()) {
	case MegaRequest::TYPE_EXPORT:
	{
        if(!exportOps && e->getErrorCode() == MegaError::API_OK)
		{
            //A public link has been created, put it in the clipboard and inform users
            QString linkForClipboard(QString::fromUtf8(request->getLink()));
            QApplication::clipboard()->setText(linkForClipboard);
            showInfoMessage(tr("The link has been copied to the clipboard"));
		}

        if(e->getErrorCode() != MegaError::API_OK)
            showErrorMessage(tr("Error getting link: ") + QCoreApplication::translate("MegaError", e->getErrorString()));

        if(infoDialog) infoDialog->disableGetLink(false);
		break;
	}
	case MegaRequest::TYPE_LOGIN:
    case MegaRequest::TYPE_FAST_LOGIN:
	{
        //This prevents to handle logins in the initial setup wizard
        if(preferences->logged())
		{
            if(e->getErrorCode() == MegaError::API_OK)
			{
                const char *session = megaApi->dumpSession();
                if(session)
                {
                    QString sessionKey = QString::fromUtf8(session);
                    preferences->setSession(sessionKey);
                    delete session;

                    //Successful login, fetch nodes
                    megaApi->fetchNodes();
                    break;
                }
			}

            //Wrong login -> logout
            unlink();
		}
		break;
	}
    case MegaRequest::TYPE_LOGOUT:
    {
        if(e->getErrorCode())
            showErrorMessage(tr("Error") + QString::fromAscii(": ") + QCoreApplication::translate("MegaError", e->getErrorString()));

        if(preferences && preferences->logged())
        {
            preferences->unlink();
            if(infoDialog) delete infoDialog;
            infoDialog = NULL;
            start();
        }
        break;
    }
	case MegaRequest::TYPE_FETCH_NODES:
	{
        //This prevents to handle node requests in the initial setup wizard
        if(preferences->logged())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
                MegaNode *rootNode = megaApi->getRootNode();
                if(rootNode)
                {
                    delete rootNode;

                    //If we have got the filesystem, start the app
                    Preferences *preferences = Preferences::instance();
                    if(preferences->logged() && preferences->wasPaused())
                        pauseTransfers(true);

                    loggedIn();
                }
                else
                {
                    QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                               "Please, try again. If the problem persists "
                                                               "please contact bug@mega.co.nz"), QMessageBox::Ok);
                    preferences->setCrashed(true);
                    preferences->unlink();
                    rebootApplication(false);
                }
			}
            else
            {
                LOG("Error fetching nodes");
                QMessageBox::warning(NULL, tr("Error"), QCoreApplication::translate("MegaError", e->getErrorString()), QMessageBox::Ok);
                unlink();
            }
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        if(!preferences->logged()) break;

		if(e->getErrorCode() != MegaError::API_OK)
			break;

        //Account details retrieved, update the preferences and the information dialog
        MegaAccountDetails *details = request->getMegaAccountDetails();
        preferences->setAccountType(details->getProLevel());
        preferences->setTotalStorage(details->getStorageMax());
        preferences->setUsedStorage(details->getStorageUsed());
        preferences->setTotalBandwidth(details->getTransferMax());
        preferences->setUsedBandwidth(details->getTransferOwnUsed());

        MegaNode *root = megaApi->getRootNode();
        MegaHandle rootHandle = root->getHandle();
        preferences->setCloudDriveStorage(details->getStorageUsed(rootHandle));
        preferences->setCloudDriveFiles(details->getNumFiles(rootHandle));
        preferences->setCloudDriveFolders(details->getNumFolders(rootHandle));
        delete root;

        MegaNode *inbox = megaApi->getInboxNode();
        MegaHandle inboxHandle = inbox->getHandle();
        preferences->setInboxStorage(details->getStorageUsed(inboxHandle));
        preferences->setInboxFiles(details->getNumFiles(inboxHandle));
        preferences->setInboxFolders(details->getNumFolders(inboxHandle));
        delete inbox;

        MegaNode *rubbish = megaApi->getRubbishNode();
        MegaHandle rubbishHandle = rubbish->getHandle();
        preferences->setRubbishStorage(details->getStorageUsed(rubbishHandle));
        preferences->setRubbishFiles(details->getNumFiles(rubbishHandle));
        preferences->setRubbishFolders(details->getNumFolders(rubbishHandle));
        delete rubbish;

        preferences->sync();

        if(infoDialog) infoDialog->setUsage(details->getStorageMax(), details->getStorageUsed());
        if(settingsDialog) settingsDialog->refreshAccountDetails();
        delete details;
        break;
    }
    case MegaRequest::TYPE_PAUSE_TRANSFERS:
    {
        paused = request->getFlag();
        preferences->setWasPaused(paused);
        onSyncStateChanged(megaApi);
        break;
    }
    case MegaRequest::TYPE_ADD_SYNC:
    {
        LOG("Sync added!");
        for(int i=preferences->getNumSyncedFolders()-1; i>=0; i--)
        {
            if((request->getNodeHandle() == preferences->getMegaFolderHandle(i)))
            {
                if(e->getErrorCode() != MegaError::API_OK)
                {
                    QString localFolder = preferences->getLocalFolder(i);
                    MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                    const char *nodePath = megaApi->getNodePath(node);
                    delete node;

                    if(!QFileInfo(localFolder).isDir())
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                            "because the local folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                                "because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                                "because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else if(e->getErrorCode() == MegaError::API_EFAILED)
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                            "because the local folder has changed")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else showErrorMessage(QCoreApplication::translate("MegaError", e->getErrorString()));

                    delete[] nodePath;

                    LOG("Sync error! Removed");
                    Platform::syncFolderRemoved(localFolder, preferences->getSyncName(i));
                    preferences->setSyncState(i, false);
                    if(settingsDialog)
                        settingsDialog->loadSettings();
                }
                else
                {
                    preferences->setLocalFingerprint(i, request->getParentHandle());
                }
                break;
            }
        }
        if(infoDialog)
        {
            LOG("Sync commited! Updating GUI");
            infoDialog->updateSyncsButton();
        }
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        if(e->getErrorCode() == MegaError::API_OK)
        {
            QString syncPath = QString::fromUtf8(request->getFile());

            #ifdef WIN32
            if(syncPath.startsWith(QString::fromAscii("\\\\?\\")))
                syncPath = syncPath.mid(4);
            #endif

            Utilities::removeRecursively(syncPath + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
            Platform::notifyItemChange(syncPath);
        }
        if(infoDialog) infoDialog->updateSyncsButton();
        if(settingsDialog) settingsDialog->loadSettings();
        onSyncStateChanged(megaApi);
        break;
    }
    default:
        break;
    }
}

//Called when a transfer is about to start
void MegaApplication::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if(finished) return;

    if(infoDialog && !totalUploadSize && !totalDownloadSize)
    {
        infoDialog->setWaiting(true);
        onSyncStateChanged(megaApi);
    }

    //Update statics
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
		downloadSpeed = 0;
		totalDownloadSize += transfer->getTotalBytes();
	}
	else
	{
		uploadSpeed = 0;
		totalUploadSize += transfer->getTotalBytes();
	}

    //Send statics to the information dialog
    if(infoDialog)
    {
        infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);
        infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
        infoDialog->updateTransfers();
    }
}

//Called when there is a temporal problem in a request
void MegaApplication::onRequestTemporaryError(MegaApi *, MegaRequest *, MegaError* )
{
}

//Called when a transfer has finished
void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{
    if(finished) return;

	//Update statics
	if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
	{
		totalDownloadedSize += transfer->getDeltaSize();
		downloadSpeed = transfer->getSpeed();

        //Show the transfer in the "recently updated" list
        if(e->getErrorCode() == MegaError::API_OK)
        {
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif

            MegaNode *node = transfer->getPublicNode();
            QString publicKey;
            if(node)
            {
                const char* key = node->getBase64Key();
                publicKey = QString::fromUtf8(key);
                delete [] key;
            }
            addRecentFile(QString::fromUtf8(transfer->getFileName()), transfer->getNodeHandle(), localPath, publicKey);
        }
	}
	else
	{
		totalUploadedSize += transfer->getDeltaSize();
		uploadSpeed = transfer->getSpeed();

        //Here the file isn't added to the "recently updated" list,
        //because the file isn't in the destination folder yet.
        //The SDK still has to put the new node.
        //onNodes update will be called with node->tag == transfer->getTag()
        //so we save the path of the file to show it later
        if((e->getErrorCode() == MegaError::API_OK) && (!transfer->isSyncTransfer()))
        {
            LOG(QString::fromAscii("Putting: %1 TAG: %2").arg(QString::fromUtf8(transfer->getPath())).arg(transfer->getTag()));
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif
            uploadLocalPaths[transfer->getTag()]=localPath;
        }
	}

    //Send updated statics to the information dialog
    if(infoDialog)
    {
        if(e->getErrorCode() == MegaError::API_OK)
        {
            if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
                ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
                infoDialog->setTransfer(transfer);

            infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
            infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
        }

        infoDialog->updateTransfers();
        infoDialog->transferFinished(e->getErrorCode());
    }

    if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
    {
        if(lastStartedDownload == transfer->getStartTime())
            lastStartedDownload = 0;
    }
    else
    {
        if(lastStartedUpload == transfer->getStartTime())
            lastStartedUpload = 0;

        if((e->getErrorCode() == MegaError::API_OK))
        {
            preferences->setUsedStorage(preferences->usedStorage()+transfer->getTotalBytes());
            preferences->setCloudDriveStorage(preferences->cloudDriveStorage() + transfer->getTotalBytes());
            preferences->setCloudDriveFiles(preferences->cloudDriveFiles()+1);
            if(settingsDialog) settingsDialog->refreshAccountDetails();

            if(infoDialog) infoDialog->increaseUsedStorage(transfer->getTotalBytes());
        }
    }

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if(!megaApi->getNumPendingDownloads() && !megaApi->getNumPendingUploads())
    {
        if(totalUploadSize || totalDownloadSize)
            onSyncStateChanged(megaApi);

		totalUploadSize = totalDownloadSize = 0;
		totalUploadedSize = totalDownloadedSize = 0;
        uploadSpeed = downloadSpeed = 0;
	}
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if(finished) return;

    //Update statics
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
        downloadSpeed = transfer->getSpeed();
        if(!lastStartedDownload || !transfer->getTransferredBytes())
            lastStartedDownload = transfer->getStartTime();
		totalDownloadedSize += transfer->getDeltaSize();
	}
	else
	{
        uploadSpeed = transfer->getSpeed();
        if(!lastStartedUpload || !transfer->getTransferredBytes())
            lastStartedUpload = transfer->getStartTime();
		totalUploadedSize += transfer->getDeltaSize();
	}

    //Send updated statics to the information dialog
    if(infoDialog)
    {
        if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
            ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
            infoDialog->setTransfer(transfer);

        infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
        infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
        infoDialog->updateTransfers();
    }
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{
    //Show information to users
    if(transfer->getNumRetry() == 1)
        showWarningMessage(tr("Temporary transmission error: ") + QCoreApplication::translate("MegaError", e->getErrorString()), QString::fromUtf8(transfer->getFileName()));
    else
        onSyncStateChanged(megaApi);
}

//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi* , UserList *)
{

}

//Called when nodes have been updated in MEGA
void MegaApplication::onNodesUpdate(MegaApi* , NodeList *nodes)
{
    if(!infoDialog) return;

    bool externalNodes = 0;

    //If this is a full reload, return
	if(!nodes) return;
    LOG(QString::fromAscii("onNodesUpdate ") + QString::number(nodes->size()));

    //Check all modified nodes
    QString localPath;
    for(int i=0; i<nodes->size(); i++)
	{
        localPath.clear();
        MegaNode *node = nodes->get(i);

        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            if(node->getType()==MegaNode::TYPE_FOLDER && (node->getHandle() == preferences->getMegaFolderHandle(i)))
            {
                MegaNode *nodeByHandle = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                const char *nodePath = megaApi->getNodePath(nodeByHandle);

                if(!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
                {
                    if(nodePath && QString::fromUtf8(nodePath).startsWith(QString::fromUtf8("//bin")))
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                            "because the remote folder is in the rubbish bin")
                                         .arg(preferences->getSyncName(i)));
                    }
                    else
                    {
                        showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                            "because the remote folder doesn't exist")
                                         .arg(preferences->getSyncName(i)));
                    }
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
                    Utilities::removeRecursively(preferences->getLocalFolder(i) + QDir::separator() + QString::fromAscii(MEGA_DEBRIS_FOLDER));
                    Platform::notifyItemChange(preferences->getLocalFolder(i));
                    megaApi->removeSync(preferences->getMegaFolderHandle(i));
                    preferences->setSyncState(i, false);
                }

                delete nodeByHandle;
                delete [] nodePath;
            }
        }

        if(!node->getTag() && !node->isRemoved() && !node->isSyncDeleted() && ((lastExit/1000)<node->getCreationTime()))
            externalNodes++;

        if(!node->isRemoved() && node->getTag() && !node->isSyncDeleted() && (node->getType()==MegaNode::TYPE_FILE))
        {
            //Get the associated local node
            LOG(QString::fromAscii("Node: %1 TAG: %2").arg(QString::fromUtf8(node->getName())).arg(node->getTag()));
            string path = node->getLocalPath();
            if(path.size())
            {
                //If the node has been uploaded by a synced folder
                //The SDK provides its local path
                LOG("Sync upload");
            #ifdef WIN32
                localPath = QString::fromWCharArray((const wchar_t *)path.data());
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #else
                localPath = QString::fromUtf8(path.data());
            #endif
                LOG(QString::fromAscii("Sync path: %1").arg(localPath));
            }
            else if(uploadLocalPaths.contains(node->getTag()))
            {
                //If the node has been uploaded by a regular upload,
                //we recover the path using the tag of the transfer
                localPath = uploadLocalPaths.value(node->getTag());
                uploadLocalPaths.remove(node->getTag());
                LOG(QString::fromAscii("Local upload: %1").arg(localPath));
            }

            addRecentFile(QString::fromUtf8(node->getName()), node->getHandle(), localPath);
        }
	}

    if(externalNodes)
    {
        updateUserStats();
        showNotificationMessage(tr("You have new or updated files in your account"));
    }
}

void MegaApplication::onReloadNeeded(MegaApi*)
{
    //Don't reload the filesystem here because it's unsafe
    //and the most probable cause for this callback is a false positive.
    //Simply set the crashed flag to force a filesystem reload in the next execution.
    preferences->setCrashed(true);
    //megaApi->fetchNodes();
}

void MegaApplication::onSyncStateChanged(MegaApi *)
{
    if(megaApi)
    {
        indexing = megaApi->isIndexing();
        waiting = megaApi->isWaiting();
    }

    if(infoDialog)
    {
        infoDialog->setIndexing(indexing);
        infoDialog->setWaiting(waiting);
        infoDialog->setPaused(paused);
        infoDialog->updateState();
        infoDialog->transferFinished(MegaError::API_OK);
        infoDialog->updateRecentFiles();
    }

    LOG("Current state: ");
    if(paused) LOG("Paused = true");
    else LOG("Paused = false");
    if(indexing) LOG("Indexing = true");
    else LOG("Indexing = false");
    if(waiting) LOG("Waiting = true");
    else LOG("Waiting = false");

    if(!isLinux) updateTrayIcon();
}

void MegaApplication::onSyncFileStateChanged(MegaApi *, const char *filePath, int)
{
    QString localPath = QString::fromUtf8(filePath);
    Platform::notifyItemChange(localPath);
}

MEGASyncDelegateListener::MEGASyncDelegateListener(MegaApi *megaApi, MegaListener *parent)
    : QTMegaListener(megaApi, parent)
{ }

void MEGASyncDelegateListener::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    QTMegaListener::onRequestFinish(api, request, e);

    if(request->getType() != MegaRequest::TYPE_FETCH_NODES || e->getErrorCode() != MegaError::API_OK)
        return;

    Preferences *preferences = Preferences::instance();
    if(preferences->logged() && !api->getNumActiveSyncs())
    {
        //Start syncs
        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            QString tmpPath = preferences->getLocalFolder(i) + QDir::separator() + QString::fromUtf8(mega::MEGA_DEBRIS_FOLDER) + QString::fromUtf8("/tmp");
            QDirIterator di(tmpPath, QDir::Files | QDir::NoDotAndDotDot);
            while (di.hasNext())
            {
                di.next();
                const QFileInfo& fi = di.fileInfo();
                if(fi.fileName().endsWith(QString::fromAscii(".mega")))
                    QFile::remove(di.filePath());
            }

            if(!preferences->isFolderActive(i))
                continue;

            MegaNode *node = api->getNodeByHandle(preferences->getMegaFolderHandle(i));
            QString localFolder = preferences->getLocalFolder(i);
            api->resumeSync(localFolder.toUtf8().constData(), preferences->getLocalFingerprint(i), node);
            delete node;
        }
    }
}

