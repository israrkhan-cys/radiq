import QtQuick
import QtQuick.Controls

// =====================================================================
// ASSUMPTIONS — verify these against your actual backend, adjust if named
// differently. Everything else in this file doesn't depend on these names.
//
//   1. Context property exposing the app catalog is called `appCatalogModel`
//      - exposes a list-model interface with roles: `name` (string) and
//        `iconSource` (url string, e.g. "image://appicon/<id>")
//      - has Q_INVOKABLE launch(int index)
//      - has a writable property `filterText` (string) that, when set,
//        re-populates the model's rows with fuzzy-filtered results
//        (clearing it / setting "" restores the default hardcoded set)
//   2. Context property `overlayController` has Q_INVOKABLE hide()
//      already wired to the fade+scale-out transition
// If any of these don't match your real names, just search/replace them
// below — nothing else needs to change.
// =====================================================================

Window {
    id: root
    width: 1280
    height: 720
    visible: false
    color: "transparent"
    title: "Radial Launcher Overlay"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint

    property int currentIndex: 0
    property real ringOuterRadius: 260
    property real ringInnerRadius: 130
    property real gapDegrees: 3

    function itemCount() {
        return appCatalogModel ? appCatalogModel.rowCount() : 0
    }

    function angleForIndex(i, count) {
        // 0 index sits at the top (-90deg), evenly spaced clockwise
        var slice = 360 / Math.max(count, 1)
        return (i * slice) - 90
    }

    function moveHighlight(delta) {
        var count = itemCount()
        if (count === 0) return
        currentIndex = (currentIndex + delta + count) % count
        wheelCanvas.requestPaint()
    }

    function launchCurrent() {
        if (itemCount() === 0) return
        appCatalogModel.launch(currentIndex)
    }

    function hideOverlay() {
        overlayController.hide()
    }

    // dark scrim background, click to dismiss
    Rectangle {
        id: scrim
        anchors.fill: parent
        color: "#80000000"
        MouseArea {
            anchors.fill: parent
            onClicked: root.hideOverlay()
        }
    }

    Item {
        id: wheelRoot
        anchors.centerIn: parent
        width: root.ringOuterRadius * 2
        height: root.ringOuterRadius * 2

        // stop scrim's click-to-dismiss from firing when clicking inside the ring
        MouseArea {
            anchors.fill: parent
            onClicked: {} // absorb, does nothing
        }

        Canvas {
            id: wheelCanvas
            anchors.fill: parent
            renderStrategy: Canvas.Cooperative

            onPaint: {
                var ctx = getContext("2d")
                ctx.reset()
                var count = root.itemCount()
                if (count === 0) return

                var cx = width / 2
                var cy = height / 2
                var outerR = root.ringOuterRadius
                var innerR = root.ringInnerRadius
                var sliceDeg = 360 / count
                var gap = root.gapDegrees * Math.PI / 180

                for (var i = 0; i < count; i++) {
                    var startDeg = root.angleForIndex(i, count)
                    var startRad = startDeg * Math.PI / 180 + gap / 2
                    var endRad = (startDeg + sliceDeg) * Math.PI / 180 - gap / 2

                    ctx.beginPath()
                    ctx.arc(cx, cy, outerR, startRad, endRad, false)
                    ctx.arc(cx, cy, innerR, endRad, startRad, true)
                    ctx.closePath()

                    var isSelected = (i === root.currentIndex)
                    ctx.fillStyle = isSelected ? "rgba(90,140,255,0.55)" : "rgba(20,20,20,0.55)"
                    ctx.fill()

                    ctx.lineWidth = isSelected ? 2 : 1
                    ctx.strokeStyle = isSelected ? "rgba(140,180,255,0.9)" : "rgba(255,255,255,0.15)"
                    ctx.stroke()
                }
            }

            Connections {
                target: appCatalogModel
                function onModelReset() { wheelCanvas.requestPaint() }
                function onRowsInserted() { wheelCanvas.requestPaint() }
                function onRowsRemoved() { wheelCanvas.requestPaint() }
                function onDataChanged() { wheelCanvas.requestPaint() }
            }
        }

        // Icons + labels placed at each wedge's mid-angle, mid-radius
        Repeater {
            model: appCatalogModel
            delegate: Item {
                id: slotItem
                property real midRadius: (root.ringOuterRadius + root.ringInnerRadius) / 2
                property real midAngleDeg: root.angleForIndex(index, root.itemCount()) + (360 / Math.max(root.itemCount(), 1)) / 2
                property real midAngleRad: midAngleDeg * Math.PI / 180
                property bool isSelected: index === root.currentIndex

                x: wheelRoot.width / 2 + midRadius * Math.cos(midAngleRad) - width / 2
                y: wheelRoot.height / 2 + midRadius * Math.sin(midAngleRad) - height / 2
                width: 64
                height: 64

                scale: isSelected ? 1.3 : 1.0
                opacity: isSelected ? 1.0 : 0.7
                Behavior on scale { NumberAnimation { duration: 130; easing.type: Easing.OutCubic } }
                Behavior on opacity { NumberAnimation { duration: 130; easing.type: Easing.OutCubic } }

                Image {
                    id: iconImg
                    anchors.fill: parent
                    source: model.iconSource
                    fillMode: Image.PreserveAspectFit
                    visible: status === Image.Ready
                    asynchronous: true
                }

                // Fallback if the icon genuinely fails to load at the QML level
                // (covers image provider crashes, not just "no icon found" —
                // the C++ side already handles the latter with its own fallback)
                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: "#444"
                    visible: iconImg.status === Image.Error || iconImg.status === Image.Null
                    Text {
                        anchors.centerIn: parent
                        text: model.name ? model.name.charAt(0).toUpperCase() : "?"
                        color: "white"
                        font.pixelSize: 24
                        font.bold: true
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: { root.currentIndex = index; wheelCanvas.requestPaint() }
                    onClicked: root.launchCurrent()
                }
            }
        }

        // Center content: search field (primary) + current-selection name (secondary)
        Column {
            anchors.centerIn: parent
            spacing: 8
            width: root.ringInnerRadius * 1.6

            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: {
                    var count = root.itemCount()
                    if (count === 0) return ""
                    return appCatalogModel.data(appCatalogModel.index(root.currentIndex, 0), 0) // fallback if `name` role lookup differs
                }
                color: "#CCFFFFFF"
                font.pixelSize: 14
                elide: Text.ElideRight
            }

            Rectangle {
                width: parent.width
                height: 40
                radius: 10
                color: "#331A1A1A"
                border.color: "#55FFFFFF"
                border.width: 1

                TextField {
                    id: searchField
                    anchors.fill: parent
                    anchors.margins: 4
                    placeholderText: "Search apps..."
                    color: "white"
                    background: Item {}
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    focus: true

                    onTextChanged: {
                        appCatalogModel.filterText = text
                        root.currentIndex = 0
                        wheelCanvas.requestPaint()
                    }

                    Keys.onPressed: (event) => {
                        if (event.key === Qt.Key_Left || event.key === Qt.Key_Up) {
                            root.moveHighlight(-1)
                            event.accepted = true
                        } else if (event.key === Qt.Key_Right || event.key === Qt.Key_Down) {
                            root.moveHighlight(1)
                            event.accepted = true
                        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                            root.launchCurrent()
                            event.accepted = true
                        } else if (event.key === Qt.Key_Escape) {
                            if (searchField.text.length > 0) {
                                searchField.text = ""
                            } else {
                                root.hideOverlay()
                            }
                            event.accepted = true
                        }
                    }
                }
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            searchField.text = ""
            currentIndex = 0
            searchField.forceActiveFocus()
            wheelCanvas.requestPaint()
        }
    }
}
