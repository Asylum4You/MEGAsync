#include "Preferences.h"
#include "Onboarding.h"
#include "MegaApplication.h"
#include "UserAttributesRequests/DeviceName.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"
#include <QQmlEngine>

using namespace mega;

Onboarding::Onboarding(QObject *parent)
    : QMLComponent(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(new QTMegaRequestListener(mMegaApi, this))
    , mPreferences(Preferences::instance())
    , mPassword(QString())
{
    qmlRegisterUncreatableType<Onboarding>("Onboarding", 1, 0, "OnboardEnum", QString::fromUtf8("Cannot create WarningLevel in QML"));

    qmlRegisterModule("Onboard", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingDialog.qml")), "Onboard", 1, 0, "OnboardingDialog");
    qmlRegisterSingletonType(QUrl(QString::fromUtf8("qrc:/content/onboard/OnboardingStrings.qml")), "Onboard", 1, 0, "OnboardingStrings");

    qmlRegisterModule("Onboard.Syncs_types", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsFlow.qml")), "Onboard.Syncs_types", 1, 0, "SyncsFlow");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Header.qml")), "Onboard.Syncs_types", 1, 0, "Header");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/Footer.qml")), "Onboard.Syncs_types", 1, 0, "Footer");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/InfoAccount.qml")), "Onboard.Syncs_types", 1, 0, "InfoAccount");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/ResumePage.qml")), "Onboard.Syncs_types", 1, 0, "ResumePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/SyncsPage.qml")), "Onboard.Syncs_types", 1, 0, "SyncsPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/InstallationType.qml")), "Onboard.Syncs_types", 1, 0, "InstallationType");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/ResumeButton.qml")), "Onboard.Syncs_types", 1, 0, "ResumeButton");

    qmlRegisterModule("Onboard.Syncs_types.Syncs", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SyncTypePage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SyncTypePage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/FullSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "FullSyncPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/syncs/SelectiveSyncPage.qml")), "Onboard.Syncs_types.Syncs", 1, 0, "SelectiveSyncPage");

    qmlRegisterModule("Onboard.Syncs_types.Left_panel", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/left_panel/StepPanel.qml")), "Onboard.Syncs_types.Left_panel", 1, 0, "StepPanel");

    qmlRegisterModule("Onboard.Syncs_types.Backups", 1, 0);
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/ConfirmFoldersPage.qml")), "Onboard.Syncs_types.Backups", 1, 0, "ConfirmFoldersPage");
    qmlRegisterType(QUrl(QString::fromUtf8("qrc:/content/onboard/syncs_types/backups/SelectFoldersPage.qml")), "Onboard.Syncs_types.Backups", 1, 0, "SelectFoldersPage");
}

QUrl Onboarding::getQmlUrl()
{
    return QUrl(QString::fromUtf8("qrc:/main.qml"));
}

QString Onboarding::contextName()
{
    return QString::fromUtf8("Onboarding");
}

void Onboarding::onRequestStart(MegaApi*, MegaRequest* request)
{
}

void Onboarding::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_LOGIN:
        {
            if (error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_OK";
                mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_OK);
                auto email = request->getEmail();
                MegaSyncApp->fetchNodes(QString::fromUtf8(email ? email : "")); //TODO: REVIEW IF THIS IS NECESSARY
                if (!mPreferences->hasLoggedIn())
                {
                    mPreferences->setHasLoggedIn(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000);
                }
                emit loginFinished();
            }
            else
            {
                mPreferences->setAccountStateInGeneral(Preferences::STATE_LOGGED_FAILED);
                if (error->getErrorCode() == MegaError::API_EMFAREQUIRED)
                {
                    qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN API_EMFAREQUIRED";
                    mPreferences->setEmail(QString::fromUtf8(request->getEmail()));
                    mPassword = QString::fromUtf8(request->getPassword());
                    emit twoFARequired();
                }
                else if (error->getErrorCode() == MegaError::API_EFAILED)
                {
                    qDebug() << "Onboarding::onRequestFinish -> API_EFAILED";
                    emit twoFAFailed();
                }
                else
                {
                    qDebug() << "Onboarding::onRequestFinish -> TYPE_LOGIN Error code -> "
                             << error->getErrorCode() << " Error string -> " << error->getErrorString();
                    emit userPassFailed();
                }
            }
            break;
        }
        case MegaRequest::TYPE_CREATE_ACCOUNT:
        {
            if(error->getErrorCode() == MegaError::API_OK)
            {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT API_OK";
                emit loginFinished(); // maybe we should change this signal
            } else {
                qDebug() << "Onboarding::onRequestFinish -> TYPE_CREATE_ACCOUNT Error code -> " << error->getErrorCode();
            }
            break;
        }
    }
}

