#pragma once

#include "HotkeyBindings.h"

#include <cstdint>

#if RO_HAS_GAMEPAD
#  include <SDL3/SDL.h>
#endif

namespace gamepad {

// Whether left stick / D-pad nudge the virtual cursor (Cursor) or walk the
// player character (Character). Persisted in settings.ini [Gamepad]MoveMode.
enum class MoveMode {
    Cursor = 0,
    Character = 1,
};

// Read-only snapshot of the controller state, suitable for rendering the
// diagnostic UI. Populated by CGamepad::Poll each frame.
struct LiveState {
    bool        connected = false;
    const char* name      = "(no controller)";
#if RO_HAS_GAMEPAD
    SDL_GamepadType type  = SDL_GAMEPAD_TYPE_UNKNOWN;
    float axes[6]     = {};                                 // LX, LY, RX, RY, LT, RT  (sticks [-1,1], triggers [0,1])
    bool  buttons[SDL_GAMEPAD_BUTTON_COUNT] = {};
#else
    int   typeStub    = 0;
    float axes[6]     = {};
    bool  buttons[16] = {};
#endif
};

class CGamepad {
public:
    CGamepad();
    ~CGamepad();

    bool Init();
    void Shutdown();

    // Pump SDL events and dispatch synthesized input. Safe to call when no
    // controller is connected (no-op). dtSeconds is wall-clock since the prior
    // Poll, used by the virtual cursor and stick smoothing.
    void Poll(float dtSeconds, int clientWidth, int clientHeight);

    bool IsConnected() const;
    bool IsEnabled() const { return m_enabled; }

    // Haptics / effects. All three are no-ops when no pad is attached, and
    // SetTriggerResistance only applies to DualSense-class controllers.
    // Force/frequency arguments are clamped to [0, 1].
    void Rumble(float lowFreq01, float highFreq01, uint32_t durationMs);
    void SetTriggerResistance(float leftForce01, float rightForce01);
    void SetLEDColor(uint8_t r, uint8_t g, uint8_t b);

    // Live state — cheap copy for the diagnostic window.
    LiveState GetLiveState() const;

    MoveMode GetMoveMode() const;
    void SetMoveMode(MoveMode mode);

    // Rebind interaction. While rebinding is active, the next pressed button
    // is assigned to the target action, bindings are saved to settings.ini,
    // and rebind mode ends. Normal action dispatch is suppressed during
    // rebinding so the rebind button doesn't also fire the action.
    bool BeginRebind(hotkeys::GamepadAction action);
    void CancelRebind();
    bool IsRebinding() const;
    hotkeys::GamepadAction RebindAction() const;
    void SyncVirtualCursorClientPos(int x, int y);

private:
    struct Impl;
    Impl* m_impl;
    bool m_enabled = false;
};

extern CGamepad g_gamepad;

}  // namespace gamepad
