import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import MiniCiv 1.0
import "qrc:/"

ApplicationWindow {
    id: root
    visible: true
    width: 1100
    height: 800
    title: "MiniCiv Client"

    property point selectedUnitPos: Qt.point(-1, -1)
    property point selectedCityPos: Qt.point(-1, -1)
    property string appState: "IDLE"
    property string errorMsg: ""
    property bool showError: false

    Timer {
        id: errTimer
        interval: 3000
        running: showError
        onTriggered: showError = false
    }

    Connections {
        target: game

        function onErrorOccurred(msg) {
            errorMsg = msg
            showError = true
            errTimer.restart()
        }

        function onGameCreated(newGameId) {
            console.log("[QML] 🎉 Game CREATED:", newGameId)
            selectionDlg.close()
            if (game.myTribeId <= 0) {
                nationDlg.open()
            } else {
                game.startPolling(50000000)
            }
        }

        function onGameJoined() {
            console.log("[QML] 🎉 Game JOINED")
            selectionDlg.close()
            if (game.myTribeId <= 0) {
                nationDlg.open()
            } else {
                game.startPolling(500)
            }
        }

        function onTribeConfirmedChanged() {
            if (game.isTribeConfirmed && game.myTribeId > 0) {
                console.log("[QML] ✅ Tribe confirmed! Showing game...")
                gameContent.visible = true
                game.startPolling(500)
            }
        }

        function onGameLeft() {
            console.log("[QML] 🚪 Game left. Returning to Lobby.")
            gameContent.visible = false
            selectedUnitPos = Qt.point(-1, -1)
            selectedCityPos = Qt.point(-1, -1)
            appState = "IDLE"
            selectionDlg.open()
        }
    }

    LoginDialog {
        id: loginDlg
        visible: true
        onLoginCompleted: (ok, _) => { if(ok) { loginDlg.close(); selectionDlg.open() } }
    }

    GameSelectionDialog { id: selectionDlg; visible: false }

    NationSelectionDialog {
        id: nationDlg
        visible: false
        onNationSelected: (tribeId) => {
            console.log("[QML] 🏳️ Selected Tribe:", tribeId)
            nationDlg.close()
            game.selectTribe(tribeId)
        }
    }

    Item {
        id: gameContent
        visible: false
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8

            RowLayout {
                Rectangle {
                    color: !game.isTribeConfirmed ? "#fdcb6e" : (game.isMyTurn ? "#00b894" : "#d63031")
                    radius: 4; width: 12; height: 12
                }
                Text {
                    text: !game.isTribeConfirmed ? "⏳ Confirming tribe..." :
                        (game.isMyTurn ? "🟢 YOUR TURN" : "🔴 OPPONENT'S TURN")
                    font.pixelSize: 14; font.bold: true
                    color: !game.isTribeConfirmed ? "#fdcb6e" : (game.isMyTurn ? "#00b894" : "#d63031")
                }
                Text { text: " | Balance: " + game.balance + " 💰"; font.pixelSize: 14; color: "#ffeaa7" }
                Text { text: " | Turn: " + game.currentPlayer; font.pixelSize: 14; color: "#74b9ff" }

                Item { Layout.fillWidth: true }

                Button {
                    text: "⏭️ End Turn"
                    enabled: game.isMyTurn && game.isTribeConfirmed
                    opacity: enabled ? 1.0 : 0.5
                    highlighted: enabled
                    onClicked: game.endTurn()
                }

                Button {
                    text: "🔄 Reconnect"
                    visible: showError
                    onClicked: { showError = false; game.reconnect() }
                }

                Button {
                    text: "🚪 Leave Game"
                    onClicked: game.leaveGame()
                }

                Button {
                    text: "🔬 Tech"
                    enabled: game.isMyTurn && game.isTribeConfirmed
                    opacity: enabled ? 1.0 : 0.5
                    onClicked: appState = "TECH_TREE"
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#0c0c1a"
                clip: true

                MapRenderer {
                    id: map
                    anchors.fill: parent
                    mapSize: 20
                    zoom: 40
                    tiles: game.tiles
                    units: game.units
                    myTribeId: game.isTribeConfirmed ? game.myTribeId : -1
                    onTileClicked: (x, y) => handleTileClick(x, y)
                }

                Item {
                    id: cityMenu
                    visible: appState === "CITY_MENU" && game.isMyTurn && game.isTribeConfirmed
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.margins: 20
                    width: 500; height: 80

                    Rectangle {
                        anchors.fill: parent
                        color: "#2d3436"; radius: 8; border.color: "#ffd700"; border.width: 2
                        Text {
                            text: "🏙️ City Production"; color: "white"; font.bold: true
                            anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 8
                        }
                        RowLayout {
                            anchors.centerIn: parent; spacing: 6
                            Repeater {
                                model: game.availableUnits.length > 0 ? game.availableUnits : ["Warrior", "Archer"]
                                delegate: Button {
                                    text: {
                                        if (modelData === "Warrior") return "⚔️ Warrior (2g)"
                                        if (modelData === "Archer") return "🏹 Archer (3g)"
                                        if (modelData === "Swordsman") return "🗡️ Swordsman (4g)"
                                        if (modelData === "Knight") return "🐎 Knight (5g)"
                                        return "🛡️ " + modelData
                                    }
                                    onClicked: {
                                        var unitTypeIndex = 0
                                        var cost = 2

                                        if (modelData === "Warrior") { unitTypeIndex = 0; cost = 2 }
                                        else if (modelData === "Archer") { unitTypeIndex = 1; cost = 3 }
                                        else if (modelData === "Swordsman") { unitTypeIndex = 2; cost = 4 }
                                        else if (modelData === "Knight") { unitTypeIndex = 3; cost = 5 }

                                        confirm("Hire " + modelData,
                                            "Recruit " + modelData + " for " + cost + " gold?",
                                                () => {
                                                game.recruitUnit(unitTypeIndex, selectedCityPos.x, selectedCityPos.y)
                                                confirmDlg.close()
                                            })
                                    }
                                }
                            }
                            Button { text: ""; onClicked: resetState() }
                        }
                    }
                }

                Item {
                    id: techOverlay
                    visible: appState === "TECH_TREE" && game.isMyTurn && game.isTribeConfirmed
                    anchors.fill: parent
                    Rectangle { anchors.fill: parent; color: "#00000090" }
                    Rectangle {
                        anchors.centerIn: parent; width: 420; height: 540
                        color: "#1a1a2e"; radius: 10; border.color: "#74b9ff"; border.width: 2
                        Text {
                            text: "🔬 Technology Tree"; color: "white"; font.bold: true; font.pixelSize: 16
                            anchors.top: parent.top; anchors.horizontalCenter: parent.horizontalCenter; anchors.topMargin: 15
                        }
                        property var techNames: [
                            [],
                            ["", "Organization", "Climbing", "Fishing", "Hunting", "Riding", "", "", "", "", ""],
                            ["", "Farming", "Strategy", "Mining", "Meditation", "Scouting", "Ramming", "Archery", "Forestry", "Roads", "FreeSpirit"],
                            ["", "Construction", "Diplomacy", "Smithery", "Philosophy", "Navigation", "Aquatism", "Spiritualism", "Mathematics", "Trade", "Chivalry"]
                        ]
                        Row { anchors.centerIn: parent; anchors.topMargin: 10
                            Column { spacing: 5
                                Repeater {
                                    model: 5
                                    Button {
                                        width: 110; height: 40
                                        text: techOverlay.techNames[1][index + 1]
                                        property int tx: 1; property int ty: index + 1
                                        enabled: techOverlay.techNames[1][index + 1] !== ""
                                        onClicked: confirmTech(tx, ty)
                                    }
                                }
                            }
                            Column { spacing: 5
                                Repeater {
                                    model: 10
                                    Button {
                                        width: 110; height: 40
                                        text: techOverlay.techNames[2][index + 1]
                                        property int tx: 2; property int ty: index + 1
                                        enabled: techOverlay.techNames[2][index + 1] !== ""
                                        onClicked: confirmTech(tx, ty)
                                    }
                                }
                            }
                            Column { spacing: 5
                                Repeater {
                                    model: 10
                                    Button {
                                        width: 110; height: 40
                                        text: techOverlay.techNames[3][index + 1]
                                        property int tx: 3; property int ty: index + 1
                                        enabled: techOverlay.techNames[3][index + 1] !== ""
                                        onClicked: confirmTech(tx, ty)
                                    }
                                }
                            }
                        }
                        Button {
                            text: "Close"
                            anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 15
                            onClicked: appState = "IDLE"
                        }
                    }
                }

                Text {
                    visible: appState === "MOVING" && game.isMyTurn && game.isTribeConfirmed
                    anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.margins: 20
                    color: "#00ff00"; font.pixelSize: 14; font.bold: true
                    text: "👆 Click tile to Move"
                }

                Item {
                    visible: (!game.isMyTurn || !game.isTribeConfirmed) && game.myTribeId > 0
                    anchors.fill: parent
                    Rectangle { anchors.fill: parent; color: "#00000060" }
                    Label {
                        anchors.centerIn: parent
                        text: !game.isTribeConfirmed ? "⏳ Waiting for tribe confirmation..." : "⏳ Waiting for opponent's turn..."
                        color: "#ffffff"; font.pixelSize: 18; font.bold: true
                    }
                }
            }
        }
    }

    Item {
        visible: showError
        anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; anchors.bottomMargin: 40
        Rectangle { color: "#ff7675"; radius: 6
            Text { anchors.centerIn: parent; anchors.margins: 10; text: "️ " + errorMsg; color: "white"; font.bold: true; wrapMode: Text.Wrap }
        }
    }

    Dialog {
        id: confirmDlg
        modal: true
        title: "Confirm Action"
        property var onConfirm: null
        ColumnLayout { anchors.fill: parent; spacing: 10
            Label { id: confirmText; Layout.fillWidth: true; wrapMode: Text.Wrap }
            RowLayout { Layout.fillWidth: true
                Button { text: "Yes"; Layout.fillWidth: true; onClicked: { if(confirmDlg.onConfirm) confirmDlg.onConfirm(); confirmDlg.close() } }
                Button { text: "No"; Layout.fillWidth: true; onClicked: confirmDlg.close() }
            }
        }
    }

    function handleTileClick(x, y) {
        if (!game.isMyTurn || !game.isTribeConfirmed) return

        var tile = getTileData(x, y)
        if (!tile) return

        var unitExists = isUnitAt(x, y)

        if (appState === "MOVING") {
            confirm("Move Unit", "Move to (" + x + "," + y + ")?", () => {
                game.moveUnit(selectedUnitPos.x, selectedUnitPos.y, x, y)
                resetState()
            })
            return
        }

        if (unitExists) {
            var u = getUnitAt(x, y)
            if (u && u.tribe === game.myTribeId) {
                selectedUnitPos = Qt.point(x, y)
                map.selectedUnitPos = selectedUnitPos
                appState = "MOVING"
            }
            return
        }

        if (tile["hasCity"] === true && tile["cityOwner"] === game.myTribeId) {
            selectedCityPos = Qt.point(x, y)
            appState = "CITY_MENU"
            return
        }

        if (tile["resources"] && tile["resources"].length > 0) {
            var resName = tile["resources"][0]
            confirm("Collect Resource", "Collect " + resName + " from (" + x + "," + y + ")?", () => {
                game.sendAction(6, 1, x, y, 0, 0)
            })
            return
        }

        if (!tile["hasCity"] && !tile["hasUnit"] && (!tile["resources"] || tile["resources"].length === 0)) {
        }

        resetState()
    }

    function getUnitAt(x, y) {
        if (!game.units || game.units.length === 0) return null
        for (var i = 0; i < game.units.length; i++) {
            var u = game.units[i]
            if (u["x"] === x && u["y"] === y) return u
        }
        return null
    }

    function isUnitAt(x, y) {
        if (!game.units || game.units.length === 0) return false
        for (var i = 0; i < game.units.length; i++) {
            var u = game.units[i]
            if (u["x"] === x && u["y"] === y) return true
        }
        return false
    }

    function confirmTech(x, y) {
        var techName = techOverlay.techNames[x][y]
        confirm("Research " + techName, "Research at Col " + x + ", Row " + y + "?", () => {
            appState = "IDLE"
        })
    }

    function resetState() {
        selectedUnitPos = Qt.point(-1, -1)
        selectedCityPos = Qt.point(-1, -1)
        map.selectedUnitPos = Qt.point(-1, -1)
        appState = "IDLE"
    }

    function getTileData(x, y) {
        if (!game.tiles || game.tiles.length === 0) return null
        for (var i = 0; i < game.tiles.length; i++) {
            var t = game.tiles[i]
            if (t.x === x && t.y === y) return t
        }
        return null
    }

    function confirm(title, msg, callback) {
        confirmDlg.title = title
        confirmText.text = msg
        confirmDlg.onConfirm = callback
        confirmDlg.open()
    }
}