#ifndef IMPORTMEGALINKSDIALOG_H
#define IMPORTMEGALINKSDIALOG_H

#include <QDialog>
#include <QStringList>
#include "megaapi.h"
#include "control/LinkProcessor.h"
#include "control/Preferences/Preferences.h"

namespace Ui {
class ImportMegaLinksDialog;
}

class UploadNodeSelector;

class ImportMegaLinksDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportMegaLinksDialog(LinkProcessor* linkProcessor, QWidget *parent = 0);
    ~ImportMegaLinksDialog();

    bool shouldImport();
    bool shouldDownload();
    QString getImportPath();
    QString getDownloadPath();

private slots:
    void on_cDownload_clicked();
    void on_cImport_clicked();
    void on_bLocalFolder_clicked();
    void on_bMegaFolder_clicked();
    void on_bOk_clicked();

public slots:
    void onLinkInfoAvailable(int id);
    void onLinkInfoRequestFinish();
    void onLinkStateChanged(int id, int state);

protected:
    void changeEvent(QEvent * event) override;

private:
    Ui::ImportMegaLinksDialog *ui;
    mega::MegaApi *mMegaApi;
    std::shared_ptr<Preferences> mPreferences;
    LinkProcessor* mLinkProcessor;
    bool mFinished;

    bool mDownloadPathChangedByUser;

    bool mUseDefaultImportPath;
    bool mImportPathChangedByUser;

    void initUiAsLogged();
    void initUiAsUnlogged();
    void initImportFolderControl();
    void setInvalidImportFolder();

    void enableOkButton() const;
    void enableLocalFolder(bool enable);
    void enableMegaFolder(bool enable);
    void checkLinkValidAndSelected();

    void onLocalFolderSet(const QString& path);

    void updateDownloadPath();
    void updateImportPath(const QString& path = QString());
};

#endif // IMPORTMEGALINKSDIALOG_H
