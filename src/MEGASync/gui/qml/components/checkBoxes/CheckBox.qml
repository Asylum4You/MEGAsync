import QtQuick 2.12
import Common 1.0
import QtQuick.Controls 2.12 as Qml
import Components 1.0

Qml.CheckBox {
    id: checkBox

    property string url: ""
    property bool indeterminate: false

    spacing: richText.text !== "" ? 8 : 0
    indicator: checkBoxOutRect
    contentItem: richText
    padding: 0

    MouseArea {
        id: mouseArea

        anchors.fill: checkBox
        onPressed: mouse.accepted = false
        cursorShape: Qt.PointingHandCursor
    }

    RichText {
        id: richText

        text: checkBox.text
        horizontalAlignment: Text.AlignLeft
        leftPadding: checkBox.indicator.width + checkBox.spacing
        anchors.top: parent.top
        wrapMode: RichText.Wrap
        font.pixelSize: 12
        url: checkBox.url
    }

    Rectangle {
        id: checkBoxOutRect

        function getBorderColor() {
            var color;
            if(checkBox.pressed) {
                color = Styles.buttonPrimaryPressed;
            } else if(checkBox.hovered) {
                color = Styles.buttonPrimaryHover;
            } else {
                color = Styles.buttonPrimary;
            }
            return color;
        }

        function getBackgroundColor() {
            var color;
            if(checkBox.pressed) {
                if(checkBox.checked) {
                    color = Styles.buttonPrimaryPressed;
                } else {
                    color = "transparent";
                }
            } else if(checkBox.hovered) {
                color = Styles.buttonPrimaryHover;
            } else {
                color = Styles.buttonPrimary;
            }
            return color;
        }

        function getIconColor() {
            var color;
            if(checkBox.pressed && !checkBox.checked) {
                color = Styles.buttonPrimaryPressed;
            } else {
                color = Styles.iconOnColor;
            }
            return color;
        }

        width: 16
        height: 16
        radius: 4
        border.color: checkBoxOutRect.getBorderColor()
        border.width: 2

        Rectangle {
            id: inside

            visible: checkBox.checked || checkBox.down || checkBox.indeterminate
            color: checkBoxOutRect.getBackgroundColor()
            radius: 1
            width: checkBoxOutRect.width - checkBoxOutRect.border.width
            height: inside.width
            anchors.centerIn: checkBoxOutRect

            SvgImage {
                id: image

                source: checkBox.indeterminate ? "images/indeterminate.svg" : "images/check.svg"
                anchors.centerIn: inside
                sourceSize: indeterminate ? Qt.size(8, 2) : Qt.size(8, 6.5)
                color: checkBoxOutRect.getIconColor()
            }

        } // Rectangle -> inside

    } // Rectangle -> checkBoxOutRect

}
