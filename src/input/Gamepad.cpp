#include "Gamepad.h"

#include "HotkeyBindings.h"
#include "VirtualCursor.h"

#include "core/SettingsIni.h"
#include "DebugLog.h"

#if RO_HAS_GAMEPAD
#  include <SDL3/SDL.h>
#endif

#if RO_PLATFORM_WINDOWS
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#else
#  include "platform/WindowsCompat.h"
#endif

#include "gamemode/CursorRenderer.h"
#include "gamemode/GameMode.h"
#include "gamemode/Mode.h"
#include "session/Session.h"
#include "qtui/QtUiRuntime.h"
#include "ui/UIVirtualKeyboardWnd.h"
#include "ui/UiScale.h"
#include "ui/UIWindowMgr.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

extern UIWindowMgr g_windowMgr;
extern CModeMgr g_modeMgr;

namespace gamepad {

CGamepad g_gamepad;

namespace {

constexpr const char* kSettingsSection = "Gamepad";

constexpr int kDefaultStickDeadzone   = 8000;   // SDL axis range is +/- 32767
constexpr int kDefaultTriggerDeadzone = 8000;
constexpr int kDefaultCursorSpeed     = 1400;   // px/sec at full deflection
constexpr uint64_t kZoomTickThresholdMs = 110;  // throttle wheel synthesis

float ApplyDeadzone(int rawAxis, int deadzone)
{
    const int abs = rawAxis < 0 ? -rawAxis : rawAxis;
    if (abs <= deadzone) {
        return 0.0f;
    }
    const int sign = rawAxis < 0 ? -1 : 1;
    const float scaled = static_cast<float>(abs - deadzone) / static_cast<float>(32767 - deadzone);
    return sign * std::min(scaled, 1.0f);
}

#if RO_HAS_GAMEPAD
int GetFallbackAxis(SDL_Gamepad* controller, SDL_GamepadAxis axis, int rawAxisIndex)
{
    if (!controller) {
        return 0;
    }

    const int mappedValue = SDL_GetGamepadAxis(controller, axis);
    if (mappedValue != 0) {
        return mappedValue;
    }

    SDL_Joystick* joystick = SDL_GetGamepadJoystick(controller);
    if (!joystick || rawAxisIndex < 0 || rawAxisIndex >= SDL_GetNumJoystickAxes(joystick)) {
        return mappedValue;
    }

    return SDL_GetJoystickAxis(joystick, rawAxisIndex);
}

bool GetFallbackButton(SDL_Gamepad* controller, SDL_GamepadButton button, int rawButtonIndex, int rawHatMask = 0)
{
    if (!controller) {
        return false;
    }

    if (SDL_GetGamepadButton(controller, button)) {
        return true;
    }

    SDL_Joystick* joystick = SDL_GetGamepadJoystick(controller);
    if (!joystick) {
        return false;
    }

    if (rawButtonIndex >= 0 && rawButtonIndex < SDL_GetNumJoystickButtons(joystick)
        && SDL_GetJoystickButton(joystick, rawButtonIndex)) {
        return true;
    }

    if (rawHatMask != 0 && SDL_GetNumJoystickHats(joystick) > 0
        && (SDL_GetJoystickHat(joystick, 0) & rawHatMask) != 0) {
        return true;
    }

    return false;
}
#endif  // RO_HAS_GAMEPAD

}  // namespace

#if RO_HAS_GAMEPAD

struct CGamepad::Impl {
    SDL_Gamepad*   controller = nullptr;
    SDL_JoystickID instanceId = 0;

    int   stickDeadzone   = kDefaultStickDeadzone;
    int   triggerDeadzone = kDefaultTriggerDeadzone;
    float cursorSpeed     = static_cast<float>(kDefaultCursorSpeed);

    VirtualCursor cursor;

    // Edge tracking (we synthesize on the down-edge for most actions).
    bool buttonState[SDL_GAMEPAD_BUTTON_COUNT] = {};
    bool buttonPrev[SDL_GAMEPAD_BUTTON_COUNT]  = {};
    bool triggerLActive = false;
    bool triggerRActive = false;
    uint64_t lastZoomTick = 0;
    bool     loggedInputActivity = false;
    bool     leftStickClickHeld = false;
    bool     pendingLeftClickDown = false;
    bool     rightDragActive = false;
    int      rightDragX = 0;
    int      rightDragY = 0;

    // Long-press Start tracking for Controller-window toggle.
    uint64_t startDownTick = 0;
    bool     startSuppressShort = false;

    // NPC dialog confirm needs to ignore the release that opened the dialog.
    bool     npcDialogConfirmArmed = false;
    bool     npcDialogWasActive = false;

