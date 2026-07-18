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
        if (isSearching && searchListView) {
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

            // Math helper for mapping mouse coordinate to wedge index
            function getSlotIndex(mx, my) {
                var dx = mx - wheelContainer.width / 2;
                var dy = my - wheelContainer.height / 2;
                var dist = Math.sqrt(dx * dx + dy * dy);
                if (dist < 100 || dist > 230) {
                    return -1; // outside the ring
                }
                
                var angle = Math.atan2(dy, dx);
                var step = 2 * Math.PI / appCatalog.wheelApps.length;
                var normalizedAngle = angle + Math.PI / 2 + (step / 2);
                if (normalizedAngle < 0) {
                    normalizedAngle += 2 * Math.PI;
                }
                
                var idx = Math.floor(normalizedAngle / step) % appCatalog.wheelApps.length;
                return idx;
            }

            // Mouse interaction area for the wedges
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                
                onPositionChanged: function(mouse) {
                    var idx = wheelContainer.getSlotIndex(mouse.x, mouse.y);
                    if (idx !== -1) {
                        activeIndex = idx;
                    }
                }
                
                onClicked: function(mouse) {
                    var idx = wheelContainer.getSlotIndex(mouse.x, mouse.y);
                    if (idx !== -1) {
                        var app = appCatalog.wheelApps[idx];
                        appCatalog.launch(app.desktopId);
                        mainWrapper.active = false;
                    }
                }
            }

            // Wheel slots (Wedges and Icons)
            Repeater {
                model: appCatalog.wheelApps

                delegate: Item {
                    id: slot
                    anchors.fill: parent

                    readonly property bool isActive: index === activeIndex
                    readonly property real angleStep: 2 * Math.PI / appCatalog.wheelApps.length
                    readonly property real midAngle: -Math.PI / 2 + index * angleStep
                    
                    // Angles for Canvas (with a small gap)
                    readonly property real gapAngle: 0.03
                    readonly property real canvasStartAngle: midAngle - angleStep / 2 + gapAngle
                    readonly property real canvasEndAngle: midAngle + angleStep / 2 - gapAngle

                    // Canvas to draw the wedge segment
                    Canvas {
                        id: wedgeCanvas
                        anchors.fill: parent
                        
                        property color fillColor: isActive ? "#2D3F76" : "#CC1F2335"
                        property color strokeColor: isActive ? "#7AA2F7" : "#414868"
                        property real scaleFactor: isActive ? 1.04 : 1.0
                        
                        Behavior on fillColor { ColorAnimation { duration: 130 } }
                        Behavior on strokeColor { ColorAnimation { duration: 130 } }
                        Behavior on scaleFactor { NumberAnimation { duration: 130; easing.type: Easing.OutQuad } }
                        
                        onFillColorChanged: requestPaint()
                        onStrokeColorChanged: requestPaint()
                        onScaleFactorChanged: requestPaint()

                        onPaint: {
                            var ctx = getContext("2d");
                            ctx.reset();
                            ctx.clearRect(0, 0, width, height);
                            
                            var centerX = width / 2;
                            var centerY = height / 2;
                            
                            ctx.save();
                            if (scaleFactor !== 1.0) {
                                ctx.translate(centerX, centerY);
                                ctx.scale(scaleFactor, scaleFactor);
                                ctx.translate(-centerX, -centerY);
                            }
                            
                            var rInner = 100;
                            var rOuter = 230;
                            
                            ctx.beginPath();
                            ctx.arc(centerX, centerY, rOuter, canvasStartAngle, canvasEndAngle, false);
                            ctx.lineTo(centerX + rInner * Math.cos(canvasEndAngle), centerY + rInner * Math.sin(canvasEndAngle));
                            ctx.arc(centerX, centerY, rInner, canvasEndAngle, canvasStartAngle, true);
                            ctx.closePath();
                            
                            ctx.fillStyle = fillColor;
                            ctx.fill();
                            
                            ctx.lineWidth = isActive ? 2.5 : 1.5;
                            ctx.strokeStyle = strokeColor;
                            ctx.stroke();
                            
                            ctx.restore();
                        }
                    }

                    // Icon / Text Fallback positioned at wedge midpoint radius
                    Item {
                        id: iconContainer
                        width: 56
                        height: 56
                        
                        readonly property real radius: 165
                        scale: isActive ? 1.25 : 1.0
                        opacity: isActive ? 1.0 : (activeIndex === -1 ? 1.0 : 0.6)
                        
                        x: wheelContainer.width / 2 + radius * Math.cos(midAngle) - width / 2
                        y: wheelContainer.height / 2 + radius * Math.sin(midAngle) - height / 2
                        
                        Behavior on scale { NumberAnimation { duration: 130; easing.type: Easing.OutQuad } }
                        Behavior on opacity { NumberAnimation { duration: 130; easing.type: Easing.OutQuad } }

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
                                font.pixelSize: 26
                            }
                        }
                    }
                }
            }
        }

        // Inner Center Circle & Search Input (Stays in center)
        Rectangle {
            id: centerCircle
            anchors.centerIn: parent
            width: 180
            height: 180
            radius: width / 2
            color: "#1A1B26"
            border.color: searchField.activeFocus ? "#7AA2F7" : "#2C2E3E"
            border.width: 2
            
            Behavior on border.color { ColorAnimation { duration: 150 } }

            Column {
                anchors.centerIn: parent
                width: 150
                spacing: 8

                // Highlighted App Name
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: activeIndex >= 0 && activeIndex < appCatalog.wheelApps.length ? appCatalog.wheelApps[activeIndex].name : ""
                    color: "white"
                    font.family: "Inter, Roboto, sans-serif"
                    font.pixelSize: 14
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                    width: parent.width
                    opacity: isSearching ? 0.3 : 1.0
                    Behavior on opacity { NumberAnimation { duration: 100 } }
                }

                // Search Input Field
                TextField {
                    id: searchField
                    width: parent.width
                    height: 36
                    horizontalAlignment: TextInput.AlignHCenter
                    verticalAlignment: TextInput.AlignVCenter
                    font.family: "Inter, Roboto, sans-serif"
                    font.pixelSize: 14
                    color: "white"
                    placeholderText: "Search..."
                    placeholderTextColor: "#565F89"

                    background: Rectangle {
                        color: "#24283B"
                        radius: 18
                        border.color: searchField.activeFocus ? "#7AA2F7" : "#414868"
                        border.width: 1
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

                // Search Status / Count Hint
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: isSearching ? (searchListView.count + " matches") : "Active slot"
                    color: "#565F89"
                    font.family: "Inter, Roboto, sans-serif"
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // Fuzzy Search Results Panel (Fades in below/around the center circle)
        Item {
            id: searchResultsContainer
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: parent.height / 2 + 105
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 40
            width: 550
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
    }
}
