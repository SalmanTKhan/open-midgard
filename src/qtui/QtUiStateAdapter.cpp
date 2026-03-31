#include "QtUiStateAdapter.h"

#include "QtUiState.h"

#include "network/Connection.h"
#include "network/Packet.h"
#include "core/ClientInfoLocale.h"
#include "gamemode/GameMode.h"
#include "gamemode/View.h"
#include "render3d/RenderBackend.h"
#include "render3d/RenderDevice.h"
#include "session/Session.h"
#include "ui/UILoginWnd.h"
#include "ui/UILoadingWnd.h"
#include "ui/UIMakeCharWnd.h"
#include "ui/UISelectCharWnd.h"
#include "ui/UISelectServerWnd.h"
#include "ui/UIWindowMgr.h"
#include "world/GameActor.h"
#include "world/World.h"

#include <QChar>
#include <QStringList>
#include <QVariantMap>

#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr DWORD kHoverNameRequestCooldownMs = 1000;

QString ToQString(const std::string& value)
{
    return QString::fromStdString(value);
}

QString ToQString(const char* value)
{
    return value ? QString::fromLocal8Bit(value) : QString();
}

QString ToCssColor(u8 red, u8 green, u8 blue)
{
    return QStringLiteral("#%1%2%3")
        .arg(static_cast<int>(red), 2, 16, QChar('0'))
        .arg(static_cast<int>(green), 2, 16, QChar('0'))
        .arg(static_cast<int>(blue), 2, 16, QChar('0'));
}

QString BackendToQString(RenderBackendType backend)
{
    return QString::fromLatin1(GetRenderBackendName(backend));
}

QString BuildRenderPathText(RenderBackendType nativeOverlayBackend)
{
    if (nativeOverlayBackend != RenderBackendType::LegacyDirect3D7) {
        return QStringLiteral("Native GPU Qt overlay on %1").arg(BackendToQString(nativeOverlayBackend));
    }
    return QStringLiteral("CPU bridge fallback");
}

QString BuildArchitectureNote(RenderBackendType nativeOverlayBackend)
{
    QtUiRenderTargetInfo targetInfo{};
    const bool nativeTargetAvailable = GetRenderDevice().GetQtUiRenderTargetInfo(&targetInfo);
    const QString backendName = BackendToQString(targetInfo.backend);

    if (nativeTargetAvailable && targetInfo.available) {
        if (nativeOverlayBackend != RenderBackendType::LegacyDirect3D7) {
            return QStringLiteral("Qt Quick is rendering into a native %1 overlay texture and the renderer composites that texture in the GPU overlay pass.")
                .arg(BackendToQString(nativeOverlayBackend));
        }
        if (targetInfo.backend == RenderBackendType::Vulkan) {
            return QStringLiteral("Renderer-native Vulkan target is available for Qt UI (%1x%2). Legacy paths can now be retired feature-by-feature.")
                .arg(targetInfo.width)
                .arg(targetInfo.height);
        }

        return QStringLiteral("Renderer-owned Qt target descriptor is available on %1 (%2x%3). Remaining legacy paths should move onto the native GPU overlay.")
            .arg(backendName)
            .arg(targetInfo.width)
            .arg(targetInfo.height);
    }

    return QStringLiteral("Qt runtime is active, but %1 still falls back to the CPU bridge until a native target is available.")
        .arg(backendName);
}

QString BuildChatPreviewText()
{
    const std::vector<UIChatEvent> preview = g_windowMgr.GetChatPreviewEvents(3);
    if (preview.empty()) {
        return QStringLiteral("No recent chat events.");
    }

    QStringList lines;
    for (const UIChatEvent& event : preview) {
        lines.push_back(ToQString(event.text));
    }
    return lines.join(QLatin1Char('\n'));
}

QString BuildMenuModeText()
{
    if (g_windowMgr.m_loadingWnd && g_windowMgr.m_loadingWnd->m_show != 0) {
        return QStringLiteral("Loading");
    }
    if (g_windowMgr.m_waitWnd && g_windowMgr.m_waitWnd->m_show != 0) {
        return QStringLiteral("Please Wait");
    }
    if (g_windowMgr.m_makeCharWnd && g_windowMgr.m_makeCharWnd->m_show != 0) {
        return QStringLiteral("Character Create");
    }
    if (g_windowMgr.m_selectCharWnd && g_windowMgr.m_selectCharWnd->m_show != 0) {
        return QStringLiteral("Character Select");
    }
    if (g_windowMgr.m_selectServerWnd && g_windowMgr.m_selectServerWnd->m_show != 0) {
        return QStringLiteral("Server Select");
    }
    if (g_windowMgr.m_loginWnd && g_windowMgr.m_loginWnd->m_show != 0) {
        return QStringLiteral("Login");
    }
    return QStringLiteral("Menu");
}