    // Character-movement throttle + preference.
    MoveMode moveMode = MoveMode::Cursor;
    uint64_t lastMoveRequestTick = 0;

    // Rebind mode.
    bool     rebindActive = false;
    hotkeys::GamepadAction rebindTarget = hotkeys::GamepadAction::Invalid;

    // Live state snapshot (read by the diagnostic window).
    LiveState live;

    void OpenFirstAvailable();
    void CloseController();
};

void CGamepad::Impl::OpenFirstAvailable()
{
    if (controller) return;
    int count = 0;
    SDL_JoystickID* ids = SDL_GetJoysticks(&count);
    if (!ids) return;

    for (int i = 0; i < count; ++i) {
        if (!SDL_IsGamepad(ids[i])) continue;
        controller = SDL_OpenGamepad(ids[i]);
        if (controller) {
            SDL_Joystick* joy = SDL_GetGamepadJoystick(controller);
            instanceId = ids[i];
            DbgLog("[Gamepad] Opened '%s' (instance %u, axes=%d buttons=%d hats=%d).\n",
                SDL_GetGamepadName(controller),
                static_cast<unsigned>(instanceId),
                joy ? SDL_GetNumJoystickAxes(joy) : 0,
                joy ? SDL_GetNumJoystickButtons(joy) : 0,
                joy ? SDL_GetNumJoystickHats(joy) : 0);
            // Brief connect rumble to prove the haptics path end-to-end.
            SDL_RumbleGamepad(controller, 0x3000, 0x6000, 200);
            break;
        }
    }
    SDL_free(ids);
}

void CGamepad::Impl::CloseController()
{
    if (controller) {
        SDL_CloseGamepad(controller);
        controller = nullptr;
        instanceId = 0;
    }
    std::memset(buttonState, 0, sizeof(buttonState));
    std::memset(buttonPrev, 0, sizeof(buttonPrev));
    triggerLActive = false;
    triggerRActive = false;
    loggedInputActivity = false;
    leftStickClickHeld = false;
    npcDialogConfirmArmed = false;
    npcDialogWasActive = false;
}

#else  // RO_HAS_GAMEPAD

struct CGamepad::Impl {
    VirtualCursor cursor;  // unused but keeps the impl object well-formed
};

#endif

CGamepad::CGamepad() : m_impl(nullptr) {}
CGamepad::~CGamepad() { Shutdown(); }

bool CGamepad::Init()
{
#if RO_HAS_GAMEPAD
    if (m_impl) return true;

    int enabled = LoadSettingsIniInt(kSettingsSection, "Enabled", 1);
    if (!enabled) {
        DbgLog("[Gamepad] Disabled via settings.ini.\n");
        m_enabled = false;
        return true;
    }

    // HIDAPI PS5 hints must be set before SDL initializes its joystick backend
    // so the DualSense driver is selected for both USB and Bluetooth.
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    const Uint32 gamepadSubsystems = SDL_INIT_GAMEPAD | SDL_INIT_JOYSTICK | SDL_INIT_EVENTS;
    if ((SDL_WasInit(gamepadSubsystems) & gamepadSubsystems) != gamepadSubsystems) {
        if (!SDL_InitSubSystem(gamepadSubsystems)) {
            DbgLog("[Gamepad] SDL_InitSubSystem failed: %s\n", SDL_GetError());
            return false;
        }
    }

    m_impl = new Impl();
    m_impl->stickDeadzone   = LoadSettingsIniInt(kSettingsSection, "StickDeadzone",   kDefaultStickDeadzone);
    m_impl->triggerDeadzone = LoadSettingsIniInt(kSettingsSection, "TriggerDeadzone", kDefaultTriggerDeadzone);
    m_impl->cursorSpeed     = static_cast<float>(LoadSettingsIniInt(kSettingsSection, "CursorSpeed", kDefaultCursorSpeed));
    m_impl->cursor.SetSpeedPxPerSec(m_impl->cursorSpeed);

    hotkeys::bindings::Load();

    {
        std::string moveModeStr = LoadSettingsIniString(kSettingsSection, "MoveMode", "Cursor");
        m_impl->moveMode = (moveModeStr == "Character") ? MoveMode::Character : MoveMode::Cursor;
    }

    m_impl->OpenFirstAvailable();
    m_enabled = true;
    return true;
#else
    DbgLog("[Gamepad] Built without RO_HAS_GAMEPAD; controller input disabled.\n");
    m_enabled = false;
    return true;
#endif
}

void CGamepad::Shutdown()
{
#if RO_HAS_GAMEPAD
    if (m_impl) {
        m_impl->CloseController();
        delete m_impl;
        m_impl = nullptr;
    }
    if (SDL_WasInit(SDL_INIT_GAMEPAD) != 0) {
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    }
#endif
    m_enabled = false;
}

bool CGamepad::IsConnected() const
{
#if RO_HAS_GAMEPAD
    return m_impl && m_impl->controller != nullptr;
#else
    return false;
#endif
}

void CGamepad::Rumble(float lowFreq01, float highFreq01, uint32_t durationMs)
{
#if RO_HAS_GAMEPAD
    if (!m_impl || !m_impl->controller) return;
    const float lo = std::clamp(lowFreq01, 0.0f, 1.0f);
    const float hi = std::clamp(highFreq01, 0.0f, 1.0f);
    SDL_RumbleGamepad(m_impl->controller,
        static_cast<Uint16>(lo * 0xFFFF),
        static_cast<Uint16>(hi * 0xFFFF),
        durationMs);
#else
    (void)lowFreq01; (void)highFreq01; (void)durationMs;
#endif
}

void CGamepad::SetTriggerResistance(float leftForce01, float rightForce01)
{
#if RO_HAS_GAMEPAD
    if (!m_impl || !m_impl->controller) return;
    if (SDL_GetGamepadType(m_impl->controller) != SDL_GAMEPAD_TYPE_PS5) return;

    const float lf = std::clamp(leftForce01, 0.0f, 1.0f);
    const float rf = std::clamp(rightForce01, 0.0f, 1.0f);

    // DualSense "effect" report fragment: bytes [0]=pad type,
    // [1]=enable mask (0x04 = right trigger, 0x08 = left trigger),
    // [2..12]=right trigger params, [13..23]=left trigger params.
    // We build a minimal section-resistance effect per trigger.
    uint8_t effect[11 + 11 + 2] = {};
    effect[0] = 0x04 | 0x08;                 // enable both triggers
    // Right trigger (offset 1): mode 0x01 = section resistance
    effect[1]  = 0x01;
    effect[2]  = 0;                          // start position
    effect[3]  = static_cast<uint8_t>(rf * 0xFF);
    // Left trigger (offset 12)
    effect[12] = 0x01;
    effect[13] = 0;
    effect[14] = static_cast<uint8_t>(lf * 0xFF);

    SDL_SendGamepadEffect(m_impl->controller, effect, sizeof(effect));
#else
    (void)leftForce01; (void)rightForce01;
#endif
}

MoveMode CGamepad::GetMoveMode() const
{
#if RO_HAS_GAMEPAD
    return m_impl ? m_impl->moveMode : MoveMode::Cursor;
#else
    return MoveMode::Cursor;
#endif
}

void CGamepad::SetMoveMode(MoveMode mode)
{
#if RO_HAS_GAMEPAD
    if (!m_impl) return;
    m_impl->moveMode = mode;
    SaveSettingsIniString(kSettingsSection, "MoveMode",
        mode == MoveMode::Character ? "Character" : "Cursor");
#else
    (void)mode;
#endif
}

LiveState CGamepad::GetLiveState() const
{
#if RO_HAS_GAMEPAD
    if (!m_impl) return LiveState{};
    return m_impl->live;
#else
    return LiveState{};
#endif
}

bool CGamepad::BeginRebind(hotkeys::GamepadAction action)
{
#if RO_HAS_GAMEPAD
    if (!m_impl) return false;
    if (action == hotkeys::GamepadAction::Invalid || action == hotkeys::GamepadAction::_Count) return false;
    m_impl->rebindActive = true;
    m_impl->rebindTarget = action;
    return true;
#else
    (void)action;
    return false;
#endif
}

void CGamepad::CancelRebind()
{
#if RO_HAS_GAMEPAD
    if (!m_impl) return;
    m_impl->rebindActive = false;
        m_impl->rebindTarget = hotkeys::GamepadAction::Invalid;
#endif
}

bool CGamepad::IsRebinding() const
{
#if RO_HAS_GAMEPAD
    return m_impl && m_impl->rebindActive;
#else
    return false;
#endif
}

hotkeys::GamepadAction CGamepad::RebindAction() const
{
#if RO_HAS_GAMEPAD
    return m_impl ? m_impl->rebindTarget : hotkeys::GamepadAction::Invalid;
#else
    return hotkeys::GamepadAction::Invalid;
#endif
}

void CGamepad::SyncVirtualCursorClientPos(int x, int y)
{
#if RO_HAS_GAMEPAD
    if (m_impl && m_impl->controller) {
        m_impl->cursor.SetPosition(x, y);
    }
#else
    (void)x;
    (void)y;
#endif
}

void CGamepad::SetLEDColor(uint8_t r, uint8_t g, uint8_t b)
{
#if RO_HAS_GAMEPAD
    if (!m_impl || !m_impl->controller) return;
    SDL_SetGamepadLED(m_impl->controller, r, g, b);
#else
    (void)r; (void)g; (void)b;
#endif
}

#if RO_HAS_GAMEPAD

namespace {

// Mirrors the WM_MOUSEMOVE handler in WinMain: update the engine's cached
// cursor pos so the in-engine cursor renderer draws at the new spot, then let
// the UI hit-test, then forward to the active mode if the UI didn't claim it.
void DispatchCursorMove(int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    const bool uiCaptured = (g_windowMgr.m_captureWindow != nullptr);
    g_windowMgr.OnMouseMove(uiX, uiY);
    if (!uiCaptured && !g_windowMgr.HasWindowAtPoint(uiX, uiY)) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_MouseMove,
            static_cast<msgparam_t>(x), static_cast<msgparam_t>(y));
    }
}

