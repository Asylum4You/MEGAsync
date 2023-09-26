#include "LocalAndRemoteDifferentWidget.h"
#include "ui_LocalAndRemoteDifferentWidget.h"

#include "MegaApplication.h"
#include "StalledIssuesModel.h"
#include "StalledIssueHeader.h"
#include <DialogOpener.h>
#include <StalledIssuesDialog.h>
#include <PlatformStrings.h>
#include <QMegaMessageBox.h>
#include <LocalOrRemoteUserMustChooseStalledIssue.h>

#include "mega/types.h"

#include <QMessageBox>
#include <QFile>

LocalAndRemoteDifferentWidget::LocalAndRemoteDifferentWidget(std::shared_ptr<mega::MegaSyncStall> originalstall, QWidget *parent) :
    StalledIssueBaseDelegateWidget(parent),
    originalStall(originalstall),
    ui(new Ui::LocalAndRemoteDifferentWidget)
{
    ui->setupUi(this);

    connect(ui->chooseLocalCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onLocalButtonClicked);
    connect(ui->chooseRemoteCopy, &StalledIssueChooseWidget::chooseButtonClicked, this, &LocalAndRemoteDifferentWidget::onRemoteButtonClicked);

    auto margins = ui->chooseLayout->contentsMargins();
    margins.setLeft(StalledIssueHeader::GROUPBOX_INDENT);
    ui->chooseLayout->setContentsMargins(margins);

    ui->selectLabel->setIndent(StalledIssueHeader::BODY_INDENT);
}

LocalAndRemoteDifferentWidget::~LocalAndRemoteDifferentWidget()
{
    delete ui;
}

void LocalAndRemoteDifferentWidget::refreshUi()
{
    auto issue = getData().convert<LocalOrRemoteUserMustChooseStalledIssue>();

    if(issue->consultLocalData())
    {
        ui->chooseLocalCopy->updateUi(issue->consultLocalData(), issue->getChosenSide());

        ui->chooseLocalCopy->show();
    }
    else
    {
        ui->chooseLocalCopy->hide();
    }

    if(issue->consultCloudData())
    {
        ui->chooseRemoteCopy->updateUi(issue->consultCloudData(), issue->getChosenSide());

        ui->chooseRemoteCopy->show();
    }
    else
    {
        ui->chooseRemoteCopy->hide();
    }

    if(issue->isPotentiallySolved())
    {
        ui->chooseLocalCopy->hideActionButton();
        ui->chooseRemoteCopy->hideActionButton();
    }
}