void PopulateLoadingState(QtUiState* state)
{
    if (!state) {
        return;
    }

    const UILoadingWnd* const loadingWnd = g_windowMgr.m_loadingWnd;
    const UIWaitWnd* const waitWnd = g_windowMgr.m_waitWnd;
    const bool loadingVisible = (loadingWnd && loadingWnd->m_show != 0)
        || (waitWnd && waitWnd->m_show != 0);
    state->setLoadingVisible(loadingVisible);
    if (loadingWnd && loadingWnd->m_show != 0) {
        state->setLoadingMessage(ToQString(loadingWnd->GetMessage().c_str()));
        state->setLoadingProgress(static_cast<double>(loadingWnd->GetProgress()));
    } else if (waitWnd && waitWnd->m_show != 0) {
        state->setLoadingMessage(ToQString(waitWnd->m_waitMsg.c_str()));
        state->setLoadingProgress(0.0);
    } else {
        state->setLoadingMessage(QString());
        state->setLoadingProgress(0.0);
    }
}

void PopulateServerSelectState(QtUiState* state)
{
    if (!state) {
        return;
    }

    const UISelectServerWnd* const serverWnd = g_windowMgr.m_selectServerWnd;
    const bool visible = serverWnd && serverWnd->m_show != 0 && GetClientInfoConnectionCount() > 1;
    state->setServerSelectVisible(visible);
    if (!visible) {
        state->setServerPanelGeometry(0, 0, 0, 0);
        state->setServerSelectedIndex(-1);
        state->setServerHoverIndex(-1);
        state->setServerEntries(QVariantList{});
        return;
    }

    state->setServerPanelGeometry(serverWnd->m_x, serverWnd->m_y, serverWnd->m_w, serverWnd->m_h);
    state->setServerSelectedIndex(GetSelectedClientInfoIndex());
    state->setServerHoverIndex(serverWnd->GetHoverIndex());

    QVariantList entries;
    const std::vector<ClientInfoConnection>& connections = GetClientInfoConnections();
    entries.reserve(static_cast<qsizetype>(connections.size()));
    for (const ClientInfoConnection& info : connections) {
        QVariantMap entry;
        entry.insert(QStringLiteral("label"), ToQString((!info.display.empty() ? info.display : info.address).c_str()));
        entry.insert(QStringLiteral("detail"), ToQString((!info.desc.empty() ? info.desc : info.port).c_str()));
        entries.push_back(entry);
    }
    state->setServerEntries(entries);
}

void PopulateLoginPanelState(QtUiState* state)
{
    if (!state) {
        return;
    }

    const UILoginWnd* const loginWnd = g_windowMgr.m_loginWnd;
    const bool visible = loginWnd && loginWnd->m_show != 0;
    state->setLoginPanelVisible(visible);
    if (!visible) {
        state->setLoginPanelGeometry(0, 0, 0, 0);
        state->setLoginPanelData(QString(), QString(), false, false);
        return;
    }

    state->setLoginPanelGeometry(loginWnd->m_x, loginWnd->m_y, loginWnd->m_w, loginWnd->m_h);
    const QString userId = ToQString(loginWnd->GetLoginText());
    const QString passwordMask(loginWnd->GetPasswordLength(), QLatin1Char('*'));
    state->setLoginPanelData(
        userId,
        passwordMask,
        loginWnd->IsSaveAccountChecked(),
        loginWnd->IsPasswordFocused());
}

bool IsMonsterLikeHoverActor(const CGameActor* actor)
{
    if (!actor) {
        return false;
    }

    const int job = actor->m_job;
    return job >= 1000 && (job < 6001 || job > 6047);
}

bool ShouldUseServerNameForHoverActor(const CGameActor* actor)
{
    if (!actor) {
        return false;
    }

    if (actor->m_isPc) {
        return true;
    }

    if (IsMonsterLikeHoverActor(actor)) {
        return true;
    }

    return actor->m_objectType == 6;
}

