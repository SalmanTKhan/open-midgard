#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class QtUiState : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString backendName READ backendName NOTIFY backendNameChanged)
    Q_PROPERTY(QString modeName READ modeName NOTIFY modeNameChanged)
    Q_PROPERTY(QString renderPath READ renderPath NOTIFY renderPathChanged)
    Q_PROPERTY(QString architectureNote READ architectureNote NOTIFY architectureNoteChanged)
    Q_PROPERTY(QString loginStatus READ loginStatus NOTIFY loginStatusChanged)
    Q_PROPERTY(QString chatPreview READ chatPreview NOTIFY chatPreviewChanged)
    Q_PROPERTY(QString lastInput READ lastInput NOTIFY lastInputChanged)
    Q_PROPERTY(bool serverSelectVisible READ serverSelectVisible NOTIFY serverSelectVisibleChanged)
    Q_PROPERTY(int serverPanelX READ serverPanelX NOTIFY serverPanelGeometryChanged)
    Q_PROPERTY(int serverPanelY READ serverPanelY NOTIFY serverPanelGeometryChanged)
    Q_PROPERTY(int serverPanelWidth READ serverPanelWidth NOTIFY serverPanelGeometryChanged)
    Q_PROPERTY(int serverPanelHeight READ serverPanelHeight NOTIFY serverPanelGeometryChanged)
    Q_PROPERTY(int serverSelectedIndex READ serverSelectedIndex NOTIFY serverSelectionChanged)
    Q_PROPERTY(int serverHoverIndex READ serverHoverIndex NOTIFY serverHoverIndexChanged)
    Q_PROPERTY(QVariantList serverEntries READ serverEntries NOTIFY serverEntriesChanged)
    Q_PROPERTY(bool loginPanelVisible READ loginPanelVisible NOTIFY loginPanelVisibleChanged)
    Q_PROPERTY(int loginPanelX READ loginPanelX NOTIFY loginPanelGeometryChanged)
    Q_PROPERTY(int loginPanelY READ loginPanelY NOTIFY loginPanelGeometryChanged)
    Q_PROPERTY(int loginPanelWidth READ loginPanelWidth NOTIFY loginPanelGeometryChanged)
    Q_PROPERTY(int loginPanelHeight READ loginPanelHeight NOTIFY loginPanelGeometryChanged)
    Q_PROPERTY(QString loginUserId READ loginUserId NOTIFY loginPanelDataChanged)
    Q_PROPERTY(QString loginPasswordMask READ loginPasswordMask NOTIFY loginPanelDataChanged)
    Q_PROPERTY(bool loginSaveAccountChecked READ loginSaveAccountChecked NOTIFY loginPanelDataChanged)
    Q_PROPERTY(bool loginPasswordFocused READ loginPasswordFocused NOTIFY loginPanelDataChanged)
    Q_PROPERTY(bool charSelectVisible READ charSelectVisible NOTIFY charSelectVisibleChanged)
    Q_PROPERTY(int charSelectPanelX READ charSelectPanelX NOTIFY charSelectPanelGeometryChanged)
    Q_PROPERTY(int charSelectPanelY READ charSelectPanelY NOTIFY charSelectPanelGeometryChanged)
    Q_PROPERTY(int charSelectPanelWidth READ charSelectPanelWidth NOTIFY charSelectPanelGeometryChanged)
    Q_PROPERTY(int charSelectPanelHeight READ charSelectPanelHeight NOTIFY charSelectPanelGeometryChanged)
    Q_PROPERTY(int charSelectPage READ charSelectPage NOTIFY charSelectPageChanged)
    Q_PROPERTY(int charSelectPageCount READ charSelectPageCount NOTIFY charSelectPageChanged)
    Q_PROPERTY(QVariantList charSelectSlots READ charSelectSlots NOTIFY charSelectSlotsChanged)
    Q_PROPERTY(QVariantMap charSelectSelectedDetails READ charSelectSelectedDetails NOTIFY charSelectSelectedDetailsChanged)
    Q_PROPERTY(bool makeCharVisible READ makeCharVisible NOTIFY makeCharVisibleChanged)
    Q_PROPERTY(int makeCharPanelX READ makeCharPanelX NOTIFY makeCharPanelGeometryChanged)
    Q_PROPERTY(int makeCharPanelY READ makeCharPanelY NOTIFY makeCharPanelGeometryChanged)
    Q_PROPERTY(int makeCharPanelWidth READ makeCharPanelWidth NOTIFY makeCharPanelGeometryChanged)
    Q_PROPERTY(int makeCharPanelHeight READ makeCharPanelHeight NOTIFY makeCharPanelGeometryChanged)
    Q_PROPERTY(QString makeCharName READ makeCharName NOTIFY makeCharDataChanged)
    Q_PROPERTY(bool makeCharNameFocused READ makeCharNameFocused NOTIFY makeCharDataChanged)
    Q_PROPERTY(QVariantList makeCharStats READ makeCharStats NOTIFY makeCharDataChanged)
    Q_PROPERTY(int makeCharHairIndex READ makeCharHairIndex NOTIFY makeCharDataChanged)
    Q_PROPERTY(int makeCharHairColor READ makeCharHairColor NOTIFY makeCharDataChanged)
    Q_PROPERTY(bool loadingVisible READ loadingVisible NOTIFY loadingVisibleChanged)
    Q_PROPERTY(QString loadingMessage READ loadingMessage NOTIFY loadingMessageChanged)
    Q_PROPERTY(double loadingProgress READ loadingProgress NOTIFY loadingProgressChanged)
    Q_PROPERTY(bool npcMenuVisible READ npcMenuVisible NOTIFY npcMenuVisibleChanged)
    Q_PROPERTY(int npcMenuX READ npcMenuX NOTIFY npcMenuGeometryChanged)
    Q_PROPERTY(int npcMenuY READ npcMenuY NOTIFY npcMenuGeometryChanged)
    Q_PROPERTY(int npcMenuWidth READ npcMenuWidth NOTIFY npcMenuGeometryChanged)
    Q_PROPERTY(int npcMenuHeight READ npcMenuHeight NOTIFY npcMenuGeometryChanged)
    Q_PROPERTY(int npcMenuSelectedIndex READ npcMenuSelectedIndex NOTIFY npcMenuSelectionChanged)
    Q_PROPERTY(int npcMenuHoverIndex READ npcMenuHoverIndex NOTIFY npcMenuHoverIndexChanged)
    Q_PROPERTY(bool npcMenuOkPressed READ npcMenuOkPressed NOTIFY npcMenuButtonsChanged)
    Q_PROPERTY(bool npcMenuCancelPressed READ npcMenuCancelPressed NOTIFY npcMenuButtonsChanged)
    Q_PROPERTY(QVariantList npcMenuOptions READ npcMenuOptions NOTIFY npcMenuOptionsChanged)
    Q_PROPERTY(bool sayDialogVisible READ sayDialogVisible NOTIFY sayDialogVisibleChanged)
    Q_PROPERTY(int sayDialogX READ sayDialogX NOTIFY sayDialogGeometryChanged)
    Q_PROPERTY(int sayDialogY READ sayDialogY NOTIFY sayDialogGeometryChanged)
    Q_PROPERTY(int sayDialogWidth READ sayDialogWidth NOTIFY sayDialogGeometryChanged)
    Q_PROPERTY(int sayDialogHeight READ sayDialogHeight NOTIFY sayDialogGeometryChanged)
    Q_PROPERTY(QString sayDialogText READ sayDialogText NOTIFY sayDialogTextChanged)
    Q_PROPERTY(bool sayDialogHasAction READ sayDialogHasAction NOTIFY sayDialogActionChanged)
    Q_PROPERTY(QString sayDialogActionLabel READ sayDialogActionLabel NOTIFY sayDialogActionChanged)
    Q_PROPERTY(bool sayDialogActionHovered READ sayDialogActionHovered NOTIFY sayDialogActionChanged)
    Q_PROPERTY(bool sayDialogActionPressed READ sayDialogActionPressed NOTIFY sayDialogActionChanged)
    Q_PROPERTY(bool npcInputVisible READ npcInputVisible NOTIFY npcInputVisibleChanged)
    Q_PROPERTY(int npcInputX READ npcInputX NOTIFY npcInputGeometryChanged)
    Q_PROPERTY(int npcInputY READ npcInputY NOTIFY npcInputGeometryChanged)
    Q_PROPERTY(int npcInputWidth READ npcInputWidth NOTIFY npcInputGeometryChanged)
    Q_PROPERTY(int npcInputHeight READ npcInputHeight NOTIFY npcInputGeometryChanged)
    Q_PROPERTY(QString npcInputLabel READ npcInputLabel NOTIFY npcInputTextChanged)
    Q_PROPERTY(QString npcInputText READ npcInputText NOTIFY npcInputTextChanged)
    Q_PROPERTY(bool npcInputOkPressed READ npcInputOkPressed NOTIFY npcInputButtonsChanged)
    Q_PROPERTY(bool npcInputCancelPressed READ npcInputCancelPressed NOTIFY npcInputButtonsChanged)
    Q_PROPERTY(bool chooseMenuVisible READ chooseMenuVisible NOTIFY chooseMenuVisibleChanged)
    Q_PROPERTY(int chooseMenuX READ chooseMenuX NOTIFY chooseMenuGeometryChanged)
    Q_PROPERTY(int chooseMenuY READ chooseMenuY NOTIFY chooseMenuGeometryChanged)
    Q_PROPERTY(int chooseMenuWidth READ chooseMenuWidth NOTIFY chooseMenuGeometryChanged)
    Q_PROPERTY(int chooseMenuHeight READ chooseMenuHeight NOTIFY chooseMenuGeometryChanged)
    Q_PROPERTY(int chooseMenuSelectedIndex READ chooseMenuSelectedIndex NOTIFY chooseMenuSelectedIndexChanged)
    Q_PROPERTY(bool itemShopVisible READ itemShopVisible NOTIFY itemShopVisibleChanged)
    Q_PROPERTY(int itemShopX READ itemShopX NOTIFY itemShopGeometryChanged)
    Q_PROPERTY(int itemShopY READ itemShopY NOTIFY itemShopGeometryChanged)
    Q_PROPERTY(int itemShopWidth READ itemShopWidth NOTIFY itemShopGeometryChanged)
    Q_PROPERTY(int itemShopHeight READ itemShopHeight NOTIFY itemShopGeometryChanged)
    Q_PROPERTY(QString itemShopTitle READ itemShopTitle NOTIFY itemShopTitleChanged)
    Q_PROPERTY(QVariantList itemShopRows READ itemShopRows NOTIFY itemShopRowsChanged)
    Q_PROPERTY(bool shopChoiceVisible READ shopChoiceVisible NOTIFY shopChoiceVisibleChanged)
    Q_PROPERTY(int shopChoiceX READ shopChoiceX NOTIFY shopChoiceGeometryChanged)
    Q_PROPERTY(int shopChoiceY READ shopChoiceY NOTIFY shopChoiceGeometryChanged)
    Q_PROPERTY(int shopChoiceWidth READ shopChoiceWidth NOTIFY shopChoiceGeometryChanged)
    Q_PROPERTY(int shopChoiceHeight READ shopChoiceHeight NOTIFY shopChoiceGeometryChanged)
    Q_PROPERTY(QVariantList shopChoiceButtons READ shopChoiceButtons NOTIFY shopChoiceButtonsChanged)
    Q_PROPERTY(QVariantList notifications READ notifications NOTIFY notificationsChanged)
    Q_PROPERTY(QVariantList anchors READ anchors NOTIFY anchorsChanged)

