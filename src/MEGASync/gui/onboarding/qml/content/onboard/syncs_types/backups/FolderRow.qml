// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.ToolTips 1.0 as MegaToolTips
import Components.Buttons 1.0 as MegaButtons
import Components.TextFields 1.0 as MegaTextFields
import Components.BusyIndicator 1.0 as MegaBusyIndicator

// Local
import Onboard 1.0

// C++
import BackupsModel 1.0
import ChooseLocalFolder 1.0

Rectangle {
    id: root

    readonly property int totalHeight: 34
    readonly property int horizontalMargin: 8
    readonly property int internalMargin: 8
    readonly property int extraMarginWhenHintShowed: 5

    property bool editMode: false

    height: totalHeight
    anchors.right: parent.right
    anchors.left: parent.left
    anchors.rightMargin: horizontalMargin
    anchors.leftMargin: horizontalMargin

    Rectangle {
        id: background

        anchors.right: root.right
        anchors.left: root.left
        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.rightMargin: internalMargin
        anchors.leftMargin: internalMargin
        radius: internalMargin
        color: (index % 2 === 0) ? Styles.pageBackground : Styles.surface1

        Loader {
            id: content

            anchors.verticalCenter: parent.verticalCenter
            anchors.fill: parent
            sourceComponent: {
                if(!backupsProxyModel.selectedFilterEnabled
                    || error === backupsModel.BackupErrorCode.None) {
                    return selectContent;
                } else {
                    if(error === backupsModel.BackupErrorCode.SyncConflict
                        || error === backupsModel.BackupErrorCode.PathRelation
                        || error === backupsModel.BackupErrorCode.UnavailableDir
                        || error === backupsModel.BackupErrorCode.SDKCreation) {
                        return conflictContent;
                    } else {
                        // DuplicatedName or ExistsRemote errors
                        if(editMode) {
                            return editContent;
                        } else {
                            return conflictContent;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: selectContent

        Item {
            id: contentRoot

            readonly property int contentMargin: 8
            readonly property int checkboxWidth: 16
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248

            property int checkboxSpacing: !checkbox.visible ? 0 : 12

            anchors.fill: parent
            anchors.margins: contentMargin

            Row {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                spacing: checkboxSpacing

                MegaCheckBoxes.CheckBox {
                    id: checkbox

                    width: backupsProxyModel.selectedFilterEnabled ? 0 : contentRoot.checkboxWidth
                    checked: selected
                    visible: !backupsProxyModel.selectedFilterEnabled
                }

                Row {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    spacing: contentRoot.imageTextSpacing

                    Image {
                        height: contentRoot.imageWidth
                        width: contentRoot.imageWidth
                        anchors.top: parent.top
                        source: done ? Images.checkCircle : Images.standard_DirIcon
                        sourceSize: Qt.size(contentRoot.imageWidth, contentRoot.imageWidth)
                    }

                    MegaTexts.ElidedText {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.topMargin: 1
                        width: contentRoot.width - checkbox.width - contentRoot.checkboxSpacing
                               - contentRoot.imageTextSpacing - contentRoot.imageWidth -
                               (folderSize.visible ? (folderSize.width + contentRoot.checkboxSpacing) : 0) -
                               (busyIndicator.visible ? (busyIndicator.width + contentRoot.checkboxSpacing) : 0)

                        font.pixelSize: MegaTexts.Text.Size.Small
                        text: name
                        color: Styles.textPrimary
                    }
                }
            }

            MegaTexts.SecondaryText {
                id: folderSize

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                text: size
                font.pixelSize: MegaTexts.Text.Size.Small
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                color: Styles.textSecondary
                visible: backupsProxyModel.selectedFilterEnabled && sizeReady
            }

            MegaBusyIndicator.BusyIndicator {
                id: busyIndicator

                visible: backupsProxyModel.selectedFilterEnabled && !sizeReady
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: Styles.textAccent
                imageSize: Qt.size(12, 12)
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: backupsProxyModel.selectedFilterEnabled
                             ? Qt.ArrowCursor
                             : Qt.PointingHandCursor
                onClicked: {
                    selected = !selected;
                }
                enabled: !backupsProxyModel.selectedFilterEnabled
            }
        }
    }

    Component {
        id: conflictContent

        Item {
            id: contentRoot

            readonly property int contentMargin: 8
            readonly property int imageTextSpacing: 8
            readonly property int imageWidth: 16
            readonly property int textWidth: 248
            readonly property int sizeTextWidth: 50

            property bool showChange: error === backupsModel.BackupErrorCode.SyncConflict
                                        || error === backupsModel.BackupErrorCode.PathRelation
                                        || error === backupsModel.BackupErrorCode.UnavailableDir
                                        || error === backupsModel.BackupErrorCode.SDKCreation

            Row {
                id: imageText

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: contentRoot.contentMargin
                anchors.topMargin: contentRoot.contentMargin
                anchors.bottomMargin: contentRoot.contentMargin
                spacing: imageTextSpacing

                MegaImages.SvgImage {
                    anchors.verticalCenter: parent.verticalCenter
                    source: error === backupsModel.BackupErrorCode.SDKCreation
                            ? Images.alertCircle
                            : Images.alertTriangle
                    sourceSize: Qt.size(contentRoot.imageWidth, contentRoot.imageWidth)
                    color: error === backupsModel.BackupErrorCode.SDKCreation
                           ? Styles.textError
                           : Styles.textWarning
                }

                MegaTexts.ElidedText {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: contentRoot.width - contentRoot.imageTextSpacing - contentRoot.imageWidth
                            - buttonRow.width - contentRoot.contentMargin
                    font.pixelSize: MegaTexts.Text.Size.Small
                    text: name
                    color: error === backupsModel.BackupErrorCode.SDKCreation
                           ? Styles.textError
                           : Styles.textWarning
                    showTooltip: false
                }
            }

            MouseArea {
                hoverEnabled: true
                anchors.fill: imageText

                MegaToolTips.ToolTip {
                    visible: parent.containsMouse
                    leftIconSource: Images.pc
                    text: folder
                    delay: 500
                    timeout: 5000

                    onVisibleChanged: {
                        if(visible) {
                            if((parent.mouseX + width) > 363) {
                                x = 363 - width;
                            } else {
                                x = parent.mouseX;
                            }
                            y = parent.mouseY - height - 2;
                        }
                    }
                }
            }

            Item {
                id: buttonRow

                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: leftButton.width + removeButton.width - leftButton.sizes.focusBorderWidth

                MegaButtons.SecondaryButton {
                    id: leftButton

                    anchors.right: removeButton.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.rightMargin: -sizes.focusBorderWidth
                    text: contentRoot.showChange ? OnboardingStrings.changeFolder : OnboardingStrings.rename
                    icons.position: MegaButtons.Icon.Position.LEFT
                    icons.source: contentRoot.showChange ? "" : Images.edit
                    onClicked: {
                        if(contentRoot.showChange) {
                            folderDialog.openFolderSelector();
                        } else {
                            editMode = true;
                        }
                    }
                    sizes: MegaButtons.SmallSizes {}

                    ChooseLocalFolder {
                        id: folderDialog
                    }

                    Connections {
                        id: chooseLocalFolderConnection

                        target: folderDialog
                        function onFolderChoosen(folder) {
                            backupsModel.change(folder, folder);
                        }
                    }
                }

                MegaButtons.SecondaryButton {
                    id: removeButton

                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    icons.source: Images.trash
                    onClicked: {
                        backupsModel.remove(folder);
                    }
                    sizes: MegaButtons.SmallSizes {}
                }
            }

        }
    }

    Component {
        id: editContent

        Row {
            spacing: 2

            MegaTextFields.TextField {
                id: editTextField

                width: parent.width - parent.spacing - doneButton.width
                text: name
                leftIcon.source: Images.edit
                leftIcon.color: Styles.iconSecondary
                error: hint.visible
                sizes: MegaTextFields.SmallSizes {}
                validator: RegExpValidator { regExp: RegexExpressions.allowedFolderChars }

                onAccepted: {
                    doneAction()
                }
            }

            MegaButtons.PrimaryButton {
                id: doneButton

                text: OnboardingStrings.done
                sizes: MegaButtons.SmallSizes {}
                onClicked: {
                    if (editTextField.acceptableInput){
                        doneAction()
                    }
                }
            }

            function doneAction()
            {
                editTextField.hint.visible = false;
                var error = backupsModel.rename(folder, editTextField.text);
                switch(error) {
                    case backupsModel.BackupErrorCode.None:
                    case backupsModel.BackupErrorCode.SyncConflict:
                    case backupsModel.BackupErrorCode.PathRelation:
                    case backupsModel.BackupErrorCode.SDKCreation:
                        root.height = root.totalHeight;
                        break;
                    case backupsModel.BackupErrorCode.ExistsRemote:
                        editTextField.hint.text = OnboardingStrings.confirmBackupErrorRemote;
                        editTextField.hint.visible = true;
                        root.height = editTextField.height + root.extraMarginWhenHintShowed;
                        break;
                    case backupsModel.BackupErrorCode.DuplicatedName:
                        editTextField.hint.text = OnboardingStrings.confirmBackupErrorDuplicated;
                        editTextField.hint.visible = true;
                        root.height = editTextField.height + root.extraMarginWhenHintShowed;
                        break;
                    default:
                        root.height = root.totalHeight;
                        console.error("FolderRow: Unexpected error after rename -> " + error);
                        break;
                }
            }
        }
    }
}
