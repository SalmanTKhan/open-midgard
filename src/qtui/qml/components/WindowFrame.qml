import QtQuick 2.15

// Reusable window chrome for QtUI overlays.
//
// Adds drop shadow, drag-to-move, snap-to-edge, focus z-order, fade + slide-in
// on show, and an optional title bar + close button on top of an otherwise
// plain Rectangle. The visual surface is an inner Rectangle with color /
// radius / border aliased to the WindowFrame root so consumers can still write
// `WindowFrame { color: ...; radius: ...; border.color: ... }` exactly like a
// Rectangle.
//
// Geometry contract: consumers seed position via initialX/initialY (typically
// bound to a uiState Q_PROPERTY). Once the user drags the window, the override
// is persisted into uiState via setWindowGeometry(name, ...) and the seed is
// no longer applied. Width / height stay consumer-controlled (no resize yet).

Item {
    id: frame

    property string windowName: ""
    property string title: ""

    // Position seeds (read once at completion or whenever they change while
    // the user has not placed the window manually).
    property int initialX: 0
    property int initialY: 0

    property int snapThreshold: 10

    // Drag handle covers the top of the frame. When showTitleBar is true the
    // handle's height is titleBarHeight; otherwise consumers can set
    // dragHandleHeight directly to match a custom header (e.g. chat's tab
    // strip occupies ~50px).
    property bool showTitleBar: true
    property int titleBarHeight: 28
    property int dragHandleHeight: showTitleBar ? titleBarHeight : 28

    property bool showCloseButton: true

    signal closeRequested()

    // Aliases so consumers' existing { color, radius, border } syntax still
    // works on the WindowFrame root.
    property alias color: surface.color
    property alias radius: surface.radius
    property alias border: surface.border

    property bool _userPlaced: false
    property int _localZ: 0
    z: _localZ

    Component.onCompleted: {
        if (uiState.hasWindowGeometryOverride(windowName)) {
            var o = uiState.windowGeometryOverride(windowName)
            if (o && o.valid) {
                frame.x = o.x
                frame.y = o.y
                _userPlaced = true
            }
        } else {
            frame.x = initialX
            frame.y = initialY
        }
        _localZ = uiState.zOrderFor(windowName)
    }

    onInitialXChanged: if (!_userPlaced) frame.x = initialX
    onInitialYChanged: if (!_userPlaced) frame.y = initialY

    // Capture helpers: assign properties from JS so QML breaks the existing
    // bindings created by consumers (visible:, x:, y:). Pure C++ setVisible
    // does not break QML bindings, which is why /uishots all couldn't force
    // hidden windows to render.
    function forceShowAt(px, py, pw, ph) {
        visible = true
        opacity = 1.0
        _slideY = 0
        _userPlaced = true
        clip = true
        if (pw > 0) width = pw
        if (ph > 0) height = ph
        x = px
        y = py
        z = 9000 + (windowName.charCodeAt(0) || 0) // ensure forced windows render above pre-existing
    }

    function forceHide() {
        visible = false
    }

    Connections {
        target: uiState
        function onWindowGeometryChanged(name) {
            if (name !== frame.windowName) return
            var o = uiState.windowGeometryOverride(frame.windowName)
            if (o && o.valid) {
                frame.x = o.x
                frame.y = o.y
                frame._userPlaced = true
            } else {
                frame._userPlaced = false
            }
        }
        function onWindowZOrderChanged(name) {
            if (name === frame.windowName) {
                frame._localZ = uiState.zOrderFor(name)
            }
        }
    }

    // ------------------------------------------------------------------ shadow
    // Two stacked rounded rects offset down-right behind the surface — gives a
    // soft drop-shadow feel without depending on Qt MultiEffect availability.
    Rectangle {
        z: -3
        anchors.fill: surface
        anchors.leftMargin: -2
        anchors.topMargin: -1
        anchors.rightMargin: -4
        anchors.bottomMargin: -5
        radius: surface.radius + 3
        color: Theme.frameShadow
        opacity: 0.55
        antialiasing: true
    }
    Rectangle {
        z: -4
        anchors.fill: surface
        anchors.leftMargin: -4
        anchors.topMargin: -2
        anchors.rightMargin: -7
        anchors.bottomMargin: -9
        radius: surface.radius + 6
        color: Theme.frameShadow
        opacity: 0.3
        antialiasing: true
    }

    // -------------------------------------------------------------- visible surface
    Rectangle {
        id: surface
        anchors.fill: parent
        color: Theme.background
        radius: 8
        border.width: 1
        border.color: Theme.borderStrong
        antialiasing: true
    }

    // Drag handle along the top edge.
    MouseArea {
        id: dragArea
        x: 0
        y: 0
        width: parent.width
        height: frame.dragHandleHeight
        cursorShape: _dragging ? Qt.ClosedHandCursor : Qt.SizeAllCursor
        hoverEnabled: false
        property real _px: 0
        property real _py: 0
        property real _fx: 0
        property real _fy: 0
        property bool _dragging: false
        z: 5
        onPressed: function(mouse) {
            uiState.raiseWindow(frame.windowName)
            _px = mouse.x
            _py = mouse.y
            _fx = frame.x
            _fy = frame.y
            _dragging = true
        }
        onPositionChanged: function(mouse) {
            if (!_dragging) return
            frame.x = _fx + (mouse.x - _px)
            frame.y = _fy + (mouse.y - _py)
        }
        onReleased: {
            if (!_dragging) return
            _dragging = false
            var rt = frame.parent
            var rw = rt ? rt.width : frame.width
            var rh = rt ? rt.height : frame.height
            var nx = frame.x
            var ny = frame.y
            var t = frame.snapThreshold
            if (Math.abs(nx) <= t) nx = 0
            else if (Math.abs((nx + frame.width) - rw) <= t) nx = rw - frame.width
            if (Math.abs(ny) <= t) ny = 0
            else if (Math.abs((ny + frame.height) - rh) <= t) ny = rh - frame.height
            // Clamp to viewport so windows can never disappear.
            nx = Math.max(0, Math.min(nx, Math.max(0, rw - frame.width)))
            ny = Math.max(0, Math.min(ny, Math.max(0, rh - frame.height)))
            frame.x = nx
            frame.y = ny
            frame._userPlaced = true
            uiState.setWindowGeometry(frame.windowName, frame.x, frame.y, frame.width, frame.height)
        }
    }

    // Title bar visual (optional).
    Rectangle {
        visible: frame.showTitleBar
        x: 0
        y: 0
        width: parent.width
        height: frame.titleBarHeight
        color: Theme.frameDragHandle
        radius: surface.radius
        z: 4
        // Mask the bottom rounding so the bar meets the body cleanly.
        Rectangle {
            x: 0
            y: parent.height - parent.radius
            width: parent.width
            height: parent.radius
            color: parent.color
            visible: parent.radius > 0
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            x: 10
            text: frame.title
            color: Theme.text
            font.family: "Segoe UI"
            font.pixelSize: 13
            font.bold: true
            elide: Text.ElideRight
            width: parent.width - 36
        }
    }

    // Close button (optional).
    Item {
        id: closeBtn
        visible: frame.showCloseButton
        x: parent.width - width - 8
        y: 6
        width: 18
        height: 18
        z: 6
        Rectangle {
            anchors.fill: parent
            radius: 9
            color: closeMa.pressed ? Theme.buttonBgPressed
                 : (closeMa.containsMouse ? Theme.buttonBgHover : Theme.buttonBg)
            border.width: 1
            border.color: Theme.buttonBorder
        }
        Text {
            anchors.centerIn: parent
            text: "✕"
            color: Theme.buttonText
            font.family: "Segoe UI Symbol"
            font.pixelSize: 11
        }
        MouseArea {
            id: closeMa
            anchors.fill: parent
            hoverEnabled: true
            onClicked: frame.closeRequested()
            cursorShape: Qt.PointingHandCursor
        }
    }

    // ----------------------------------------------------- show/hide animation
    // Fade + a small slide-in when becoming visible. Slide-out is a no-op
    // because Qt drops the Item from rendering as soon as visible flips false.
    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 130; easing.type: Easing.OutCubic } }

    property real _slideY: visible ? 0 : -8
    transform: Translate { y: frame._slideY }
    Behavior on _slideY { NumberAnimation { duration: 170; easing.type: Easing.OutCubic } }
}