void LocalAndRemoteDifferentWidget::onLocalButtonClicked(int)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    if(checkIssue(dialog ? dialog->getDialog() : nullptr))
    {
        return;
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto reasons(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                 << mega::MegaSyncStall::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose);

    auto selection = dialog->getDialog()->getSelection(reasons);
    auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);

    auto pluralNumber(1);
    if(selection.size() <= 1)
    {
        if(allSimilarIssues.size() != selection.size())
        {
            msgInfo.buttons |= QMessageBox::Yes;
            textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issue"));
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Ok"));
        }
    }
    else
    {
        pluralNumber = selection.size();
        textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
    }

    msgInfo.buttonsText = textsByButton;

    if(localInfo.isFile())
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>local file</b> %1?", "", pluralNumber).arg(ui->chooseLocalCopy->data()->getFileName());
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>local folder</b> %1?", "", pluralNumber).arg(ui->chooseLocalCopy->data()->getFileName());
    }

    if(node->isFile())
    {
        msgInfo.informativeText = tr("The <b>local file</b> %1 will be uploaded to MEGA and added as a version to the remote file.\nPlease wait for the upload to complete.</br>", "", pluralNumber).arg(localInfo.fileName());
    }
    else
    {
        msgInfo.informativeText = tr("The <b>remote folder</b> %1 will be moved to MEGA Rubbish Bin.<br>You will be able to retrieve the folder from there.</br>", "", pluralNumber).arg(localInfo.fileName());
    }

    if(MegaSyncApp->getTransfersModel()->areAllPaused())
    {
        msgInfo.informativeText.append(tr("<br><b>Please, resume your transfers to fix the issue</b></br>", "", pluralNumber));
    }

    msgInfo.finishFunc = [this, selection, allSimilarIssues, dialog](QMessageBox* msgBox)
    {
        if(msgBox->result() == QDialogButtonBox::Ok)
        {
            MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, selection);
        }
        else if(msgBox->result() == QDialogButtonBox::Yes)
        {
            MegaSyncApp->getStalledIssuesModel()->chooseSideManually(false, allSimilarIssues);
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

void LocalAndRemoteDifferentWidget::onRemoteButtonClicked(int)
{
    auto dialog = DialogOpener::findDialog<StalledIssuesDialog>();

    if(checkIssue(dialog ? dialog->getDialog() : nullptr))
    {
        return;
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByPath(ui->chooseRemoteCopy->data()->getFilePath().toUtf8().constData()));
    QFileInfo localInfo(ui->chooseLocalCopy->data()->getFilePath());

    QMegaMessageBox::MessageBoxInfo msgInfo;
    msgInfo.parent = dialog ? dialog->getDialog() : nullptr;
    msgInfo.title = MegaSyncApp->getMEGAString();
    msgInfo.textFormat = Qt::RichText;
    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;

    msgInfo.buttons = QMessageBox::Ok | QMessageBox::Cancel;
    QMap<QMessageBox::Button, QString> textsByButton;
    textsByButton.insert(QMessageBox::No, tr("Cancel"));

    auto reasons(QList<mega::MegaSyncStall::SyncStallReason>() << mega::MegaSyncStall::LocalAndRemoteChangedSinceLastSyncedState_userMustChoose
                     << mega::MegaSyncStall::LocalAndRemotePreviouslyUnsyncedDiffer_userMustChoose);
    auto selection = dialog->getDialog()->getSelection(reasons);
    auto allSimilarIssues = MegaSyncApp->getStalledIssuesModel()->getIssuesByReason(reasons);

    auto pluralNumber(1);
    if(selection.size() <= 1)
    {
        if(allSimilarIssues.size() != selection.size())
        {
            msgInfo.buttons |= QMessageBox::Yes;
            textsByButton.insert(QMessageBox::Yes, tr("Apply to all similar issues (%1)").arg(allSimilarIssues.size()));
            textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issue"));
        }
        else
        {
            textsByButton.insert(QMessageBox::Ok, tr("Ok"));
        }
    }
    else
    {
        pluralNumber = selection.size();
        textsByButton.insert(QMessageBox::Ok, tr("Apply to selected issues (%1)").arg(selection.size()));
    }

    msgInfo.buttonsText = textsByButton;

    if(node)
    {
        if(node->isFile())
        {
            msgInfo.text = tr("Are you sure you want to keep the <b>remote file</b> %1?", "", pluralNumber).arg(ui->chooseRemoteCopy->data()->getFileName());
        }
        else
        {
            msgInfo.text = tr("Are you sure you want to keep the <b>remote folder</b> %1?", "", pluralNumber).arg(ui->chooseRemoteCopy->data()->getFileName());
        }
    }
    else
    {
        msgInfo.text = tr("Are you sure you want to keep the <b>remote item</b> %1?", "", pluralNumber).arg(ui->chooseRemoteCopy->data()->getFileName());
    }

    //For the moment, TYPE_TWOWAY or TYPE_UNKNOWN
    if(getData().consultData()->getSyncType() != mega::MegaSync::SyncType::TYPE_BACKUP)
    {
        if(localInfo.isFile())
        {
            msgInfo.informativeText = tr("The <b>local file</b> %1 will be moved to the sync debris folder").arg(localInfo.fileName());
        }
        else
        {
            msgInfo.informativeText = tr("The <b>local folder</b> %1 will be moved to the sync debris folder").arg(localInfo.fileName());
        }
    }
    else
    {
        if(localInfo.isFile())
        {
            msgInfo.informativeText = tr("The backup will be disabled in order to protect the local file %1", "", pluralNumber).arg(localInfo.fileName());
        }
        else
        {
            msgInfo.informativeText = tr("The backup will be disabled in order to protect the local folder %1", "", pluralNumber).arg(localInfo.fileName());
        }
    }

    msgInfo.finishFunc = [this, selection, allSimilarIssues](QMessageBox* msgBox)
    {
        if(getData().consultData()->getSyncType() == mega::MegaSync::SyncType::TYPE_TWOWAY)
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(true, selection);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                MegaSyncApp->getStalledIssuesModel()->chooseSideManually(true, allSimilarIssues);
            }
        }
        else
        {
            if(msgBox->result() == QDialogButtonBox::Ok)
            {
                MegaSyncApp->getStalledIssuesModel()->chooseRemoteForBackups(selection);
            }
            else if(msgBox->result() == QDialogButtonBox::Yes)
            {
                 MegaSyncApp->getStalledIssuesModel()->chooseRemoteForBackups(allSimilarIssues);
            }
        }
    };

    QMegaMessageBox::warning(msgInfo);
}

bool LocalAndRemoteDifferentWidget::checkIssue(QDialog *dialog)
{
    if(MegaSyncApp->getStalledIssuesModel()->checkForExternalChanges(getCurrentIndex()))
    {
        QMegaMessageBox::MessageBoxInfo msgInfo;
        msgInfo.parent = dialog;
        msgInfo.title = MegaSyncApp->getMEGAString();
        msgInfo.textFormat = Qt::RichText;
        msgInfo.buttons = QMessageBox::Ok;
        QMap<QMessageBox::StandardButton, QString> buttonsText;
        buttonsText.insert(QMessageBox::Ok, tr("Refresh"));
        msgInfo.buttonsText = buttonsText;
        msgInfo.text = tr("The issue may have been solved externally.\nPlease, refresh the list.");
        msgInfo.finishFunc = [this](QPointer<QMessageBox>){
            MegaSyncApp->getStalledIssuesModel()->updateStalledIssues();
        };

        QMegaMessageBox::warning(msgInfo);


        ui->chooseLocalCopy->hideActionButton();
        ui->chooseRemoteCopy->hideActionButton();

        updateSizeHint();

        return true;
    }

    return false;
}