void DispatchLeftClickDown(int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    const bool uiHit = g_windowMgr.HasWindowAtPoint(uiX, uiY);
    g_windowMgr.OnLBtnDown(uiX, uiY);
    if (!uiHit) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_LButtonDown,
            static_cast<msgparam_t>(x), static_cast<msgparam_t>(y));
    }
}

void DispatchLeftClickUp(int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    const bool uiCaptured = (g_windowMgr.m_captureWindow != nullptr);
    const bool uiHit = g_windowMgr.HasWindowAtPoint(uiX, uiY);
    g_windowMgr.OnLBtnUp(uiX, uiY);
    if (!uiCaptured && !uiHit) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_LButtonUp,
            static_cast<msgparam_t>(x), static_cast<msgparam_t>(y));
    }
}

void DispatchRightClickDown(int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    if (!g_windowMgr.HasWindowAtPoint(uiX, uiY)) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_RButtonDown,
            static_cast<msgparam_t>(x), static_cast<msgparam_t>(y));
    }
}

void DispatchRightClickUp(int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    if (!g_windowMgr.HasWindowAtPoint(uiX, uiY)) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_RButtonUp,
            static_cast<msgparam_t>(x), static_cast<msgparam_t>(y));
    }
}

void DispatchHotbarPage(int delta)
{
    const int currentPage = g_session.GetShortcutPage();
    g_session.SetShortcutPage(currentPage + delta);
}

