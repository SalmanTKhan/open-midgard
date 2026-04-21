#pragma once

#if RO_HAS_GAMEPAD
#  include <SDL3/SDL.h>
#endif

namespace gamepad {

// High-level actions produced by the gamepad. Each Action maps to a single
// SDL_GamepadButton at runtime via a user-editable table persisted in
// settings.ini under [GamepadBindings]. Context-sensitive behavior
// (e.g. Cancel = Escape over UI, SitStand in world) stays in Poll; the
// binding only controls which physical button triggers the action.
enum class Action {
    Invalid = 0,

    Confirm,          // default: SOUTH   (A / Cross)
    Cancel,           // default: EAST    (B / Circle)
    Menu,             // default: START
    ChatToggle,       // default: BACK    (Select / Share)
    InventoryToggle,  // default: NORTH   (Y / Triangle)
    EquipToggle,      // default: WEST    (X / Square)
    SkillToggle,      // default: DPAD_UP
    StatusToggle,     // default: DPAD_DOWN
    MapToggle,        // default: DPAD_LEFT
    QuestToggle,      // default: DPAD_RIGHT
    HotbarPagePrev,   // default: LEFT_SHOULDER
    HotbarPageNext,   // default: RIGHT_SHOULDER
    Sit,              // default: RIGHT_STICK
    CameraReset,      // default: LEFT_STICK

    _Count
};

constexpr int kActionCount = static_cast<int>(Action::_Count);

const char* ActionName(Action action);

#if RO_HAS_GAMEPAD

namespace bindings {

// Load bindings from settings.ini; missing keys fall back to defaults.
void Load();

// Persist the current table to settings.ini.
void Save();

// Reset every action to its default button.
void ResetDefaults();

// Return the SDL button currently bound to `action`, or SDL_GAMEPAD_BUTTON_INVALID.
SDL_GamepadButton GetButton(Action action);

// Rebind a single action. Caller is expected to call Save() afterwards when
// the change is user-initiated.
void SetBinding(Action action, SDL_GamepadButton button);

}  // namespace bindings

#endif  // RO_HAS_GAMEPAD

}  // namespace gamepad
