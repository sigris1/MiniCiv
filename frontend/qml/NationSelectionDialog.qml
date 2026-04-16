import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: nationDlg
    title: "Choose Your Nation"
    modal: true
    width: 480
    height: 420

    signal nationSelected(int tribeId)

    property var nationNames: [
        "Climbers", "FruitCollectors", "Hunters", "Riders",
        "Fishermen", "Archers", "Rich", "Swordsmen",
        "Farmers", "Peacemakers", "ShieldBearers", "RoadCreators"
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label {
            text: "Select a tribe to join the game:"
            font.pixelSize: 14
            color: "#74b9ff"
            Layout.alignment: Qt.AlignHCenter
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            GridLayout {
                columns: 3
                rowSpacing: 8
                columnSpacing: 8
                Layout.fillWidth: true

                Repeater {
                    model: nationDlg.nationNames.length
                    Button {
                        Layout.fillWidth: true
                        text: nationDlg.nationNames[index]
                        onClicked: {
                            console.log("[Nation] Selected:", nationDlg.nationNames[index], "(ID:", index, ")")
                            nationDlg.nationSelected(index)
                        }
                    }
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Random Nation"
            highlighted: true
            font.bold: true
            onClicked: {
                var randomIndex = Math.floor(Math.random() * nationDlg.nationNames.length)
                console.log("[Nation] Random selected:", nationDlg.nationNames[randomIndex], "(ID:", randomIndex, ")")
                nationDlg.nationSelected(randomIndex)
            }
        }
    }
}