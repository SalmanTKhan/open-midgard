import QtQuick 2.15

// Three-state themed button with hover/press/disabled visual feedback.
Item {
    id: btn
    property string text: ""
    property bool enabled: true
    property string font: "Segoe UI"
    property int fontPixelSize: 12
    property int radius: 6
    signal clicked()

    implicitWidth: 64
    implicitHeight: 22

    Rectangle {
        anchors.fill: parent
        radius: btn.radius
        color: !btn.enabled ? Qt.darker(Theme.buttonBg, 1.15)
             : (ma.pressed ? Theme.buttonBgPressed
             : (ma.containsMouse ? Theme.buttonBgHover : Theme.buttonBg))
        border.width: 1
        border.color: Theme.buttonBorder
        opacity: btn.enabled ? 1.0 : 0.55
    }

    Text {
        anchors.centerIn: parent
        text: btn.text
        color: Theme.buttonText
        font.family: btn.font
        font.pixelSize: btn.fontPixelSize
        font.bold: true
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        enabled: btn.enabled
        hoverEnabled: true
        cursorShape: btn.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: btn.clicked()
    }
}
