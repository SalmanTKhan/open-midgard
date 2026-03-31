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

    Repeater {
        model: uiState.notifications

        delegate: Rectangle {
            required property var modelData
            x: modelData.x
            y: modelData.y
            width: modelData.width
            height: modelData.height
            radius: 6
            color: "#de1f2328"
            border.width: 1
            border.color: modelData.accent

            Rectangle {
                x: 2
                y: 2
                width: parent.width - 4
                height: parent.height - 4
                radius: 5
                color: "#ee2b3138"
                border.width: 1
                border.color: "#70ffffff"
            }

            Text {
                anchors.centerIn: parent
                text: modelData.title
                color: "#f7f2df"
                font.pixelSize: 18
                font.bold: true
            }
        }
    }

    Rectangle {
        x: uiState.npcMenuX
        y: uiState.npcMenuY
        width: uiState.npcMenuWidth
        height: uiState.npcMenuHeight
        radius: 10
        color: "#f8f8f8"
        border.width: 1
        border.color: "#828282"
        visible: uiState.npcMenuVisible

        Column {
            x: 10
            y: 10
            width: parent.width - 20
            spacing: 0

            Repeater {
                model: uiState.npcMenuOptions

                delegate: Rectangle {
                    required property var modelData
                    width: parent.width
                    height: 18
                    color: index === uiState.npcMenuSelectedIndex
                        ? "#d1e0f4"
                        : (index === uiState.npcMenuHoverIndex ? "#e8eff8" : "transparent")
                    border.width: (index === uiState.npcMenuSelectedIndex || index === uiState.npcMenuHoverIndex) ? 1 : 0
                    border.color: "#a0b4cd"

                    Text {
                        anchors.left: parent.left
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 8
                        text: modelData
                        color: "#000000"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }
            }
        }

        Rectangle {
            x: width - 10 - 68 - 8 - 68
            y: height - 10 - 22
            width: 68
            height: 22
            color: uiState.npcMenuOkPressed ? "#c4c4c4" : "#f0f0f0"
            border.width: 1
            border.color: "#6e6e6e"

            Text {
                anchors.centerIn: parent
                text: "OK"
                color: "#000000"
                font.pixelSize: 12
            }
        }

        Rectangle {
            x: width - 10 - 68
            y: height - 10 - 22
            width: 68
            height: 22
            color: uiState.npcMenuCancelPressed ? "#c4c4c4" : "#f0f0f0"
            border.width: 1
            border.color: "#6e6e6e"

            Text {
                anchors.centerIn: parent
                text: "Cancel"
                color: "#000000"
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        x: uiState.sayDialogX
        y: uiState.sayDialogY
        width: uiState.sayDialogWidth
        height: uiState.sayDialogHeight
        radius: 10
        color: "#f8f8f8"
        border.width: 1
        border.color: "#828282"
        visible: uiState.sayDialogVisible

        Text {
            x: 10
            y: 10
            width: parent.width - 20
            height: uiState.sayDialogHasAction ? (parent.height - 10 - 10 - 22 - 8) : (parent.height - 20)
            text: uiState.sayDialogText
            color: "#000000"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
        }

        Rectangle {
            x: width - 10 - 68
            y: height - 10 - 22
            width: 68
            height: 22
            visible: uiState.sayDialogHasAction
            color: uiState.sayDialogActionPressed ? "#c4c4c4" : (uiState.sayDialogActionHovered ? "#e4e4e4" : "#f0f0f0")
            border.width: 1
            border.color: "#6e6e6e"

            Text {
                anchors.centerIn: parent
                text: uiState.sayDialogActionLabel
                color: "#000000"
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        x: uiState.npcInputX
        y: uiState.npcInputY
        width: uiState.npcInputWidth
        height: uiState.npcInputHeight
        radius: 10
        color: "#f8f8f8"
        border.width: 1
        border.color: "#828282"
        visible: uiState.npcInputVisible

        Text {
            x: 10
            y: 9
            width: parent.width - 20
            text: uiState.npcInputLabel
            color: "#000000"
            font.pixelSize: 12
        }

        Rectangle {
            x: 10
            y: 26
            width: parent.width - 20
            height: 22
            color: "#fff7c8"
            border.width: 1
            border.color: "#000000"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 12
                text: uiState.npcInputText
                color: "#000000"
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        Rectangle {
            x: width - 10 - 68 - 8 - 68
            y: height - 10 - 22
            width: 68
            height: 22
            color: uiState.npcInputOkPressed ? "#c4c4c4" : "#f0f0f0"
            border.width: 1
            border.color: "#6e6e6e"

            Text {
                anchors.centerIn: parent
                text: "OK"
                color: "#000000"
                font.pixelSize: 12
            }
        }

        Rectangle {
            x: width - 10 - 68
            y: height - 10 - 22
            width: 68
            height: 22
            color: uiState.npcInputCancelPressed ? "#c4c4c4" : "#f0f0f0"
            border.width: 1
            border.color: "#6e6e6e"

            Text {
                anchors.centerIn: parent
                text: "Cancel"
                color: "#000000"
                font.pixelSize: 12
            }
        }
    }

    Rectangle {
        x: uiState.chooseMenuX
        y: uiState.chooseMenuY
        width: uiState.chooseMenuWidth
        height: uiState.chooseMenuHeight
        radius: 8
        color: "#ffffff"
        border.width: 1
        border.color: "#b8b8b8"
        visible: uiState.chooseMenuVisible

        Repeater {
            model: [
                "Character Select",
                "Return To Game",
                "Exit To Windows",
                "Return To Save Point"
            ]

            delegate: Rectangle {
                required property string modelData
                x: (parent.width - 221) / 2
                y: 12 + index * 23
                width: 221
                height: 20
                color: index === uiState.chooseMenuSelectedIndex ? "#cad8ea" : "#f1f1f1"
                border.width: 1
                border.color: "#8f8f8f"

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: "#1a1a1a"
                    font.pixelSize: 12
                }
            }
        }
    }

    Rectangle {
        x: uiState.itemShopX
        y: uiState.itemShopY
        width: uiState.itemShopWidth
        height: uiState.itemShopHeight
        color: "#ececec"
        border.width: 1
        border.color: "#484848"
        visible: uiState.itemShopVisible

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 17
            color: "#52657b"
        }

        Text {
            x: 6
            y: 1
            text: uiState.itemShopTitle
            color: "#ffffff"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 8
            y: 22
            width: parent.width - 16
            height: parent.height - 34
            color: "#f8f8f8"
            border.width: 1
            border.color: "#787878"

            Column {
                x: 1
                y: 1
                width: parent.width - 2
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 17
                    color: "#dee5ed"

                    Text {
                        x: 26
                        y: 2
                        text: "Item"
                        color: "#1e1e1e"
                        font.pixelSize: 11
                    }

                    Text {
                        x: width - 60
                        y: 2
                        width: 54
                        text: "Price"
                        color: "#1e1e1e"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Repeater {
                    model: uiState.itemShopRows

                    delegate: Rectangle {
                        required property var modelData
                        width: parent.width
                        height: 18
                        color: modelData.selected ? "#bccce2" : (modelData.hover ? "#e2eaf4" : "transparent")

                        Text {
                            x: 24
                            y: 2
                            width: parent.width - 110
                            text: modelData.name
                            color: "#1a1a1a"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        Text {
                            x: parent.width - 76
                            y: 2
                            width: 70
                            text: modelData.price
                            color: "#1c3c62"
                            font.pixelSize: 11
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        x: uiState.shopChoiceX
        y: uiState.shopChoiceY
        width: uiState.shopChoiceWidth
        height: uiState.shopChoiceHeight
        radius: 8
        color: "#f4f1ea"
        border.width: 1
        border.color: "#7f7a70"
        visible: uiState.shopChoiceVisible

        Text {
            x: 12
            y: 10
            text: "Shop"
            color: "#2a2a2a"
            font.pixelSize: 15
            font.bold: true
        }

        Text {
            x: 14
            y: 28
            width: parent.width - 28
            text: "Choose a transaction type."
            color: "#343434"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
        }

        Repeater {
            model: uiState.shopChoiceButtons

            delegate: Rectangle {
                required property var modelData
                x: modelData.x - uiState.shopChoiceX
                y: modelData.y - uiState.shopChoiceY
                width: modelData.width
                height: modelData.height
                radius: 4
                color: modelData.pressed ? "#aab9cd" : (modelData.hot ? "#c4d2e4" : "#dcdcdc")
                border.width: 1
                border.color: "#585858"

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: "#181818"
                    font.pixelSize: 12
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

    Rectangle {
        x: uiState.charSelectPanelX
        y: uiState.charSelectPanelY
        width: uiState.charSelectPanelWidth
        height: uiState.charSelectPanelHeight
        color: "#f5efe6"
        border.width: 1
        border.color: "#6a4c34"
        visible: uiState.charSelectVisible

        Repeater {
            model: uiState.charSelectSlots

            delegate: Rectangle {
                required property var modelData
                x: modelData.x - uiState.charSelectPanelX
                y: modelData.y - uiState.charSelectPanelY
                width: modelData.width
                height: modelData.height
                color: modelData.selected ? "#e8d9b9" : "#fffaf2"
                border.width: 2
                border.color: modelData.selected ? "#9a6d38" : "#b89d79"

                Column {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    Text {
                        text: modelData.occupied ? modelData.name : "Empty Slot"
                        color: "#50321e"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Text {
                        text: modelData.occupied ? modelData.job : ""
                        color: "#704c30"
                        font.pixelSize: 13
                    }

                    Text {
                        text: modelData.occupied ? ("Lv. " + modelData.level) : ""
                        color: "#704c30"
                        font.pixelSize: 13
                    }
                }
            }
        }

        Text {
            x: 69
            y: 206
            text: uiState.charSelectSelectedDetails.name || ""
            color: "#50321e"
            font.pixelSize: 13
        }

        Text {
            x: 69
            y: 222
            text: uiState.charSelectSelectedDetails.job || ""
            color: "#50321e"
            font.pixelSize: 13
        }

        Repeater {
            model: [
                { x: 69, y: 238, key: "level" },
                { x: 69, y: 254, key: "exp" },
                { x: 69, y: 270, key: "hp" },
                { x: 69, y: 286, key: "sp" },
                { x: 213, y: 206, key: "str" },
                { x: 213, y: 222, key: "agi" },
                { x: 213, y: 238, key: "vit" },
                { x: 213, y: 254, key: "int" },
                { x: 213, y: 270, key: "dex" },
                { x: 213, y: 286, key: "luk" }
            ]

            delegate: Text {
                required property var modelData
                x: modelData.x
                y: modelData.y
                text: uiState.charSelectSelectedDetails[modelData.key] !== undefined
                    ? uiState.charSelectSelectedDetails[modelData.key]
                    : ""
                color: "#50321e"
                font.pixelSize: 13
            }
        }

        Text {
            x: 44
            y: 110
            width: 12
            height: 48
            text: uiState.charSelectPageCount > 1 ? "<" : ""
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "#50321e"
            font.pixelSize: 18
            font.bold: true
        }

        Text {
            x: 520
            y: 110
            width: 12
            height: 48
            text: uiState.charSelectPageCount > 1 ? ">" : ""
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: "#50321e"
            font.pixelSize: 18
            font.bold: true
        }
    }

    Rectangle {
        x: uiState.makeCharPanelX
        y: uiState.makeCharPanelY
        width: uiState.makeCharPanelWidth
        height: uiState.makeCharPanelHeight
        color: "#f5efe6"
        border.width: 1
        border.color: "#6a4c34"
        visible: uiState.makeCharVisible

        Rectangle {
            x: 62
            y: 244
            width: 100
            height: 18
            color: "#f2f2f2"
            border.width: 1
            border.color: uiState.makeCharNameFocused ? "#6a4c34" : "#909090"
        }

        Text {
            x: 68
            y: 246
            width: 88
            text: uiState.makeCharName
            color: "#202020"
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Rectangle {
            x: 30
            y: 72
            width: 130
            height: 170
            radius: 8
            color: "#fffaf2"
            border.width: 1
            border.color: "#b89d79"

            Column {
                anchors.centerIn: parent
                spacing: 8

                Text {
                    text: "Preview"
                    color: "#50321e"
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    text: "Hair " + uiState.makeCharHairIndex
                    color: "#704c30"
                    font.pixelSize: 13
                }

                Text {
                    text: "Color " + uiState.makeCharHairColor
                    color: "#704c30"
                    font.pixelSize: 13
                }
            }
        }

        Rectangle {
            x: 190
            y: 40
            width: 190
            height: 240
            radius: 12
            color: "#fffaf2"
            border.width: 1
            border.color: "#b89d79"
        }

        Repeater {
            model: [
                { label: "STR", x: 484, y: 40, idx: 0 },
                { label: "AGI", x: 484, y: 56, idx: 1 },
                { label: "VIT", x: 484, y: 72, idx: 2 },
                { label: "INT", x: 484, y: 88, idx: 3 },
                { label: "DEX", x: 484, y: 104, idx: 4 },
                { label: "LUK", x: 484, y: 120, idx: 5 }
            ]

            delegate: Row {
                required property var modelData
                x: modelData.x - 28
                y: modelData.y
                spacing: 8

                Text {
                    width: 24
                    text: modelData.label
                    color: "#50321e"
                    font.pixelSize: 12
                    font.bold: true
                }

                Text {
                    text: uiState.makeCharStats.length > modelData.idx ? uiState.makeCharStats[modelData.idx] : ""
                    color: "#3c2414"
                    font.pixelSize: 12
                }
            }
        }

        Repeater {
            model: [
                { x: 483, y: 318, w: 40, text: "OK" },
                { x: 530, y: 318, w: 40, text: "Cancel" },
                { x: 48, y: 135, w: 16, text: "<" },
                { x: 130, y: 135, w: 16, text: ">" },
                { x: 89, y: 101, w: 16, text: "^" }
            ]

            delegate: Rectangle {
                required property var modelData
                x: modelData.x
                y: modelData.y
                width: modelData.w
                height: 18
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
