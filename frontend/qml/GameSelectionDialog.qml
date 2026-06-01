import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: selectionDlg
    title: "Game Lobby"
    modal: true
    width: 400
    height: 300

    signal gameSelected()

    ColumnLayout {
        anchors.fill: parent
        spacing: 20
        anchors.margins: 20

        Label {
            text: "Welcome, " + game.status + "!";
            font.bold: true; font.pixelSize: 16
            Layout.alignment: Qt.AlignHCenter
        }

        GroupBox {
            title: "Join Existing Game"
            Layout.fillWidth: true
            ColumnLayout {
                anchors.fill: parent
                Label { text: "Enter Game ID:" }
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: gameIdInput
                        Layout.fillWidth: true
                        placeholderText: "e.g. 7"
                        validator: IntValidator { bottom: 1 }
                    }
                    Button {
                        text: "Join"
                        onClicked: {
                            if (gameIdInput.text.length > 0) {
                                game.joinGame(parseInt(gameIdInput.text))
                            }
                        }
                    }
                }
            }
        }

        Label { text: "— OR —"; color: "#74b9ff"; Layout.alignment: Qt.AlignHCenter }

        GroupBox {
            title: "Create New Game"
            Layout.fillWidth: true
            Button {
                text: "🌍 Create New World"
                Layout.fillWidth: true
                onClicked: {
                    game.createGame()
                }
            }
        }
    }
}