void SendActorNameRequest(CGameMode& mode, u32 gid)
{
    if (gid == 0 || gid == g_session.m_gid) {
        return;
    }

    const u32 now = GetTickCount();
    const auto timerIt = mode.m_actorNameByGIDReqTimer.find(gid);
    if (timerIt != mode.m_actorNameByGIDReqTimer.end() && now - timerIt->second < kHoverNameRequestCooldownMs) {
        return;
    }

    PACKET_CZ_REQNAME2 packet{};
    packet.PacketType = PacketProfile::ActiveMapServerSend::kGetCharNameRequest;
    packet.GID = gid;
    if (CRagConnection::instance()->SendPacket(reinterpret_cast<const char*>(&packet), static_cast<int>(sizeof(packet)))) {
        mode.m_actorNameByGIDReqTimer[gid] = now;
    }
}

std::string ResolveActorLabel(const CGameMode& mode, CGameActor* actor)
{
    if (!actor) {
        return std::string();
    }

    if (actor->m_gid == g_session.m_gid) {
        const char* playerName = g_session.GetPlayerName();
        if (playerName && *playerName) {
            return playerName;
        }
    }

    const auto cachedNameIt = mode.m_actorNameListByGID.find(actor->m_gid);
    if (cachedNameIt != mode.m_actorNameListByGID.end() && !cachedNameIt->second.name.empty()) {
        return cachedNameIt->second.name;
    }

    if (ShouldUseServerNameForHoverActor(actor)) {
        SendActorNameRequest(const_cast<CGameMode&>(mode), actor->m_gid);
        if (actor->m_isPc) {
            return "Player";
        }
        if (IsMonsterLikeHoverActor(actor)) {
            return "Monster";
        }
        return "NPC";
    }

    const char* jobName = g_session.GetJobName(actor->m_job);
    if (jobName && *jobName) {
        return jobName;
    }

    return "Entity";
}

std::string ResolveGroundItemLabel(const CItem* item)
{
    if (!item) {
        return std::string();
    }
    std::string itemName = item->m_itemName;
    if (itemName.empty() && item->m_itemId != 0) {
        itemName = g_ttemmgr.GetDisplayName(item->m_itemId, item->m_identified != 0);
    }
    if (itemName.empty()) {
        itemName = "Item";
    }

    char amountText[64]{};
    std::snprintf(amountText, sizeof(amountText), "%s: %u ea", itemName.c_str(), static_cast<unsigned int>(item->m_amount));
    return amountText;
}

QString ResolveHoverForeground(const CGameActor* actor)
{
    if (!actor) {
        return QStringLiteral("#ffffff");
    }

    u32 actorId = actor->m_gid;
    if (actorId != 0 && (actorId == g_session.m_gid || actorId == g_session.m_aid)) {
        actorId = g_session.m_aid != 0 ? g_session.m_aid : actorId;
    }
    return IsNameYellow(actorId) ? QStringLiteral("#ffff00") : QStringLiteral("#ffffff");
}

QVariantMap MakeAnchor(const QString& text,
    int screenX,
    int screenY,
    const QString& background,
    const QString& foreground = QStringLiteral("#ffffff"),
    bool showBubble = true,
    int fontPixelSize = 14,
    bool bold = true)
{
    QVariantMap anchor;
    anchor.insert(QStringLiteral("text"), text);
    anchor.insert(QStringLiteral("x"), screenX + 12);
    anchor.insert(QStringLiteral("y"), screenY - 26);
    anchor.insert(QStringLiteral("background"), background);
    anchor.insert(QStringLiteral("foreground"), foreground);
    anchor.insert(QStringLiteral("showBubble"), showBubble);
    anchor.insert(QStringLiteral("fontPixelSize"), fontPixelSize);
    anchor.insert(QStringLiteral("bold"), bold);
    return anchor;
}

} // namespace

QtUiStateAdapter::QtUiStateAdapter()
    : m_state(new QtUiState())
{
    m_state->setLastInput(QStringLiteral("No routed input yet."));
}

QtUiStateAdapter::~QtUiStateAdapter()
{
    delete m_state;
    m_state = nullptr;
}

QObject* QtUiStateAdapter::stateObject() const
{
    return m_state;
}

void QtUiStateAdapter::setLastInput(const QString& value)
{
    if (m_state) {
        m_state->setLastInput(value.isEmpty() ? QStringLiteral("No routed input yet.") : value);
    }
}

