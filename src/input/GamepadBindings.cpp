#include "GamepadBindings.h"

#include "core/SettingsIni.h"

#include <array>
#include <string>

namespace gamepad {

namespace {

struct ActionInfo {
    Action             action;
    const char*        name;      // stable ini key
#if RO_HAS_GAMEPAD
    SDL_GamepadButton  defaultButton;
#endif
};

#if RO_HAS_GAMEPAD
constexpr ActionInfo kActions[] = {
    { Action::Confirm,         "Confirm",         SDL_GAMEPAD_BUTTON_SOUTH },
    { Action::Cancel,          "Cancel",          SDL_GAMEPAD_BUTTON_EAST },
    { Action::Menu,            "Menu",            SDL_GAMEPAD_BUTTON_START },
    { Action::ChatToggle,      "ChatToggle",      SDL_GAMEPAD_BUTTON_BACK },
    { Action::InventoryToggle, "InventoryToggle", SDL_GAMEPAD_BUTTON_NORTH },
    { Action::EquipToggle,     "EquipToggle",     SDL_GAMEPAD_BUTTON_WEST },
    { Action::SkillToggle,     "SkillToggle",     SDL_GAMEPAD_BUTTON_DPAD_UP },
    { Action::StatusToggle,    "StatusToggle",    SDL_GAMEPAD_BUTTON_DPAD_DOWN },
    { Action::MapToggle,       "MapToggle",       SDL_GAMEPAD_BUTTON_DPAD_LEFT },
    { Action::QuestToggle,     "QuestToggle",     SDL_GAMEPAD_BUTTON_DPAD_RIGHT },
    { Action::HotbarPagePrev,  "HotbarPagePrev",  SDL_GAMEPAD_BUTTON_LEFT_SHOULDER },
    { Action::HotbarPageNext,  "HotbarPageNext",  SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER },
    { Action::Sit,             "Sit",             SDL_GAMEPAD_BUTTON_RIGHT_STICK },
    { Action::CameraReset,     "CameraReset",     SDL_GAMEPAD_BUTTON_LEFT_STICK },
};
#else
constexpr ActionInfo kActions[] = {
    { Action::Confirm,         "Confirm" },
    { Action::Cancel,          "Cancel" },
    { Action::Menu,            "Menu" },
    { Action::ChatToggle,      "ChatToggle" },
    { Action::InventoryToggle, "InventoryToggle" },
    { Action::EquipToggle,     "EquipToggle" },
    { Action::SkillToggle,     "SkillToggle" },
    { Action::StatusToggle,    "StatusToggle" },
    { Action::MapToggle,       "MapToggle" },
    { Action::QuestToggle,     "QuestToggle" },
    { Action::HotbarPagePrev,  "HotbarPagePrev" },
    { Action::HotbarPageNext,  "HotbarPageNext" },
    { Action::Sit,             "Sit" },
    { Action::CameraReset,     "CameraReset" },
};
#endif

static_assert(sizeof(kActions) / sizeof(kActions[0]) == kActionCount - 1,
    "kActions must cover every Action except Invalid");

constexpr const char* kSettingsSection = "GamepadBindings";

const ActionInfo* FindInfo(Action action)
{
    for (const ActionInfo& info : kActions) {
        if (info.action == action) return &info;
    }
    return nullptr;
}

}  // namespace

const char* ActionName(Action action)
{
    const ActionInfo* info = FindInfo(action);
    return info ? info->name : "Invalid";
}

#if RO_HAS_GAMEPAD

namespace bindings {

namespace {

std::array<SDL_GamepadButton, kActionCount> g_table = [] {
    std::array<SDL_GamepadButton, kActionCount> t{};
    for (auto& b : t) b = SDL_GAMEPAD_BUTTON_INVALID;
    for (const ActionInfo& info : kActions) {
        t[static_cast<int>(info.action)] = info.defaultButton;
    }
    return t;
}();

}  // namespace

void Load()
{
    for (const ActionInfo& info : kActions) {
        std::string value;
        if (!TryLoadSettingsIniString(kSettingsSection, info.name, &value)) {
            g_table[static_cast<int>(info.action)] = info.defaultButton;
            continue;
        }
        const SDL_GamepadButton parsed = SDL_GetGamepadButtonFromString(value.c_str());
        g_table[static_cast<int>(info.action)] =
            (parsed == SDL_GAMEPAD_BUTTON_INVALID) ? info.defaultButton : parsed;
    }
}

void Save()
{
    for (const ActionInfo& info : kActions) {
        const SDL_GamepadButton button = g_table[static_cast<int>(info.action)];
        const char* str = SDL_GetGamepadStringForButton(button);
        SaveSettingsIniString(kSettingsSection, info.name, str ? str : "invalid");
    }
}

void ResetDefaults()
{
    for (const ActionInfo& info : kActions) {
        g_table[static_cast<int>(info.action)] = info.defaultButton;
    }
}

SDL_GamepadButton GetButton(Action action)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kActionCount) return SDL_GAMEPAD_BUTTON_INVALID;
    return g_table[index];
}

void SetBinding(Action action, SDL_GamepadButton button)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kActionCount) return;
    g_table[index] = button;
}

}  // namespace bindings

#endif  // RO_HAS_GAMEPAD

}  // namespace gamepad