public:
    explicit QtUiState(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

    const QString& backendName() const { return m_backendName; }
    const QString& modeName() const { return m_modeName; }
    const QString& renderPath() const { return m_renderPath; }
    const QString& architectureNote() const { return m_architectureNote; }
    const QString& loginStatus() const { return m_loginStatus; }
    const QString& chatPreview() const { return m_chatPreview; }
    const QString& lastInput() const { return m_lastInput; }
    bool serverSelectVisible() const { return m_serverSelectVisible; }
    int serverPanelX() const { return m_serverPanelX; }
    int serverPanelY() const { return m_serverPanelY; }
    int serverPanelWidth() const { return m_serverPanelWidth; }
    int serverPanelHeight() const { return m_serverPanelHeight; }
    int serverSelectedIndex() const { return m_serverSelectedIndex; }
    int serverHoverIndex() const { return m_serverHoverIndex; }
    const QVariantList& serverEntries() const { return m_serverEntries; }
    bool loginPanelVisible() const { return m_loginPanelVisible; }
    int loginPanelX() const { return m_loginPanelX; }
    int loginPanelY() const { return m_loginPanelY; }
    int loginPanelWidth() const { return m_loginPanelWidth; }
    int loginPanelHeight() const { return m_loginPanelHeight; }
    const QString& loginUserId() const { return m_loginUserId; }
    const QString& loginPasswordMask() const { return m_loginPasswordMask; }
    bool loginSaveAccountChecked() const { return m_loginSaveAccountChecked; }
    bool loginPasswordFocused() const { return m_loginPasswordFocused; }
    bool charSelectVisible() const { return m_charSelectVisible; }
    int charSelectPanelX() const { return m_charSelectPanelX; }
    int charSelectPanelY() const { return m_charSelectPanelY; }
    int charSelectPanelWidth() const { return m_charSelectPanelWidth; }
    int charSelectPanelHeight() const { return m_charSelectPanelHeight; }
    int charSelectPage() const { return m_charSelectPage; }
    int charSelectPageCount() const { return m_charSelectPageCount; }
    const QVariantList& charSelectSlots() const { return m_charSelectSlots; }
    const QVariantMap& charSelectSelectedDetails() const { return m_charSelectSelectedDetails; }
    bool makeCharVisible() const { return m_makeCharVisible; }
    int makeCharPanelX() const { return m_makeCharPanelX; }
    int makeCharPanelY() const { return m_makeCharPanelY; }
    int makeCharPanelWidth() const { return m_makeCharPanelWidth; }
    int makeCharPanelHeight() const { return m_makeCharPanelHeight; }
    const QString& makeCharName() const { return m_makeCharName; }
    bool makeCharNameFocused() const { return m_makeCharNameFocused; }
    const QVariantList& makeCharStats() const { return m_makeCharStats; }
    int makeCharHairIndex() const { return m_makeCharHairIndex; }
    int makeCharHairColor() const { return m_makeCharHairColor; }
    bool loadingVisible() const { return m_loadingVisible; }
    const QString& loadingMessage() const { return m_loadingMessage; }
    double loadingProgress() const { return m_loadingProgress; }
    bool npcMenuVisible() const { return m_npcMenuVisible; }
    int npcMenuX() const { return m_npcMenuX; }
    int npcMenuY() const { return m_npcMenuY; }
    int npcMenuWidth() const { return m_npcMenuWidth; }
    int npcMenuHeight() const { return m_npcMenuHeight; }
    int npcMenuSelectedIndex() const { return m_npcMenuSelectedIndex; }
    int npcMenuHoverIndex() const { return m_npcMenuHoverIndex; }
    bool npcMenuOkPressed() const { return m_npcMenuOkPressed; }
    bool npcMenuCancelPressed() const { return m_npcMenuCancelPressed; }
    const QVariantList& npcMenuOptions() const { return m_npcMenuOptions; }
    bool sayDialogVisible() const { return m_sayDialogVisible; }
    int sayDialogX() const { return m_sayDialogX; }
    int sayDialogY() const { return m_sayDialogY; }
    int sayDialogWidth() const { return m_sayDialogWidth; }
    int sayDialogHeight() const { return m_sayDialogHeight; }
    const QString& sayDialogText() const { return m_sayDialogText; }
    bool sayDialogHasAction() const { return m_sayDialogHasAction; }
    const QString& sayDialogActionLabel() const { return m_sayDialogActionLabel; }
    bool sayDialogActionHovered() const { return m_sayDialogActionHovered; }
    bool sayDialogActionPressed() const { return m_sayDialogActionPressed; }
    bool npcInputVisible() const { return m_npcInputVisible; }
    int npcInputX() const { return m_npcInputX; }
    int npcInputY() const { return m_npcInputY; }
    int npcInputWidth() const { return m_npcInputWidth; }
    int npcInputHeight() const { return m_npcInputHeight; }
    const QString& npcInputLabel() const { return m_npcInputLabel; }
    const QString& npcInputText() const { return m_npcInputText; }
    bool npcInputOkPressed() const { return m_npcInputOkPressed; }
    bool npcInputCancelPressed() const { return m_npcInputCancelPressed; }
    bool chooseMenuVisible() const { return m_chooseMenuVisible; }
    int chooseMenuX() const { return m_chooseMenuX; }
    int chooseMenuY() const { return m_chooseMenuY; }
    int chooseMenuWidth() const { return m_chooseMenuWidth; }
    int chooseMenuHeight() const { return m_chooseMenuHeight; }
    int chooseMenuSelectedIndex() const { return m_chooseMenuSelectedIndex; }
    bool itemShopVisible() const { return m_itemShopVisible; }
    int itemShopX() const { return m_itemShopX; }
    int itemShopY() const { return m_itemShopY; }
    int itemShopWidth() const { return m_itemShopWidth; }
    int itemShopHeight() const { return m_itemShopHeight; }
    const QString& itemShopTitle() const { return m_itemShopTitle; }
    const QVariantList& itemShopRows() const { return m_itemShopRows; }
    bool shopChoiceVisible() const { return m_shopChoiceVisible; }
    int shopChoiceX() const { return m_shopChoiceX; }
    int shopChoiceY() const { return m_shopChoiceY; }
    int shopChoiceWidth() const { return m_shopChoiceWidth; }
    int shopChoiceHeight() const { return m_shopChoiceHeight; }
    const QVariantList& shopChoiceButtons() const { return m_shopChoiceButtons; }
    const QVariantList& notifications() const { return m_notifications; }
    const QVariantList& anchors() const { return m_anchors; }

    void setBackendName(const QString& value) {
        if (m_backendName == value) {
            return;
        }
        m_backendName = value;
        emit backendNameChanged();
    }

    void setModeName(const QString& value) {
        if (m_modeName == value) {
            return;
        }
        m_modeName = value;
        emit modeNameChanged();
    }

    void setRenderPath(const QString& value) {
        if (m_renderPath == value) {
            return;
        }
        m_renderPath = value;
        emit renderPathChanged();
    }

    void setArchitectureNote(const QString& value) {
        if (m_architectureNote == value) {
            return;
        }
        m_architectureNote = value;
        emit architectureNoteChanged();
    }

    void setLoginStatus(const QString& value) {
        if (m_loginStatus == value) {
            return;
        }
        m_loginStatus = value;
        emit loginStatusChanged();
    }

    void setChatPreview(const QString& value) {
        if (m_chatPreview == value) {
            return;
        }
        m_chatPreview = value;
        emit chatPreviewChanged();
    }

    void setLastInput(const QString& value) {
        if (m_lastInput == value) {
            return;
        }
        m_lastInput = value;
        emit lastInputChanged();
    }

    void setServerSelectVisible(bool value) {
        if (m_serverSelectVisible == value) {
            return;
        }
        m_serverSelectVisible = value;
        emit serverSelectVisibleChanged();
    }

    void setServerPanelGeometry(int x, int y, int width, int height) {
        if (m_serverPanelX == x && m_serverPanelY == y
            && m_serverPanelWidth == width && m_serverPanelHeight == height) {
            return;
        }
        m_serverPanelX = x;
        m_serverPanelY = y;
        m_serverPanelWidth = width;
        m_serverPanelHeight = height;
        emit serverPanelGeometryChanged();
    }

    void setServerSelectedIndex(int value) {
        if (m_serverSelectedIndex == value) {
            return;
        }
        m_serverSelectedIndex = value;
        emit serverSelectionChanged();
    }

    void setServerHoverIndex(int value) {
        if (m_serverHoverIndex == value) {
            return;
        }
        m_serverHoverIndex = value;
        emit serverHoverIndexChanged();
    }

    void setServerEntries(const QVariantList& value) {
        m_serverEntries = value;
        emit serverEntriesChanged();
    }

    void setLoginPanelVisible(bool value) {
        if (m_loginPanelVisible == value) {
            return;
        }
        m_loginPanelVisible = value;
        emit loginPanelVisibleChanged();
    }

    void setLoginPanelGeometry(int x, int y, int width, int height) {
        if (m_loginPanelX == x && m_loginPanelY == y
            && m_loginPanelWidth == width && m_loginPanelHeight == height) {
            return;
        }
        m_loginPanelX = x;
        m_loginPanelY = y;
        m_loginPanelWidth = width;
        m_loginPanelHeight = height;
        emit loginPanelGeometryChanged();
    }

    void setLoginPanelData(const QString& userId,
        const QString& passwordMask,
        bool saveAccountChecked,
        bool passwordFocused) {
        const bool changed = m_loginUserId != userId
            || m_loginPasswordMask != passwordMask
            || m_loginSaveAccountChecked != saveAccountChecked
            || m_loginPasswordFocused != passwordFocused;
        if (!changed) {
            return;
        }
        m_loginUserId = userId;
        m_loginPasswordMask = passwordMask;
        m_loginSaveAccountChecked = saveAccountChecked;
        m_loginPasswordFocused = passwordFocused;
        emit loginPanelDataChanged();
    }

    void setCharSelectVisible(bool value) {
        if (m_charSelectVisible == value) {
            return;
        }
        m_charSelectVisible = value;
        emit charSelectVisibleChanged();
    }

    void setCharSelectPanelGeometry(int x, int y, int width, int height) {
        if (m_charSelectPanelX == x && m_charSelectPanelY == y
            && m_charSelectPanelWidth == width && m_charSelectPanelHeight == height) {
            return;
        }
        m_charSelectPanelX = x;
        m_charSelectPanelY = y;
        m_charSelectPanelWidth = width;
        m_charSelectPanelHeight = height;
        emit charSelectPanelGeometryChanged();
    }

    void setCharSelectPageState(int page, int pageCount) {
        if (m_charSelectPage == page && m_charSelectPageCount == pageCount) {
            return;
        }
        m_charSelectPage = page;
        m_charSelectPageCount = pageCount;
        emit charSelectPageChanged();
    }

    void setCharSelectSlots(const QVariantList& value) {
        m_charSelectSlots = value;
        emit charSelectSlotsChanged();
    }

    void setCharSelectSelectedDetails(const QVariantMap& value) {
        m_charSelectSelectedDetails = value;
        emit charSelectSelectedDetailsChanged();
    }

    void setMakeCharVisible(bool value) {
        if (m_makeCharVisible == value) {
            return;
        }
        m_makeCharVisible = value;
        emit makeCharVisibleChanged();
    }

    void setMakeCharPanelGeometry(int x, int y, int width, int height) {
        if (m_makeCharPanelX == x && m_makeCharPanelY == y
            && m_makeCharPanelWidth == width && m_makeCharPanelHeight == height) {
            return;
        }
        m_makeCharPanelX = x;
        m_makeCharPanelY = y;
        m_makeCharPanelWidth = width;
        m_makeCharPanelHeight = height;
        emit makeCharPanelGeometryChanged();
    }

    void setMakeCharData(const QString& name,
        bool nameFocused,
        const QVariantList& stats,
        int hairIndex,
        int hairColor) {
        const bool changed = m_makeCharName != name
            || m_makeCharNameFocused != nameFocused
            || m_makeCharStats != stats
            || m_makeCharHairIndex != hairIndex
            || m_makeCharHairColor != hairColor;
        if (!changed) {
            return;
        }
        m_makeCharName = name;
        m_makeCharNameFocused = nameFocused;
        m_makeCharStats = stats;
        m_makeCharHairIndex = hairIndex;
        m_makeCharHairColor = hairColor;
        emit makeCharDataChanged();
    }

    void setLoadingVisible(bool value) {
        if (m_loadingVisible == value) {
            return;
        }
        m_loadingVisible = value;
        emit loadingVisibleChanged();
    }

    void setLoadingMessage(const QString& value) {
        if (m_loadingMessage == value) {
            return;
        }
        m_loadingMessage = value;
        emit loadingMessageChanged();
    }

    void setLoadingProgress(double value) {
        if (m_loadingProgress == value) {
            return;
        }
        m_loadingProgress = value;
        emit loadingProgressChanged();
    }

    void setNpcMenuVisible(bool value) {
        if (m_npcMenuVisible == value) {
            return;
        }
        m_npcMenuVisible = value;
        emit npcMenuVisibleChanged();
    }

    void setNpcMenuGeometry(int x, int y, int width, int height) {
        if (m_npcMenuX == x && m_npcMenuY == y
            && m_npcMenuWidth == width && m_npcMenuHeight == height) {
            return;
        }
        m_npcMenuX = x;
        m_npcMenuY = y;
        m_npcMenuWidth = width;
        m_npcMenuHeight = height;
        emit npcMenuGeometryChanged();
    }

    void setNpcMenuSelection(int selectedIndex) {
        if (m_npcMenuSelectedIndex == selectedIndex) {
            return;
        }
        m_npcMenuSelectedIndex = selectedIndex;
        emit npcMenuSelectionChanged();
    }

    void setNpcMenuHoverIndex(int hoverIndex) {
        if (m_npcMenuHoverIndex == hoverIndex) {
            return;
        }
        m_npcMenuHoverIndex = hoverIndex;
        emit npcMenuHoverIndexChanged();
    }

    void setNpcMenuButtons(bool okPressed, bool cancelPressed) {
        if (m_npcMenuOkPressed == okPressed && m_npcMenuCancelPressed == cancelPressed) {
            return;
        }
        m_npcMenuOkPressed = okPressed;
        m_npcMenuCancelPressed = cancelPressed;
        emit npcMenuButtonsChanged();
    }

    void setNpcMenuOptions(const QVariantList& value) {
        m_npcMenuOptions = value;
        emit npcMenuOptionsChanged();
    }

    void setSayDialogVisible(bool value) {
        if (m_sayDialogVisible == value) {
            return;
        }
        m_sayDialogVisible = value;
        emit sayDialogVisibleChanged();
    }

    void setSayDialogGeometry(int x, int y, int width, int height) {
        if (m_sayDialogX == x && m_sayDialogY == y
            && m_sayDialogWidth == width && m_sayDialogHeight == height) {
            return;
        }
        m_sayDialogX = x;
        m_sayDialogY = y;
        m_sayDialogWidth = width;
        m_sayDialogHeight = height;
        emit sayDialogGeometryChanged();
    }

    void setSayDialogText(const QString& value) {
        if (m_sayDialogText == value) {
            return;
        }
        m_sayDialogText = value;
        emit sayDialogTextChanged();
    }

    void setSayDialogAction(bool hasAction,
        const QString& label,
        bool hovered,
        bool pressed) {
        const bool changed = m_sayDialogHasAction != hasAction
            || m_sayDialogActionLabel != label
            || m_sayDialogActionHovered != hovered
            || m_sayDialogActionPressed != pressed;
        if (!changed) {
            return;
        }
        m_sayDialogHasAction = hasAction;
        m_sayDialogActionLabel = label;
        m_sayDialogActionHovered = hovered;
        m_sayDialogActionPressed = pressed;
        emit sayDialogActionChanged();
    }

    void setNpcInputVisible(bool value) {
        if (m_npcInputVisible == value) {
            return;
        }
        m_npcInputVisible = value;
        emit npcInputVisibleChanged();
    }

    void setNpcInputGeometry(int x, int y, int width, int height) {
        if (m_npcInputX == x && m_npcInputY == y
            && m_npcInputWidth == width && m_npcInputHeight == height) {
            return;
        }
        m_npcInputX = x;
        m_npcInputY = y;
        m_npcInputWidth = width;
        m_npcInputHeight = height;
        emit npcInputGeometryChanged();
    }

    void setNpcInputText(const QString& label, const QString& text) {
        if (m_npcInputLabel == label && m_npcInputText == text) {
            return;
        }
        m_npcInputLabel = label;
        m_npcInputText = text;
        emit npcInputTextChanged();
    }

    void setNpcInputButtons(bool okPressed, bool cancelPressed) {
        if (m_npcInputOkPressed == okPressed && m_npcInputCancelPressed == cancelPressed) {
            return;
        }
        m_npcInputOkPressed = okPressed;
        m_npcInputCancelPressed = cancelPressed;
        emit npcInputButtonsChanged();
    }

    void setChooseMenuVisible(bool value) {
        if (m_chooseMenuVisible == value) {
            return;
        }
        m_chooseMenuVisible = value;
        emit chooseMenuVisibleChanged();
    }

    void setChooseMenuGeometry(int x, int y, int width, int height) {
        if (m_chooseMenuX == x && m_chooseMenuY == y
            && m_chooseMenuWidth == width && m_chooseMenuHeight == height) {
            return;
        }
        m_chooseMenuX = x;
        m_chooseMenuY = y;
        m_chooseMenuWidth = width;
        m_chooseMenuHeight = height;
        emit chooseMenuGeometryChanged();
    }

    void setChooseMenuSelectedIndex(int value) {
        if (m_chooseMenuSelectedIndex == value) {
            return;
        }
        m_chooseMenuSelectedIndex = value;
        emit chooseMenuSelectedIndexChanged();
    }

    void setItemShopVisible(bool value) {
        if (m_itemShopVisible == value) {
            return;
        }
        m_itemShopVisible = value;
        emit itemShopVisibleChanged();
    }

    void setItemShopGeometry(int x, int y, int width, int height) {
        if (m_itemShopX == x && m_itemShopY == y
            && m_itemShopWidth == width && m_itemShopHeight == height) {
            return;
        }
        m_itemShopX = x;
        m_itemShopY = y;
        m_itemShopWidth = width;
        m_itemShopHeight = height;
        emit itemShopGeometryChanged();
    }

    void setItemShopTitle(const QString& value) {
        if (m_itemShopTitle == value) {
            return;
        }
        m_itemShopTitle = value;
        emit itemShopTitleChanged();
    }

    void setItemShopRows(const QVariantList& value) {
        m_itemShopRows = value;
        emit itemShopRowsChanged();
    }

    void setShopChoiceVisible(bool value) {
        if (m_shopChoiceVisible == value) {
            return;
        }
        m_shopChoiceVisible = value;
        emit shopChoiceVisibleChanged();
    }

    void setShopChoiceGeometry(int x, int y, int width, int height) {
        if (m_shopChoiceX == x && m_shopChoiceY == y
            && m_shopChoiceWidth == width && m_shopChoiceHeight == height) {
            return;
        }
        m_shopChoiceX = x;
        m_shopChoiceY = y;
        m_shopChoiceWidth = width;
        m_shopChoiceHeight = height;
        emit shopChoiceGeometryChanged();
    }

    void setShopChoiceButtons(const QVariantList& value) {
        m_shopChoiceButtons = value;
        emit shopChoiceButtonsChanged();
    }

    void setNotifications(const QVariantList& value) {
        m_notifications = value;
        emit notificationsChanged();
    }

    void setAnchors(const QVariantList& value) {
        m_anchors = value;
        emit anchorsChanged();
    }

signals:
    void backendNameChanged();
    void modeNameChanged();
    void renderPathChanged();
    void architectureNoteChanged();
    void loginStatusChanged();
    void chatPreviewChanged();
    void lastInputChanged();
    void serverSelectVisibleChanged();
    void serverPanelGeometryChanged();
    void serverSelectionChanged();
    void serverHoverIndexChanged();
    void serverEntriesChanged();
    void loginPanelVisibleChanged();
    void loginPanelGeometryChanged();
    void loginPanelDataChanged();
    void charSelectVisibleChanged();
    void charSelectPanelGeometryChanged();
    void charSelectPageChanged();
    void charSelectSlotsChanged();
    void charSelectSelectedDetailsChanged();
    void makeCharVisibleChanged();
    void makeCharPanelGeometryChanged();
    void makeCharDataChanged();
    void loadingVisibleChanged();
    void loadingMessageChanged();
    void loadingProgressChanged();
    void npcMenuVisibleChanged();
    void npcMenuGeometryChanged();
    void npcMenuSelectionChanged();
    void npcMenuHoverIndexChanged();
    void npcMenuButtonsChanged();
    void npcMenuOptionsChanged();
    void sayDialogVisibleChanged();
    void sayDialogGeometryChanged();
    void sayDialogTextChanged();
    void sayDialogActionChanged();
    void npcInputVisibleChanged();
    void npcInputGeometryChanged();
    void npcInputTextChanged();
    void npcInputButtonsChanged();
    void chooseMenuVisibleChanged();
    void chooseMenuGeometryChanged();
    void chooseMenuSelectedIndexChanged();
    void itemShopVisibleChanged();
    void itemShopGeometryChanged();
    void itemShopTitleChanged();
    void itemShopRowsChanged();
    void shopChoiceVisibleChanged();
    void shopChoiceGeometryChanged();
    void shopChoiceButtonsChanged();
    void notificationsChanged();
    void anchorsChanged();

private:
    QString m_backendName;
    QString m_modeName;
    QString m_renderPath;
    QString m_architectureNote;
    QString m_loginStatus;
    QString m_chatPreview;
    QString m_lastInput;
    bool m_serverSelectVisible = false;
    int m_serverPanelX = 0;
    int m_serverPanelY = 0;
    int m_serverPanelWidth = 0;
    int m_serverPanelHeight = 0;
    int m_serverSelectedIndex = -1;
    int m_serverHoverIndex = -1;
    QVariantList m_serverEntries;
    bool m_loginPanelVisible = false;
    int m_loginPanelX = 0;
    int m_loginPanelY = 0;
    int m_loginPanelWidth = 0;
    int m_loginPanelHeight = 0;
    QString m_loginUserId;
    QString m_loginPasswordMask;
    bool m_loginSaveAccountChecked = false;
    bool m_loginPasswordFocused = false;
    bool m_charSelectVisible = false;
    int m_charSelectPanelX = 0;
    int m_charSelectPanelY = 0;
    int m_charSelectPanelWidth = 0;
    int m_charSelectPanelHeight = 0;
    int m_charSelectPage = 0;
    int m_charSelectPageCount = 0;
    QVariantList m_charSelectSlots;
    QVariantMap m_charSelectSelectedDetails;
    bool m_makeCharVisible = false;
    int m_makeCharPanelX = 0;
    int m_makeCharPanelY = 0;
    int m_makeCharPanelWidth = 0;
    int m_makeCharPanelHeight = 0;
    QString m_makeCharName;
    bool m_makeCharNameFocused = false;
    QVariantList m_makeCharStats;
    int m_makeCharHairIndex = 0;
    int m_makeCharHairColor = 0;
    bool m_loadingVisible = false;
    QString m_loadingMessage;
    double m_loadingProgress = 0.0;
    bool m_npcMenuVisible = false;
    int m_npcMenuX = 0;
    int m_npcMenuY = 0;
    int m_npcMenuWidth = 0;
    int m_npcMenuHeight = 0;
    int m_npcMenuSelectedIndex = -1;
    int m_npcMenuHoverIndex = -1;
    bool m_npcMenuOkPressed = false;
    bool m_npcMenuCancelPressed = false;
    QVariantList m_npcMenuOptions;
    bool m_sayDialogVisible = false;
    int m_sayDialogX = 0;
    int m_sayDialogY = 0;
    int m_sayDialogWidth = 0;
    int m_sayDialogHeight = 0;
    QString m_sayDialogText;
    bool m_sayDialogHasAction = false;
    QString m_sayDialogActionLabel;
    bool m_sayDialogActionHovered = false;
    bool m_sayDialogActionPressed = false;
    bool m_npcInputVisible = false;
    int m_npcInputX = 0;
    int m_npcInputY = 0;
    int m_npcInputWidth = 0;
    int m_npcInputHeight = 0;
    QString m_npcInputLabel;
    QString m_npcInputText;
    bool m_npcInputOkPressed = false;
    bool m_npcInputCancelPressed = false;
    bool m_chooseMenuVisible = false;
    int m_chooseMenuX = 0;
    int m_chooseMenuY = 0;
    int m_chooseMenuWidth = 0;
    int m_chooseMenuHeight = 0;
    int m_chooseMenuSelectedIndex = -1;
    bool m_itemShopVisible = false;
    int m_itemShopX = 0;
    int m_itemShopY = 0;
    int m_itemShopWidth = 0;
    int m_itemShopHeight = 0;
    QString m_itemShopTitle;
    QVariantList m_itemShopRows;
    bool m_shopChoiceVisible = false;
    int m_shopChoiceX = 0;
    int m_shopChoiceY = 0;
    int m_shopChoiceWidth = 0;
    int m_shopChoiceHeight = 0;
    QVariantList m_shopChoiceButtons;
    QVariantList m_notifications;
    QVariantList m_anchors;
};
