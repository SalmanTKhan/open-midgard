import QtQuick 2.15

Item {
    id: root
    width: 1280
    height: 720

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 16
        width: 440
        radius: 8
        color: "#b01a1f2b"
        border.width: 1
        border.color: "#50d7dde8"

        Column {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6

            Text {
                text: "Qt 6 GPU UI"
                color: "#ffffff"
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                text: "Backend: " + uiState.backendName
                color: "#d7dde8"
                font.pixelSize: 14
            }

            Text {
                text: "Mode: " + uiState.modeName
                color: "#d7dde8"
                font.pixelSize: 14
            }

            Text {
                text: "Render path: " + uiState.renderPath
                color: "#9ee7a0"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: uiState.architectureNote
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
                text: "Login status: " + uiState.loginStatus
                color: "#d7dde8"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: "Recent chat:\n" + uiState.chatPreview
                color: "#b5d9ff"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }

            Text {
                text: "Input: " + uiState.lastInput
                color: "#9ee7a0"
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                width: parent.width
            }
        }
    }

    Repeater {
        model: uiState.anchors

        delegate: Item {
            required property var modelData
            x: modelData.x
            y: modelData.y
            width: label.implicitWidth + (modelData.showBubble === false ? 0 : 12)
            height: label.implicitHeight + (modelData.showBubble === false ? 0 : 8)

            Rectangle {
                anchors.fill: parent
                radius: 6
                color: modelData.background
                visible: modelData.showBubble !== false
                border.width: 1
                border.color: "#80ffffff"
            }

            Text {
                id: label
                anchors.centerIn: parent
                text: modelData.text
                color: modelData.foreground
                font.pixelSize: modelData.fontPixelSize || 14
                font.bold: modelData.bold !== false
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 42
        width: Math.min(540, parent.width - 80)
        height: 92
        radius: 8
        color: "#e6191f26"
        border.width: 1
        border.color: "#c2b067"
        visible: uiState.loadingVisible

        Column {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 8

            Row {
                width: parent.width

                Text {
                    id: loadingTitle
                    text: uiState.loadingProgress > 0 ? "Entering World" : uiState.modeName
                    color: "#f5efdd"
                    font.pixelSize: 22
                    font.bold: true
                }

                Item {
                    width: Math.max(0, parent.width - loadingTitle.implicitWidth - loadingPercent.implicitWidth)
                    height: 1
                }

                Text {
                    id: loadingPercent
                    text: Math.round(uiState.loadingProgress * 100) + "%"
                    color: "#f5efdd"
                    font.pixelSize: 22
                    font.bold: true
                    visible: uiState.loadingProgress > 0
                }
            }

            Text {
                width: parent.width
                text: uiState.loadingMessage
                color: "#f5efdd"
                font.pixelSize: 16
                elide: Text.ElideRight
            }

            Rectangle {
                width: parent.width
                height: 20
                radius: 4
                color: "#323a44"
                border.width: 1
                border.color: "#c2b067"
                visible: uiState.loadingProgress > 0

                Rectangle {
                    anchors.left: parent.left
                    anchors.leftMargin: 2
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.max(0, (parent.width - 4) * uiState.loadingProgress)
                    height: parent.height - 4
                    radius: 3
                    color: "#e6c658"
                }
            }
        }
    }

    Rectangle {
        x: uiState.serverPanelX
        y: uiState.serverPanelY
        width: uiState.serverPanelWidth
        height: uiState.serverPanelHeight
        radius: 8
        color: "#f3f0e7"
        border.width: 1
        border.color: "#787060"
        visible: uiState.serverSelectVisible

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 4

            Text {
                text: "Select Server"
                color: "#303030"
                font.pixelSize: 14
                font.bold: true
            }

            Repeater {
                model: uiState.serverEntries

                delegate: Rectangle {
                    required property var modelData

                    width: parent ? parent.width : 0
                    height: 22
                    radius: 2
                    color: index === uiState.serverSelectedIndex
                        ? "#d6e0c6"
                        : (index === uiState.serverHoverIndex ? "#e5e9e0" : "#faf8f2")
                    border.width: 1
                    border.color: "#a0a0a0"

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.label
                        color: "#181818"
                        font.pixelSize: 12
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.detail
                        color: "#606060"
                        font.pixelSize: 12
                    }
                }
            }
        }
    }

    Rectangle {
        x: uiState.loginPanelX
        y: uiState.loginPanelY
        width: uiState.loginPanelWidth
        height: uiState.loginPanelHeight
        color: "#ebe5dc"
        border.width: 1
        border.color: "#303030"
        visible: uiState.loginPanelVisible

        Text {
            x: 14
            y: 10
            text: "Login"
            color: "#303030"
            font.pixelSize: 16
            font.bold: true
        }

        Rectangle {
            x: 92
            y: 29
            width: 125
            height: 18
            color: "#f2f2f2"
            border.width: 1
            border.color: uiState.loginPasswordFocused ? "#b0b0b0" : "#707070"
        }

        Text {
            x: 98
            y: 31
            width: 112
            text: uiState.loginUserId
            color: "#202020"
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Rectangle {
            x: 92
            y: 61
            width: 125
            height: 18
            color: "#f2f2f2"
            border.width: 1
            border.color: uiState.loginPasswordFocused ? "#707070" : "#b0b0b0"
        }

        Text {
            x: 98
            y: 63
            width: 112
            text: uiState.loginPasswordMask
            color: "#202020"
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Rectangle {
            x: 232
            y: 33
            width: 14
            height: 14
            color: uiState.loginSaveAccountChecked ? "#6e8b3d" : "#ffffff"
            border.width: 1
            border.color: "#404040"
        }

        Text {
            x: 250
            y: 31
            text: "Save"
            color: "#303030"
            font.pixelSize: 11
        }

        Repeater {
            model: [
                { x: 4, y: 96, w: 52, text: "Request" },
                { x: 137, y: 96, w: 44, text: "Intro" },
                { x: 189, y: 96, w: 40, text: "Connect" },
                { x: 234, y: 96, w: 32, text: "Exit" }
            ]

            delegate: Rectangle {
                required property var modelData
                x: modelData.x
                y: modelData.y
                width: modelData.w
                height: 20
                radius: 3
                color: "#d8d0c4"
                border.width: 1
                border.color: "#6f6558"

                Text {
                    anchors.centerIn: parent
                    text: modelData.text
                    color: "#202020"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }
    }
}