void DispatchKey(int virtualKey)
{
    g_windowMgr.OnKeyDown(virtualKey);
}

void DispatchToggleWindow(int windowId)
{
    g_windowMgr.ToggleWindow(windowId);
}

void DispatchWheel(int delta, int x, int y)
{
    SetModeCursorClientPos(x, y);
    const int uiX = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(x) : x;
    const int uiY = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(y) : y;
    if (!g_windowMgr.OnWheel(uiX, uiY, delta)) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_MouseWheel,
            static_cast<msgparam_t>(delta), 0);
    }
}

}  // namespace

#endif  // RO_HAS_GAMEPAD

void CGamepad::Poll(float dtSeconds, int clientWidth, int clientHeight)
{
#if RO_HAS_GAMEPAD
    if (!m_enabled || !m_impl) return;

    SDL_PumpEvents();
    SDL_UpdateGamepads();

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_EVENT_GAMEPAD_ADDED:
            if (!m_impl->controller) {
                m_impl->OpenFirstAvailable();
                m_impl->cursor.Reset(clientWidth, clientHeight);
            }
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            if (m_impl->controller && ev.gdevice.which == m_impl->instanceId) {
                DbgLog("[Gamepad] Controller disconnected.\n");
                m_impl->CloseController();
            }
            break;
        default:
            break;
        }
    }

    if (!m_impl->controller) {
        g_windowMgr.HideVirtualKeyboard();
        m_impl->live = LiveState{};
        return;
    }

    m_impl->cursor.SetClientSize(clientWidth, clientHeight);
    if (m_impl->cursor.X() == 0 && m_impl->cursor.Y() == 0) {
        m_impl->cursor.Reset(clientWidth, clientHeight);
    }

    SDL_Gamepad* gc = m_impl->controller;

    // Edge-tracking lambdas are needed by both the OSK branch and the normal
    // branch, so they're defined up front.
    auto pressed = [this, gc](SDL_GamepadButton b, int rawButton = -1, int rawHatMask = 0) {
        const bool now = GetFallbackButton(gc, b, rawButton, rawHatMask);
        if (now && !m_impl->loggedInputActivity) {
            DbgLog("[Gamepad] Received button input from '%s'.\n", SDL_GetGamepadName(gc));
            m_impl->loggedInputActivity = true;
        }
        m_impl->buttonState[b] = now;
        const bool wasDown = m_impl->buttonPrev[b];
        return now && !wasDown;
    };
    auto released = [this, gc](SDL_GamepadButton b, int rawButton = -1, int rawHatMask = 0) {
        const bool now = GetFallbackButton(gc, b, rawButton, rawHatMask);
        const bool wasDown = m_impl->buttonPrev[b];
        return !now && wasDown;
    };

    // ── OSK mode ─────────────────────────────────────────────────────────
    // When a legacy text field is focused, route all gamepad input into the
    // on-screen keyboard and skip the cursor / drag / zoom synthesis below.
    if (g_windowMgr.IsTextInputActive()) {
        g_windowMgr.ShowVirtualKeyboard(clientWidth, clientHeight);
        auto* kb = g_windowMgr.m_virtualKeyboardWnd;

        if (kb) {
            if (pressed(SDL_GAMEPAD_BUTTON_DPAD_LEFT,  -1, SDL_HAT_LEFT))  kb->MoveHighlight(-1, 0);
            if (pressed(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, -1, SDL_HAT_RIGHT)) kb->MoveHighlight(+1, 0);
            if (pressed(SDL_GAMEPAD_BUTTON_DPAD_UP,    -1, SDL_HAT_UP))    kb->MoveHighlight(0, -1);
            if (pressed(SDL_GAMEPAD_BUTTON_DPAD_DOWN,  -1, SDL_HAT_DOWN))  kb->MoveHighlight(0, +1);

            if (pressed(SDL_GAMEPAD_BUTTON_SOUTH, 1)) kb->ActivateHighlight();
            if (pressed(SDL_GAMEPAD_BUTTON_EAST,  2)) kb->SendBackspace();
            if (pressed(SDL_GAMEPAD_BUTTON_WEST,  0)) kb->SendSpace();
            if (pressed(SDL_GAMEPAD_BUTTON_NORTH, 3)) kb->ToggleShift();

            if (pressed(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  4)) kb->CyclePage(-1);
            if (pressed(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, 5)) kb->CyclePage(+1);
            if (pressed(SDL_GAMEPAD_BUTTON_START, 9)) kb->Submit();
            if (pressed(SDL_GAMEPAD_BUTTON_BACK,  8)) kb->Cancel();

            // Triggers nudge the caret of the focused edit field.
            const int tlOsk = GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  4);
            const int trOsk = GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 5);
            const uint64_t nowTickOsk = SDL_GetTicks();
            if (tlOsk > m_impl->triggerDeadzone
                && nowTickOsk - m_impl->lastZoomTick > kZoomTickThresholdMs) {
                g_windowMgr.OnKeyDown(VK_LEFT);
                m_impl->lastZoomTick = nowTickOsk;
            }
            if (trOsk > m_impl->triggerDeadzone
                && nowTickOsk - m_impl->lastZoomTick > kZoomTickThresholdMs) {
                g_windowMgr.OnKeyDown(VK_RIGHT);
                m_impl->lastZoomTick = nowTickOsk;
            }
        }

        std::memcpy(m_impl->buttonPrev, m_impl->buttonState, sizeof(m_impl->buttonPrev));
        return;
    }

    g_windowMgr.HideVirtualKeyboard();

    // ── Sticks ───────────────────────────────────────────────────────────
    const float lx = ApplyDeadzone(GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_LEFTX, 0),  m_impl->stickDeadzone);
    const float ly = ApplyDeadzone(GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_LEFTY, 1),  m_impl->stickDeadzone);
    const float rx = ApplyDeadzone(GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_RIGHTX, 2), m_impl->stickDeadzone);
    const float ry = ApplyDeadzone(GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_RIGHTY, 3), m_impl->stickDeadzone);

    if (!m_impl->loggedInputActivity
        && (lx != 0.0f
            || ly != 0.0f
            || rx != 0.0f
            || ry != 0.0f
            || GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 4) > m_impl->triggerDeadzone
            || GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 5) > m_impl->triggerDeadzone)) {
        DbgLog("[Gamepad] Received analog input from '%s'.\n", SDL_GetGamepadName(gc));
        m_impl->loggedInputActivity = true;
    }

    // In Character mode the left stick walks the player, not the cursor.
    // Right stick then takes over cursor nudging (see below) so the user can
    // still reach UI elements without flipping modes.
    const bool characterMove = (m_impl->moveMode == MoveMode::Character);
    const float cursorLx = characterMove ? rx : lx;
    const float cursorLy = characterMove ? ry : ly;

    const bool cursorMoved = m_impl->cursor.Tick(cursorLx, cursorLy, dtSeconds);
    const int cx = m_impl->cursor.X();
    const int cy = m_impl->cursor.Y();
    const int uiCx = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(cx) : cx;
    const int uiCy = IsQtUiRuntimeEnabled() ? UiScaleRawToLogicalCoordinate(cy) : cy;
    const bool overUi = g_windowMgr.HasWindowAtPoint(uiCx, uiCy)
                     || g_windowMgr.m_modalWindow
                     || g_windowMgr.m_editWindow
                     || g_windowMgr.m_chatActiveInputField != 0;

    if (cursorMoved) {
        DispatchCursorMove(cx, cy);
    }

    // Right stick: scroll lists when over UI; in Cursor mode it otherwise
    // orbits the camera via synthesized right-button drag. In Character mode
    // the right stick has already been consumed by the virtual cursor above,
    // so skip the camera drag to avoid double-duty.
    if (!characterMove && (rx != 0.0f || ry != 0.0f)) {
        if (overUi) {
            const int wheelDelta = static_cast<int>(-ry * 120.0f * dtSeconds * 6.0f);
            if (wheelDelta != 0) {
                DispatchWheel(wheelDelta, cx, cy);
            }
        } else {
            const int dx = static_cast<int>(rx * 600.0f * dtSeconds);
            const int dy = static_cast<int>(ry * 600.0f * dtSeconds);
            if (dx != 0 || dy != 0) {
                if (!m_impl->rightDragActive) {
                    DispatchRightClickDown(cx, cy);
                    m_impl->rightDragX = cx;
                    m_impl->rightDragY = cy;
                    m_impl->rightDragActive = true;
                }
                m_impl->rightDragX += dx;
                m_impl->rightDragY += dy;
                g_modeMgr.SendMsg(CGameMode::GameMsg_MouseMove,
                    static_cast<msgparam_t>(m_impl->rightDragX),
                    static_cast<msgparam_t>(m_impl->rightDragY));
            }
        }
    } else if (m_impl->rightDragActive) {
        DispatchRightClickUp(m_impl->rightDragX, m_impl->rightDragY);
        m_impl->rightDragActive = false;
    }

    // Character-mode movement: left stick + D-pad walk the player in the
    // pressed direction. Throttled so we don't spam the server with move
    // requests. UI-focused state defers to cursor behavior so the user can
    // still aim at buttons.
    if (characterMove && !overUi) {
        const int dpadX = (m_impl->buttonState[SDL_GAMEPAD_BUTTON_DPAD_RIGHT] ? 1 : 0)
                        - (m_impl->buttonState[SDL_GAMEPAD_BUTTON_DPAD_LEFT]  ? 1 : 0);
        // RO world Y: +1 is north, -1 is south. D-pad up / stick up → +1.
        const int dpadY = (m_impl->buttonState[SDL_GAMEPAD_BUTTON_DPAD_UP]    ? 1 : 0)
                        - (m_impl->buttonState[SDL_GAMEPAD_BUTTON_DPAD_DOWN]  ? 1 : 0);
        const int stickX = (lx >  0.5f) ? 1 : (lx < -0.5f ? -1 : 0);
        const int stickY = (ly < -0.5f) ? 1 : (ly >  0.5f ? -1 : 0);
        const int moveX = (dpadX != 0) ? dpadX : stickX;
        const int moveY = (dpadY != 0) ? dpadY : stickY;

        if ((moveX != 0 || moveY != 0)
            && SDL_GetTicks() - m_impl->lastMoveRequestTick > 200) {
            g_modeMgr.SendMsg(CGameMode::GameMsg_RequestMoveDelta,
                static_cast<msgparam_t>(moveX),
                static_cast<msgparam_t>(moveY));
            m_impl->lastMoveRequestTick = SDL_GetTicks();
        }
    }

    // ── Triggers (zoom) ──────────────────────────────────────────────────
    const int tl = GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_LEFT_TRIGGER, 4);
    const int tr = GetFallbackAxis(gc, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 5);
    const uint64_t nowTick = SDL_GetTicks();
    if (tl > m_impl->triggerDeadzone) {
        if (nowTick - m_impl->lastZoomTick > kZoomTickThresholdMs) {
            DispatchWheel(-120, cx, cy);   // zoom out
            m_impl->lastZoomTick = nowTick;
        }
    }
    if (tr > m_impl->triggerDeadzone) {
        if (nowTick - m_impl->lastZoomTick > kZoomTickThresholdMs) {
            DispatchWheel(+120, cx, cy);   // zoom in
            m_impl->lastZoomTick = nowTick;
        }
    }

    // ── Buttons ──────────────────────────────────────────────────────────
    // Sample every button up front so edge detection is consistent whether
    // or not the button is bound to an action.
    for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i) {
        const SDL_GamepadButton b = static_cast<SDL_GamepadButton>(i);
        m_impl->buttonState[i] = GetFallbackButton(gc, b, -1, 0);
    }
    auto wasPressed = [this](SDL_GamepadButton b) {
        return m_impl->buttonState[b] && !m_impl->buttonPrev[b];
    };
    auto wasReleased = [this](SDL_GamepadButton b) {
        return !m_impl->buttonState[b] && m_impl->buttonPrev[b];
    };

    // Rebind mode: the first fresh button press becomes the new binding,
    // then rebind mode ends. Skip all other dispatch this frame.
    if (m_impl->rebindActive) {
        for (int i = 0; i < SDL_GAMEPAD_BUTTON_COUNT; ++i) {
            const SDL_GamepadButton b = static_cast<SDL_GamepadButton>(i);
            if (wasPressed(b)) {
                hotkeys::bindings::SetGamepadBinding(m_impl->rebindTarget, b);
                hotkeys::bindings::Save();
                m_impl->rebindActive = false;
                m_impl->rebindTarget = hotkeys::GamepadAction::Invalid;
                break;
            }
        }
        std::memcpy(m_impl->buttonPrev, m_impl->buttonState, sizeof(m_impl->buttonPrev));
        // Still populate live state so the diagnostic UI keeps updating.
        m_impl->live.connected = true;
        m_impl->live.name = SDL_GetGamepadName(gc);
        m_impl->live.type = SDL_GetGamepadType(gc);
        m_impl->live.axes[0] = lx; m_impl->live.axes[1] = ly;
        m_impl->live.axes[2] = rx; m_impl->live.axes[3] = ry;
        m_impl->live.axes[4] = std::max(0, tl) / 32767.0f;
        m_impl->live.axes[5] = std::max(0, tr) / 32767.0f;
        std::memcpy(m_impl->live.buttons, m_impl->buttonState, sizeof(m_impl->live.buttons));
        return;
    }

    // Helper: is the button bound to `action` pressed on this frame's edge?
    auto act = [&](hotkeys::GamepadAction a) {
        const SDL_GamepadButton b = hotkeys::bindings::GetGamepadButton(a);
        return (b != SDL_GAMEPAD_BUTTON_INVALID) && wasPressed(b);
    };

    const bool npcDialogActive = g_windowMgr.HasActiveNpcDialog();
    const bool southReleasedThisFrame =
        !m_impl->buttonState[SDL_GAMEPAD_BUTTON_SOUTH] && m_impl->buttonPrev[SDL_GAMEPAD_BUTTON_SOUTH];
    if (npcDialogActive && !m_impl->npcDialogWasActive) {
        m_impl->npcDialogConfirmArmed = !southReleasedThisFrame;
    } else if (!npcDialogActive) {
        m_impl->npcDialogConfirmArmed = false;
    }
    m_impl->npcDialogWasActive = npcDialogActive;

    // South (A / Cross) normally drives the virtual cursor's left click.
    // Active NPC dialogs already expose Enter/Return semantics, so route
    // controller confirm through that path, but only after a fresh release
    // so the press that opened the dialog does not instantly advance it.
    if (npcDialogActive) {
        if (wasReleased(SDL_GAMEPAD_BUTTON_SOUTH)) {
            if (m_impl->npcDialogConfirmArmed) {
                DispatchKey(VK_RETURN);
            } else {
                m_impl->npcDialogConfirmArmed = true;
            }
        }
    } else {
        if (wasPressed(SDL_GAMEPAD_BUTTON_SOUTH)) {
            DispatchLeftClickDown(cx, cy);
        }
        if (wasReleased(SDL_GAMEPAD_BUTTON_SOUTH)) {
            DispatchLeftClickUp(cx, cy);
            // Phase B: confirm focused button when not in text-input mode.
            if (overUi && !g_windowMgr.IsTextInputActive()) {
                g_windowMgr.OnKeyDown(VK_RETURN);
            }
        }
    }

    // Cancel — Escape over UI, SitStand in the world.
    if (act(hotkeys::GamepadAction::Cancel)) {
        if (overUi) {
            DispatchKey(VK_ESCAPE);
        } else {
            g_modeMgr.SendMsg(CGameMode::GameMsg_ToggleSitStand, 0, 0);
        }
    }

    if (act(hotkeys::GamepadAction::EquipToggle))     DispatchToggleWindow(UIWindowMgr::WID_EQUIPWND);
    if (act(hotkeys::GamepadAction::InventoryToggle)) DispatchToggleWindow(UIWindowMgr::WID_ITEMWND);
    if (act(hotkeys::GamepadAction::ChatToggle))      DispatchKey(VK_RETURN);

    // Menu: short-press toggles Options; long-press (≥500 ms) toggles the
    // Controller window. Track press/release on whichever button is bound
    // to ToggleOptionsWindow so rebinding still works.
    {
        const SDL_GamepadButton menuBtn = hotkeys::bindings::GetGamepadButton(hotkeys::GamepadAction::ToggleOptionsWindow);
        if (menuBtn != SDL_GAMEPAD_BUTTON_INVALID) {
            if (wasPressed(menuBtn)) {
                m_impl->startDownTick = SDL_GetTicks();
                m_impl->startSuppressShort = false;
            } else if (m_impl->buttonState[menuBtn] && !m_impl->startSuppressShort) {
                const uint64_t heldMs = SDL_GetTicks() - m_impl->startDownTick;
                if (heldMs >= 500) {
                    g_windowMgr.ToggleWindow(UIWindowMgr::WID_CONTROLLERWND);
                    m_impl->startSuppressShort = true;
                }
            } else if (wasReleased(menuBtn)) {
                if (!m_impl->startSuppressShort) {
                    DispatchToggleWindow(UIWindowMgr::WID_OPTIONWND);
                }
                m_impl->startSuppressShort = false;
            }
        }
    }

    // Shoulders cycle the visible hotbar page.
    if (act(hotkeys::GamepadAction::HotbarPagePrev)) DispatchHotbarPage(-1);
    if (act(hotkeys::GamepadAction::HotbarPageNext)) DispatchHotbarPage(+1);

    // D-Pad has dual behavior: menu nav over UI, window toggles in world.
    if (act(hotkeys::GamepadAction::SkillToggle)) {
        if (overUi) DispatchKey(VK_UP);
        else if (!characterMove) DispatchToggleWindow(UIWindowMgr::WID_SKILLLISTWND);
    }
    if (act(hotkeys::GamepadAction::StatusToggle)) {
        if (overUi) DispatchKey(VK_DOWN);
        else if (!characterMove) DispatchToggleWindow(UIWindowMgr::WID_STATUSWND);
    }
    if (act(hotkeys::GamepadAction::MapToggle)) {
        if (overUi) DispatchKey(VK_LEFT);
        else if (!characterMove) DispatchToggleWindow(UIWindowMgr::WID_ROMAPWND);
    }
    if (act(hotkeys::GamepadAction::QuestToggle)) {
        if (overUi) DispatchKey(VK_RIGHT);
        else if (!characterMove) g_modeMgr.SendMsg(CGameMode::GameMsg_RequestShortcutUse, 8, 0, 0);
    }

    if (!npcDialogActive && act(hotkeys::GamepadAction::Confirm)) {
        if (overUi) {
            DispatchKey(VK_RETURN);
        }
    }

    if (act(hotkeys::GamepadAction::ResetCamera) && !overUi) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_ResetCamera, 0, 0);
    }
    if (act(hotkeys::GamepadAction::SitStand) && !overUi) {
        g_modeMgr.SendMsg(CGameMode::GameMsg_ToggleSitStand, 0, 0);
    }

    // Populate the live-state snapshot for the diagnostic UI.
    m_impl->live.connected = true;
    m_impl->live.name = SDL_GetGamepadName(gc);
    m_impl->live.type = SDL_GetGamepadType(gc);
    m_impl->live.axes[0] = lx; m_impl->live.axes[1] = ly;
    m_impl->live.axes[2] = rx; m_impl->live.axes[3] = ry;
    m_impl->live.axes[4] = std::max(0, tl) / 32767.0f;
    m_impl->live.axes[5] = std::max(0, tr) / 32767.0f;
    std::memcpy(m_impl->live.buttons, m_impl->buttonState, sizeof(m_impl->live.buttons));

    std::memcpy(m_impl->buttonPrev, m_impl->buttonState, sizeof(m_impl->buttonPrev));
#else
    (void)dtSeconds;
    (void)clientWidth;
    (void)clientHeight;
#endif
}

}  // namespace gamepad
