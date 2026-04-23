#pragma once

#include "platform/WindowsCompat.h"

#include <string>

#if RO_HAS_GAMEPAD
#  include <SDL3/SDL.h>
#endif

namespace hotkeys {

enum class KeyboardAction {
    Invalid = 0,
    Quickslot1,
    Quickslot2,
    Quickslot3,
    Quickslot4,
    Quickslot5,
    Quickslot6,
    Quickslot7,
    Quickslot8,
    Quickslot9,
    ToggleOptionsWindow,
    ToggleControllerWindow,
    ResetCamera,
    SitStand,
    ChatToggle,
    InventoryToggle,
    EquipToggle,
    SkillToggle,
    StatusToggle,
    MapToggle,
    QuestToggle,
    HotbarPagePrev,
    HotbarPageNext,
    CaptureScreenshot,
    ToggleRecording,
    _Count
};

enum class GamepadAction {
    Invalid = 0,
    Confirm,
    Cancel,
    ToggleOptionsWindow,
    ChatToggle,
    InventoryToggle,
    EquipToggle,
    SkillToggle,
    StatusToggle,
    MapToggle,
    QuestToggle,
    HotbarPagePrev,
    HotbarPageNext,
    SitStand,
    ResetCamera,
    _Count
};

constexpr int kKeyboardActionCount = static_cast<int>(KeyboardAction::_Count);
constexpr int kGamepadActionCount = static_cast<int>(GamepadAction::_Count);

const char* KeyboardActionName(KeyboardAction action);
const char* GamepadActionName(GamepadAction action);

struct KeyboardBinding {
    int  virtualKey = 0;
    bool alt = false;
    bool ctrl = false;
    bool shift = false;
};

namespace bindings {

void Load();
void Save();
void ResetDefaults();

KeyboardBinding GetKeyboardBinding(KeyboardAction action);
void SetKeyboardBinding(KeyboardAction action, const KeyboardBinding& binding);
bool MatchKeyboardBinding(KeyboardAction action, int virtualKey, bool isAltDown, bool isCtrlDown, bool isShiftDown);
std::string FormatKeyboardBinding(KeyboardAction action);

#if RO_HAS_GAMEPAD
SDL_GamepadButton GetGamepadButton(GamepadAction action);
void SetGamepadBinding(GamepadAction action, SDL_GamepadButton button);
std::string FormatGamepadBinding(GamepadAction action);
#endif

}  // namespace bindings

}  // namespace hotkeys
