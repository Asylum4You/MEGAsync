#include "StalledIssuesCaseHeaders.h"

#include <Utilities.h>
#include <Preferences.h>
#include <MegaApplication.h>

#include <StalledIssuesModel.h>
#include <StalledIssue.h>
#include <NameConflictStalledIssue.h>
#include <QMegaMessageBox.h>
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>

#ifdef _WIN32
    #include "minwindef.h"
#elif defined Q_OS_MACOS
    #include "sys/syslimits.h"
#elif defined Q_OS_LINUX
    #include "limits.h"
#endif

StalledIssueHeaderCase::StalledIssueHeaderCase(StalledIssueHeader *header)
    :QObject(header)
{
    header->setData(this);
}

//Local folder not scannable
DefaultHeader::DefaultHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DefaultHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Error detected with <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Reason not found."));
}

//Detected Hard Link
SymLinkHeader::SymLinkHeader(StalledIssueHeader *header)
    : StalledIssueHeaderCase(header)
{}

void SymLinkHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader*, int index)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto isSymLinkChecker = [](const std::shared_ptr<const StalledIssue> issue){
        return issue->isSymLink();
    };

    auto selection = dialog->getDialog()->getSelection(isSymLinkChecker);

    if(index == IgnoreType::IgnoreAll)
    {
         textsByButton.insert(QMessageBox::Ok, tr("Ok"));
    }
    else
    {
        if(selection.size() <= 1)
        {
            auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(isSymLinkChecker);

            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar symlinks (%1)").arg(allSimilarIssues.size()));
                textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
            }
            else
            {
                textsByButton.insert(QMessageBox::Ok, tr("Ok"));
            }
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }
    }

    msgInfo.buttonsText = textsByButton;

    if(index == IgnoreType::IgnoreAll)
    {
        msgInfo.text = tr("Are you sure you want to ignore all symlinks in this sync?");
        msgInfo.informativeText = tr("This action will ignore all present and future symlinks in this sync.");
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to ignore this symlink?");
        msgInfo.informativeText = tr("This action will ignore this symlink and it will not be synced.");
    }

    msgInfo.finishFunc = [this, index, selection](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            if(index == IgnoreType::IgnoreAll)
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreSymLinks(selection.first());
            }
            else
            {
                MegaSyncApp->getStalledIssuesModel()->ignoreItems(selection);
            }
        }
        else if(msgBox->result() == QDialogButtonBox::Yes)
        {
            MegaSyncApp->getStalledIssuesModel()->ignoreItems(QModelIndexList());
        }
    };

    QMegaMessageBox::warning(msgInfo);

}

void SymLinkHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Detected sym link: <b>%1</b>").arg(header->getData().consultData()->consultLocalData()->getNativeFilePath()));
    header->setTitleDescriptionText(QString());
    header->setIsExpandable(false);
}

void SymLinkHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(!header->getData().consultData()->isSolved())
    {
        QList<StalledIssueHeader::ActionInfo> actions;
        actions << StalledIssueHeader::ActionInfo(tr("Ignore symlink"), IgnoreType::IgnoreThis);
        actions << StalledIssueHeader::ActionInfo(tr("Ignore all symlinks in sync"), IgnoreType::IgnoreAll);

        header->showMultipleAction(tr("Ignore"), actions);
        header->hideIgnoreFile();
    }
}

//Local folder not scannable
FileIssueHeader::FileIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void FileIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    if(header->getData().consultData()->hasFiles() > 0)
    {
        header->setTitleDescriptionText(tr("A single file had an issue that needs a user decision to solve"));
    }
    else if(header->getData().consultData()->hasFolders() > 0)
    {
        header->setTitleDescriptionText(tr("A single folder had an issue that needs a user decision to solve."));
    }
}

//Local folder not scannable
MoveOrRenameCannotOccurHeader::MoveOrRenameCannotOccurHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void MoveOrRenameCannotOccurHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Cannot move or rename <b>%1</b>").arg(header->displayFileName()));
    if (header->getData().consultData()->mDetectedMEGASide)
    {
        header->setTitleDescriptionText(tr("A move or rename was detected in MEGA, but could not be replicated in the local filesystem."));
    }
    else
    {
        header->setTitleDescriptionText(tr("A move or rename was detected in the local filesystem, but could not be replicated in MEGA."));
    }
}

//Delete or Move Waiting onScanning
DeleteOrMoveWaitingOnScanningHeader::DeleteOrMoveWaitingOnScanningHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteOrMoveWaitingOnScanningHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t find <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting to finish scan to see if the file was moved or deleted."));
}

//Local folder not scannable
DeleteWaitingOnMovesHeader::DeleteWaitingOnMovesHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void DeleteWaitingOnMovesHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Waiting to move <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Waiting for other processes to complete."));
}

