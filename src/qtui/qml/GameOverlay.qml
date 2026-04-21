import QtQuick 2.15

Item {
    id: root
    width: parent ? parent.width : 1280
    height: parent ? parent.height : 720
    property bool loginCaretVisible: true
    property real uiScale: Math.max(0.5, uiState.uiScale || 1.0)
    readonly property bool darkTheme: Theme.mode === "dark"

    function rgbaColor(red, green, blue, alpha) {
        return Qt.rgba(red / 255.0, green / 255.0, blue / 255.0, Math.max(0.0, Math.min(1.0, alpha)))
    }

    function chatChromeOpacity(chatUi) {
        return Math.max(0.2, Math.min(1.0, ((chatUi && chatUi.windowOpacityPercent) || 84) / 100.0))
    }

    function clampValue(value, minValue, maxValue) {
        if (maxValue < minValue) {
            return minValue
        }
        return Math.max(minValue, Math.min(maxValue, value))
    }

    function clampOverlayX(x, overlayWidth) {
        return clampValue(x, 0, root.width - overlayWidth)
    }

    function clampOverlayY(y, overlayHeight) {
        return clampValue(y, 0, root.height - overlayHeight)
    }

    function asObject(value) {
        return (value && typeof value === "object") ? value : ({})
    }

    function itemIconSource(itemId, itemIndex, identified) {
        if (!(itemId > 0) && !(itemIndex > 0)) {
            return ""
        }
        let source = "image://openmidgard/item/" + itemId
        if (itemIndex > 0) {
            source += "?itemIndex=" + itemIndex
        }
        if (identified !== undefined) {
            source += (source.indexOf("?") >= 0 ? "&" : "?") + "identified=" + (identified ? "1" : "0")
        }
        return source
    }

    function collectionImageSource(itemId, itemIndex, identified) {
        if (!(itemId > 0) && !(itemIndex > 0)) {
            return ""
        }
        let source = "image://openmidgard/collection/" + itemId
        if (itemIndex > 0) {
            source += "?itemIndex=" + itemIndex
        }
        if (identified !== undefined) {
            source += (source.indexOf("?") >= 0 ? "&" : "?") + "identified=" + (identified ? "1" : "0")
        }
        return source
    }

    function illustImageSource(itemId) {
        return itemId > 0 ? "image://openmidgard/illust/" + itemId : ""
    }

    function isHexDigit(ch) {
        return (ch >= "0" && ch <= "9")
            || (ch >= "a" && ch <= "f")
            || (ch >= "A" && ch <= "F")
    }

    function escapeHtml(text) {
        return String(text)
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/\"/g, "&quot;")
            .replace(/'/g, "&#39;")
    }

    function adjustTextColorForTheme(hex) {
        const r = parseInt(hex.substring(0, 2), 16)
        const g = parseInt(hex.substring(2, 4), 16)
        const b = parseInt(hex.substring(4, 6), 16)
        const bg = Theme.background
        const bgLum = 0.2126 * bg.r + 0.7152 * bg.g + 0.0722 * bg.b
        const darkBg = bgLum < 0.5
        const textLum = (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255
        if (darkBg && textLum < 0.55) {
            const t = 0.75
            const nr = Math.round(r + (255 - r) * t)
            const ng = Math.round(g + (255 - g) * t)
            const nb = Math.round(b + (255 - b) * t)
            return "#" + ("0" + nr.toString(16)).slice(-2)
                + ("0" + ng.toString(16)).slice(-2)
                + ("0" + nb.toString(16)).slice(-2)
        }
        if (!darkBg && textLum > 0.85) {
            const t = 0.65
            const nr = Math.round(r * (1 - t))
            const ng = Math.round(g * (1 - t))
            const nb = Math.round(b * (1 - t))
            return "#" + ("0" + nr.toString(16)).slice(-2)
                + ("0" + ng.toString(16)).slice(-2)
                + ("0" + nb.toString(16)).slice(-2)
        }
        return "#" + hex
    }

    function roColorTextToRichText(text, defaultTextColor) {
        if (!text) {
            return ""
        }

        const baseColor = defaultTextColor ? String(defaultTextColor) : String(Theme.text)
        let out = "<span style=\"color:" + baseColor + "\">"
        for (let index = 0; index < text.length; ++index) {
            const ch = text[index]
            if (ch === "^" && index + 6 < text.length) {
                let isColor = true
                for (let colorIndex = 1; colorIndex <= 6; ++colorIndex) {
                    if (!isHexDigit(text[index + colorIndex])) {
                        isColor = false
                        break
                    }
                }
                if (isColor) {
                    const currentColor = text.substring(index + 1, index + 7)
                    out += "</span><span style=\"color:" + (currentColor === "000000" ? baseColor : adjustTextColorForTheme(currentColor)) + "\">"
                    index += 6
                    continue
                }
            }

            if (ch === "\r") {
                continue
            }
            if (ch === "\n") {
                out += "<br>"
                continue
            }

            out += escapeHtml(ch === "_" ? " " : ch)
        }

        out += "</span>"
        return out
    }

    function skillIconSource(skillId) {
        return skillId > 0 ? "image://openmidgard/skill/" + skillId : ""
    }

    function statusIconSource(statusType) {
        return statusType > 0 ? "image://openmidgard/status/" + statusType : ""
    }

    function formatStatusDuration(remainingMs) {
        var totalSeconds = Math.max(0, Math.ceil((remainingMs || 0) / 1000))
        var seconds = totalSeconds % 60
        var totalMinutes = Math.floor(totalSeconds / 60)
        var hours = Math.floor(totalMinutes / 60)
        var minutes = totalMinutes % 60

        if (hours > 0) {
            return hours + ":" + ("0" + minutes).slice(-2) + ":" + ("0" + seconds).slice(-2)
        }
        return totalMinutes + ":" + ("0" + seconds).slice(-2)
    }

    function statusTooltipText(statusData) {
        if (!statusData) {
            return ""
        }
        if (statusData.timed) {
            return (statusData.name || "") + "\n" + formatStatusDuration(statusData.remainingMs || 0) + " remaining"
        }
        return (statusData.name || "") + "\nActive"
    }

    function statusExpiryTintFraction(statusData) {
        if (!statusData || !statusData.timed) {
            return 0
        }
        var remainingMs = Math.max(0, statusData.remainingMs || 0)
        if (remainingMs >= 60000) {
            return 0
        }
        return 1 - (remainingMs / 60000)
    }

    function equipPreviewSource() {
        const revision = uiState.equipData.previewRevision || "0"
        return "image://openmidgard/equippreview?rev=" + revision
    }

    function makeCharPanelButtonsKey() {
        var buttons = uiState && uiState.makeCharButtons ? uiState.makeCharButtons : []
        var key = ""
        for (var i = 0; i < buttons.length; ++i) {
            key += (buttons[i].id || i) + ":" + (buttons[i].pressed ? "1" : "0") + ";"
        }
        return key
    }

    function charSelectPanelKey() {
        var details = uiState && uiState.charSelectSelectedDetails ? uiState.charSelectSelectedDetails : {}
        return (details.imageRevision || "0") + "-s" + (uiState.skinRevision || "0")
    }

    Timer {
        interval: 500
        running: uiState.loginPanelVisible || uiState.makeCharVisible
        repeat: true
        onTriggered: root.loginCaretVisible = !root.loginCaretVisible
    }
    Image {
        anchors.fill: parent
        visible: source !== ""
        fillMode: Image.Stretch
        smooth: false
        cache: false
        source: (uiState.loginPanelVisible
            || uiState.serverSelectVisible
            || uiState.charSelectVisible
            || uiState.makeCharVisible
            || uiState.loadingVisible)
            ? "image://openmidgard/wallpaper?rev=" + encodeURIComponent(uiState.wallpaperRevision || "none") + "&skin=" + encodeURIComponent(uiState.skinRevision || "0")
            : ""
    }

    Repeater {
        model: uiState.anchors

        delegate: Item {
            required property var modelData
            property bool isPartyHpBar: modelData.kind === "partyHpBar"
            property int bubblePaddingX: modelData.showBubble === false ? 0 : (modelData.paddingX || 12)
            property int bubblePaddingY: modelData.showBubble === false ? 0 : (modelData.paddingY || 8)
            property int maxTextWidth: modelData.maxTextWidth || 0
            property bool wrapText: modelData.wrap === true
            property int textWidth: {
                const measured = Math.max(1, Math.ceil(label.contentWidth || label.implicitWidth || 1))
                return maxTextWidth > 0 ? Math.min(maxTextWidth, measured) : measured
            }
            property int anchorX: modelData.centerX !== undefined
                ? Math.round((modelData.centerX || 0) - width / 2)
                : (modelData.x || 0)
            property int anchorY: modelData.bottomY !== undefined
                ? Math.round((modelData.bottomY || 0) - height)
                : (modelData.y || 0)
            x: root.clampOverlayX(anchorX, width)
            y: root.clampOverlayY(anchorY, height)
            z: modelData.z || 2000
            width: isPartyHpBar ? (modelData.width || 60) : (textWidth + bubblePaddingX)
            height: isPartyHpBar ? (modelData.height || 5) : (Math.max(1, Math.ceil(label.implicitHeight)) + bubblePaddingY)

            Rectangle {
                visible: parent.isPartyHpBar
                x: 0
                y: 0
                width: parent.width
                height: parent.height
                color: "#14199b"

                Rectangle {
                    x: 1
                    y: 1
                    width: parent.width - 2
                    height: parent.height - 2
                    color: "#1e243a"
                }

                Rectangle {
                    x: 1
                    y: 1
                    width: {
                        const maxHp = Math.max(1, modelData.maxHp || 0)
                        const currentHp = Math.max(0, Math.min(modelData.currentHp || 0, maxHp))
                        return Math.max(0, Math.floor((parent.width - 2) * currentHp / maxHp))
                    }
                    height: parent.height - 2
                    color: (modelData.maxHp > 0 && (modelData.currentHp || 0) * 4 < (modelData.maxHp || 0)) ? "#ea1616" : "#16ea25"
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: modelData.radius || 6
                color: modelData.background
                visible: !parent.isPartyHpBar && modelData.showBubble !== false
                border.width: 1
                border.color: modelData.borderColor || "#80ffffff"
            }

            Text {
                id: label
                x: Math.floor(parent.bubblePaddingX / 2)
                y: Math.floor(parent.bubblePaddingY / 2)
                width: parent.textWidth
                text: modelData.text
                visible: !parent.isPartyHpBar
                color: modelData.foreground
                font.pixelSize: modelData.fontPixelSize || 14
                font.bold: modelData.bold !== false
                wrapMode: parent.wrapText ? Text.Wrap : Text.NoWrap
                horizontalAlignment: Text.AlignHCenter
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

    Item {
        id: uiLayer
        x: 0
        y: 0
        width: root.uiScale > 0 ? root.width / root.uiScale : root.width
        height: root.uiScale > 0 ? root.height / root.uiScale : root.height
        scale: root.uiScale
        transformOrigin: Item.TopLeft

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
                    required property int index
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
                        textFormat: Text.RichText
                        font.pixelSize: 12
                        clip: true
                    }
                }
            }
        }

        Repeater {
            model: uiState.npcMenuButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.npcMenuX
                y: (modelData.y || 0) - uiState.npcMenuY
                width: modelData.width || 0
                height: modelData.height || 0
                color: modelData.pressed ? "#c4c4c4" : "#f0f0f0"
                border.width: 1
                border.color: "#6e6e6e"

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: "#000000"
                    font.pixelSize: 12
                }
            }
        }
    }

    Rectangle {
        x: uiState.sayDialogX
        y: uiState.sayDialogY
        width: uiState.sayDialogWidth
        height: uiState.sayDialogHeight
        radius: 10
        color: Theme.surface
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.sayDialogVisible

        Text {
            x: 10
            y: 10
            width: parent.width - 20
            height: uiState.sayDialogHasAction ? (parent.height - 10 - 10 - 22 - 8) : (parent.height - 20)
            text: uiState.sayDialogText
            textFormat: Text.RichText
            color: Theme.text
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
        }

        Rectangle {
            x: (uiState.sayDialogActionButton.x || 0) - uiState.sayDialogX
            y: (uiState.sayDialogActionButton.y || 0) - uiState.sayDialogY
            width: uiState.sayDialogActionButton.width || 0
            height: uiState.sayDialogActionButton.height || 0
            visible: uiState.sayDialogActionButton.visible || false
            radius: 4
            color: (uiState.sayDialogActionButton.pressed || false)
                ? Theme.buttonBgPressed
                : ((uiState.sayDialogActionButton.hovered || false) ? Theme.buttonBgHover : Theme.buttonBg)
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: uiState.sayDialogActionButton.label || ""
                color: Theme.buttonText
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
        color: Theme.surface
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.npcInputVisible

        Text {
            x: 10
            y: 9
            width: parent.width - 20
            text: uiState.npcInputLabel
            color: Theme.text
            font.pixelSize: 12
        }

        Rectangle {
            x: 10
            y: 26
            width: parent.width - 20
            height: 22
            radius: 3
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.inputBorder

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 12
                text: uiState.npcInputText
                color: Theme.inputText
                font.pixelSize: 12
                elide: Text.ElideRight
            }
        }

        Repeater {
            model: uiState.npcInputButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.npcInputX
                y: (modelData.y || 0) - uiState.npcInputY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 4
                color: modelData.pressed ? Theme.buttonBgPressed : Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 12
                }
            }
        }
    }

    Rectangle {
        x: uiState.chooseMenuX
        y: uiState.chooseMenuY
        width: uiState.chooseMenuWidth
        height: uiState.chooseMenuHeight
        radius: 8
        color: Theme.surface
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.chooseMenuVisible

        Repeater {
            model: uiState.chooseMenuOptions

            delegate: Rectangle {
                required property int index
                required property var modelData
                x: (parent.width - 221) / 2
                y: 12 + index * 23
                width: 221
                height: 20
                radius: 3
                color: index === uiState.chooseMenuPressedIndex
                    ? Theme.buttonBgPressed
                    : (index === uiState.chooseMenuSelectedIndex ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: Theme.buttonText
                    font.pixelSize: 12
                }
            }
        }
    }

    Rectangle {
        id: itemShopWindow
        x: uiState.itemShopX
        y: uiState.itemShopY
        width: uiState.itemShopWidth
        height: uiState.itemShopHeight
        radius: 8
        clip: true
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.itemShopVisible

        readonly property bool showQuantity: uiState.itemShopData.showQuantity || false
        readonly property int qtyColX: 188
        readonly property int qtyColWidth: 40
        readonly property int priceColWidth: 78
        readonly property int priceColMargin: 8
        readonly property int shopRowHeight: uiState.itemShopData.rowHeight || 20
        readonly property int shopIconSize: Math.max(14, shopRowHeight - 4)

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 22
            color: Theme.accent
        }

        Text {
            x: 10
            y: 4
            text: uiState.itemShopData.title || uiState.itemShopTitle
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 8
            y: 26
            width: parent.width - 16
            height: parent.height - 34
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            radius: 4
            clip: true

            Column {
                x: 1
                y: 1
                width: parent.width - 2
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 20
                    color: Theme.surfaceAlt

                    Text {
                        x: 26
                        y: 3
                        width: itemShopWindow.showQuantity
                            ? (itemShopWindow.qtyColX - 30)
                            : (parent.width - itemShopWindow.priceColWidth - itemShopWindow.priceColMargin - 30)
                        text: uiState.itemShopData.nameLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        x: itemShopWindow.qtyColX
                        y: 3
                        width: itemShopWindow.qtyColWidth
                        visible: itemShopWindow.showQuantity
                        text: uiState.itemShopData.quantityLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        x: parent.width - itemShopWindow.priceColWidth - itemShopWindow.priceColMargin
                        y: 3
                        width: itemShopWindow.priceColWidth
                        text: uiState.itemShopData.priceLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Repeater {
                    model: uiState.itemShopRows

                    delegate: Rectangle {
                        required property var modelData
                        width: parent.width
                        height: itemShopWindow.shopRowHeight
                        color: modelData.selected
                            ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.28)
                            : (modelData.hover
                                ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
                                : "transparent")

                        Image {
                            x: 4
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemShopWindow.shopIconSize
                            height: itemShopWindow.shopIconSize
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            source: root.itemIconSource(
                                modelData.itemId || 0,
                                modelData.itemIndex || 0,
                                modelData.identified)
                            visible: (modelData.itemId || 0) > 0 || (modelData.itemIndex || 0) > 0
                        }

                        Text {
                            x: itemShopWindow.shopIconSize + 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemShopWindow.showQuantity
                                ? (itemShopWindow.qtyColX - (itemShopWindow.shopIconSize + 14))
                                : (parent.width - itemShopWindow.priceColWidth - itemShopWindow.priceColMargin - (itemShopWindow.shopIconSize + 14))
                            text: modelData.name
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemShopWindow.shopRowHeight - 8))
                            elide: Text.ElideRight
                        }

                        Text {
                            x: itemShopWindow.qtyColX
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemShopWindow.qtyColWidth
                            visible: itemShopWindow.showQuantity
                            text: modelData.quantity
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemShopWindow.shopRowHeight - 8))
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            x: parent.width - itemShopWindow.priceColWidth - itemShopWindow.priceColMargin
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemShopWindow.priceColWidth
                            text: modelData.price
                            color: Theme.accent
                            font.pixelSize: Math.min(13, Math.max(11, itemShopWindow.shopRowHeight - 8))
                            font.bold: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: itemPurchaseWindow
        x: uiState.itemPurchaseX
        y: uiState.itemPurchaseY
        width: uiState.itemPurchaseWidth
        height: uiState.itemPurchaseHeight
        radius: 8
        clip: true
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.itemPurchaseVisible

        readonly property int qtyColX: 132
        readonly property int qtyColWidth: 36
        readonly property int costColWidth: 60
        readonly property int costColMargin: 8
        readonly property int shopRowHeight: uiState.itemPurchaseData.rowHeight || 20
        readonly property int shopIconSize: Math.max(14, shopRowHeight - 4)

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 22
            color: Theme.accent
        }

        Text {
            x: 10
            y: 4
            text: uiState.itemPurchaseData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 8
            y: 26
            width: parent.width - 16
            height: parent.height - 80
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            radius: 4
            clip: true

            Column {
                x: 1
                y: 1
                width: parent.width - 2
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 20
                    color: Theme.surfaceAlt

                    Text {
                        x: 26
                        y: 3
                        width: itemPurchaseWindow.qtyColX - 30
                        text: uiState.itemPurchaseData.nameLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        x: itemPurchaseWindow.qtyColX
                        y: 3
                        width: itemPurchaseWindow.qtyColWidth
                        text: uiState.itemPurchaseData.quantityLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        x: parent.width - itemPurchaseWindow.costColWidth - itemPurchaseWindow.costColMargin
                        y: 3
                        width: itemPurchaseWindow.costColWidth
                        text: uiState.itemPurchaseData.amountLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Repeater {
                    model: uiState.itemPurchaseRows

                    delegate: Rectangle {
                        required property var modelData
                        width: parent.width
                        height: itemPurchaseWindow.shopRowHeight
                        color: modelData.selected
                            ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.28)
                            : (modelData.hover
                                ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
                                : "transparent")

                        Image {
                            x: 4
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemPurchaseWindow.shopIconSize
                            height: itemPurchaseWindow.shopIconSize
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            source: root.itemIconSource(
                                modelData.itemId || 0,
                                modelData.itemIndex || 0,
                                modelData.identified)
                            visible: (modelData.itemId || 0) > 0 || (modelData.itemIndex || 0) > 0
                        }

                        Text {
                            x: itemPurchaseWindow.shopIconSize + 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemPurchaseWindow.qtyColX - (itemPurchaseWindow.shopIconSize + 14)
                            text: modelData.name
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemPurchaseWindow.shopRowHeight - 8))
                            elide: Text.ElideRight
                        }

                        Text {
                            x: itemPurchaseWindow.qtyColX
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemPurchaseWindow.qtyColWidth
                            text: "x" + modelData.quantity
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemPurchaseWindow.shopRowHeight - 8))
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            x: parent.width - itemPurchaseWindow.costColWidth - itemPurchaseWindow.costColMargin
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemPurchaseWindow.costColWidth
                            text: modelData.cost
                            color: Theme.accent
                            font.pixelSize: Math.min(13, Math.max(11, itemPurchaseWindow.shopRowHeight - 8))
                            font.bold: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
        }

        Rectangle {
            x: 10
            y: itemPurchaseWindow.height - 58
            width: itemPurchaseWindow.width - 20
            height: 22
            radius: 4
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border

            Text {
                x: 10
                anchors.verticalCenter: parent.verticalCenter
                text: uiState.itemPurchaseData.totalLabel || ""
                color: Theme.textMuted
                font.pixelSize: 11
                font.bold: true
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: uiState.itemPurchaseTotal
                color: Theme.accent
                font.pixelSize: 12
                font.bold: true
            }
        }

        Repeater {
            model: uiState.itemPurchaseButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.itemPurchaseX
                y: (modelData.y || 0) - uiState.itemPurchaseY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 4
                color: modelData.pressed
                    ? Theme.buttonBgPressed
                    : (modelData.hot ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        id: itemSellWindow
        x: uiState.itemSellX
        y: uiState.itemSellY
        width: uiState.itemSellWidth
        height: uiState.itemSellHeight
        radius: 8
        clip: true
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.itemSellVisible

        readonly property int qtyColX: 132
        readonly property int qtyColWidth: 36
        readonly property int gainColWidth: 60
        readonly property int gainColMargin: 8
        readonly property int shopRowHeight: uiState.itemSellData.rowHeight || 20
        readonly property int shopIconSize: Math.max(14, shopRowHeight - 4)

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 22
            color: Theme.accent
        }

        Text {
            x: 10
            y: 4
            text: uiState.itemSellData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 8
            y: 26
            width: parent.width - 16
            height: parent.height - 80
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            radius: 4
            clip: true

            Column {
                x: 1
                y: 1
                width: parent.width - 2
                spacing: 0

                Rectangle {
                    width: parent.width
                    height: 20
                    color: Theme.surfaceAlt

                    Text {
                        x: 26
                        y: 3
                        width: itemSellWindow.qtyColX - 30
                        text: uiState.itemSellData.nameLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        x: itemSellWindow.qtyColX
                        y: 3
                        width: itemSellWindow.qtyColWidth
                        text: uiState.itemSellData.quantityLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Text {
                        x: parent.width - itemSellWindow.gainColWidth - itemSellWindow.gainColMargin
                        y: 3
                        width: itemSellWindow.gainColWidth
                        text: uiState.itemSellData.amountLabel || ""
                        color: Theme.textMuted
                        font.pixelSize: 11
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Repeater {
                    model: uiState.itemSellRows

                    delegate: Rectangle {
                        required property var modelData
                        width: parent.width
                        height: itemSellWindow.shopRowHeight
                        color: modelData.selected
                            ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.28)
                            : (modelData.hover
                                ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
                                : "transparent")

                        Image {
                            x: 4
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemSellWindow.shopIconSize
                            height: itemSellWindow.shopIconSize
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            source: root.itemIconSource(
                                modelData.itemId || 0,
                                modelData.itemIndex || 0,
                                modelData.identified)
                            visible: (modelData.itemId || 0) > 0 || (modelData.itemIndex || 0) > 0
                        }

                        Text {
                            x: itemSellWindow.shopIconSize + 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemSellWindow.qtyColX - (itemSellWindow.shopIconSize + 14)
                            text: modelData.name
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemSellWindow.shopRowHeight - 8))
                            elide: Text.ElideRight
                        }

                        Text {
                            x: itemSellWindow.qtyColX
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemSellWindow.qtyColWidth
                            text: "x" + modelData.quantity
                            color: Theme.text
                            font.pixelSize: Math.min(13, Math.max(11, itemSellWindow.shopRowHeight - 8))
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            x: parent.width - itemSellWindow.gainColWidth - itemSellWindow.gainColMargin
                            anchors.verticalCenter: parent.verticalCenter
                            width: itemSellWindow.gainColWidth
                            text: modelData.gain
                            color: Theme.accent
                            font.pixelSize: Math.min(13, Math.max(11, itemSellWindow.shopRowHeight - 8))
                            font.bold: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
        }

        Rectangle {
            x: 10
            y: itemSellWindow.height - 58
            width: itemSellWindow.width - 20
            height: 22
            radius: 4
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border

            Text {
                x: 10
                anchors.verticalCenter: parent.verticalCenter
                text: uiState.itemSellData.totalLabel || ""
                color: Theme.textMuted
                font.pixelSize: 11
                font.bold: true
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: uiState.itemSellTotal
                color: Theme.accent
                font.pixelSize: 12
                font.bold: true
            }
        }

        Repeater {
            model: uiState.itemSellButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.itemSellX
                y: (modelData.y || 0) - uiState.itemSellY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 4
                color: modelData.pressed
                    ? Theme.buttonBgPressed
                    : (modelData.hot ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        x: uiState.shortCutX
        y: uiState.shortCutY
        width: uiState.shortCutWidth
        height: uiState.shortCutHeight
        radius: 4
        color: Theme.surfaceAlt
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.shortCutVisible

        Repeater {
            model: uiState.shortCutSlots

            delegate: Rectangle {
                required property int index
                required property var modelData
                x: 5 + index * 29
                y: 4
                width: 24
                height: 24
                color: modelData.hover ? Theme.buttonBgHover : Theme.inputBg
                border.width: 1
                border.color: modelData.isSkill ? Theme.accent : Theme.border

                Image {
                    id: shortCutIcon
                    anchors.fill: parent
                    anchors.margins: 1
                    fillMode: Image.PreserveAspectFit
                    smooth: false
                    source: modelData.isSkill
                        ? root.skillIconSource(modelData.skillId || 0)
                        : root.itemIconSource(modelData.itemId || 0, 0)
                    visible: modelData.occupied && source !== ""
                }

                Text {
                    anchors.centerIn: parent
                    width: parent.width - 4
                    text: modelData.occupied ? (modelData.isSkill ? "S" : "I") : ""
                    visible: !shortCutIcon.visible
                    color: Theme.text
                    font.pixelSize: 10
                    font.bold: true
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 2
                    anchors.bottom: parent.bottom
                    text: modelData.count > 0 ? modelData.count : ""
                    color: Theme.accentText
                    font.pixelSize: 9
                    font.bold: true
                    style: Text.Outline
                    styleColor: Theme.text
                }
            }
        }

        Text {
            x: width - 25
            y: 16
            width: 12
            text: uiState.shortCutPage
            color: Theme.text
            font.pixelSize: 11
            font.bold: true
            style: Text.Outline
            styleColor: "#000000"
            horizontalAlignment: Text.AlignRight
        }
    }

    Rectangle {
        x: uiState.basicInfoX
        y: uiState.basicInfoY
        width: uiState.basicInfoWidth
        height: uiState.basicInfoHeight
        radius: 8
        color: uiState.basicInfoMini ? Theme.surfaceAlt : Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.basicInfoVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: uiState.basicInfoMini ? 9 : 17
            y: 3
            text: uiState.basicInfoMini ? (uiState.basicInfoData.name || "") : "Basic Info"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.basicInfoData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.basicInfoX
                y: (modelData.y || 0) - uiState.basicInfoY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Text {
            x: 17
            y: 18
            visible: uiState.basicInfoMini
            text: uiState.basicInfoData.miniHeaderText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Text {
            x: 17
            y: 33
            visible: uiState.basicInfoMini
            text: uiState.basicInfoData.miniStatusText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Text {
            x: 9
            y: 24
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.name || ""
            color: Theme.text
            font.pixelSize: 12
        }

        Text {
            x: 9
            y: 38
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.jobName || ""
            color: Theme.text
            font.pixelSize: 12
        }

        Rectangle {
            x: 110
            y: 22
            width: 85
            height: 9
            visible: !uiState.basicInfoMini
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.borderStrong

            Rectangle {
                x: 1
                y: 1
                width: Math.max(0, (parent.width - 2) * ((uiState.basicInfoData.maxHp || 0) > 0 ? (uiState.basicInfoData.hp || 0) / uiState.basicInfoData.maxHp : 0))
                height: parent.height - 2
                color: Theme.hpFill
            }
        }

        Rectangle {
            x: 110
            y: 43
            width: 85
            height: 9
            visible: !uiState.basicInfoMini
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.borderStrong

            Rectangle {
                x: 1
                y: 1
                width: Math.max(0, (parent.width - 2) * ((uiState.basicInfoData.maxSp || 0) > 0 ? (uiState.basicInfoData.sp || 0) / uiState.basicInfoData.maxSp : 0))
                height: parent.height - 2
                color: Theme.spFill
            }
        }

        Text {
            x: 95
            y: 31
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.hpText || ""
            color: Theme.text
            font.pixelSize: 10
        }

        Text {
            x: 95
            y: 52
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.spText || ""
            color: Theme.text
            font.pixelSize: 10
        }

        Text {
            x: 17
            y: 72
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.baseLevelText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Rectangle {
            x: 84
            y: 77
            width: 102
            height: 6
            visible: !uiState.basicInfoMini
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border

            Rectangle {
                x: 1
                y: 1
                width: Math.max(0, (parent.width - 2) * ((uiState.basicInfoData.expPercent || 0) / 100.0))
                height: parent.height - 2
                color: Theme.expFill
            }
        }

        Text {
            x: 17
            y: 84
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.jobLevelText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Rectangle {
            x: 84
            y: 87
            width: 102
            height: 6
            visible: !uiState.basicInfoMini
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border

            Rectangle {
                x: 1
                y: 1
                width: Math.max(0, (parent.width - 2) * ((uiState.basicInfoData.jobExpPercent || 0) / 100.0))
                height: parent.height - 2
                color: Theme.spFill
            }
        }

        Text {
            x: 5
            y: 103
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.weightText || ""
            color: ((uiState.basicInfoData.maxWeight || 0) > 0 && (uiState.basicInfoData.weight || 0) * 100 >= uiState.basicInfoData.maxWeight * 50)
                ? Theme.hpFill
                : Theme.text
            font.pixelSize: 11
        }

        Text {
            x: 107
            y: 103
            visible: !uiState.basicInfoMini
            text: uiState.basicInfoData.moneyText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Text {
            x: 5
            y: 118
            visible: !uiState.basicInfoMini && (uiState.basicInfoData.cartActive || false)
            text: "Cart : " + (uiState.basicInfoData.cartCurrentCount || 0)
                + " / " + (uiState.basicInfoData.cartMaxCount || 0)
                + "  (" + (uiState.basicInfoData.cartCurrentWeight || 0)
                + " / " + (uiState.basicInfoData.cartMaxWeight || 0) + ")"
            color: ((uiState.basicInfoData.cartMaxWeight || 0) > 0
                    && (uiState.basicInfoData.cartCurrentWeight || 0) * 100
                        >= (uiState.basicInfoData.cartMaxWeight || 0) * 90)
                ? Theme.hpFill
                : Theme.text
            font.pixelSize: 11
        }

        Repeater {
            model: uiState.basicInfoData.menuButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.basicInfoX
                y: (modelData.y || 0) - uiState.basicInfoY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 3
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        x: uiState.statusX
        y: uiState.statusY
        width: uiState.statusWidth
        height: uiState.statusHeight
        radius: 8
        color: uiState.statusMini ? Theme.surfaceAlt : Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.statusVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: uiState.statusMini ? Theme.surface : Theme.accent
            border.width: 1
            border.color: uiState.statusMini ? Theme.border : Theme.borderStrong
        }

        Text {
            x: 17
            y: 3
            text: uiState.statusData.title || ""
            color: uiState.statusMini ? Theme.text : Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.statusData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.statusX
                y: (modelData.y || 0) - uiState.statusY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Text {
            x: 96
            y: 3
            visible: uiState.statusMini
            text: uiState.statusData.miniPointsText || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Rectangle {
            x: 0
            y: 17
            width: parent.width
            height: parent.height - 17
            color: Theme.surface
            border.width: 0
            visible: !uiState.statusMini

            Rectangle {
                x: 0
                y: 0
                width: 20
                height: parent.height
                color: Theme.surfaceAlt
            }

            Repeater {
                model: uiState.statusData.pageTabs || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.statusX
                    y: (modelData.y || 0) - uiState.statusY - 17
                    width: modelData.width || 0
                    height: modelData.height || 0
                    color: modelData.active ? Theme.surface : Theme.surfaceAlt
                    border.width: 1
                    border.color: modelData.active ? Theme.borderStrong : Theme.border
                    visible: modelData.visible || false

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label || ""
                        color: Theme.text
                        font.pixelSize: 10
                        font.bold: modelData.active || false
                    }
                }
            }

            Column {
                x: 28
                y: 5
                spacing: 2
                visible: uiState.statusPage === 0

                Repeater {
                    model: uiState.statusData.stats || []

                    delegate: Item {
                        required property var modelData
                        width: 90
                        height: 14

                        Text {
                            x: 0
                            y: 0
                            text: modelData.label
                            color: Theme.text
                            font.pixelSize: 11
                            font.bold: true
                        }

                        Text {
                            x: 26
                            y: 0
                            width: 34
                            text: modelData.value
                            color: Theme.text
                            font.pixelSize: 11
                        }

                        Rectangle {
                            x: 64
                            y: 1
                            width: modelData.increaseWidth || 0
                            height: modelData.increaseHeight || 0
                            radius: 2
                            visible: modelData.canIncrease
                            color: Theme.buttonBg
                            border.width: 1
                            border.color: Theme.buttonBorder

                            Text {
                                anchors.centerIn: parent
                                text: modelData.increaseLabel || ""
                                color: Theme.buttonText
                                font.pixelSize: 8
                                font.bold: true
                            }
                        }

                        Text {
                            x: 78
                            y: 0
                            width: 10
                            text: modelData.cost > 0 ? modelData.cost : ""
                            color: Theme.textMuted
                            font.pixelSize: 10
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }

            Text {
                x: 132
                y: 10
                visible: uiState.statusPage === 0
                text: "Atk"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 152
                y: 10
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.attackText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 132
                y: 26
                visible: uiState.statusPage === 0
                text: "Matk"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 152
                y: 26
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.matkText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 132
                y: 42
                visible: uiState.statusPage === 0
                text: "Hit"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 152
                y: 42
                width: 40
                visible: uiState.statusPage === 0
                text: (uiState.statusData.hit || 0).toString()
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 132
                y: 58
                visible: uiState.statusPage === 0
                text: "Crit"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 152
                y: 58
                width: 40
                visible: uiState.statusPage === 0
                text: (uiState.statusData.critical || 0).toString()
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 204
                y: 74
                visible: uiState.statusPage === 0
                text: "Pts"
                color: Theme.text
                font.pixelSize: 11
                font.bold: true
            }

            Text {
                x: 243
                y: 74
                width: 30
                visible: uiState.statusPage === 0
                text: (uiState.statusData.statusPoint || 0).toString()
                color: Theme.text
                font.pixelSize: 11
                font.bold: true
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 204
                y: 10
                visible: uiState.statusPage === 0
                text: "Def"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 233
                y: 10
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.itemDefText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 204
                y: 26
                visible: uiState.statusPage === 0
                text: "Mdef"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 233
                y: 26
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.itemMdefText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 204
                y: 42
                visible: uiState.statusPage === 0
                text: "Flee"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 233
                y: 42
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.fleeText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }

            Text {
                x: 204
                y: 58
                visible: uiState.statusPage === 0
                text: "Aspd"
                color: Theme.text
                font.pixelSize: 11
            }

            Text {
                x: 233
                y: 58
                width: 40
                visible: uiState.statusPage === 0
                text: uiState.statusData.aspdText || ""
                color: Theme.text
                font.pixelSize: 11
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    Rectangle {
        id: chatWindowChrome
        x: uiState.chatWindowX
        y: uiState.chatWindowY
        width: uiState.chatWindowWidth
        height: uiState.chatWindowHeight
        radius: 14
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.chatWindowVisible

        readonly property var chatUi: uiState.chatWindowUi || ({})
        readonly property var scrollBar: uiState.chatWindowScrollBar || ({})
        readonly property bool scrollBarVisible: scrollBar.visible || false
        readonly property int scrollBarWidth: scrollBarVisible ? 8 : 0
        readonly property int scrollBarGap: scrollBarVisible ? 4 : 0
        readonly property int chatFontSize: chatUi.fontPixelSize || 13
        readonly property real chromeOpacity: root.chatChromeOpacity(chatUi)
        color: Theme.background

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Theme.surfaceAlt
            opacity: 0.35 + (0.25 * chatWindowChrome.chromeOpacity)
        }

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 52
            radius: 13
            color: Theme.surfaceAlt
            opacity: 0.55 + (0.15 * chatWindowChrome.chromeOpacity)
            border.width: 0
        }

        Row {
            x: 10
            y: 10
            height: 34
            width: parent.width - 54
            spacing: 6

            Repeater {
                model: parent.parent.chatUi.tabs || []

                delegate: Rectangle {
                    required property var modelData
                    width: Math.max(68, Math.floor((parent.width - ((parent.spacing || 0) * Math.max(0, ((parent.parent.chatUi.tabs || []).length || 1) - 1))) / Math.max(1, ((parent.parent.chatUi.tabs || []).length || 1))))
                    height: 34
                    radius: 10
                    color: modelData.active ? Theme.surface : Theme.surfaceAlt
                    border.width: 1
                    border.color: modelData.active ? Theme.borderStrong : Theme.border

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        color: modelData.active ? Theme.text : Theme.textMuted
                        font.family: "Segoe UI"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }
            }
        }

        Rectangle {
            x: parent.width - 34
            y: 15
            width: 24
            height: 24
            radius: 8
            color: parent.chatUi.configVisible ? Theme.buttonBgHover : Theme.buttonBg
            border.width: 1
            border.color: parent.chatUi.configVisible ? Theme.borderStrong : Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: "\u2699"
                color: Theme.buttonText
                font.family: "Segoe UI Symbol"
                font.pixelSize: 15
            }
        }

        Rectangle {
            x: 10
            y: 54
            width: parent.width - 20
            height: parent.height - 92
            radius: 12
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border
            clip: true

            readonly property int chatTextWidth: width - 10 - parent.scrollBarWidth - parent.scrollBarGap

            Rectangle {
                anchors.fill: parent
                radius: 12
                color: Theme.surface
                opacity: 0.12 + (0.08 * chatWindowChrome.chromeOpacity)
            }

            Column {
                id: chatLinesColumn
                x: 5
                y: Math.max(6, parent.height - height - 6)
                width: parent.chatTextWidth
                spacing: 2

                Repeater {
                    model: uiState.chatWindowLines

                    delegate: Text {
                        required property var modelData
                        width: chatLinesColumn.width
                        text: modelData.text
                        textFormat: Text.PlainText
                        color: modelData.color
                        font.family: "Segoe UI"
                        font.pixelSize: parent.parent.parent.chatFontSize
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Rectangle {
                x: parent.width - 5 - width
                y: 5
                width: parent.scrollBarWidth
                height: parent.height - 10
                readonly property var scrollBarModel: chatWindowChrome.scrollBar
                visible: chatWindowChrome.scrollBarVisible
                radius: 4
                color: Theme.scrollTrack
                border.width: 1
                border.color: Theme.border

                Rectangle {
                    readonly property int totalLines: Math.max(1, parent.scrollBarModel.totalLines || 0)
                    readonly property int visibleLineCount: Math.max(1, parent.scrollBarModel.visibleLineCount || 0)
                    readonly property int firstVisibleLine: Math.max(0, parent.scrollBarModel.firstVisibleLine || 0)
                    readonly property int thumbHeight: Math.max(18, Math.round(parent.height * visibleLineCount / totalLines))
                    readonly property int maxTravel: Math.max(0, parent.height - thumbHeight)
                    readonly property int scrollDenominator: Math.max(1, totalLines - visibleLineCount)
                    x: 1
                    y: Math.round(maxTravel * firstVisibleLine / scrollDenominator)
                    width: parent.width - 2
                    height: thumbHeight
                    radius: 3
                    color: Theme.scrollThumb
                }
            }
        }

        Rectangle {
            x: 10
            y: parent.height - 38
            width: 126
            height: 28
            radius: 9
            color: uiState.chatWindowWhisperInputActive ? Theme.surface : Theme.inputBg
            border.width: 1
            border.color: uiState.chatWindowWhisperInputActive ? Theme.borderStrong : Theme.inputBorder

            Text {
                x: 8
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 16
                text: {
                    const baseText = uiState.chatWindowWhisperTargetText.length > 0 || uiState.chatWindowWhisperInputActive
                        ? uiState.chatWindowWhisperTargetText
                        : "To"
                    return baseText + (uiState.chatWindowWhisperInputActive ? "_" : "")
                }
                color: uiState.chatWindowWhisperTargetText.length > 0 || uiState.chatWindowWhisperInputActive ? Theme.inputText : Theme.textMuted
                font.family: "Segoe UI"
                font.pixelSize: 13
                elide: Text.ElideLeft
            }
        }

        Rectangle {
            x: 144
            y: parent.height - 38
            width: parent.width - 154
            height: 28
            radius: 9
            color: uiState.chatWindowMessageInputActive ? Theme.surface : Theme.inputBg
            border.width: 1
            border.color: uiState.chatWindowMessageInputActive ? Theme.borderStrong : Theme.inputBorder

            Text {
                x: 8
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 16
                text: uiState.chatWindowInputText + (uiState.chatWindowMessageInputActive ? "_" : "")
                color: Theme.inputText
                font.family: "Segoe UI"
                font.pixelSize: 13
                elide: Text.ElideLeft
            }
        }
    }

    Rectangle {
        visible: uiState.chatWindowVisible && (uiState.chatWindowUi.configVisible || false)
        x: uiState.chatWindowUi.configX || 0
        y: uiState.chatWindowUi.configY || 0
        width: uiState.chatWindowUi.configWidth || 0
        height: uiState.chatWindowUi.configHeight || 0
        radius: 14
        color: Theme.surface
        border.width: 1
        border.color: Theme.borderStrong

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 42
            radius: 13
            color: Theme.accent
        }

        Text {
            x: 14
            y: 12
            text: "Chat Settings"
            color: Theme.accentText
            font.family: "Segoe UI"
            font.pixelSize: 17
            font.bold: true
        }

        Text {
            x: 14
            y: 50
            text: "Tabs"
            color: Theme.text
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Column {
            x: 12
            y: 72
            width: 92
            spacing: 6

            Repeater {
                model: uiState.chatWindowUi.tabs || []

                delegate: Rectangle {
                    required property var modelData
                    width: parent.width
                    height: 28
                    radius: 8
                    color: modelData.active ? Theme.buttonBgHover : Theme.surface
                    border.width: 1
                    border.color: modelData.active ? Theme.borderStrong : Theme.border

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        color: Theme.text
                        font.family: "Segoe UI"
                        font.pixelSize: 12
                        font.bold: modelData.active
                    }
                }
            }
        }

        Text {
            x: 120
            y: 50
            text: "Font"
            color: Theme.text
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 120
            y: 72
            width: parent.width - 132
            height: 28
            radius: 8
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.inputBorder
        }

        Text {
            x: 132
            y: 79
            text: "Size " + (uiState.chatWindowUi.fontPixelSize || 13)
            color: Theme.inputText
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: parent.width - 76
            y: 72
            width: 28
            height: 28
            radius: 8
            color: Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: "-"
                color: Theme.buttonText
                font.family: "Segoe UI"
                font.pixelSize: 16
                font.bold: true
            }
        }

        Rectangle {
            x: parent.width - 40
            y: 72
            width: 28
            height: 28
            radius: 8
            color: Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: "+"
                color: Theme.buttonText
                font.family: "Segoe UI"
                font.pixelSize: 16
                font.bold: true
            }
        }

        Text {
            x: 120
            y: 112
            text: "Transparency"
            color: Theme.text
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 120
            y: 128
            width: parent.width - 132
            height: 28
            radius: 8
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.inputBorder
        }

        Text {
            x: 132
            y: 135
            text: (uiState.chatWindowUi.windowOpacityPercent || 84) + "%"
            color: Theme.inputText
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: 130
            y: 168
            width: parent.width - 152
            height: 8
            radius: 4
            color: Theme.scrollTrack
            border.width: 0

            Rectangle {
                width: Math.max(8, parent.width * (((uiState.chatWindowUi.windowOpacityPercent || 84) - 20) / 80))
                height: parent.height
                radius: 4
                color: Theme.accent
            }

            Rectangle {
                x: Math.max(0, Math.min(parent.width - 14, parent.width * (((uiState.chatWindowUi.windowOpacityPercent || 84) - 20) / 80) - 7))
                y: -5
                width: 14
                height: 18
                radius: 7
                color: Theme.surface
                border.width: 1
                border.color: Theme.borderStrong
            }
        }

        Text {
            x: 120
            y: 192
            text: "Message Types"
            color: Theme.text
            font.family: "Segoe UI"
            font.pixelSize: 12
            font.bold: true
        }

        Grid {
            x: 120
            y: 216
            width: parent.width - 132
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            Repeater {
                model: uiState.chatWindowUi.filters || []

                delegate: Rectangle {
                    required property var modelData
                    width: Math.floor((parent.width - 8) / 2)
                    height: 28
                    radius: 8
                    color: modelData.enabled ? Theme.accent : Theme.surface
                    border.width: 1
                    border.color: modelData.enabled ? Theme.borderStrong : Theme.border

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label
                        color: modelData.enabled ? Theme.accentText : Theme.text
                        font.family: "Segoe UI"
                        font.pixelSize: 12
                        font.bold: modelData.enabled
                    }
                }
            }
        }

        Rectangle {
            x: parent.width - 116
            y: parent.height - 40
            width: 104
            height: 28
            radius: 8
            color: Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: "Reset Tab"
                color: Theme.buttonText
                font.family: "Segoe UI"
                font.pixelSize: 12
                font.bold: true
            }
        }
    }

    Rectangle {
        x: uiState.rechargeGaugeX
        y: uiState.rechargeGaugeY
        width: uiState.rechargeGaugeWidth
        height: uiState.rechargeGaugeHeight
        color: "#523f2d"
        border.width: 1
        border.color: "#523f2d"
        visible: uiState.rechargeGaugeVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: parent.height - 2
            color: "#f4efe4"
        }

        Rectangle {
            x: 1
            y: 1
            width: Math.max(0, (parent.width - 2) * ((uiState.rechargeGaugeTotal || 0) > 0
                ? (uiState.rechargeGaugeAmount || 0) / uiState.rechargeGaugeTotal : 0))
            height: parent.height - 2
            color: "#307830"
        }
    }

    Rectangle {
        x: uiState.minimapX
        y: uiState.minimapY
        width: uiState.minimapWidth
        height: uiState.minimapHeight
        radius: 9
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.minimapVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 8
            color: Theme.accent
        }

        Rectangle {
            id: minimapMapFrame
            x: (uiState.minimapData.mapX || 0) - parent.x
            y: (uiState.minimapData.mapY || 0) - parent.y
            width: uiState.minimapData.mapWidth || 0
            height: uiState.minimapData.mapHeight || 0
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border
            clip: true

            Image {
                anchors.fill: parent
                fillMode: Image.Stretch
                smooth: false
                cache: false
                source: parent.visible ? ("image://openmidgard/minimap?rev=" + (uiState.minimapData.imageRevision || 0)) : ""
            }

            Repeater {
                model: uiState.minimapData.markers || []

                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x - (uiState.minimapData.mapX || 0) - modelData.radius
                    y: modelData.y - (uiState.minimapData.mapY || 0) - modelData.radius
                    width: modelData.radius * 2 + 1
                    height: modelData.radius * 2 + 1
                    radius: width / 2
                    color: modelData.color
                    border.width: 1
                    border.color: "#181818"
                }
            }

            Item {
                    property real headingDegrees: ((180 - (((uiState.minimapData.playerDirection || 0) % 8) * 45)) + 360) % 360

                    width: 19
                    height: 19
                visible: uiState.minimapData.playerVisible || false
                x: (uiState.minimapData.playerX || 0) - (uiState.minimapData.mapX || 0) - width / 2
                y: (uiState.minimapData.playerY || 0) - (uiState.minimapData.mapY || 0) - height / 2
                transformOrigin: Item.Center
                rotation: headingDegrees

                Canvas {
                    anchors.fill: parent
                    antialiasing: false

                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.fillStyle = "#ffffff"
                        ctx.strokeStyle = "#000000"
                        ctx.lineWidth = 1
                        ctx.beginPath()
                        ctx.moveTo(width / 2, 0.5)
                        ctx.lineTo(width - 1.5, height - 3.5)
                        ctx.lineTo(width / 2, height - 6.5)
                        ctx.lineTo(1.5, height - 3.5)
                        ctx.closePath()
                        ctx.fill()
                        ctx.stroke()
                    }
                }
            }
        }

        Text {
            x: 18
            y: 2
            width: Math.max(0, parent.width - 40)
            text: uiState.minimapData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
            elide: Text.ElideRight
        }

        Text {
            x: (uiState.minimapData.coordsX || 0) - parent.x
            y: (uiState.minimapData.coordsY || 0) - parent.y
            width: Math.max(0, uiState.minimapData.coordsWidth || 0)
            horizontalAlignment: Text.AlignRight
            text: uiState.minimapData.coordsText || ""
            color: Theme.text
            font.pixelSize: 11
            elide: Text.ElideRight
        }

        Rectangle {
            x: (uiState.minimapData.closeX || 0) - parent.x
            y: (uiState.minimapData.closeY || 0) - parent.y
            width: Math.max(10, uiState.minimapData.closeWidth || 0)
            height: Math.max(10, uiState.minimapData.closeHeight || 0)
            radius: 2
            color: uiState.minimapData.closePressed ? Theme.buttonBgPressed : Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: uiState.minimapData.closeLabel || ""
                color: Theme.buttonText
                font.pixelSize: 10
                font.bold: true
            }
        }
    }

    Item {
        x: uiState.minimapX + Math.max(0, uiState.minimapWidth - width)
        y: uiState.minimapY + uiState.minimapHeight + 6
        width: 56
        height: {
            const iconCount = (uiState.statusIcons || []).length
            return iconCount > 0 ? (iconCount * 56) + ((iconCount - 1) * 6) : 0
        }
        visible: uiState.minimapVisible && (uiState.statusIcons || []).length > 0
        z: 72

        Column {
            anchors.fill: parent
            spacing: 6

            Repeater {
                model: uiState.statusIcons || []

                delegate: Item {
                    required property var modelData
                    property real tintFraction: statusExpiryTintFraction(modelData)

                    width: 56
                    height: 56

                    Item {
                        anchors.fill: parent
                        clip: true

                        Image {
                            id: statusIconImage
                            anchors.fill: parent
                            source: statusIconSource(modelData.statusType || 0)
                            visible: source !== "" && status === Image.Ready
                            smooth: false
                            cache: false
                        }

                        Rectangle {
                            anchors.fill: parent
                            visible: !statusIconImage.visible
                            color: "#556574"
                            radius: 8
                        }

                        Text {
                            anchors.centerIn: parent
                            visible: !statusIconImage.visible
                            text: modelData.shortName || "?"
                            color: "#f6f1e0"
                            font.pixelSize: 18
                            font.bold: true
                        }

                        Rectangle {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: Math.round(parent.height * tintFraction)
                            visible: tintFraction > 0
                            color: "#c92f2f"
                            opacity: 0.2 + (0.45 * tintFraction)
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        x: uiState.inventoryX
        y: uiState.inventoryY
        width: uiState.inventoryWidth
        height: uiState.inventoryHeight
        radius: 8
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.inventoryVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 17
            y: 3
            text: uiState.inventoryData.title || "Inventory"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.inventoryData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.inventoryX
                y: (modelData.y || 0) - uiState.inventoryY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: 0
            y: 17
            width: parent.width
            height: parent.height - 17
            color: Theme.surface
            visible: !uiState.inventoryMini

            Repeater {
                model: uiState.inventoryData.tabs || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.inventoryX
                    y: (modelData.y || 0) - uiState.inventoryY - 17
                    width: modelData.width || 0
                    height: modelData.height || 0
                    color: modelData.active ? Theme.surface : Theme.surfaceAlt
                    border.width: 1
                    border.color: Theme.buttonBorder
                    visible: modelData.visible || false

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label || ""
                        color: Theme.text
                        font.pixelSize: 9
                        font.bold: modelData.active || false
                    }
                }
            }

            Repeater {
                model: uiState.inventoryData.slots || []

                delegate: Item {
                    required property var modelData
                    x: modelData.x - uiState.inventoryX
                    y: modelData.y - uiState.inventoryY - 17
                    width: modelData.width
                    height: modelData.height

                    Rectangle {
                        x: 1
                        y: 2
                        width: parent.width
                        height: parent.height
                        radius: 6
                        color: Qt.rgba(0, 0, 0, modelData.hovered ? 0.35 : 0.2)
                        z: -1
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: modelData.hovered ? Theme.buttonBgHover : Theme.inputBg
                        border.width: 1
                        border.color: modelData.hovered ? Theme.accent : Theme.border

                    Image {
                        id: inventoryIcon
                        anchors.centerIn: parent
                        width: Math.max(1, parent.width - 4)
                        height: Math.max(1, parent.height - 4)
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                        cache: false
                        source: modelData.occupied && ((modelData.itemId || 0) > 0 || (modelData.itemIndex || 0) > 0)
                            ? root.itemIconSource(modelData.itemId || 0, modelData.itemIndex || 0)
                            : ""
                        visible: source !== "" && status === Image.Ready
                    }

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 4
                        text: modelData.occupied ? modelData.label : ""
                        color: Theme.text
                        font.pixelSize: 9
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        visible: modelData.occupied && !inventoryIcon.visible
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 3
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                        text: (modelData.count || 0) > 1 ? modelData.count : ""
                        color: Theme.accent
                        font.pixelSize: 9
                        font.bold: true
                    }
                    }
                }
            }

            Rectangle {
                x: (uiState.inventoryData.scrollTrackX || 0) - uiState.inventoryX
                y: (uiState.inventoryData.scrollTrackY || 0) - uiState.inventoryY - 17
                width: uiState.inventoryData.scrollTrackWidth || 0
                height: uiState.inventoryData.scrollTrackHeight || 0
                visible: uiState.inventoryData.scrollBarVisible || false
                color: Theme.scrollTrack
                border.width: 1
                border.color: Theme.border

                Rectangle {
                    x: (uiState.inventoryData.scrollThumbX || 0) - (uiState.inventoryData.scrollTrackX || 0)
                    y: (uiState.inventoryData.scrollThumbY || 0) - (uiState.inventoryData.scrollTrackY || 0)
                    width: uiState.inventoryData.scrollThumbWidth || 0
                    height: uiState.inventoryData.scrollThumbHeight || 0
                    color: Theme.scrollThumb
                    border.width: 1
                    border.color: Theme.borderStrong
                }
            }

            Rectangle {
                x: 0
                y: parent.height - height
                width: parent.width
                height: 21
                color: Theme.surfaceAlt
                border.width: 1
                border.color: Theme.border

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: (uiState.inventoryData.currentItemCount || 0) + " / " + (uiState.inventoryData.maxItemCount || 0)
                    color: Theme.textMuted
                    font.pixelSize: 10
                }
            }
        }
    }

    Rectangle {
        x: uiState.storageX
        y: uiState.storageY
        width: uiState.storageWidth
        height: uiState.storageHeight
        radius: 4
        color: "#ede7dc"
        border.width: 1
        border.color: "#7a6d57"
        visible: uiState.storageVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: "#8c7551"
            border.width: 1
            border.color: "#655238"
        }

        Text {
            x: 17
            y: 3
            text: uiState.storageData.title || "Kafra Storage"
            color: "#fff8ee"
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.storageData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.storageX
                y: (modelData.y || 0) - uiState.storageY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: "#ddd0b8"
                border.width: 1
                border.color: "#8a775a"
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: "#000000"
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: 0
            y: 17
            width: parent.width
            height: parent.height - 17
            color: "#e8decc"
            visible: !uiState.storageMini

            Repeater {
                model: uiState.storageData.tabs || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.storageX
                    y: (modelData.y || 0) - uiState.storageY - 17
                    width: modelData.width || 0
                    height: modelData.height || 0
                    color: modelData.active ? "#efe6d5" : "#cfbea1"
                    border.width: 1
                    border.color: "#8d7b62"
                    visible: modelData.visible || false

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label || ""
                        color: "#000000"
                        font.pixelSize: 9
                        font.bold: modelData.active || false
                    }
                }
            }

            Rectangle {
                x: 36
                y: 0
                width: parent.width - 56
                height: 18
                color: "#dfd6c6"
                border.width: 1
                border.color: "#b6a891"

                Text {
                    x: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Item"
                    color: "#4a3a24"
                    font.pixelSize: 10
                    font.bold: true
                }

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Qty"
                    color: "#4a3a24"
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Repeater {
                model: uiState.storageData.slots || []

                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x - uiState.storageX
                    y: modelData.y - uiState.storageY - 17
                    width: modelData.width
                    height: modelData.height
                    color: modelData.hovered ? "#e5ddce" : "#f5eee3"
                    border.width: 1
                    border.color: modelData.hovered ? "#9b7f56" : "#ab9c86"

                    Image {
                        id: storageIcon
                        x: 5
                        y: Math.floor((parent.height - height) / 2)
                        width: 20
                        height: 20
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                        cache: false
                        source: modelData.occupied && (modelData.itemId || 0) > 0 ? root.itemIconSource(modelData.itemId || 0, 0) : ""
                        visible: source !== "" && status === Image.Ready
                    }

                    Rectangle {
                        x: 4
                        y: 3
                        width: 22
                        height: 20
                        color: "#ebe4d6"
                        border.width: 1
                        border.color: "#a59882"
                        visible: modelData.occupied && !storageIcon.visible
                    }

                    Text {
                        x: 30
                        width: parent.width - 88
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.occupied ? modelData.label : ""
                        color: "#000000"
                        font.pixelSize: 9
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        visible: modelData.occupied && !storageIcon.visible
                    }

                    Text {
                        x: 30
                        width: parent.width - 88
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.occupied ? modelData.label : ""
                        color: "#201c16"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        visible: modelData.occupied && storageIcon.visible
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 6
                        anchors.verticalCenter: parent.verticalCenter
                        text: modelData.occupied ? Math.max(1, modelData.count || 0) : ""
                        color: "#5a4020"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Rectangle {
                x: (uiState.storageData.scrollTrackX || 0) - uiState.storageX
                y: (uiState.storageData.scrollTrackY || 0) - uiState.storageY - 17
                width: uiState.storageData.scrollTrackWidth || 0
                height: uiState.storageData.scrollTrackHeight || 0
                visible: uiState.storageData.scrollBarVisible || false
                color: "#e8dfd3"
                border.width: 1
                border.color: "#aea08b"

                Rectangle {
                    x: (uiState.storageData.scrollThumbX || 0) - (uiState.storageData.scrollTrackX || 0)
                    y: (uiState.storageData.scrollThumbY || 0) - (uiState.storageData.scrollTrackY || 0)
                    width: uiState.storageData.scrollThumbWidth || 0
                    height: uiState.storageData.scrollThumbHeight || 0
                    color: "#b99f74"
                    border.width: 1
                    border.color: "#755a33"
                }
            }

            Rectangle {
                x: 0
                y: parent.height - height
                width: parent.width
                height: 21
                color: "#ddd3c4"
                border.width: 1
                border.color: "#bcaf9b"

                Text {
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: (uiState.storageData.currentItemCount || 0) + " / " + (uiState.storageData.maxItemCount || 0)
                    color: "#5a5144"
                    font.pixelSize: 10
                }
            }
        }
    }

    Rectangle {
        x: uiState.equipX
        y: uiState.equipY
        width: uiState.equipWidth
        height: uiState.equipHeight
        radius: 8
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.equipVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 17
            y: 3
            text: uiState.equipData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.equipData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.equipX
                y: (modelData.y || 0) - uiState.equipY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: 0
            y: 17
            width: parent.width
            height: parent.height - 17
            color: Theme.surface
            visible: !uiState.equipMini

            Rectangle {
                x: 98
                y: 15
                width: Math.max(1, 182 - 98)
                height: Math.max(1, parent.height - y - 12)
                color: Theme.surfaceAlt
                border.width: 1
                border.color: Theme.border

                Image {
                    id: equipPreviewImage
                    anchors.centerIn: parent
                    readonly property real availableWidth: Math.max(1, parent.width - 6)
                    readonly property real availableHeight: Math.max(1, parent.height - 6)
                    readonly property real sourceAspectRatio: (status === Image.Ready && sourceSize.height > 0)
                        ? (sourceSize.width / sourceSize.height)
                        : 1.0
                    width: Math.min(availableWidth, availableHeight * sourceAspectRatio)
                    height: Math.min(availableHeight, availableWidth / Math.max(0.001, sourceAspectRatio))
                    fillMode: Image.PreserveAspectFit
                    smooth: false
                    cache: false
                    source: parent.visible ? root.equipPreviewSource() : ""
                }
            }

            Repeater {
                model: uiState.equipData.slots || []

                delegate: Item {
                    required property var modelData
                    x: modelData.x - uiState.equipX
                    y: modelData.y - uiState.equipY - 17
                    width: modelData.width
                    height: modelData.height
                    z: modelData.hovered ? 5 : 0

                    Rectangle {
                        x: 1
                        y: 2
                        width: modelData.width
                        height: modelData.height
                        radius: 6
                        color: Qt.rgba(0, 0, 0, modelData.hovered ? 0.35 : 0.22)
                        z: -1
                    }

                    Rectangle {
                        anchors.fill: parent
                        radius: 6
                        color: modelData.hovered ? Theme.buttonBgHover : (modelData.occupied ? Theme.buttonBgHover : Theme.inputBg)
                        border.width: 1
                        border.color: modelData.hovered ? Theme.accent : (modelData.occupied ? Theme.accent : Theme.border)

                        Text {
                            anchors.centerIn: parent
                            text: modelData.slotGlyph || ""
                            color: Theme.textMuted
                            font.pixelSize: Math.max(14, parent.height - 12)
                            opacity: 0.55
                            visible: !modelData.occupied
                        }

                        Image {
                            id: equipIcon
                            anchors.centerIn: parent
                            width: Math.max(1, parent.width - 4)
                            height: Math.max(1, parent.height - 4)
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            cache: false
                            source: modelData.occupied && ((modelData.itemId || 0) > 0 || (modelData.itemIndex || 0) > 0)
                                ? root.itemIconSource(modelData.itemId || 0, modelData.itemIndex || 0)
                                : ""
                            visible: source !== "" && status === Image.Ready
                        }
                    }

                    Rectangle {
                        id: slotTooltip
                        visible: modelData.hovered && (tooltipText.text.length > 0)
                        width: tooltipText.implicitWidth + 10
                        height: tooltipText.implicitHeight + 6
                        x: modelData.leftColumn ? (modelData.width + 4) : -width - 4
                        y: (modelData.height - height) / 2
                        radius: 4
                        color: Theme.surface
                        border.width: 1
                        border.color: Theme.border
                        z: 10

                        Text {
                            id: tooltipText
                            anchors.centerIn: parent
                            text: modelData.occupied
                                ? ((modelData.slotTypeName || "") + (modelData.label ? (": " + modelData.label) : ""))
                                : (modelData.slotTypeName || "")
                            color: Theme.text
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        x: uiState.friendPartyX
        y: uiState.friendPartyY
        width: uiState.friendPartyWidth
        height: uiState.friendPartyHeight
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.friendPartyVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 10
            y: 2
            text: uiState.friendPartyData.title || "Friend / Party"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.friendPartyData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.friendPartyX
                y: (modelData.y || 0) - uiState.friendPartyY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 9
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: 0
            y: 17
            width: parent.width
            height: parent.height - 17
            color: Theme.surface

            Repeater {
                model: uiState.friendPartyData.tabs || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.friendPartyX
                    y: (modelData.y || 0) - uiState.friendPartyY - 17
                    width: modelData.width || 0
                    height: modelData.height || 0
                    radius: 3
                    color: modelData.active ? Theme.surfaceAlt : Theme.buttonBg
                    border.width: 1
                    border.color: modelData.active ? Theme.borderStrong : Theme.buttonBorder

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label || ""
                        color: Theme.text
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Rectangle {
                x: 10
                y: 23
                width: parent.width - 26
                height: 18
                color: Theme.surfaceAlt
                border.width: 1
                border.color: Theme.border

                Text {
                    x: 6
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Name"
                    color: Theme.text
                    font.pixelSize: 10
                    font.bold: true
                }

                Text {
                    x: 110
                    anchors.verticalCenter: parent.verticalCenter
                    text: (uiState.friendPartyData.currentTab || 0) === 0 ? "State" : "Role"
                    color: Theme.text
                    font.pixelSize: 10
                    font.bold: true
                }

                Text {
                    x: 171
                    anchors.verticalCenter: parent.verticalCenter
                    text: (uiState.friendPartyData.currentTab || 0) === 0 ? "Memo" : "Map"
                    color: Theme.text
                    font.pixelSize: 10
                    font.bold: true
                }
            }

            Rectangle {
                x: 10
                y: 41
                width: parent.width - 26
                height: parent.height - 125
                color: Theme.inputBg
                border.width: 1
                border.color: Theme.inputBorder

                Repeater {
                    model: uiState.friendPartyData.rows || []

                    delegate: Rectangle {
                        required property var modelData
                        x: (modelData.x || 0) - uiState.friendPartyX - 10
                        y: (modelData.y || 0) - uiState.friendPartyY - 58
                        width: modelData.width || 0
                        height: modelData.height || 0
                        color: modelData.selected ? Theme.surfaceAlt : Theme.inputBg

                        Rectangle {
                            x: 3
                            y: 3
                            width: 6
                            height: parent.height - 6
                            color: Qt.rgba(((modelData.color || 0) >> 16 & 255) / 255,
                                           ((modelData.color || 0) >> 8 & 255) / 255,
                                           ((modelData.color || 0) & 255) / 255,
                                           1.0)
                        }

                        Text {
                            x: 14
                            width: 88
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.name || ""
                            color: Theme.text
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }

                        Text {
                            x: 109
                            width: 50
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.status || ""
                            color: Theme.text
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }

                        Text {
                            x: 171
                            width: parent.width - 175
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData.detail || ""
                            color: Theme.textMuted
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }
                    }
                }

                Rectangle {
                    x: width - 12
                    y: 1
                    width: 11
                    height: parent.height - 2
                    color: Theme.scrollTrack
                    border.width: 1
                    border.color: Theme.border
                    visible: uiState.friendPartyData.scrollBarVisible || false

                    Rectangle {
                        x: 1
                        y: (uiState.friendPartyData.scrollThumbY || 0) - (uiState.friendPartyData.scrollTrackY || 0)
                        width: parent.width - 2
                        height: uiState.friendPartyData.scrollThumbHeight || 0
                        color: Theme.scrollThumb
                        border.width: 1
                        border.color: Theme.borderStrong
                    }
                }
            }

            Repeater {
                model: uiState.friendPartyData.actionButtons || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.friendPartyX
                    y: (modelData.y || 0) - uiState.friendPartyY - 17
                    width: modelData.width || 0
                    height: modelData.height || 0
                    radius: 3
                    color: (modelData.active || false) ? Theme.buttonBgHover : Theme.buttonBg
                    border.width: 1
                    border.color: Theme.buttonBorder
                    visible: modelData.visible || false

                    Text {
                        anchors.centerIn: parent
                        text: modelData.label || ""
                        color: Theme.buttonText
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Rectangle {
                x: 10
                y: parent.height - 43
                width: parent.width - 20
                height: 34
                color: Theme.surfaceAlt
                border.width: 1
                border.color: Theme.border

                Text {
                    x: 6
                    y: 4
                    text: uiState.friendPartyData.summaryTitle || ""
                    color: Theme.text
                    font.pixelSize: 10
                    font.bold: true
                }

                Text {
                    x: 86
                    width: parent.width - 92
                    y: 4
                    text: uiState.friendPartyData.summaryValue || ""
                    color: Theme.text
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Text {
                    x: 6
                    y: 18
                    width: parent.width - 12
                    text: uiState.friendPartyData.summarySecondaryValue || ""
                    color: Theme.text
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }
    }

    Rectangle {
        x: uiState.partySetupX
        y: uiState.partySetupY
        width: uiState.partySetupWidth
        height: uiState.partySetupHeight
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.partySetupVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 10
            y: 2
            text: uiState.partySetupData.title || "Party Setup"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.partySetupData.labels || []

            delegate: Text {
                required property var modelData
                x: (modelData.x || 0) - uiState.partySetupX
                y: (modelData.y || 0) - uiState.partySetupY
                width: modelData.width || 0
                height: modelData.height || 0
                text: modelData.text || ""
                color: (modelData.header || false) ? Theme.accent : Theme.text
                font.pixelSize: 10
                font.bold: modelData.header || false
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
        }

        Repeater {
            model: uiState.partySetupData.helpIcons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.partySetupX
                y: (modelData.y || 0) - uiState.partySetupY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: width / 2
                color: (modelData.hovered || false) ? Theme.buttonBgHover : Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: "?"
                    color: Theme.buttonText
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: (uiState.partySetupData.nameFieldX || 0) - uiState.partySetupX
            y: (uiState.partySetupData.nameFieldY || 0) - uiState.partySetupY
            width: uiState.partySetupData.nameFieldWidth || 0
            height: uiState.partySetupData.nameFieldHeight || 0
            color: (uiState.partySetupData.showNameEdit || false)
                ? ((uiState.partySetupData.nameFocused || false) ? Theme.accent : Theme.inputBg)
                : "transparent"
            border.width: (uiState.partySetupData.showNameEdit || false) ? 1 : 0
            border.color: Theme.inputBorder

            Text {
                x: (uiState.partySetupData.showNameEdit || false) ? 6 : 0
                width: parent.width - ((uiState.partySetupData.showNameEdit || false) ? 12 : 0)
                anchors.verticalCenter: parent.verticalCenter
                text: uiState.partySetupData.nameValue || ""
                color: Theme.inputText
                font.pixelSize: 11
                elide: Text.ElideRight
            }
        }

        Repeater {
            model: uiState.partySetupData.choices || []

            delegate: Item {
                required property var modelData
                x: (modelData.x || 0) - uiState.partySetupX
                y: (modelData.y || 0) - uiState.partySetupY
                width: modelData.width || 0
                height: modelData.height || 0

                Rectangle {
                    x: 0
                    y: 2
                    width: 14
                    height: 14
                    color: Theme.inputBg
                    border.width: 1
                    border.color: Theme.inputBorder

                    Rectangle {
                        x: 3
                        y: 3
                        width: 8
                        height: 8
                        color: Theme.accent
                        visible: modelData.selected || false
                    }
                }

                Text {
                    x: 20
                    width: parent.width - 20
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.label || ""
                    color: Theme.text
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }

        Repeater {
            model: uiState.partySetupData.buttons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.partySetupX
                y: (modelData.y || 0) - uiState.partySetupY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 3
                color: (modelData.active || false) ? Theme.buttonBgHover : Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        Rectangle {
            visible: !!(uiState.partySetupData.tooltipText || "")
            x: Math.max(4, Math.min(parent.width - width - 4, ((uiState.partySetupData.tooltipX || 0) - uiState.partySetupX) - Math.floor(width / 2)))
            y: Math.max(4, Math.min(parent.height - height - 4, ((uiState.partySetupData.tooltipY || 0) - uiState.partySetupY) - height - 14))
            width: 248
            height: tooltipText.paintedHeight + 12
            color: Theme.borderStrong
            border.width: 1
            border.color: Theme.border
            z: 10

            Text {
                id: tooltipText
                x: 8
                y: 6
                width: parent.width - 16
                text: uiState.partySetupData.tooltipText || ""
                color: Theme.accentText
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
        }
    }

    Rectangle {
        x: uiState.partyInviteX
        y: uiState.partyInviteY
        width: uiState.partyInviteWidth
        height: uiState.partyInviteHeight
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.partyInviteVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Rectangle {
            x: 14
            y: 28
            width: parent.width - 28
            height: 52
            radius: 2
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border
        }

        Text {
            x: 10
            y: 2
            text: uiState.partyInviteData.title || "Party Invitation"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Text {
            x: 20
            y: 34
            width: parent.width - 40
            height: 42
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
            text: uiState.partyInviteData.bodyText || ((uiState.partyInviteData.messageLines || []).join("\n"))
            color: Theme.text
            font.pixelSize: 11
        }

        Repeater {
            model: uiState.partyInviteData.buttons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.partyInviteX
                y: (modelData.y || 0) - uiState.partyInviteY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 3
                color: (modelData.active || false) ? Theme.buttonBgHover : Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        x: uiState.skillListX
        y: uiState.skillListY
        width: uiState.skillListWidth
        height: uiState.skillListHeight
        radius: 8
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.skillListVisible

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 17
            y: 3
            text: uiState.skillListData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.skillListData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.skillListX
                y: (modelData.y || 0) - uiState.skillListY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: 41
            y: 17
            width: parent.width - 41
            height: parent.height - 67
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border
        }

        Rectangle {
            x: 0
            y: 17
            width: 41
            height: parent.height - 67
            color: Theme.surfaceAlt
        }

        Repeater {
            model: uiState.skillListData.rows || []

            delegate: Item {
                required property var modelData
                x: modelData.x - uiState.skillListX
                y: modelData.y - uiState.skillListY
                width: modelData.width
                height: modelData.height

                Rectangle {
                    anchors.fill: parent
                    color: modelData.selected ? Theme.buttonBgHover : (modelData.hovered ? Theme.surfaceAlt : "transparent")
                    border.width: 1
                    border.color: modelData.selected ? Theme.accent : (modelData.hovered ? Theme.border : "transparent")
                }

                Rectangle {
                    x: 4
                    y: 1
                    width: 32
                    height: 32
                    color: Theme.inputBg
                    border.width: 1
                    border.color: Theme.border

                    Image {
                        anchors.fill: parent
                        anchors.margins: 1
                        fillMode: Image.PreserveAspectFit
                        smooth: false
                        source: root.skillIconSource(modelData.skillId || 0)
                        visible: (modelData.iconVisible !== false) && source !== ""
                    }
                }

                Text {
                    x: 48
                    y: 3
                    width: parent.width - 88
                    text: modelData.name
                    color: Theme.text
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Text {
                    x: 48
                    y: 18
                    width: 80
                    text: modelData.levelText
                    color: Theme.text
                    font.pixelSize: 10
                }

                Text {
                    x: 165
                    y: 18
                    width: parent.width - 198
                    text: modelData.rightText
                    color: Theme.text
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                Rectangle {
                    x: modelData.upgradeX - modelData.x
                    y: modelData.upgradeY - modelData.y
                    width: modelData.upgradeWidth
                    height: modelData.upgradeHeight
                    visible: modelData.upgradeVisible
                    color: modelData.upgradePressed ? Theme.buttonBgPressed : Theme.buttonBgHover
                    border.width: 1
                    border.color: Theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: uiState.skillListData.upgradeLabel || ""
                        color: Theme.buttonText
                        font.pixelSize: 12
                        font.bold: true
                    }
                }
            }
        }

        Rectangle {
            x: (uiState.skillListData.scrollTrackX || 0) - uiState.skillListX
            y: (uiState.skillListData.scrollTrackY || 0) - uiState.skillListY
            width: uiState.skillListData.scrollTrackWidth || 0
            height: uiState.skillListData.scrollTrackHeight || 0
            visible: uiState.skillListData.scrollBarVisible || false
            color: Theme.scrollTrack
            border.width: 1
            border.color: Theme.border

            Rectangle {
                x: (uiState.skillListData.scrollThumbX || 0) - (uiState.skillListData.scrollTrackX || 0)
                y: (uiState.skillListData.scrollThumbY || 0) - (uiState.skillListData.scrollTrackY || 0)
                width: uiState.skillListData.scrollThumbWidth || 0
                height: uiState.skillListData.scrollThumbHeight || 0
                color: Theme.scrollThumb
                border.width: 1
                border.color: Theme.borderStrong
            }
        }

        Rectangle {
            x: 0
            y: parent.height - 50
            width: parent.width
            height: 50
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border
        }

        Text {
            x: 13
            y: parent.height - 18
            text: uiState.skillListData.skillPointText || ""
            color: "#b09130"
            font.pixelSize: 11
            font.bold: true
        }

        Repeater {
            model: uiState.skillListData.bottomButtons || []

            delegate: Rectangle {
                required property var modelData
                x: modelData.x - uiState.skillListX
                y: modelData.y - uiState.skillListY
                width: modelData.width
                height: modelData.height
                radius: 3
                color: modelData.pressed ? Theme.buttonBgPressed : (modelData.hovered ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: Theme.buttonText
                    font.pixelSize: 10
                }
            }
        }
    }

    Rectangle {
        x: uiState.itemInfoX
        y: uiState.itemInfoY
        width: uiState.itemInfoWidth
        height: uiState.itemInfoHeight
        readonly property var closeButtonData: root.asObject(uiState.itemInfoData.closeButton)
        readonly property var graphicsButtonData: root.asObject(uiState.itemInfoData.graphicsButton)
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.itemInfoVisible
        z: 40

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 8
            y: 3
            text: uiState.itemInfoData.title || "Item Information"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: (parent.closeButtonData.x || 0) - uiState.itemInfoX
            y: (parent.closeButtonData.y || 0) - uiState.itemInfoY
            width: parent.closeButtonData.width || 0
            height: parent.closeButtonData.height || 0
            radius: 2
            color: (parent.closeButtonData.pressed || false) ? Theme.buttonBgPressed : ((parent.closeButtonData.hovered || false) ? Theme.buttonBgHover : Theme.surface)
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: parent.parent.closeButtonData.label || "X"
                color: Theme.buttonText
                font.pixelSize: 8
                font.bold: true
            }
        }

        Rectangle {
            x: 8
            y: 25
            width: 96
            height: 96
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border

            Image {
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                smooth: false
                cache: false
                source: (uiState.itemInfoData.previewUsesCollection || false)
                    ? root.collectionImageSource(uiState.itemInfoData.itemId || 0, uiState.itemInfoData.itemIndex || 0, uiState.itemInfoData.identified)
                    : root.itemIconSource(uiState.itemInfoData.itemId || 0, uiState.itemInfoData.itemIndex || 0, uiState.itemInfoData.identified)
            }

            Rectangle {
                x: 4
                y: 4
                width: parent.parent.graphicsButtonData.width || 0
                height: parent.parent.graphicsButtonData.height || 0
                radius: 2
                visible: parent.parent.graphicsButtonData.visible || false
                color: (parent.parent.graphicsButtonData.pressed || false) ? Theme.buttonBgPressed : ((parent.parent.graphicsButtonData.hovered || false) ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: parent.parent.parent.graphicsButtonData.label || "View"
                    color: Theme.buttonText
                    font.pixelSize: 9
                }
            }
        }

        Text {
            x: 112
            y: 25
            width: parent.width - 120
            text: uiState.itemInfoData.name || ""
            color: Theme.text
            font.pixelSize: 11
            font.bold: true
            elide: Text.ElideRight
        }

        Column {
            id: itemInfoDetailsColumn
            x: 112
            y: 45
            width: parent.width - 120
            spacing: 2

            Repeater {
                model: uiState.itemInfoData.detailLines || []

                delegate: Text {
                    required property var modelData
                    width: parent.width
                    text: modelData
                    color: Theme.textMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }

        Text {
            x: 112
            y: itemInfoDetailsColumn.y + itemInfoDetailsColumn.height + 4
            width: parent.width - 120
            height: parent.height - y - (((uiState.itemInfoData.slots || []).length > 0) ? 56 : 20)
            text: root.roColorTextToRichText(uiState.itemInfoData.description || "", Theme.text)
            textFormat: Text.RichText
            color: Theme.text
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignTop
        }

        Repeater {
            model: uiState.itemInfoData.slots || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.itemInfoX
                y: (modelData.y || 0) - uiState.itemInfoY
                width: modelData.width || 0
                height: modelData.height || 0
                color: !(modelData.available || false)
                    ? Theme.overlayDim
                    : ((modelData.occupied || false) ? Theme.inputBg : Theme.surfaceAlt)
                border.width: 1
                border.color: Theme.border

                Image {
                    anchors.fill: parent
                    anchors.margins: 2
                    fillMode: Image.PreserveAspectFit
                    smooth: false
                    cache: false
                    visible: modelData.occupied || false
                    source: root.itemIconSource(modelData.itemId || 0, 0)
                }

                Rectangle {
                    visible: (modelData.hovered || false) && (modelData.occupied || false) && !!modelData.tooltip
                    x: (parent.width - width) / 2
                    y: -24
                    width: tooltipText.implicitWidth + 12
                    height: tooltipText.implicitHeight + 8
                    color: "#303030"
                    border.width: 1
                    border.color: "#606060"

                    Text {
                        id: tooltipText
                        anchors.centerIn: parent
                        text: modelData.tooltip || ""
                        color: "#ffffff"
                        font.pixelSize: 10
                    }
                }
            }
        }
    }

    Rectangle {
        x: uiState.skillDescribeX
        y: uiState.skillDescribeY
        width: uiState.skillDescribeWidth
        height: uiState.skillDescribeHeight
        readonly property var closeButtonData: root.asObject(uiState.skillDescribeData.closeButton)
        readonly property var descriptionScrollBarData: root.asObject(uiState.skillDescribeData.descriptionScrollBar)
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.skillDescribeVisible
        z: 41

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 8
            y: 3
            text: uiState.skillDescribeData.title || "Skill Information"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: (parent.closeButtonData.x || 0) - uiState.skillDescribeX
            y: (parent.closeButtonData.y || 0) - uiState.skillDescribeY
            width: parent.closeButtonData.width || 0
            height: parent.closeButtonData.height || 0
            radius: 2
            color: (parent.closeButtonData.pressed || false) ? Theme.buttonBgPressed : ((parent.closeButtonData.hovered || false) ? Theme.buttonBgHover : Theme.surface)
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: parent.parent.closeButtonData.label || "X"
                color: Theme.buttonText
                font.pixelSize: 8
                font.bold: true
            }
        }

        Rectangle {
            x: 8
            y: 25
            width: 52
            height: 52
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border

            Image {
                anchors.fill: parent
                anchors.margins: 2
                fillMode: Image.PreserveAspectFit
                smooth: false
                cache: false
                source: root.skillIconSource(uiState.skillDescribeData.skillId || 0)
            }
        }

        Text {
            x: 68
            y: 25
            width: parent.width - 76
            text: uiState.skillDescribeData.name || ""
            color: Theme.text
            font.pixelSize: 11
            font.bold: true
            elide: Text.ElideRight
        }

        Column {
            x: 68
            y: 45
            width: parent.width - 76
            spacing: 2

            Repeater {
                model: uiState.skillDescribeData.detailLines || []

                delegate: Text {
                    required property var modelData
                    width: parent.width
                    text: modelData
                    color: Theme.textMuted
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }

        Rectangle {
            x: (uiState.skillDescribeData.descriptionX || 0) - uiState.skillDescribeX
            y: (uiState.skillDescribeData.descriptionY || 0) - uiState.skillDescribeY
            width: uiState.skillDescribeData.descriptionWidth || 0
            height: uiState.skillDescribeData.descriptionHeight || 0
            color: "transparent"
            clip: true

            Text {
                x: 0
                y: -(uiState.skillDescribeData.descriptionScrollOffset || 0)
                width: parent.width
                height: Math.max(implicitHeight, uiState.skillDescribeData.descriptionContentHeight || 0)
                text: root.roColorTextToRichText(uiState.skillDescribeData.description || "", Theme.text)
                textFormat: Text.RichText
                color: Theme.text
                font.pixelSize: 10
                wrapMode: Text.WordWrap
                verticalAlignment: Text.AlignTop
            }
        }

        Rectangle {
            x: (parent.descriptionScrollBarData.trackX || 0) - uiState.skillDescribeX
            y: (parent.descriptionScrollBarData.trackY || 0) - uiState.skillDescribeY
            width: parent.descriptionScrollBarData.trackWidth || 0
            height: parent.descriptionScrollBarData.trackHeight || 0
            visible: parent.descriptionScrollBarData.visible || false
            color: "#e3e7ee"
            border.width: 1
            border.color: "#a4adbd"

            Rectangle {
                x: (parent.parent.descriptionScrollBarData.thumbX || 0) - (parent.parent.descriptionScrollBarData.trackX || 0)
                y: (parent.parent.descriptionScrollBarData.thumbY || 0) - (parent.parent.descriptionScrollBarData.trackY || 0)
                width: parent.parent.descriptionScrollBarData.thumbWidth || 0
                height: parent.parent.descriptionScrollBarData.thumbHeight || 0
                color: "#b4bccd"
                border.width: 1
                border.color: "#788296"
            }
        }
    }

    Rectangle {
        x: uiState.itemCollectionX
        y: uiState.itemCollectionY
        width: uiState.itemCollectionWidth
        height: uiState.itemCollectionHeight
        readonly property var closeButtonData: root.asObject(uiState.itemCollectionData.closeButton)
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.itemCollectionVisible
        z: 42

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
        }

        Text {
            x: 8
            y: 3
            text: uiState.itemCollectionData.title || "Card illustration"
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: (parent.closeButtonData.x || 0) - uiState.itemCollectionX
            y: (parent.closeButtonData.y || 0) - uiState.itemCollectionY
            width: parent.closeButtonData.width || 0
            height: parent.closeButtonData.height || 0
            radius: 2
            color: (parent.closeButtonData.pressed || false) ? Theme.buttonBgPressed : ((parent.closeButtonData.hovered || false) ? Theme.buttonBgHover : Theme.surface)
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: parent.parent.closeButtonData.label || "X"
                color: Theme.buttonText
                font.pixelSize: 8
                font.bold: true
            }
        }

        Rectangle {
            x: 8
            y: 25
            width: parent.width - 16
            height: parent.height - 33
            color: Theme.inputBg
            border.width: 1
            border.color: Theme.border

            Image {
                anchors.fill: parent
                anchors.margins: 0
                fillMode: Image.PreserveAspectFit
                smooth: false
                cache: false
                source: (uiState.itemCollectionData.mainUsesIllust || false)
                    ? root.illustImageSource(uiState.itemCollectionData.itemId || 0)
                    : ((uiState.itemCollectionData.mainUsesCollection || false)
                        ? root.collectionImageSource(uiState.itemCollectionData.itemId || 0, 0)
                        : root.itemIconSource(uiState.itemCollectionData.itemId || 0, 0))
            }
        }
    }

    Rectangle {
        x: uiState.itemIdentifyX
        y: uiState.itemIdentifyY
        width: uiState.itemIdentifyWidth
        height: uiState.itemIdentifyHeight
        readonly property var closeButtonData: root.asObject(uiState.itemIdentifyData.closeButton)
        readonly property var scrollBarData: root.asObject(uiState.itemIdentifyData.scrollBar)
        readonly property var okButtonData: root.asObject(uiState.itemIdentifyData.okButton)
        readonly property var cancelButtonData: root.asObject(uiState.itemIdentifyData.cancelButton)
        radius: 4
        color: "#ede9df"
        border.width: 1
        border.color: "#6b675f"
        visible: uiState.itemIdentifyVisible
        z: 43

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: "#6e8194"
            border.width: 1
            border.color: "#4e5d6c"
        }

        Text {
            x: 8
            y: 3
            text: uiState.itemIdentifyData.title || "Identify Item"
            color: "#ffffff"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: (parent.closeButtonData.x || 0) - uiState.itemIdentifyX
            y: (parent.closeButtonData.y || 0) - uiState.itemIdentifyY
            width: parent.closeButtonData.width || 0
            height: parent.closeButtonData.height || 0
            radius: 2
            color: (parent.closeButtonData.pressed || false) ? "#b9c7de" : ((parent.closeButtonData.hovered || false) ? "#d7dff0" : "#e7e2d6")
            border.width: 1
            border.color: "#7f7a70"

            Text {
                anchors.centerIn: parent
                text: parent.parent.closeButtonData.label || "X"
                color: "#000000"
                font.pixelSize: 8
                font.bold: true
            }
        }

        Rectangle {
            id: itemIdentifyList
            x: 8
            y: 27
            width: Math.max(0, parent.width - 30)
            height: 128
            color: "#ffffff"
            border.width: 1
            border.color: "#c4c0b5"
            clip: true

            Text {
                anchors.centerIn: parent
                visible: (uiState.itemIdentifyData.rows || []).length === 0
                text: uiState.itemIdentifyData.emptyText || "No unidentified items available."
                color: "#505050"
                font.pixelSize: 10
            }

            Repeater {
                model: uiState.itemIdentifyData.rows || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.itemIdentifyX - itemIdentifyList.x
                    y: (modelData.y || 0) - uiState.itemIdentifyY - itemIdentifyList.y
                    width: modelData.width || 0
                    height: modelData.height || 0
                    color: (modelData.selected || false) ? "#d7dff0" : ((modelData.hovered || false) ? "#ece8de" : "#f9f7f3")
                    border.width: 1
                    border.color: (modelData.selected || false) ? "#7e95bf" : "#bcb4a7"

                    Rectangle {
                        x: 4
                        y: 4
                        width: 24
                        height: 24
                        color: "#f5f2ea"
                        border.width: 1
                        border.color: "#a69f91"

                        Image {
                            anchors.fill: parent
                            anchors.margins: 1
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            cache: false
                            source: root.itemIconSource(modelData.itemId || 0, 0, modelData.identified)
                        }
                    }

                    Text {
                        x: 34
                        y: 5
                        width: parent.width - 70
                        text: modelData.label || ""
                        color: "#000000"
                        font.pixelSize: 11
                        font.bold: modelData.selected || false
                        elide: Text.ElideRight
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        y: 5
                        visible: (modelData.count || 0) > 1
                        text: modelData.count || 0
                        color: "#5a4020"
                        font.pixelSize: 10
                    }
                }
            }
        }

        Rectangle {
            x: (parent.scrollBarData.trackX || 0) - uiState.itemIdentifyX
            y: (parent.scrollBarData.trackY || 0) - uiState.itemIdentifyY
            width: parent.scrollBarData.trackWidth || 0
            height: parent.scrollBarData.trackHeight || 0
            visible: parent.scrollBarData.visible || false
            color: "#e3e7ee"
            border.width: 1
            border.color: "#a4adbd"

            Rectangle {
                x: (parent.parent.scrollBarData.thumbX || 0) - (parent.parent.scrollBarData.trackX || 0)
                y: (parent.parent.scrollBarData.thumbY || 0) - (parent.parent.scrollBarData.trackY || 0)
                width: parent.parent.scrollBarData.thumbWidth || 0
                height: parent.parent.scrollBarData.thumbHeight || 0
                color: "#b4bccd"
                border.width: 1
                border.color: "#788296"
            }
        }

        Rectangle {
            x: (parent.okButtonData.x || 0) - uiState.itemIdentifyX
            y: (parent.okButtonData.y || 0) - uiState.itemIdentifyY
            width: parent.okButtonData.width || 0
            height: parent.okButtonData.height || 0
            radius: 3
            color: !(parent.okButtonData.enabled || false)
                ? "#d0d0d0"
                : ((parent.okButtonData.pressed || false) ? "#b9c7de" : ((parent.okButtonData.hovered || false) ? "#d7dff0" : "#e9e4d8"))
            border.width: 1
            border.color: "#8c8578"

            Text {
                anchors.centerIn: parent
                text: parent.parent.okButtonData.label || "OK"
                color: "#000000"
                font.pixelSize: 10
            }
        }

        Rectangle {
            x: (parent.cancelButtonData.x || 0) - uiState.itemIdentifyX
            y: (parent.cancelButtonData.y || 0) - uiState.itemIdentifyY
            width: parent.cancelButtonData.width || 0
            height: parent.cancelButtonData.height || 0
            radius: 3
            color: (parent.cancelButtonData.pressed || false) ? "#b9c7de" : ((parent.cancelButtonData.hovered || false) ? "#d7dff0" : "#e9e4d8")
            border.width: 1
            border.color: "#8c8578"

            Text {
                anchors.centerIn: parent
                text: parent.parent.cancelButtonData.label || "Cancel"
                color: "#000000"
                font.pixelSize: 10
            }
        }
    }

    Rectangle {
        x: uiState.itemCompositionX
        y: uiState.itemCompositionY
        width: uiState.itemCompositionWidth
        height: uiState.itemCompositionHeight
        readonly property var closeButtonData: root.asObject(uiState.itemCompositionData.closeButton)
        readonly property var scrollBarData: root.asObject(uiState.itemCompositionData.scrollBar)
        readonly property var okButtonData: root.asObject(uiState.itemCompositionData.okButton)
        readonly property var cancelButtonData: root.asObject(uiState.itemCompositionData.cancelButton)
        radius: 4
        color: "#ede9df"
        border.width: 1
        border.color: "#6b675f"
        visible: uiState.itemCompositionVisible
        z: 43

        Rectangle {
            x: 1
            y: 1
            width: parent.width - 2
            height: 16
            radius: 3
            color: "#6e8194"
            border.width: 1
            border.color: "#4e5d6c"
        }

        Text {
            x: 8
            y: 3
            text: uiState.itemCompositionData.title || "Compound Item"
            color: "#ffffff"
            font.pixelSize: 12
            font.bold: true
        }

        Rectangle {
            x: (parent.closeButtonData.x || 0) - uiState.itemCompositionX
            y: (parent.closeButtonData.y || 0) - uiState.itemCompositionY
            width: parent.closeButtonData.width || 0
            height: parent.closeButtonData.height || 0
            radius: 2
            color: (parent.closeButtonData.pressed || false) ? "#b9c7de" : ((parent.closeButtonData.hovered || false) ? "#d7dff0" : "#e7e2d6")
            border.width: 1
            border.color: "#7f7a70"

            Text {
                anchors.centerIn: parent
                text: parent.parent.closeButtonData.label || "X"
                color: "#000000"
                font.pixelSize: 8
                font.bold: true
            }
        }

        Rectangle {
            id: itemCompositionList
            x: 8
            y: 27
            width: Math.max(0, parent.width - 30)
            height: 128
            color: "#ffffff"
            border.width: 1
            border.color: "#c4c0b5"
            clip: true

            Text {
                anchors.centerIn: parent
                visible: (uiState.itemCompositionData.rows || []).length === 0
                text: uiState.itemCompositionData.emptyText || "No equipment can accept this card."
                color: "#505050"
                font.pixelSize: 10
            }

            Repeater {
                model: uiState.itemCompositionData.rows || []

                delegate: Rectangle {
                    required property var modelData
                    x: (modelData.x || 0) - uiState.itemCompositionX - itemCompositionList.x
                    y: (modelData.y || 0) - uiState.itemCompositionY - itemCompositionList.y
                    width: modelData.width || 0
                    height: modelData.height || 0
                    color: (modelData.selected || false) ? "#d7dff0" : ((modelData.hovered || false) ? "#ece8de" : "#f9f7f3")
                    border.width: 1
                    border.color: (modelData.selected || false) ? "#7e95bf" : "#bcb4a7"

                    Rectangle {
                        x: 4
                        y: 4
                        width: 24
                        height: 24
                        color: "#f5f2ea"
                        border.width: 1
                        border.color: "#a69f91"

                        Image {
                            anchors.fill: parent
                            anchors.margins: 1
                            fillMode: Image.PreserveAspectFit
                            smooth: false
                            cache: false
                            source: root.itemIconSource(modelData.itemId || 0, 0, modelData.identified)
                        }
                    }

                    Text {
                        x: 34
                        y: 5
                        width: parent.width - 70
                        text: modelData.label || ""
                        color: "#000000"
                        font.pixelSize: 11
                        font.bold: modelData.selected || false
                        elide: Text.ElideRight
                    }

                    Text {
                        anchors.right: parent.right
                        anchors.rightMargin: 8
                        y: 5
                        visible: (modelData.count || 0) > 1
                        text: modelData.count || 0
                        color: "#5a4020"
                        font.pixelSize: 10
                    }
                }
            }
        }

        Rectangle {
            x: (parent.scrollBarData.trackX || 0) - uiState.itemCompositionX
            y: (parent.scrollBarData.trackY || 0) - uiState.itemCompositionY
            width: parent.scrollBarData.trackWidth || 0
            height: parent.scrollBarData.trackHeight || 0
            visible: parent.scrollBarData.visible || false
            color: "#e3e7ee"
            border.width: 1
            border.color: "#a4adbd"

            Rectangle {
                x: (parent.parent.scrollBarData.thumbX || 0) - (parent.parent.scrollBarData.trackX || 0)
                y: (parent.parent.scrollBarData.thumbY || 0) - (parent.parent.scrollBarData.trackY || 0)
                width: parent.parent.scrollBarData.thumbWidth || 0
                height: parent.parent.scrollBarData.thumbHeight || 0
                color: "#b4bccd"
                border.width: 1
                border.color: "#788296"
            }
        }

        Rectangle {
            x: (parent.okButtonData.x || 0) - uiState.itemCompositionX
            y: (parent.okButtonData.y || 0) - uiState.itemCompositionY
            width: parent.okButtonData.width || 0
            height: parent.okButtonData.height || 0
            radius: 3
            color: !(parent.okButtonData.enabled || false)
                ? "#d0d0d0"
                : ((parent.okButtonData.pressed || false) ? "#b9c7de" : ((parent.okButtonData.hovered || false) ? "#d7dff0" : "#e9e4d8"))
            border.width: 1
            border.color: "#8c8578"

            Text {
                anchors.centerIn: parent
                text: parent.parent.okButtonData.label || "OK"
                color: "#000000"
                font.pixelSize: 10
            }
        }

        Rectangle {
            x: (parent.cancelButtonData.x || 0) - uiState.itemCompositionX
            y: (parent.cancelButtonData.y || 0) - uiState.itemCompositionY
            width: parent.cancelButtonData.width || 0
            height: parent.cancelButtonData.height || 0
            radius: 3
            color: (parent.cancelButtonData.pressed || false) ? "#b9c7de" : ((parent.cancelButtonData.hovered || false) ? "#d7dff0" : "#e9e4d8")
            border.width: 1
            border.color: "#8c8578"

            Text {
                anchors.centerIn: parent
                text: parent.parent.cancelButtonData.label || "Cancel"
                color: "#000000"
                font.pixelSize: 10
            }
        }
    }

    Rectangle {
        x: uiState.optionX
        y: uiState.optionY
        width: uiState.optionWidth
        height: uiState.optionHeight
        radius: 8
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.optionVisible

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 17
            radius: 8
            color: Theme.accent
            border.width: 0
        }

        Rectangle {
            x: 0
            y: 8
            width: parent.width
            height: 9
            color: Theme.accent
        }

        Text {
            x: 17
            y: 2
            text: uiState.optionData.title || ""
            color: Theme.accentText
            font.pixelSize: 12
            font.bold: true
        }

        Repeater {
            model: uiState.optionData.systemButtons || []

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.optionX
                y: (modelData.y || 0) - uiState.optionY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 4
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 9
                    font.bold: true
                }
            }
        }

        Rectangle {
            x: (uiState.optionData.contentX || 0) - uiState.optionX
            y: (uiState.optionData.contentY || 0) - uiState.optionY
            width: uiState.optionData.contentWidth || 0
            height: uiState.optionData.contentHeight || 0
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            visible: !(uiState.optionData.collapsed || false)
        }

        Repeater {
            model: uiState.optionData.tabs || []

            delegate: Rectangle {
                required property var modelData
                x: modelData.x - uiState.optionX
                y: modelData.y - uiState.optionY
                width: modelData.width
                height: modelData.height
                radius: 6
                visible: !(uiState.optionData.collapsed || false)
                color: modelData.active ? Theme.surface : Theme.surfaceAlt
                border.width: 1
                border.color: modelData.active ? Theme.borderStrong : Theme.border

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: Theme.text
                    font.pixelSize: 11
                }
            }
        }

        Repeater {
            model: uiState.optionData.toggles || []

            delegate: Item {
                required property var modelData
                x: modelData.x - uiState.optionX
                y: modelData.y - uiState.optionY
                width: (uiState.optionData.contentWidth || 0) - 24
                height: Math.max(modelData.height, 16)
                visible: !(uiState.optionData.collapsed || false)

                Rectangle {
                    id: optionToggleBox
                    x: 0
                    y: 0
                    width: modelData.width
                    height: modelData.height
                    color: modelData.checked ? Theme.accent : Theme.inputBg
                    border.width: 1
                    border.color: Theme.border
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: modelData.width + 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: modelData.label
                    color: Theme.text
                    font.pixelSize: 11
                }

                Text {
                    anchors.centerIn: optionToggleBox
                    text: modelData.checked ? "X" : ""
                    color: Theme.accentText
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        Repeater {
            model: uiState.optionData.sliders || []

            delegate: Item {
                required property var modelData
                x: modelData.x - uiState.optionX
                y: modelData.y - uiState.optionY
                width: modelData.width
                height: modelData.height
                visible: !(uiState.optionData.collapsed || false)
                readonly property int sliderMin: modelData.minValue !== undefined ? modelData.minValue : 0
                readonly property int sliderMax: modelData.maxValue !== undefined ? Math.max(sliderMin + 1, modelData.maxValue) : 127
                readonly property real sliderRange: Math.max(1, sliderMax - sliderMin)

                Text {
                    x: -60
                    y: -1
                    width: 56
                    text: modelData.label
                    color: Theme.text
                    font.pixelSize: 11
                    horizontalAlignment: Text.AlignRight
                }

                Rectangle {
                    x: 0
                    y: 4
                    width: parent.width
                    height: Math.max(1, parent.height - 8)
                    color: Theme.surfaceAlt
                    border.width: 1
                    border.color: Theme.border
                }

                Rectangle {
                    x: 4 + ((Math.max(0, parent.width - 8) * ((modelData.value || 0) - parent.sliderMin)) / parent.sliderRange) - 4
                    y: -2
                    width: 8
                    height: parent.height + 4
                    radius: 4
                    color: Theme.surface
                    border.width: 1
                    border.color: Theme.borderStrong
                }

                Text {
                    x: parent.width + 8
                    y: -1
                    width: 42
                    text: modelData.valueText || ""
                    color: Theme.text
                    font.pixelSize: 11
                }
            }
        }

        Repeater {
            model: uiState.optionData.graphicsRows || []

            delegate: Rectangle {
                required property var modelData
                x: modelData.x - uiState.optionX
                y: modelData.y - uiState.optionY
                width: modelData.width
                height: modelData.height
                radius: 6
                visible: !(uiState.optionData.collapsed || false)
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                Text {
                    x: 8
                    y: 4
                    text: modelData.label
                    color: Theme.text
                    font.pixelSize: 11
                }

                Text {
                    x: 110
                    y: 4
                    width: Math.max(0, parent.width - 170)
                    text: modelData.value
                    color: Theme.text
                    font.pixelSize: 11
                    elide: Text.ElideRight
                }

                Rectangle {
                    x: modelData.prevX - modelData.x
                    y: modelData.prevY - modelData.y
                    width: modelData.prevWidth
                    height: modelData.prevHeight
                    radius: 4
                    color: Theme.buttonBg
                    border.width: 1
                    border.color: Theme.buttonBorder

                    Text {
                        anchors.centerIn: parent
                        text: modelData.prevLabel || ""
                        color: Theme.buttonText
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Rectangle {
                    x: modelData.nextX - modelData.x
                    y: modelData.nextY - modelData.y
                    width: modelData.nextWidth
                    height: modelData.nextHeight
                    radius: 4
                    color: Theme.buttonBg
                    border.width: 1
                    border.color: Theme.buttonBorder

                    Text {
                        anchors.centerIn: parent
                        text: modelData.nextLabel || ""
                        color: Theme.buttonText
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }
        }

        Rectangle {
            id: optionRestartButton
            readonly property var restartButtonData: uiState.optionData.restartButton || ({})
            x: (restartButtonData.x || 0) - uiState.optionX
            y: (restartButtonData.y || 0) - uiState.optionY
            width: restartButtonData.width || 0
            height: restartButtonData.height || 0
            radius: 4
            visible: restartButtonData.visible || false
            color: Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: optionRestartButton.restartButtonData.label || ""
                color: Theme.buttonText
                font.pixelSize: 10
            }
        }

        Rectangle {
            id: optionApplyButton
            readonly property var applyButtonData: uiState.optionData.applyButton || ({})
            x: (applyButtonData.x || 0) - uiState.optionX
            y: (applyButtonData.y || 0) - uiState.optionY
            width: applyButtonData.width || 0
            height: applyButtonData.height || 0
            radius: 4
            visible: applyButtonData.visible || false
            color: Theme.buttonBg
            border.width: 1
            border.color: Theme.buttonBorder

            Text {
                anchors.centerIn: parent
                text: optionApplyButton.applyButtonData.label || ""
                color: Theme.buttonText
                font.pixelSize: 10
            }
        }
    }

    Rectangle {
        x: uiState.shopChoiceX
        y: uiState.shopChoiceY
        width: uiState.shopChoiceWidth
        height: uiState.shopChoiceHeight
        radius: 10
        color: Theme.surface
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.shopChoiceVisible

        Text {
            x: 12
            y: 10
            text: uiState.shopChoiceTitle
            color: Theme.text
            font.pixelSize: 15
            font.bold: true
        }

        Text {
            x: 14
            y: 30
            width: parent.width - 28
            text: uiState.shopChoicePrompt
            color: Theme.textMuted
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
                color: modelData.pressed
                    ? Theme.buttonBgPressed
                    : (modelData.hot ? Theme.buttonBgHover : Theme.buttonBg)
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: Theme.buttonText
                    font.pixelSize: 12
                }
            }
        }
    }

    Rectangle {
        id: controllerPanel
        x: uiState.controllerX
        y: uiState.controllerY
        width: uiState.controllerWidth
        height: uiState.controllerHeight
        radius: 10
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.controllerVisible
        clip: true

        readonly property var _rows: uiState.controllerData.rows || []
        readonly property var _tabs: uiState.controllerData.tabs || []
        readonly property int _activeTab: uiState.controllerData.activeTab || 0
        readonly property bool _gamepadTab: _activeTab === 1
        readonly property bool _rebinding: {
            var r = _rows;
            for (var i = 0; i < r.length; ++i) {
                if (r[i].rebinding) return true;
            }
            return false;
        }
        readonly property var _rebindingRow: {
            var r = _rows;
            for (var i = 0; i < r.length; ++i) {
                if (r[i].rebinding) return r[i];
            }
            return null;
        }

        // Title strip
        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 32
            radius: 10
            color: Theme.accent
            z: 0
        }
        Rectangle {
            x: 0
            y: 20
            width: parent.width
            height: 14
            color: Theme.accent
            z: 0
        }
        Text {
            x: 16
            y: 8
            width: parent.width - 32
            text: "Controller / Hotkeys"
            color: Theme.accentText
            font.pixelSize: 14
            font.bold: true
            z: 1
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 16
            y: 10
            text: (uiState.controllerData.connected ? "●  " : "○  ")
                + (uiState.controllerData.connected ? "Connected" : "Disconnected")
                + "   │   "
                + (uiState.controllerData.controllerName || "No controller")
                + "   │   Move: "
                + (uiState.controllerData.moveModeText || "")
            color: Theme.accentText
            font.pixelSize: 11
            z: 1
        }

        // Live controller / hint area (matches backend kHeaderH + kLiveH region: y=16..208)
        Rectangle {
            x: 16
            y: 58
            width: parent.width - 32
            height: 150
            radius: 8
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            z: 0

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 10
                text: uiState.controllerData.connected
                    ? "Live controller state"
                    : "No controller detected"
                color: Theme.textMuted
                font.pixelSize: 11
                font.bold: true
            }

            Text {
                anchors.centerIn: parent
                width: parent.width - 40
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                text: uiState.controllerData.connected
                    ? (uiState.controllerData.controllerName || "Controller")
                      + "\n\nPress any button to see it light up here."
                    : "Connect an Xbox, PlayStation, or other SDL-compatible\ncontroller to begin binding gamepad actions."
                color: Theme.text
                font.pixelSize: 12
            }
        }

        // Movement-mode toggle row (matches backend click zone: y=212..236)
        Rectangle {
            x: 16
            y: 212
            width: parent.width - 32
            height: 24
            radius: 6
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.borderStrong
            z: 1

            Text {
                x: 10
                anchors.verticalCenter: parent.verticalCenter
                text: "Movement mode"
                color: Theme.text
                font.pixelSize: 11
                font.bold: true
            }

            Rectangle {
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                width: 180
                height: 16
                radius: 8
                color: Theme.accent
                Text {
                    anchors.centerIn: parent
                    text: (uiState.controllerData.moveModeText || "Cursor") + "   (click to switch)"
                    color: Theme.accentText
                    font.pixelSize: 10
                    font.bold: true
                }
            }
        }

        // Tabs (use backend positions so clicks line up with Win32 hit-test)
        Repeater {
            model: controllerPanel._tabs

            delegate: Rectangle {
                required property var modelData
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height + 4
                radius: 6
                color: modelData.active ? Theme.background : Theme.surfaceAlt
                border.width: modelData.active ? 2 : 1
                border.color: modelData.active ? Theme.accent : Theme.border
                z: 2

                Text {
                    anchors.centerIn: parent
                    text: (modelData.id === 1 ? "🎮  " : "⌨  ") + (modelData.label || "")
                    color: modelData.active ? Theme.text : Theme.textMuted
                    font.pixelSize: 12
                    font.bold: modelData.active
                }

                Rectangle {
                    visible: modelData.active
                    anchors.bottom: parent.bottom
                    x: 0
                    width: parent.width
                    height: 2
                    color: Theme.background
                    z: 3
                }
            }
        }

        // Bindings list background (starts at backend row y=266; leaves room for footer)
        Rectangle {
            x: 14
            y: 262
            width: parent.width - 28
            height: parent.height - 262 - 32
            radius: 6
            color: Theme.surface
            border.width: 1
            border.color: Theme.border
            z: 0
        }

        Text {
            x: 20
            y: 244
            text: controllerPanel._gamepadTab ? "Controller bindings" : "Keyboard bindings"
            color: Theme.text
            font.pixelSize: 11
            font.bold: true
            z: 2
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 20
            y: 244
            text: controllerPanel._rows.length + " actions"
            color: Theme.textMuted
            font.pixelSize: 10
            z: 2
        }

        Text {
            x: 20
            y: 280
            visible: !controllerPanel._rows.length
            text: controllerPanel._gamepadTab
                ? "No gamepad actions available."
                : "No bindings loaded."
            color: Theme.textMuted
            font.pixelSize: 11
            z: 2
        }

        // Binding rows — use backend positions so visuals align with Win32 hit-testing
        Repeater {
            model: controllerPanel._rows

            delegate: Rectangle {
                required property int index
                required property var modelData
                x: modelData.x
                y: modelData.y
                width: modelData.width
                height: modelData.height
                radius: 3
                color: modelData.rebinding
                    ? Theme.accent
                    : modelData.selected
                        ? Theme.surfaceAlt
                        : (index % 2 === 0 ? Theme.background : Theme.surface)
                border.width: modelData.rebinding || modelData.selected ? 1 : 0
                border.color: modelData.rebinding ? Theme.accent : Theme.borderStrong
                z: 3

                Text {
                    x: 10
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.floor(parent.width * 0.55) - 10
                    elide: Text.ElideRight
                    text: modelData.label || ""
                    color: modelData.rebinding ? Theme.accentText : Theme.text
                    font.pixelSize: 12
                    font.bold: modelData.selected || modelData.rebinding
                }

                Rectangle {
                    x: Math.floor(parent.width * 0.55)
                    anchors.verticalCenter: parent.verticalCenter
                    width: Math.floor(parent.width * 0.45) - 12
                    height: parent.height - 4
                    radius: 3
                    color: modelData.rebinding
                        ? Theme.accentText
                        : (modelData.selected ? Theme.background : "transparent")
                    border.width: modelData.rebinding ? 0 : 1
                    border.color: modelData.selected ? Theme.borderStrong : Theme.border
                    opacity: modelData.rebinding ? 0.15 : 1.0

                    Text {
                        anchors.centerIn: parent
                        width: parent.width - 12
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        text: modelData.bindingText || "—"
                        color: modelData.rebinding ? Theme.accentText : Theme.text
                        font.pixelSize: 11
                        font.bold: true
                        font.family: "Consolas"
                    }
                }
            }
        }

        // Footer hints
        Rectangle {
            x: 0
            y: parent.height - 28
            width: parent.width
            height: 28
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border
            z: 2

            Text {
                anchors.verticalCenter: parent.verticalCenter
                x: 16
                text: "Click / Enter: rebind    •    Esc: cancel / close    •    Del: reset defaults    •    Tab: switch tab    •    ↑↓: move"
                color: Theme.textMuted
                font.pixelSize: 10
            }
        }

        // Rebinding overlay — dims the panel and shows a prominent prompt
        Rectangle {
            anchors.fill: parent
            visible: controllerPanel._rebinding
            color: Theme.overlayDim
            opacity: 0.88
            z: 10
        }

        Rectangle {
            visible: controllerPanel._rebinding
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height / 2 - 80
            width: Math.min(parent.width - 60, 520)
            height: 160
            radius: 10
            color: Theme.accent
            border.width: 2
            border.color: Theme.borderStrong
            z: 11

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 16
                text: controllerPanel._gamepadTab ? "Waiting for controller input" : "Waiting for key press"
                color: Theme.accentText
                font.pixelSize: 14
                font.bold: true
                opacity: 0.85
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 44
                text: (controllerPanel._rebindingRow ? controllerPanel._rebindingRow.label : "") + "  →"
                color: Theme.accentText
                font.pixelSize: 18
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 78
                text: controllerPanel._gamepadTab ? "Press any button…" : "Press any key…"
                color: Theme.accentText
                font.pixelSize: 22
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 124
                text: "Esc to cancel"
                color: Theme.accentText
                font.pixelSize: 11
                opacity: 0.75
            }
        }
    }

    Rectangle {
        id: emotionPanel
        x: uiState.emotionData.x || 0
        y: uiState.emotionData.y || 0
        width: uiState.emotionData.width || 0
        height: uiState.emotionData.height || 0
        radius: 10
        color: Theme.background
        border.width: 1
        border.color: Theme.borderStrong
        visible: uiState.emotionVisible
        clip: true

        readonly property var _buttons: uiState.emotionData.buttons || []
        readonly property string _title: uiState.emotionData.title || "Emotion Picker"
        readonly property string _footer: uiState.emotionData.footer || "/emotion N supports all 0-87 emotes"

        Rectangle {
            x: 0
            y: 0
            width: parent.width
            height: 32
            radius: 10
            color: Theme.accent
            z: 0
        }
        Rectangle {
            x: 0
            y: 20
            width: parent.width
            height: 14
            color: Theme.accent
            z: 0
        }
        Text {
            x: 16
            y: 8
            width: parent.width - 32
            text: emotionPanel._title
            color: Theme.accentText
            font.pixelSize: 14
            font.bold: true
            z: 1
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: 16
            y: 10
            text: "Alt+E opens   •   Alt+1..0 sends"
            color: Theme.accentText
            font.pixelSize: 11
            z: 1
        }

        Repeater {
            model: emotionPanel._buttons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - emotionPanel.x
                y: (modelData.y || 0) - emotionPanel.y
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 6
                color: modelData.pressed
                    ? Theme.accent
                    : (modelData.hovered ? Theme.surfaceAlt : Theme.surface)
                border.width: 1
                border.color: modelData.pressed ? Theme.accentText : Theme.borderStrong
                z: 2

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 5
                    text: modelData.label || ""
                    color: modelData.pressed ? Theme.accentText : Theme.text
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    y: 24
                    text: modelData.hotkey || ""
                    color: modelData.pressed ? Theme.accentText : Theme.textMuted
                    font.pixelSize: 10
                }
            }
        }

        Rectangle {
            x: 0
            y: parent.height - 28
            width: parent.width
            height: 28
            color: Theme.surfaceAlt
            border.width: 1
            border.color: Theme.border
            z: 2

            Text {
                anchors.verticalCenter: parent.verticalCenter
                x: 16
                text: emotionPanel._footer
                color: Theme.textMuted
                font.pixelSize: 10
            }
        }
    }

    Rectangle {
        x: uiState.serverPanelX
        y: uiState.serverPanelY
        width: uiState.serverPanelWidth
        height: uiState.serverPanelHeight
        radius: 8
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        visible: uiState.serverSelectVisible

        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 4

            Text {
                text: uiState.serverPanelData.title || ""
                color: Theme.text
                font.pixelSize: 14
                font.bold: true
            }

            Repeater {
                model: uiState.serverEntryLabels.length

                delegate: Rectangle {
                    readonly property bool isSelected: index === uiState.serverSelectedIndex
                    readonly property bool isHovered: index === uiState.serverHoverIndex
                    readonly property int statusWidth: 64
                    readonly property int labelLeft: isSelected ? 14 : 8
                    readonly property string labelText: {
                        var base = String(uiState.serverEntryLabels[index] || "")
                        if (base.length === 0) {
                            base = "Server " + (index + 1)
                        }
                        return "#" + (index + 1) + " " + base
                    }
                    readonly property string detailText: {
                        if (isSelected) {
                            return "Selected"
                        }
                        var value = String(uiState.serverEntryDetails[index] || "")
                        if (value.length > 0) {
                            return value
                        }
                        return ""
                    }

                    width: parent ? parent.width : 0
                    height: 24
                    radius: 2
                    color: isSelected
                        ? Theme.accent
                        : (isHovered ? Theme.surfaceAlt : Theme.surface)
                    border.width: isSelected ? 2 : 1
                    border.color: isSelected ? Theme.borderStrong : Theme.border
                    clip: true

                    Rectangle {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: isSelected ? 8 : 0
                        color: Theme.borderStrong
                        visible: isSelected
                    }

                    Text {
                        id: serverLabel
                        x: labelLeft
                        width: Math.max(1, parent.width - labelLeft - statusWidth - 12)
                        anchors.verticalCenter: parent.verticalCenter
                        text: labelText
                        color: isSelected ? Theme.accentText : Theme.text
                        font.pixelSize: 12
                        font.bold: isSelected
                        elide: Text.ElideRight
                        textFormat: Text.PlainText
                        z: 1
                    }

                    Text {
                        id: serverStatus
                        x: parent.width - statusWidth - 8
                        width: statusWidth
                        anchors.verticalCenter: parent.verticalCenter
                        text: detailText
                        color: isSelected ? Theme.accentText : Theme.textMuted
                        font.pixelSize: 12
                        font.bold: isSelected
                        textFormat: Text.PlainText
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                        z: 1
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
        color: "transparent"
        border.width: 0
        visible: uiState.loginPanelVisible

        Image {
            anchors.fill: parent
            fillMode: Image.Stretch
            smooth: false
            cache: false
            source: parent.visible ? ("image://openmidgard/loginpanel?skin=" + encodeURIComponent(uiState.skinRevision || "0")) : ""
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.background
            opacity: darkTheme ? 0.44 : 0.10
        }

        Rectangle {
            x: 92
            y: 29
            width: 125
            height: 18
            color: Theme.inputBg
            border.width: 1
            border.color: uiState.loginPasswordFocused ? Theme.inputBorder : Theme.borderStrong
        }

        Text {
            x: 98
            y: 31
            width: 112
            text: uiState.loginUserId + (!uiState.loginPasswordFocused && root.loginCaretVisible ? "|" : "")
            color: Theme.inputText
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Rectangle {
            x: 92
            y: 61
            width: 125
            height: 18
            color: Theme.inputBg
            border.width: 1
            border.color: uiState.loginPasswordFocused ? Theme.borderStrong : Theme.inputBorder
        }

        Text {
            x: 98
            y: 63
            width: 112
            text: uiState.loginPasswordMask + (uiState.loginPasswordFocused && root.loginCaretVisible ? "|" : "")
            color: Theme.inputText
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Rectangle {
            x: 232
            y: 33
            width: 16
            height: 16
            color: Theme.surface
            border.width: 1
            border.color: Theme.borderStrong

            Text {
                anchors.centerIn: parent
                text: uiState.loginSaveAccountChecked ? "X" : ""
                color: Theme.accent
                font.pixelSize: 11
                font.bold: true
            }
        }

        Text {
            x: 250
            y: 31
            text: uiState.loginPanelLabels.saveLabel || ""
            color: Theme.text
            font.pixelSize: 11
        }

        Repeater {
            model: uiState.loginButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.loginPanelX
                y: (modelData.y || 0) - uiState.loginPanelY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 3
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
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
        color: "transparent"
        border.width: 0
        visible: uiState.charSelectVisible

        Image {
            anchors.fill: parent
            fillMode: Image.Stretch
            smooth: false
            cache: false
            source: parent.visible ? ("image://openmidgard/charselectpanel?rev=" + root.charSelectPanelKey()) : ""
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.background
            opacity: darkTheme ? 0.40 : 0.08
        }

        Repeater {
            model: uiState.charSelectSlots

            delegate: Item {
                required property var modelData
                x: modelData.x - uiState.charSelectPanelX
                y: modelData.y - uiState.charSelectPanelY
                width: modelData.width
                height: modelData.height

                Image {
                    x: 0
                    y: 0
                    width: implicitWidth
                    height: implicitHeight
                    fillMode: Image.Pad
                    smooth: false
                    cache: false
                    visible: modelData.selected
                    source: visible ? ("image://openmidgard/charselectslotselected?skin=" + encodeURIComponent(uiState.skinRevision || "0")) : ""
                }
            }
        }

        Repeater {
            model: uiState.charSelectSelectedDetails.fields || []

            delegate: Text {
                required property var modelData
                x: (modelData.x || 0) - uiState.charSelectPanelX
                y: (modelData.y || 0) - uiState.charSelectPanelY
                width: modelData.width || 0
                height: modelData.height || 0
                text: modelData.text || ""
                color: Theme.text
                font.pixelSize: 11
            }
        }

        Repeater {
            model: uiState.charSelectPageButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.charSelectPanelX
                y: (modelData.y || 0) - uiState.charSelectPanelY
                width: modelData.width || 0
                height: modelData.height || 0
                visible: modelData.visible || false
                color: "transparent"
                border.width: 0

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.accent
                    font.pixelSize: 18
                    font.bold: true
                }
            }
        }

        Repeater {
            model: uiState.charSelectActionButtons

            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - uiState.charSelectPanelX
                y: (modelData.y || 0) - uiState.charSelectPanelY
                width: modelData.width || 0
                height: modelData.height || 0
                radius: 3
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                visible: modelData.visible || false

                Text {
                    anchors.centerIn: parent
                    text: modelData.label || ""
                    color: Theme.buttonText
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }
    }

    Rectangle {
        x: uiState.makeCharPanelX
        y: uiState.makeCharPanelY
        width: uiState.makeCharPanelWidth
        height: uiState.makeCharPanelHeight
        color: "transparent"
        border.width: 0
        visible: uiState.makeCharVisible

        Image {
            anchors.fill: parent
            fillMode: Image.Stretch
            smooth: false
            cache: false
            source: parent.visible
                ? ("image://openmidgard/makecharpanel?rev="
                    + ((uiState.makeCharPanelData && uiState.makeCharPanelData.imageRevision) || 0)
                    + "&buttons=" + root.makeCharPanelButtonsKey()
                    + "&skin=" + encodeURIComponent(uiState.skinRevision || "0"))
                : ""
        }

        Rectangle {
            anchors.fill: parent
            color: Theme.background
            opacity: darkTheme ? 0.34 : 0.06
        }

        Rectangle {
            x: 62
            y: 244
            width: 100
            height: 18
            color: Theme.inputBg
            border.width: 1
            border.color: uiState.makeCharNameFocused ? Theme.borderStrong : Theme.inputBorder
        }

        Text {
            x: 68
            y: 246
            width: 88
            text: uiState.makeCharName + (uiState.makeCharNameFocused && root.loginCaretVisible ? "|" : "")
            color: Theme.inputText
            font.pixelSize: 12
            elide: Text.ElideRight
        }

        Repeater {
            model: uiState.makeCharStatFields

            delegate: Text {
                required property var modelData
                x: (modelData.x || 0) - uiState.makeCharPanelX
                y: (modelData.y || 0) - uiState.makeCharPanelY
                width: 30
                text: modelData.value !== undefined ? modelData.value : ""
                color: Theme.text
                font.pixelSize: 12
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Repeater {
            model: uiState.makeCharButtons

            delegate: Item {
                required property var modelData
                x: (modelData.x || 0) - uiState.makeCharPanelX
                y: (modelData.y || 0) - uiState.makeCharPanelY
                width: modelData.width || 0
                height: modelData.height || 0
            }
        }
    }

    // Cart panel
    Rectangle {
        visible: (uiState.cartPanelData.visible || false)
        x: uiState.cartPanelData.x || 0
        y: uiState.cartPanelData.y || 0
        width: uiState.cartPanelData.width || 180
        height: uiState.cartPanelData.height || 240
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: (uiState.cartPanelData.title || "Cart")
                    + " (" + (uiState.cartPanelData.currentCount || 0)
                    + "/" + (uiState.cartPanelData.maxCount || 0) + ")"
            }
        }
        Repeater {
            model: uiState.cartPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.cartPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.cartPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Text {
            visible: !(uiState.cartPanelData.minimized || false)
            x: 6; y: 20; font.pixelSize: 10; color: Theme.text
            text: "Weight: " + (uiState.cartPanelData.currentWeight || 0)
                + " / " + (uiState.cartPanelData.maxWeight || 0)
        }
        ListView {
            visible: !(uiState.cartPanelData.minimized || false)
            x: 6; y: 36
            width: parent.width - 12
            height: parent.height - 42
            clip: true
            model: uiState.cartPanelData.entries || []
            delegate: Text {
                required property var modelData
                text: (modelData.label || "")
                color: Theme.text
                font.pixelSize: 10
                height: 14
            }
        }
    }

    // Guild panel
    Rectangle {
        visible: (uiState.guildPanelData.visible || false)
        x: uiState.guildPanelData.x || 0
        y: uiState.guildPanelData.y || 0
        width: uiState.guildPanelData.width || 300
        height: uiState.guildPanelData.height || 280
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: (uiState.guildPanelData.title || "Guild")
                    + " — " + (uiState.guildPanelData.guildName || "")
            }
        }
        Repeater {
            model: uiState.guildPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.guildPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.guildPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        // Info tab
        Column {
            x: 6; y: 20; spacing: 2
            visible: !(uiState.guildPanelData.minimized || false) && (uiState.guildPanelData.activeTab || 0) === 0
            Text { text: "Master: " + (uiState.guildPanelData.masterName || ""); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Guild ID: " + (uiState.guildPanelData.guildId || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Emblem: " + (uiState.guildPanelData.emblemId || 0); color: Theme.text; font.pixelSize: 10 }
            Text {
                text: "Members: " + ((uiState.guildPanelData.members || []).length)
                color: Theme.text; font.pixelSize: 10
            }
        }

        // Members tab
        ListView {
            x: 6; y: 20
            width: parent.width - 12
            height: parent.height - 26
            visible: !(uiState.guildPanelData.minimized || false) && (uiState.guildPanelData.activeTab || 0) === 1
            clip: true
            model: uiState.guildPanelData.members || []
            delegate: Rectangle {
                required property var modelData
                width: ListView.view ? ListView.view.width : 0
                height: 14
                color: (modelData.online ? Theme.inputBg : Theme.surfaceAlt)
                Text {
                    x: 4
                    anchors.verticalCenter: parent.verticalCenter
                    text: (modelData.online ? "●  " : "○  ")
                        + (modelData.name || "")
                        + "  Lv" + (modelData.level || 0)
                        + "  [" + (modelData.positionId || 0) + "]"
                    color: Theme.text
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }
            }
        }
    }

    // MailBox panel
    Rectangle {
        visible: (uiState.mailBoxPanelData.visible || false)
        x: uiState.mailBoxPanelData.x || 0
        y: uiState.mailBoxPanelData.y || 0
        width: uiState.mailBoxPanelData.width || 320
        height: uiState.mailBoxPanelData.height || 240
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: uiState.mailBoxPanelData.title || "Mailbox"
            }
        }
        Repeater {
            model: uiState.mailBoxPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.mailBoxPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.mailBoxPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        ListView {
            visible: !(uiState.mailBoxPanelData.minimized || false)
            x: 6; y: 20
            width: parent.width - 12
            height: parent.height - 26
            clip: true
            model: uiState.mailBoxPanelData.entries || []
            delegate: Text {
                required property var modelData
                text: (modelData.isRead ? "  " : "* ")
                    + (modelData.sender || "?") + " — " + (modelData.title || "")
                color: Theme.text
                font.pixelSize: 10
                height: 14
            }
        }
    }

    // MailRead panel
    Rectangle {
        visible: (uiState.mailReadPanelData.visible || false)
        x: uiState.mailReadPanelData.x || 0
        y: uiState.mailReadPanelData.y || 0
        width: uiState.mailReadPanelData.width || 320
        height: uiState.mailReadPanelData.height || 280
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: uiState.mailReadPanelData.title || "Read Mail"
            }
        }
        Repeater {
            model: uiState.mailReadPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.mailReadPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.mailReadPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Column {
            visible: !(uiState.mailReadPanelData.minimized || false)
            x: 6; y: 20; spacing: 3
            Text { text: "From: " + (uiState.mailReadPanelData.sender || ""); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Subject: " + (uiState.mailReadPanelData.subject || ""); color: Theme.text; font.pixelSize: 10 }
            Text {
                text: "Zeny: " + (uiState.mailReadPanelData.zeny || 0)
                    + "    Attach: item#" + (uiState.mailReadPanelData.attachItemId || 0)
                    + " x" + (uiState.mailReadPanelData.attachAmount || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text {
                width: parent.parent.width - 12
                wrapMode: Text.WordWrap
                text: uiState.mailReadPanelData.body || ""
                color: Theme.text
                font.pixelSize: 10
            }
        }
    }

    // MailSend panel
    Rectangle {
        visible: (uiState.mailSendPanelData.visible || false)
        x: uiState.mailSendPanelData.x || 0
        y: uiState.mailSendPanelData.y || 0
        width: uiState.mailSendPanelData.width || 320
        height: uiState.mailSendPanelData.height || 280
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: uiState.mailSendPanelData.title || "Send Mail"
            }
        }
        Repeater {
            model: uiState.mailSendPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.mailSendPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.mailSendPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Column {
            visible: !(uiState.mailSendPanelData.minimized || false)
            x: 6; y: 20; spacing: 3
            Text { text: "To: " + (uiState.mailSendPanelData.recipient || ""); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Subject: " + (uiState.mailSendPanelData.subject || ""); color: Theme.text; font.pixelSize: 10 }
            Text {
                text: "Zeny: " + (uiState.mailSendPanelData.zeny || 0)
                    + "    Attach inv#" + (uiState.mailSendPanelData.attachInventoryIndex || 0)
                    + " x" + (uiState.mailSendPanelData.attachAmount || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text {
                width: parent.parent.width - 12
                wrapMode: Text.WordWrap
                text: uiState.mailSendPanelData.body || ""
                color: Theme.text
                font.pixelSize: 10
            }
        }
    }

    // Pet info panel
    Rectangle {
        visible: (uiState.petInfoPanelData.visible || false)
        x: uiState.petInfoPanelData.x || 0
        y: uiState.petInfoPanelData.y || 0
        width: uiState.petInfoPanelData.width || 200
        height: uiState.petInfoPanelData.height || 200
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: (uiState.petInfoPanelData.title || "Pet")
                    + ": " + (uiState.petInfoPanelData.petName || "")
            }
        }
        Repeater {
            model: uiState.petInfoPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.petInfoPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.petInfoPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Column {
            visible: !(uiState.petInfoPanelData.minimized || false)
            x: 6; y: 20; spacing: 2
            Text { text: "Level: " + (uiState.petInfoPanelData.level || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Fullness: " + (uiState.petInfoPanelData.fullness || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Intimacy: " + (uiState.petInfoPanelData.intimacy || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Item ID: " + (uiState.petInfoPanelData.itemId || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Job: " + (uiState.petInfoPanelData.job || 0); color: Theme.text; font.pixelSize: 10 }
        }
    }

    // Egg list panel
    Rectangle {
        visible: (uiState.eggListPanelData.visible || false)
        x: uiState.eggListPanelData.x || 0
        y: uiState.eggListPanelData.y || 0
        width: uiState.eggListPanelData.width || 200
        height: uiState.eggListPanelData.height || 240
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: uiState.eggListPanelData.title || "Pet Eggs"
            }
        }
        Repeater {
            model: uiState.eggListPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.eggListPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.eggListPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        ListView {
            visible: !(uiState.eggListPanelData.minimized || false)
            x: 6; y: 20
            width: parent.width - 12
            height: parent.height - 26
            clip: true
            model: uiState.eggListPanelData.eggItemIds || []
            delegate: Text {
                required property var modelData
                text: "Egg #" + modelData
                color: Theme.text
                font.pixelSize: 10
                height: 14
            }
        }
    }

    // Homun info panel
    Rectangle {
        visible: (uiState.homunInfoPanelData.visible || false)
        x: uiState.homunInfoPanelData.x || 0
        y: uiState.homunInfoPanelData.y || 0
        width: uiState.homunInfoPanelData.width || 220
        height: uiState.homunInfoPanelData.height || 220
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: (uiState.homunInfoPanelData.title || "Homunculus")
                    + ": " + (uiState.homunInfoPanelData.homunName || "")
            }
        }
        Repeater {
            model: uiState.homunInfoPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.homunInfoPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.homunInfoPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Column {
            visible: !(uiState.homunInfoPanelData.minimized || false)
            x: 6; y: 20; spacing: 2
            Text { text: "Level: " + (uiState.homunInfoPanelData.level || 0); color: Theme.text; font.pixelSize: 10 }
            Text {
                text: "HP: " + (uiState.homunInfoPanelData.hp || 0)
                    + " / " + (uiState.homunInfoPanelData.maxHp || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text {
                text: "SP: " + (uiState.homunInfoPanelData.sp || 0)
                    + " / " + (uiState.homunInfoPanelData.maxSp || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text { text: "Hunger: " + (uiState.homunInfoPanelData.hunger || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Intimacy: " + (uiState.homunInfoPanelData.intimacy || 0); color: Theme.text; font.pixelSize: 10 }
        }
    }

    // Merc info panel
    Rectangle {
        visible: (uiState.mercInfoPanelData.visible || false)
        x: uiState.mercInfoPanelData.x || 0
        y: uiState.mercInfoPanelData.y || 0
        width: uiState.mercInfoPanelData.width || 220
        height: uiState.mercInfoPanelData.height || 220
        radius: 4
        color: Theme.background
        border.width: 1
        border.color: Theme.border
        Rectangle {
            x: 1; y: 1; width: parent.width - 2; height: 16
            radius: 3
            color: Theme.accent
            border.width: 1
            border.color: Theme.borderStrong
            Text {
                anchors.centerIn: parent
                color: Theme.accentText
                font.pixelSize: 11
                text: (uiState.mercInfoPanelData.title || "Mercenary")
                    + ": " + (uiState.mercInfoPanelData.mercName || "")
            }
        }
        Repeater {
            model: uiState.mercInfoPanelData.systemButtons || []
            delegate: Rectangle {
                required property var modelData
                x: (modelData.x || 0) - (uiState.mercInfoPanelData.x || 0)
                y: (modelData.y || 0) - (uiState.mercInfoPanelData.y || 0)
                width: modelData.width || 13
                height: modelData.height || 13
                radius: 2
                color: Theme.buttonBg
                border.width: 1
                border.color: Theme.buttonBorder
                Text { anchors.centerIn: parent; text: modelData.label || ""; color: Theme.buttonText; font.pixelSize: 10; font.bold: true }
            }
        }
        Column {
            visible: !(uiState.mercInfoPanelData.minimized || false)
            x: 6; y: 20; spacing: 2
            Text { text: "Level: " + (uiState.mercInfoPanelData.level || 0); color: Theme.text; font.pixelSize: 10 }
            Text {
                text: "HP: " + (uiState.mercInfoPanelData.hp || 0)
                    + " / " + (uiState.mercInfoPanelData.maxHp || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text {
                text: "SP: " + (uiState.mercInfoPanelData.sp || 0)
                    + " / " + (uiState.mercInfoPanelData.maxSp || 0)
                color: Theme.text
                font.pixelSize: 10
            }
            Text { text: "Faith: " + (uiState.mercInfoPanelData.faith || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Calls: " + (uiState.mercInfoPanelData.calls || 0); color: Theme.text; font.pixelSize: 10 }
            Text { text: "Expires: " + (uiState.mercInfoPanelData.expireTime || 0); color: Theme.text; font.pixelSize: 10 }
        }
    }
    }
}
