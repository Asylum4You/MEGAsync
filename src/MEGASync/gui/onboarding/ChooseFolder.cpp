#include <QDir>
#include <QDebug>

#include "ChooseFolder.h"
#include <Platform.h>
#include <MegaApplication.h>
#include <node_selector/gui/NodeSelector.h>
#include <DialogOpener.h>


ChooseLocalFolder::ChooseLocalFolder(QObject* parent)
    : QObject(parent)
    , mFolderName(QString())
{
    const auto standardPaths (QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation));
    QDir dir (QDir::cleanPath(standardPaths.first()));
    mFolder = QDir::toNativeSeparators(dir.canonicalPath());
}

void ChooseLocalFolder::openFolderSelector()
{
    Platform::getInstance()->folderSelector(tr("Select local folder"), mFolder, false, nullptr, [this](QStringList selection){
        if(!selection.isEmpty())
        {
            QString fPath = selection.first();
            mFolder = fPath;
            mFolderName = QDir::fromNativeSeparators(mFolder).split(QString::fromLatin1("/")).last().prepend(QString::fromLatin1("/"));
            emit folderChanged(mFolderName);
        }
    });
}

const QString ChooseLocalFolder::getFolder()
{
    return mFolder;
}


ChooseRemoteFolder::ChooseRemoteFolder(QObject *parent)
    : QObject(parent)
    , mFolderName(QString())
    , mFolderHandle(mega::INVALID_HANDLE)
{
}

void ChooseRemoteFolder::openFolderSelector()
{
    QPointer<SyncNodeSelector> nodeSelector = new SyncNodeSelector();

    if(mFolderHandle != mega::INVALID_HANDLE)
    {
        std::shared_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(mFolderHandle));
        nodeSelector->setSelectedNodeHandle(node);
    }

    DialogOpener::showDialog<SyncNodeSelector>(nodeSelector, [nodeSelector, this]()
    {
        if (nodeSelector->result() == QDialog::Accepted)
        {
            mFolderHandle = nodeSelector->getSelectedNodeHandle();
            if(auto node = MegaSyncApp->getMegaApi()->getNodeByHandle(mFolderHandle))
            {
                mFolderName = QString::fromUtf8(node->getName());
                emit folderChanged(mFolderName.prepend(QString::fromLatin1("/")));
            }
        }
    });
}

const mega::MegaHandle ChooseRemoteFolder::getHandle()
{
    return mFolderHandle;
}