//Upsync needs target folder
UploadIssueHeader::UploadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void UploadIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t upload <b>%1</b> to the selected location").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot reach the destination folder."));
}

//Downsync needs target folder
DownloadIssueHeader::DownloadIssueHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void DownloadIssueHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t download <b>%1</b> to the selected location").arg(header->displayFileName(true)));
    header->setTitleDescriptionText(tr("A failure occurred either downloading the file, or moving the downloaded temporary file to its final name and location."));
}

//Create folder failed
CannotCreateFolderHeader::CannotCreateFolderHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotCreateFolderHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Cannot create <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//Create folder failed
CannotPerformDeletionHeader::CannotPerformDeletionHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{}

void CannotPerformDeletionHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Cannot perform deletion <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Filesystem error preventing folder access."));
}

//SyncItemExceedsSupoortedTreeDepth
SyncItemExceedsSupoortedTreeDepthHeader::SyncItemExceedsSupoortedTreeDepthHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void SyncItemExceedsSupoortedTreeDepthHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Unable to sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Target is too deep on your folder structure.\nPlease move it to a location that is less than 64 folders deep."));
}

////MoveTargetNameTooLongHeader
//CreateFolderNameTooLongHeader::CreateFolderNameTooLongHeader(StalledIssueHeader* header)
//    : StalledIssueHeaderCase(header)
//{}

//void CreateFolderNameTooLongHeader::refreshCaseTitles(StalledIssueHeader* header)
//{
//    auto maxCharacters(0);

//#ifdef Q_OS_MACX
//    maxCharacters = NAME_MAX;
//#elif defined(_WIN32)
//    maxCharacters = MAX_PATH;
//#elif defined (Q_OS_LINUX)
//    maxCharacters = NAME_MAX;
//#endif

//    setLeftTitleText(tr("Unable to sync"));
//    addFileName();
//    setTitleDescriptionText(tr("Folder name too long. Your Operating System only supports folder"
//                               "\nnames up to <b>%1</b> characters.").arg(QString::number(maxCharacters)));
//}

//SymlinksNotSupported
FolderMatchedAgainstFileHeader::FolderMatchedAgainstFileHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void FolderMatchedAgainstFileHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("Cannot sync folders against files."));
}

LocalAndRemotePreviouslyUnsyncedDifferHeader::LocalAndRemotePreviouslyUnsyncedDifferHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::onActionButtonClicked(StalledIssueHeader *header)
{
    LocalAndRemoteActionButtonClicked::actionClicked(header);
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        if(conflict->isSolvable() && !conflict->isSolved())
        {
            header->showAction(tr("Solve"));
        }
        else
        {
            header->hideAction();
        }

        if(conflict->isSolved())
        {
            header->showSolvedMessage();
        }
    }
}

void LocalAndRemotePreviouslyUnsyncedDifferHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has conflicting copies"));
}

//Local and remote previously synced differ
LocalAndRemoteChangedSinceLastSyncedStateHeader::LocalAndRemoteChangedSinceLastSyncedStateHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::onActionButtonClicked(StalledIssueHeader *header)
{
    LocalAndRemoteActionButtonClicked::actionClicked(header);
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        if(conflict->isSolvable() && !conflict->isSolved())
        {
            header->showAction(tr("Solve"));
        }
        else
        {
            header->hideAction();
        }

        if(conflict->isSolved())
        {
            header->showSolvedMessage();
        }
    }
}

void LocalAndRemoteChangedSinceLastSyncedStateHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    header->setText(tr("Can´t sync <b>%1</b>").arg(header->displayFileName()));
    header->setTitleDescriptionText(tr("This file has been changed both in MEGA and locally since it it was last synced."));
}

void LocalAndRemoteActionButtonClicked::actionClicked(StalledIssueHeader *header)
{
    if(auto conflict = header->getData().convert<LocalOrRemoteUserMustChooseStalledIssue>())
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::No, tr("Cancel"));

        auto reasons(QList<mega::MegaSyncStall::SyncStallReason>()
                     << mega::MegaSyncStall::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                     << mega::MegaSyncStall::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose);
        auto selection = dialog->getDialog()->getSelection(reasons);

        if(selection.size() <= 1)
        {
            auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);

            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
                textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
            }
            else
            {
                textsByButton.insert(QMessageBox::Ok, tr("Ok"));
            }
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }

        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Are you sure you want to solve the issue?");
        msgInfo.informativeText = tr("This action will choose the latest modified side");

        msgInfo.finishFunc = [selection, header, conflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveLocalRemoteIssues(selection);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveLocalRemoteIssues(QModelIndexList());
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
}

//Name Conflicts
NameConflictsHeader::NameConflictsHeader(StalledIssueHeader* header)
    : StalledIssueHeaderCase(header)
{
}

