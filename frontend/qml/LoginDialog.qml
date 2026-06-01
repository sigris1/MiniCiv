import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: dlg
    title: "MiniCiv Auth"
    modal: true
    width: 320
    height: 260
    signal loginCompleted(bool success, string msg)

    Connections {
        target: game
        function onLoginSuccess() { dlg.loginCompleted(true, "OK"); dlg.close() }
        function onErrorOccurred(m) { status.text = "️" + m; status.color = "#ff6b6b" }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        anchors.margins: 10

        Label { text: "Nick:"; font.bold: true }
        TextField { id: nick; Layout.fillWidth: true; placeholderText: "Enter nickname" }
        Label { text: "Password:"; font.bold: true }
        TextField { id: pass; Layout.fillWidth: true; placeholderText: "Enter password"; echoMode: TextInput.Password }
        Text { id: status; Layout.fillWidth: true; color: "#74b9ff"; horizontalAlignment: Text.AlignHCenter }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            Button { text: "Login"; Layout.fillWidth: true; onClicked: { if(nick.text && pass.text) { status.text="Checking..."; game.login(nick.text, pass.text) } } }
            Button { text: "Register"; Layout.fillWidth: true; onClicked: { if(nick.text.length>=2 && pass.text.length>=4) { status.text="Creating..."; game.registerUser(nick.text, pass.text) } else status.text="Nick≥2, Pass≥4" } }
        }
    }
}