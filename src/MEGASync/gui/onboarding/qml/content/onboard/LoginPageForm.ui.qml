// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Components.Texts 1.0 as MegaTexts
import Components.TextFields 1.0 as MegaTextFields
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0
import LoginController 1.0
import ApiEnums 1.0

StackViewPage {
    id: root

    property alias signUpButton: signUpButton
    property alias loginButton: loginButton

    property alias email: email
    property alias password: password

    Column {
        anchors.verticalCenter: root.verticalCenter
        anchors.left: root.left
        anchors.right: root.right
        spacing: contentSpacing

        MegaTexts.RichText {
            id: pageTitle

            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Large
            text: LoginControllerAccess.state === LoginController.EMAIL_CONFIRMED
                  ? OnboardingStrings.confirmEmailAndPassword
                  : OnboardingStrings.loginTitle
        }

        MegaTexts.RichText {
            id: confirmText

            visible: LoginControllerAccess.state === LoginController.EMAIL_CONFIRMED;
            anchors.left: parent.left
            anchors.right: parent.right
            font.pixelSize: MegaTexts.Text.Size.Medium
            text: OnboardingStrings.accountWillBeActivated
        }

        MegaTextFields.EmailTextField {
            id: email

            width: parent.width + 2 * email.sizes.focusBorderWidth
            anchors.left: parent.left
            anchors.leftMargin: -email.sizes.focusBorderWidth
            title: OnboardingStrings.email
            text: LoginControllerAccess.email
            error: LoginControllerAccess.loginError !== ApiEnums.API_OK;
        }

        MegaTextFields.PasswordTextField {
            id: password

            width: parent.width + 2 * password.sizes.focusBorderWidth
            anchors.left: parent.left
            anchors.leftMargin: -password.sizes.focusBorderWidth
            title: OnboardingStrings.password
            hint.icon: Images.alertTriangle
            error: LoginControllerAccess.loginError !== ApiEnums.API_OK ;
            hint.text: LoginControllerAccess.loginErrorMsg;
            hint.visible: LoginControllerAccess.loginErrorMsg.length !== 0;
        }

        MegaButtons.HelpButton {
            anchors.left: parent.left
            text: OnboardingStrings.forgotPassword
            url: Links.recovery
        }
    }

    RowLayout {
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.bottomMargin: 29
        anchors.left: root.left

        MegaButtons.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: -signUpButton.sizes.focusBorderWidth
        }

        MegaButtons.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
            progress.value: LoginControllerAccess.progress
            Layout.rightMargin: -loginButton.sizes.focusBorderWidth
        }
    }
}