void NameConflictsHeader::refreshCaseActions(StalledIssueHeader *header)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        if(!nameConflict->isSolved())
        {
            QList<StalledIssueHeader::ActionInfo> actions;

            if(header->getData().consultData()->hasFiles() > 0)
            {
                if(nameConflict->areAllDuplicatedNodes())
                {
                    actions << StalledIssueHeader::ActionInfo(tr("Remove duplicates"), NameConflictedStalledIssue::RemoveDuplicated);
                }
                else if(nameConflict->hasDuplicatedNodes())
                {
                    actions << StalledIssueHeader::ActionInfo(tr("Remove duplicates and rename the rest"), NameConflictedStalledIssue::RemoveDuplicatedAndRename);
                }
            }

            actions << StalledIssueHeader::ActionInfo(tr("Rename all items"), NameConflictedStalledIssue::RenameAll);

            header->showMultipleAction(tr("Solve options"), actions);
        }
    }
}

void NameConflictsHeader::refreshCaseTitles(StalledIssueHeader* header)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        QString text(tr("Name Conflicts: <b>%1</b>"));

        auto cloudData = nameConflict->getNameConflictCloudData();
        if(cloudData.firstNameConflict())
        {
            text = text.arg(cloudData.firstNameConflict()->getConflictedName());
        }
        else
        {
            auto localConflictedNames = nameConflict->getNameConflictLocalData();
            if(!localConflictedNames.isEmpty())
            {
                text = text.arg(localConflictedNames.first()->getConflictedName());
            }
        }

        header->setText(text);

        if(header->getData().consultData()->hasFiles() > 0 && header->getData().consultData()->hasFolders() > 0)
        {
            header->setTitleDescriptionText(tr("These items contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->hasFiles() > 0)
        {
            header->setTitleDescriptionText(tr("These files contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
        else if(header->getData().consultData()->hasFolders() > 0)
        {
            header->setTitleDescriptionText(tr("These folders contain multiple names on one side, that would all become the same single name on the other side."
                                               "\nThis may be due to syncing to case insensitive local filesystems, or the effects of escaped characters."));
        }
    }
}

void NameConflictsHeader::onMultipleActionButtonOptionSelected(StalledIssueHeader* header, int index)
{
    if(auto nameConflict = header->getData().convert<NameConflictedStalledIssue>())
    {
        auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
        QMap<QMessageBox::Button, QString> textsByButton;
        textsByButton.insert(QMessageBox::No, tr("Cancel"));

        auto solutionCanBeApplied = [index](const std::shared_ptr<const StalledIssue> issue) -> bool{
            auto result = issue->getReason() == mega::MegaSyncStall::NamesWouldClashWhenSynced;
            if(result)
            {
                if(auto nameIssue = StalledIssue::convert<NameConflictedStalledIssue>(issue))
                {
                    if(index == NameConflictedStalledIssue::RemoveDuplicated)
                    {
                        result = nameIssue->hasFiles() > 0 && nameIssue->areAllDuplicatedNodes();
                    }
                    else if(index == NameConflictedStalledIssue::RemoveDuplicatedAndRename)
                    {
                        result = nameIssue->hasFiles() > 0 &&
                                 nameIssue->hasDuplicatedNodes() &&
                                 !nameIssue->areAllDuplicatedNodes();
                    }
                }
            }
            return result;
        };

        auto selection = dialog->getDialog()->getSelection(solutionCanBeApplied);

        if(selection.size() <= 1)
        {
            auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssues(solutionCanBeApplied);

            if(allSimilarIssues.size() != selection.size())
            {
                msgInfo.buttons |= QMessageBox::Yes;
                textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
                textsByButton.insert(QMessageBox::Ok, tr("Apply only to this issue"));
            }
            else
            {
                textsByButton.insert(QMessageBox::Ok, tr("Ok"));
            }
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
        }

        msgInfo.buttonsText = textsByButton;
        msgInfo.text = tr("Are you sure you want to solve the issue?");

        if(index == NameConflictedStalledIssue::RenameAll)
        {
            msgInfo.informativeText = tr("This action will rename the conflicted items (adding a suffix like (1)).");
        }
        else
        {
            if(index == NameConflictedStalledIssue::RemoveDuplicated)
            {
               msgInfo.informativeText = tr("This action will delete the duplicate files.");
            }
            else
            {
                msgInfo.informativeText = tr("This action will delete the duplicate files and rename the rest of names (adding a suffix like (1)).");
            }
        }

        msgInfo.finishFunc = [this, index, selection, header, nameConflict](QMessageBox* msgBox)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveNameConflictIssues(selection, index);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->semiAutoSolveNameConflictIssues(QModelIndexList(), index);
            }
        };

        QMegaMessageBox::warning(msgInfo);
    }
}
