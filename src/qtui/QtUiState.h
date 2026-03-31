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
    Q_PROPERTY(bool loadingVisible READ loadingVisible NOTIFY loadingVisibleChanged)
    Q_PROPERTY(QString loadingMessage READ loadingMessage NOTIFY loadingMessageChanged)
    Q_PROPERTY(double loadingProgress READ loadingProgress NOTIFY loadingProgressChanged)
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
    bool loadingVisible() const { return m_loadingVisible; }
    const QString& loadingMessage() const { return m_loadingMessage; }
    double loadingProgress() const { return m_loadingProgress; }
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
    void loadingVisibleChanged();
    void loadingMessageChanged();
    void loadingProgressChanged();
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
    bool m_loadingVisible = false;
    QString m_loadingMessage;
    double m_loadingProgress = 0.0;
    QVariantList m_anchors;
};
