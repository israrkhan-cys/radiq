import QtQuick
import QtQuick.Controls

Window {
    id: root
    width: 1280
    height: 720
    visible: false
    color: "transparent"
    title: "Radial Launcher Overlay"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property int activeIndex: 0
    property int searchListIndex: 0
    readonly property bool isSearching: appCatalog.searchQuery.length > 0

    onSearchListIndexChanged: {
        if (isSearching) {
            searchListView.positionViewAtIndex(searchListIndex, ListView.Contain)
        }
    }

    onVisibleChanged: {
        if (visible) {
            searchField.text = "";
            appCatalog.searchQuery = "";
            activeIndex = 0;
            searchListIndex = 0;
            mainWrapper.active = true;
            searchField.forceActiveFocus();
        }
    }

    Connections {
        target: overlayController
        function onCloseRequested() {
            mainWrapper.active = false;
        }
    }

    Item {
        id: mainWrapper
        anchors.fill: parent
        opacity: 0
        scale: 0.95

        property bool active: false

        states: [
            State {
                name: "visible"
                when: root.visible && mainWrapper.active
                PropertyChanges { target: mainWrapper; opacity: 1; scale: 1.0 }
            },
            State {
                name: "hidden"
                when: !root.visible || !mainWrapper.active
                PropertyChanges { target: mainWrapper; opacity: 0; scale: 0.95 }
            }
        ]

        transitions: [
            Transition {
                from: "hidden"
                to: "visible"
                NumberAnimation { properties: "opacity,scale"; duration: 150; easing.type: Easing.OutQuad }
            },
            Transition {
                from: "visible"
                to: "hidden"
                SequentialAnimation {
                    NumberAnimation { properties: "opacity,scale"; duration: 120; easing.type: Easing.InQuad }
                    ScriptAction { script: { overlayController.finishHide(); } }
                }
            }
        ]

        // Background Scrim (clicking here closes launcher)
        Rectangle {
            anchors.fill: parent
            color: "#CC0F0F15" // Semi-transparent premium dark scrim

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mainWrapper.active = false;
                }
            }
        }

        // Radial Wheel Container
        Item {
            id: wheelContainer
            anchors.centerIn: parent
            width: 600
            height: 600
            opacity: isSearching ? 0 : 1
            scale: isSearching ? 0.8 : 1
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
            Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

            // Inner Center Circle
            Rectangle {
                anchors.centerIn: parent
                width: 160
                height: 160
                radius: width / 2
                color: "#1A1B26"
                border.color: "#2C2E3E"
                border.width: 2

                Column {
                    anchors.centerIn: parent
                    width: 140
                    spacing: 4

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: activeIndex >= 0 && activeIndex < appCatalog.wheelApps.length ? appCatalog.wheelApps[activeIndex].name : ""
                        color: "white"
                        font.family: "Inter, Roboto, sans-serif"
                        font.pixelSize: 18
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }
            }

            // Wheel slots
            Repeater {
                model: appCatalog.wheelApps

                delegate: Item {
                    id: slot
                    readonly property real angle: -Math.PI / 2 + index * (2 * Math.PI / appCatalog.wheelApps.length)
                    readonly property real radius: 180

                    x: wheelContainer.width / 2 + radius * Math.cos(angle) - width / 2
                    y: wheelContainer.height / 2 + radius * Math.sin(angle) - height / 2

                    width: 100
                    height: 100

                    readonly property bool isActive: index === activeIndex

                    scale: isActive ? 1.3 : 1.0
                    opacity: isActive ? 1.0 : (activeIndex === -1 ? 1.0 : 0.6)

                    Behavior on scale { NumberAnimation { duration: 130; easing.type: Easing.OutQuad } }
                    Behavior on opacity { NumberAnimation { duration: 130; easing.type: Easing.OutQuad } }

                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: isActive ? "#2D3F76" : "#1F2335"
                        border.color: isActive ? "#7AA2F7" : "#414868"
                        border.width: isActive ? 2 : 1

                        Behavior on color { ColorAnimation { duration: 100 } }
                        Behavior on border.color { ColorAnimation { duration: 100 } }

                        Item {
                            anchors.fill: parent
                            anchors.margins: 22

                            Image {
                                anchors.fill: parent
                                source: modelData.hasIcon ? "image://appicon/" + modelData.iconName : ""
                                visible: modelData.hasIcon
                                fillMode: Image.PreserveAspectFit
                            }

                            Rectangle {
                                anchors.fill: parent
                                visible: !modelData.hasIcon
                                color: "transparent"

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.firstLetter
                                    color: isActive ? "#7AA2F7" : "white"
                                    font.family: "Inter, Roboto, sans-serif"
                                    font.bold: true
                                    font.pixelSize: 28
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: activeIndex = index
                            onClicked: {
                                appCatalog.launch(modelData.desktopId);
                                mainWrapper.active = false;
                            }
                        }
                    }
                }
            }
        }

        // Fuzzy Search Results Panel (Fades in over/instead of the wheel)
        Item {
            id: searchResultsContainer
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -40
            width: 550
            height: 380
            opacity: isSearching ? 1 : 0
            scale: isSearching ? 1 : 0.8
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
            Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }

            Rectangle {
                anchors.fill: parent
                color: "#1A1B26"
                radius: 16
                border.color: "#2C2E3E"
                border.width: 1
                clip: true

                ListView {
                    id: searchListView
                    anchors.fill: parent
                    anchors.margins: 12
                    model: appCatalog
                    spacing: 6
                    currentIndex: searchListIndex

                    onCountChanged: {
                        if (searchListIndex >= count) {
                            searchListIndex = Math.max(0, count - 1);
                        }
                    }

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 58
                        radius: 8

                        readonly property bool isCurrent: index === searchListIndex

                        color: isCurrent ? "#24283B" : "transparent"
                        border.color: isCurrent ? "#7AA2F7" : "transparent"
                        border.width: 1

                        Row {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 14

                            Item {
                                width: 42
                                height: 42
                                anchors.verticalCenter: parent.verticalCenter

                                Image {
                                    anchors.fill: parent
                                    source: model.hasIcon ? "image://appicon/" + model.iconName : ""
                                    visible: model.hasIcon
                                    fillMode: Image.PreserveAspectFit
                                }

                                Rectangle {
                                    anchors.fill: parent
                                    visible: !model.hasIcon
                                    color: "#1F2335"
                                    radius: width / 2

                                    Text {
                                        anchors.centerIn: parent
                                        text: model.firstLetter
                                        color: "white"
                                        font.family: "Inter, Roboto, sans-serif"
                                        font.bold: true
                                        font.pixelSize: 18
                                    }
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2

                                Text {
                                    text: model.name
                                    color: "white"
                                    font.family: "Inter, Roboto, sans-serif"
                                    font.bold: true
                                    font.pixelSize: 15
                                }

                                Text {
                                    text: model.exec
                                    color: "#787C99"
                                    font.family: "Inter, Roboto, sans-serif"
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                    width: 440
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: searchListIndex = index
                            onClicked: {
                                appCatalog.launch(model.desktopId);
                                mainWrapper.active = false;
                            }
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "No applications found"
                    color: "#565F89"
                    font.family: "Inter, Roboto, sans-serif"
                    font.pixelSize: 16
                    visible: searchListView.count === 0
                }
            }
        }

        // Docked Search Bar
        Rectangle {
            id: searchBarContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 80
            width: 450
            height: 52
            color: "#1A1B26"
            radius: 26
            border.color: searchField.activeFocus ? "#7AA2F7" : "#2C2E3E"
            border.width: 1

            Behavior on border.color { ColorAnimation { duration: 150 } }

            Row {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 10

                Item {
                    width: 40
                    height: 40
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: "🔍"
                        font.pixelSize: 18
                        color: searchField.activeFocus ? "#7AA2F7" : "#565F89"
                    }
                }

                TextField {
                    id: searchField
                    width: parent.width - 60
                    height: parent.height
                    verticalAlignment: TextInput.AlignVCenter
                    font.family: "Inter, Roboto, sans-serif"
                    font.pixelSize: 16
                    color: "white"
                    placeholderText: "Type to search..."
                    placeholderTextColor: "#565F89"

                    background: Rectangle {
                        color: "transparent"
                    }

                    selectByMouse: true

                    Keys.onPressed: function(event) {
                        if (event.key === Qt.Key_Escape) {
                            if (text.length > 0) {
                                text = "";
                            } else {
                                mainWrapper.active = false;
                            }
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Down || event.key === Qt.Key_Right) {
                            if (isSearching) {
                                if (searchListView.count > 0) {
                                    searchListIndex = (searchListIndex + 1) % searchListView.count;
                                }
                            } else {
                                activeIndex = (activeIndex + 1) % appCatalog.wheelApps.length;
                            }
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Up || event.key === Qt.Key_Left) {
                            if (isSearching) {
                                if (searchListView.count > 0) {
                                    searchListIndex = (searchListIndex - 1 + searchListView.count) % searchListView.count;
                                }
                            } else {
                                activeIndex = (activeIndex - 1 + appCatalog.wheelApps.length) % appCatalog.wheelApps.length;
                            }
                            event.accepted = true;
                        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            if (isSearching) {
                                if (searchListView.count > 0 && searchListIndex >= 0 && searchListIndex < searchListView.count) {
                                    var desktopId = appCatalog.desktopIdAt(searchListIndex);
                                    if (desktopId) {
                                        appCatalog.launch(desktopId);
                                        mainWrapper.active = false;
                                    }
                                }
                            } else {
                                if (activeIndex >= 0 && activeIndex < appCatalog.wheelApps.length) {
                                    var app = appCatalog.wheelApps[activeIndex];
                                    appCatalog.launch(app.desktopId);
                                    mainWrapper.active = false;
                                }
                            }
                            event.accepted = true;
                        }
                    }

                    onTextChanged: {
                        appCatalog.searchQuery = text;
                        searchListIndex = 0;
                    }
                }
            }
        }
    }
}
