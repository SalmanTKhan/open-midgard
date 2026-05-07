import QtQuick 2.15

// Single shared tooltip overlay. Bound to uiState.tooltipPayload — when the
// payload's `visible` flag flips true, this anchors near (anchorX, anchorY)
// in viewport coordinates and renders kind/title/body. The C++ adapter is
// expected to populate iconSource / title / richBody on tooltipRequested;
// until that lands, the QML still renders a minimal kind+id stub so the
// pipeline can be validated end-to-end.
Item {
    id: tooltip
    z: 9999
    visible: payload && payload.visible === true

    readonly property var payload: uiState.tooltipPayload

    readonly property string kind: payload && payload.kind ? payload.kind : ""
    readonly property int   itemId: payload && payload.id !== undefined ? payload.id : 0
    readonly property string iconSource: payload && payload.iconSource ? payload.iconSource : ""
    readonly property string title: payload && payload.title ? payload.title
        : (kind && itemId > 0 ? (kind + " #" + itemId) : "")
    readonly property string richBody: payload && payload.richBody ? payload.richBody : ""

    readonly property int margin: 8
    readonly property int maxWidth: 280

    width: Math.min(maxWidth, Math.max(120, contentColumn.implicitWidth + 16))
    height: contentColumn.implicitHeight + 16

    // Anchor + flip so we never go offscreen.
    x: {
        var ax = (payload && payload.anchorX !== undefined) ? payload.anchorX : 0
        var rt = parent
        var rw = rt ? rt.width : width
        var px = ax + 14
        if (px + width > rw - margin) px = ax - width - 14
        if (px < margin) px = margin
        return Math.round(px)
    }
    y: {
        var ay = (payload && payload.anchorY !== undefined) ? payload.anchorY : 0
        var rt = parent
        var rh = rt ? rt.height : height
        var py = ay + 14
        if (py + height > rh - margin) py = ay - height - 14
        if (py < margin) py = margin
        return Math.round(py)
    }

    Rectangle {
        anchors.fill: parent
        radius: 6
        color: Theme.tooltipBg
        border.width: 1
        border.color: Theme.tooltipBorder
    }

    Column {
        id: contentColumn
        x: 8
        y: 8
        width: parent.width - 16
        spacing: 4

        Row {
            spacing: 6
            visible: tooltip.iconSource.length > 0 || tooltip.title.length > 0
            Image {
                visible: tooltip.iconSource.length > 0
                source: tooltip.iconSource
                width: 24
                height: 24
                fillMode: Image.PreserveAspectFit
                smooth: true
                asynchronous: true
            }
            Text {
                text: tooltip.title
                color: Theme.text
                font.family: "Segoe UI"
                font.pixelSize: 13
                font.bold: true
                wrapMode: Text.WordWrap
                width: contentColumn.width - (tooltip.iconSource.length > 0 ? 30 : 0)
            }
        }

        Text {
            visible: tooltip.richBody.length > 0
            text: tooltip.richBody
            color: Theme.textMuted
            font.family: "Segoe UI"
            font.pixelSize: 12
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            width: contentColumn.width
        }
    }

    opacity: visible ? 1.0 : 0.0
    Behavior on opacity { NumberAnimation { duration: 90; easing.type: Easing.OutCubic } }
}
