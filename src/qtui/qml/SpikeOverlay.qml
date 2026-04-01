import QtQuick 2.15

Item {
    id: root
    width: parent ? parent.width : 1280
    height: parent ? parent.height : 720

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 16
        width: 420
        radius: 8
        color: "#b01a1f2b"
        border.width: 1
        border.color: "#50d7dde8"

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6

            Text {
                text: "Qt 6 UI spike"
                color: "#ffffff"
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                text: "Backend: " + spikeState.backendName
                color: "#d7dde8"
                font.pixelSize: 14
            }

            Text {
                text: "Mode: " + spikeState.modeName
                color: "#d7dde8"
                font.pixelSize: 14
            }

            Text {
                text: spikeState.architectureNote
                color: "#ffcc66"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Rectangle {
                width: parent.width
                height: 1
                color: "#304050"
            }

            Text {
                text: "Login status: " + spikeState.loginStatus
                color: "#d7dde8"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: "Recent chat:\n" + spikeState.chatPreview
                color: "#b5d9ff"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: "Input: " + spikeState.lastInput
                color: "#9ee7a0"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    Repeater {
        model: spikeState.anchors

        delegate: Item {
            required property var modelData
            x: modelData.x
            y: modelData.y
            width: label.implicitWidth + 12
            height: label.implicitHeight + 8

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: modelData.background
                border.width: 1
                border.color: "#80ffffff"
            }

            Text {
                id: label
                anchors.centerIn: parent
                text: modelData.text
                color: modelData.foreground
                font.pixelSize: 14
                font.bold: true
            }
        }
    }
}