void Onboarding::onLoginClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onLoginClicked" << data;

    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();
    mMegaApi->login(email.c_str(), password.c_str(), this->mDelegateListener.get());
}

void Onboarding::onRegisterClicked(const QVariantMap& data)
{
    qDebug() << "Onboarding::onRegisterClicked" << data;

    std::string firstName = data.value(QString::number(FIRST_NAME)).toString().toStdString();
    std::string lastName = data.value(QString::number(LAST_NAME)).toString().toStdString();
    std::string email = data.value(QString::number(EMAIL)).toString().toStdString();
    std::string password = data.value(QString::number(PASSWORD)).toString().toStdString();

    mMegaApi->createAccount(email.c_str(),
                            password.c_str(),
                            firstName.c_str(),
                            lastName.c_str(),
                            this->mDelegateListener.get());
}

void Onboarding::onTwoFARequested(const QString& pin)
{
    qDebug() << "Onboarding::onTwoFARequested -> pin = " << pin;
    qDebug() << "Onboarding::onTwoFARequested -> mEmail = " << mPreferences->email();
    qDebug() << "Onboarding::onTwoFARequested -> mPassword = " << mPassword;

    mMegaApi->multiFactorAuthLogin(mPreferences->email().toUtf8().constData(),
                                   mPassword.toUtf8().constData(),
                                   pin.toUtf8().constData(),
                                   this->mDelegateListener.get());
}

QString Onboarding::convertUrlToNativeFilePath(const QUrl &urlStylePath) const
{
    return QDir::toNativeSeparators(urlStylePath.toLocalFile());
}

void Onboarding::addSync(const QString &localPath, const mega::MegaHandle &remoteHandle)
{
    connect(&mSyncController, &SyncController::syncAddStatus, this, [this](int errorCode, const QString errorMsg)
    {
        if (errorCode != MegaError::API_OK)
        {
            Text::Link link(Utilities::SUPPORT_URL);
            Text::Decorator dec(&link);
            QString msg = errorMsg;
            dec.process(msg);
            QMegaMessageBox::warning(nullptr, tr("Error adding sync"), msg, QMessageBox::Ok, QMessageBox::NoButton, QMap<QMessageBox::StandardButton, QString>(), Qt::RichText);
        }
        else
        {
            emit syncSetupSucces();
        }
    }, Qt::UniqueConnection);

    QString warningMessage;
    auto syncability (SyncController::isLocalFolderAllowedForSync(localPath, MegaSync::TYPE_TWOWAY, warningMessage));
    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = SyncController::areLocalFolderAccessRightsOk(localPath, MegaSync::TYPE_TWOWAY, warningMessage);
    }

    // If OK, check that we can sync the selected remote folder
    std::shared_ptr<MegaNode> node (mMegaApi->getNodeByHandle(remoteHandle));

    if (syncability != SyncController::CANT_SYNC)
    {
        syncability = std::max(SyncController::isRemoteFolderSyncable(node, warningMessage), syncability);
    }

    if (syncability == SyncController::CANT_SYNC)
    {
        // If can't sync because remote node does not exist, try to create it
        if (!node)
        {
            auto rootNode = MegaSyncApp->getRootNode();
            if (!rootNode)
            {
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Unable to get the filesystem.\n"
                                                                  "Please, try again. If the problem persists "
                                                                  "please contact bug@mega.co.nz"));
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Setting isCrashed true: !rootNode (Onboarding)");
                mPreferences->setCrashed(true);
                MegaSyncApp->rebootApplication(false);
            }
        }
        else
        {
            QMegaMessageBox::warning(nullptr, tr("Warning"), warningMessage, QMessageBox::Ok);
        }
    }
    else if (syncability == SyncController::CAN_SYNC
             || (syncability == SyncController::WARN_SYNC
                 && QMegaMessageBox::warning(nullptr, tr("Warning"), warningMessage
                                             + QLatin1Char('\n')
                                             + tr("Do you want to continue?"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                 == QMessageBox::Yes))
    {
        mSyncController.addSync(localPath, remoteHandle);
    }
    qDebug()<<localPath<<":"<<remoteHandle;
}

void Onboarding::addBackup(const QString &localPath)
{
    mSyncController.addBackup(localPath);
}

void Onboarding::onNotNowClicked() {
    std::unique_ptr<char[]> email(mMegaApi->getMyEmail());
    mPreferences->setEmailAndGeneralSettings(QString::fromUtf8(email.get()));
    emit notNowFinished();
}

QString Onboarding::getComputerName()
{
    return UserAttributes::DeviceName::requestDeviceName()->getDeviceName();
}