bool QtUiStateAdapter::syncMenu(RenderBackendType activeBackend,
    RenderBackendType nativeOverlayBackend)
{
    if (!m_state) {
        return false;
    }

    m_state->setBackendName(BackendToQString(activeBackend));
    m_state->setModeName(BuildMenuModeText());
    m_state->setRenderPath(BuildRenderPathText(nativeOverlayBackend));
    m_state->setArchitectureNote(BuildArchitectureNote(nativeOverlayBackend));
    m_state->setLoginStatus(ToQString(g_windowMgr.GetLoginStatus()));
    m_state->setChatPreview(BuildChatPreviewText());
    PopulateLoginPanelState(m_state);
    PopulateServerSelectState(m_state);
    PopulateLoadingState(m_state);
    m_state->setAnchors(QVariantList{});
    return true;
}

bool QtUiStateAdapter::syncGameplay(CGameMode& mode,
    RenderBackendType activeBackend,
    RenderBackendType nativeOverlayBackend,
    int mouseX,
    int mouseY)
{
    if (!m_state) {
        return false;
    }

    m_state->setBackendName(BackendToQString(activeBackend));
    m_state->setModeName(QStringLiteral("Gameplay"));
    m_state->setRenderPath(BuildRenderPathText(nativeOverlayBackend));
    m_state->setArchitectureNote(BuildArchitectureNote(nativeOverlayBackend));
    m_state->setLoginStatus(ToQString(g_windowMgr.GetLoginStatus()));
    m_state->setChatPreview(BuildChatPreviewText());
    PopulateLoginPanelState(m_state);
    PopulateServerSelectState(m_state);
    PopulateLoadingState(m_state);

    QVariantList anchors;
    if (mode.m_world && mode.m_view) {
        const matrix& viewMatrix = mode.m_view->GetViewMatrix();
        const float cameraLongitude = mode.m_view->GetCameraLongitude();

        int labelX = 0;
        int labelY = 0;
        if (mode.m_world->GetPlayerScreenLabel(viewMatrix, cameraLongitude, &labelX, &labelY)) {
            QString playerName = ToQString(g_session.GetPlayerName());
            if (playerName.isEmpty()) {
                playerName = QStringLiteral("Player");
            }
            anchors.push_back(MakeAnchor(playerName, labelX, labelY, QStringLiteral("#c0813cf2")));
        }

        CGameActor* hoveredActor = nullptr;
        if (mode.m_world->FindHoveredActorScreen(viewMatrix,
                cameraLongitude,
                mouseX,
                mouseY,
                &hoveredActor,
                &labelX,
                &labelY)) {
            if (!hoveredActor || hoveredActor->m_gid != mode.m_lastLockOnMonGid) {
                anchors.push_back(MakeAnchor(ToQString(ResolveActorLabel(mode, hoveredActor)),
                    labelX,
                    labelY,
                    QStringLiteral("#c0be185d"),
                    ResolveHoverForeground(hoveredActor)));
            }
        } else {
            CItem* hoveredItem = nullptr;
            if (mode.m_world->FindHoveredGroundItemScreen(viewMatrix,
                    mouseX,
                    mouseY,
                    &hoveredItem,
                    &labelX,
                    &labelY)) {
                anchors.push_back(MakeAnchor(ToQString(ResolveGroundItemLabel(hoveredItem)),
                    labelX,
                    labelY,
                    QStringLiteral("#c0087f5b")));
            }
        }

        if (mode.m_lastLockOnMonGid != 0) {
            const auto actorIt = mode.m_runtimeActors.find(mode.m_lastLockOnMonGid);
            if (actorIt != mode.m_runtimeActors.end() && actorIt->second && actorIt->second->m_isVisible) {
                if (mode.m_world->GetActorScreenMarker(viewMatrix,
                        cameraLongitude,
                        mode.m_lastLockOnMonGid,
                        &labelX,
                        nullptr,
                        &labelY)) {
                    const QString lockLabel = ToQString(ResolveActorLabel(mode, actorIt->second));
                    if (!lockLabel.isEmpty()) {
                        anchors.push_back(MakeAnchor(QStringLiteral("▼"),
                            labelX - 6,
                            labelY - 24,
                            QStringLiteral("transparent"),
                            ToCssColor(255, 226, 120),
                            false,
                            20,
                            true));
                        anchors.push_back(MakeAnchor(lockLabel,
                            labelX,
                            labelY,
                            QStringLiteral("#c05a1620"),
                            ResolveHoverForeground(actorIt->second)));
                    }
                }
            }
        }
    }

    m_state->setAnchors(anchors);
    return true;
}
