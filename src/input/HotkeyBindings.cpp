#include "HotkeyBindings.h"

#include "core/SettingsIni.h"

#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace hotkeys {

namespace {

constexpr const char* kKeyboardSettingsSection = "KeyboardBindings";
constexpr const char* kGamepadSettingsSection = "GamepadBindings";

constexpr int kVkHome = 0x24;
constexpr int kVkSpace = 0x20;
constexpr int kVkF1 = 0x70;
constexpr int kVkF12 = 0x7B;
constexpr int kVkSnapshot = 0x2C;  // Print Screen
constexpr int kVkOemQuestion = 0xBF;  // '/?' key, used as free default for QuestToggle
constexpr int kVkPageUp = 0x21;
constexpr int kVkPageDown = 0x22;
constexpr int kVkInsert = 0x2D;
constexpr int kVkDelete = 0x2E;
constexpr int kVkTab = 0x09;
constexpr int kVkEnter = 0x0D;
constexpr int kVkEscape = 0x1B;
constexpr int kVkLeft = 0x25;
constexpr int kVkUp = 0x26;
constexpr int kVkRight = 0x27;
constexpr int kVkDown = 0x28;

struct KeyboardActionInfo {
    KeyboardAction action;
    const char* name;
    KeyboardBinding defaultBinding;
};

#if RO_HAS_GAMEPAD
struct GamepadActionInfo {
    GamepadAction action;
    const char* name;
    SDL_GamepadButton defaultButton;
};
#endif

constexpr KeyboardActionInfo kKeyboardActions[] = {
    { KeyboardAction::Quickslot1,            "Quickslot1",            { kVkF1,   false, false, false } },
    { KeyboardAction::Quickslot2,            "Quickslot2",            { kVkF1+1, false, false, false } },
    { KeyboardAction::Quickslot3,            "Quickslot3",            { kVkF1+2, false, false, false } },
    { KeyboardAction::Quickslot4,            "Quickslot4",            { kVkF1+3, false, false, false } },
    { KeyboardAction::Quickslot5,            "Quickslot5",            { kVkF1+4, false, false, false } },
    { KeyboardAction::Quickslot6,            "Quickslot6",            { kVkF1+5, false, false, false } },
    { KeyboardAction::Quickslot7,            "Quickslot7",            { kVkF1+6, false, false, false } },
    { KeyboardAction::Quickslot8,            "Quickslot8",            { kVkF1+7, false, false, false } },
    { KeyboardAction::Quickslot9,            "Quickslot9",            { kVkF1+8, false, false, false } },
    { KeyboardAction::ToggleOptionsWindow,    "ToggleOptionsWindow",    { kVkF1 + 10, false, false, false } }, // F11
    { KeyboardAction::ToggleControllerWindow, "ToggleControllerWindow", { kVkF1 + 9, false, false, false } },  // F10
    { KeyboardAction::ResetCamera,           "ResetCamera",           { kVkHome, false, false, false } },
    { KeyboardAction::SitStand,              "SitStand",              { kVkInsert, false, false, false } },
    { KeyboardAction::ChatToggle,            "ChatToggle",            { kVkEnter, false, false, false } },
    { KeyboardAction::InventoryToggle,        "InventoryToggle",       { static_cast<int>('E'), false, true, false } },
    { KeyboardAction::EquipToggle,            "EquipToggle",           { static_cast<int>('Q'), false, true, false } },
    { KeyboardAction::SkillToggle,            "SkillToggle",           { static_cast<int>('S'), false, true, false } },
    { KeyboardAction::StatusToggle,           "StatusToggle",          { static_cast<int>('A'), false, true, false } },
    { KeyboardAction::MapToggle,              "MapToggle",             { kVkTab, false, true, false } },
    { KeyboardAction::QuestToggle,            "QuestToggle",           { kVkOemQuestion, false, false, false } },
    { KeyboardAction::HotbarPagePrev,         "HotbarPagePrev",        { kVkPageUp, false, false, false } },
    { KeyboardAction::HotbarPageNext,         "HotbarPageNext",        { kVkPageDown, false, false, false } },
    { KeyboardAction::CaptureScreenshot,      "CaptureScreenshot",     { kVkSnapshot, false, false, false } },
    { KeyboardAction::ToggleRecording,        "ToggleRecording",       { kVkF12, false, false, false } },
};

#if RO_HAS_GAMEPAD
constexpr GamepadActionInfo kGamepadActions[] = {
    { GamepadAction::Confirm,            "Confirm",            SDL_GAMEPAD_BUTTON_SOUTH },
    { GamepadAction::Cancel,             "Cancel",             SDL_GAMEPAD_BUTTON_EAST },
    { GamepadAction::ToggleOptionsWindow, "ToggleOptionsWindow", SDL_GAMEPAD_BUTTON_START },
    { GamepadAction::ChatToggle,         "ChatToggle",         SDL_GAMEPAD_BUTTON_BACK },
    { GamepadAction::InventoryToggle,    "InventoryToggle",    SDL_GAMEPAD_BUTTON_NORTH },
    { GamepadAction::EquipToggle,        "EquipToggle",        SDL_GAMEPAD_BUTTON_WEST },
    { GamepadAction::SkillToggle,        "SkillToggle",        SDL_GAMEPAD_BUTTON_DPAD_UP },
    { GamepadAction::StatusToggle,       "StatusToggle",       SDL_GAMEPAD_BUTTON_DPAD_DOWN },
    { GamepadAction::MapToggle,          "MapToggle",          SDL_GAMEPAD_BUTTON_DPAD_LEFT },
    { GamepadAction::QuestToggle,        "QuestToggle",        SDL_GAMEPAD_BUTTON_DPAD_RIGHT },
    { GamepadAction::HotbarPagePrev,     "HotbarPagePrev",     SDL_GAMEPAD_BUTTON_LEFT_SHOULDER },
    { GamepadAction::HotbarPageNext,     "HotbarPageNext",     SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER },
    { GamepadAction::SitStand,           "SitStand",           SDL_GAMEPAD_BUTTON_RIGHT_STICK },
    { GamepadAction::ResetCamera,        "ResetCamera",        SDL_GAMEPAD_BUTTON_LEFT_STICK },
};
#endif

static_assert(sizeof(kKeyboardActions) / sizeof(kKeyboardActions[0]) == kKeyboardActionCount - 1,
    "keyboard action table must cover every action except Invalid");

#if RO_HAS_GAMEPAD
static_assert(sizeof(kGamepadActions) / sizeof(kGamepadActions[0]) == kGamepadActionCount - 1,
    "gamepad action table must cover every action except Invalid");
#endif

const KeyboardActionInfo* FindKeyboardInfo(KeyboardAction action)
{
    for (const KeyboardActionInfo& info : kKeyboardActions) {
        if (info.action == action) {
            return &info;
        }
    }
    return nullptr;
}

#if RO_HAS_GAMEPAD
const GamepadActionInfo* FindGamepadInfo(GamepadAction action)
{
    for (const GamepadActionInfo& info : kGamepadActions) {
        if (info.action == action) {
            return &info;
        }
    }
    return nullptr;
}
#endif

std::string ToLowerAscii(std::string value)
{
    for (char& ch : value) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

std::vector<std::string> SplitBindingTokens(const std::string& value)
{
    std::vector<std::string> out;
    size_t start = 0;
    while (start < value.size()) {
        const size_t plusPos = value.find('+', start);
        const size_t count = plusPos == std::string::npos ? value.size() - start : plusPos - start;
        if (count > 0) {
            out.push_back(value.substr(start, count));
        }
        if (plusPos == std::string::npos) {
            break;
        }
        start = plusPos + 1;
    }
    return out;
}

bool TryParseKeyToken(const std::string& token, int* outVirtualKey)
{
    if (!outVirtualKey) {
        return false;
    }

    const std::string lowered = ToLowerAscii(token);
    if (lowered == "enter" || lowered == "return") { *outVirtualKey = kVkEnter; return true; }
    if (lowered == "esc" || lowered == "escape") { *outVirtualKey = kVkEscape; return true; }
    if (lowered == "tab") { *outVirtualKey = kVkTab; return true; }
    if (lowered == "space") { *outVirtualKey = kVkSpace; return true; }
    if (lowered == "home") { *outVirtualKey = kVkHome; return true; }
    if (lowered == "insert") { *outVirtualKey = kVkInsert; return true; }
    if (lowered == "delete" || lowered == "del") { *outVirtualKey = kVkDelete; return true; }
    if (lowered == "pageup") { *outVirtualKey = kVkPageUp; return true; }
    if (lowered == "pagedown") { *outVirtualKey = kVkPageDown; return true; }
    if (lowered == "printscreen" || lowered == "prtsc" || lowered == "snapshot") { *outVirtualKey = kVkSnapshot; return true; }
    if (lowered == "/" || lowered == "slash") { *outVirtualKey = kVkOemQuestion; return true; }
    if (lowered == "left") { *outVirtualKey = kVkLeft; return true; }
    if (lowered == "right") { *outVirtualKey = kVkRight; return true; }
    if (lowered == "up") { *outVirtualKey = kVkUp; return true; }
    if (lowered == "down") { *outVirtualKey = kVkDown; return true; }

    if (lowered.size() == 1) {
        const unsigned char ch = static_cast<unsigned char>(lowered.front());
        if (std::isalnum(ch)) {
            *outVirtualKey = static_cast<int>(std::toupper(ch));
            return true;
        }
    }

    if (lowered.size() >= 2 && lowered.front() == 'f') {
        const int number = std::atoi(lowered.c_str() + 1);
        if (number >= 1 && number <= 12) {
            *outVirtualKey = kVkF1 + (number - 1);
            return true;
        }
    }

    return false;
}

std::string KeyTokenName(int virtualKey)
{
    if (virtualKey >= kVkF1 && virtualKey <= kVkF12) {
        return "F" + std::to_string(virtualKey - kVkF1 + 1);
    }

    switch (virtualKey) {
    case kVkEnter: return "Enter";
    case kVkEscape: return "Esc";
    case kVkTab: return "Tab";
    case kVkSpace: return "Space";
    case kVkHome: return "Home";
    case kVkInsert: return "Insert";
    case kVkDelete: return "Delete";
    case kVkPageUp: return "PageUp";
    case kVkPageDown: return "PageDown";
    case kVkSnapshot: return "PrintScreen";
    case kVkOemQuestion: return "/";
    case kVkLeft: return "Left";
    case kVkRight: return "Right";
    case kVkUp: return "Up";
    case kVkDown: return "Down";
    default:
        break;
    }

    if (virtualKey >= '0' && virtualKey <= '9') {
        return std::string(1, static_cast<char>(virtualKey));
    }
    if (virtualKey >= 'A' && virtualKey <= 'Z') {
        return std::string(1, static_cast<char>(virtualKey));
    }

    char buffer[16] = {};
    std::snprintf(buffer, sizeof(buffer), "VK_%02X", virtualKey & 0xFF);
    return std::string(buffer);
}

std::string FormatKeyboardBindingText(const KeyboardBinding& binding)
{
    if (binding.virtualKey == 0) {
        return "(unbound)";
    }

    std::string result;
    if (binding.alt) {
        result += "Alt+";
    }
    if (binding.ctrl) {
        result += "Ctrl+";
    }
    if (binding.shift) {
        result += "Shift+";
    }
    result += KeyTokenName(binding.virtualKey);
    return result;
}

bool TryParseKeyboardBinding(const std::string& value, KeyboardBinding* outBinding)
{
    if (!outBinding) {
        return false;
    }

    KeyboardBinding binding{};
    const std::vector<std::string> tokens = SplitBindingTokens(value);
    if (tokens.empty()) {
        return false;
    }

    bool keyParsed = false;
    for (const std::string& tokenRaw : tokens) {
        const std::string token = ToLowerAscii(tokenRaw);
        if (token == "alt") {
            binding.alt = true;
            continue;
        }
        if (token == "ctrl" || token == "control") {
            binding.ctrl = true;
            continue;
        }
        if (token == "shift") {
            binding.shift = true;
            continue;
        }

        if (keyParsed) {
            return false;
        }
        keyParsed = TryParseKeyToken(tokenRaw, &binding.virtualKey);
    }

    if (!keyParsed || binding.virtualKey == 0) {
        return false;
    }

    *outBinding = binding;
    return true;
}

template <size_t N>
void CopyDefaults(const KeyboardActionInfo (&infos)[N], std::array<KeyboardBinding, kKeyboardActionCount>* table)
{
    for (KeyboardBinding& binding : *table) {
        binding = KeyboardBinding{};
    }
    for (const KeyboardActionInfo& info : infos) {
        (*table)[static_cast<int>(info.action)] = info.defaultBinding;
    }
}

#if RO_HAS_GAMEPAD
template <size_t N>
void CopyDefaults(const GamepadActionInfo (&infos)[N], std::array<SDL_GamepadButton, kGamepadActionCount>* table)
{
    for (SDL_GamepadButton& binding : *table) {
        binding = SDL_GAMEPAD_BUTTON_INVALID;
    }
    for (const GamepadActionInfo& info : infos) {
        (*table)[static_cast<int>(info.action)] = info.defaultButton;
    }
}
#endif

}  // namespace

const char* KeyboardActionName(KeyboardAction action)
{
    const KeyboardActionInfo* info = FindKeyboardInfo(action);
    return info ? info->name : "Invalid";
}

const char* GamepadActionName(GamepadAction action)
{
#if RO_HAS_GAMEPAD
    const GamepadActionInfo* info = FindGamepadInfo(action);
    return info ? info->name : "Invalid";
#else
    (void)action;
    return "Invalid";
#endif
}

namespace bindings {

namespace {

std::array<KeyboardBinding, kKeyboardActionCount> g_keyboardBindings = [] {
    std::array<KeyboardBinding, kKeyboardActionCount> table{};
    CopyDefaults(kKeyboardActions, &table);
    return table;
}();

#if RO_HAS_GAMEPAD
std::array<SDL_GamepadButton, kGamepadActionCount> g_gamepadBindings = [] {
    std::array<SDL_GamepadButton, kGamepadActionCount> table{};
    CopyDefaults(kGamepadActions, &table);
    return table;
}();
#endif

}  // namespace

void Load()
{
    for (const KeyboardActionInfo& info : kKeyboardActions) {
        std::string value;
        if (!TryLoadSettingsIniString(kKeyboardSettingsSection, info.name, &value)) {
            SetKeyboardBinding(info.action, info.defaultBinding);
            continue;
        }

        KeyboardBinding parsed{};
        if (!TryParseKeyboardBinding(value, &parsed)) {
            parsed = info.defaultBinding;
        }
        SetKeyboardBinding(info.action, parsed);
    }

#if RO_HAS_GAMEPAD
    for (const GamepadActionInfo& info : kGamepadActions) {
        std::string value;
        if (!TryLoadSettingsIniString(kGamepadSettingsSection, info.name, &value)) {
            SetGamepadBinding(info.action, info.defaultButton);
            continue;
        }
        const SDL_GamepadButton parsed = SDL_GetGamepadButtonFromString(value.c_str());
        SetGamepadBinding(info.action,
            (parsed == SDL_GAMEPAD_BUTTON_INVALID) ? info.defaultButton : parsed);
    }
#endif
}

void Save()
{
    for (const KeyboardActionInfo& info : kKeyboardActions) {
        const KeyboardBinding binding = g_keyboardBindings[static_cast<int>(info.action)];
        SaveSettingsIniString(kKeyboardSettingsSection, info.name, FormatKeyboardBindingText(binding));
    }

#if RO_HAS_GAMEPAD
    for (const GamepadActionInfo& info : kGamepadActions) {
        const SDL_GamepadButton button = g_gamepadBindings[static_cast<int>(info.action)];
        const char* text = SDL_GetGamepadStringForButton(button);
        SaveSettingsIniString(kGamepadSettingsSection, info.name, text ? text : "invalid");
    }
#endif
}

void ResetDefaults()
{
    CopyDefaults(kKeyboardActions, &g_keyboardBindings);
#if RO_HAS_GAMEPAD
    CopyDefaults(kGamepadActions, &g_gamepadBindings);
#endif
}

KeyboardBinding GetKeyboardBinding(KeyboardAction action)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kKeyboardActionCount) {
        return KeyboardBinding{};
    }
    return g_keyboardBindings[index];
}

void SetKeyboardBinding(KeyboardAction action, const KeyboardBinding& binding)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kKeyboardActionCount) {
        return;
    }
    for (int other = 1; other < kKeyboardActionCount; ++other) {
        if (other == index) {
            continue;
        }
        const KeyboardBinding current = g_keyboardBindings[other];
        if (current.virtualKey == binding.virtualKey
            && current.alt == binding.alt
            && current.ctrl == binding.ctrl
            && current.shift == binding.shift) {
            g_keyboardBindings[other] = KeyboardBinding{};
        }
    }
    g_keyboardBindings[index] = binding;
}

bool MatchKeyboardBinding(KeyboardAction action, int virtualKey, bool isAltDown, bool isCtrlDown, bool isShiftDown)
{
    const KeyboardBinding binding = GetKeyboardBinding(action);
    if (binding.virtualKey == 0 || virtualKey == 0) {
        return false;
    }
    return binding.virtualKey == virtualKey
        && binding.alt == isAltDown
        && binding.ctrl == isCtrlDown
        && binding.shift == isShiftDown;
}

std::string FormatKeyboardBinding(KeyboardAction action)
{
    return FormatKeyboardBindingText(GetKeyboardBinding(action));
}

#if RO_HAS_GAMEPAD
SDL_GamepadButton GetGamepadButton(GamepadAction action)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kGamepadActionCount) {
        return SDL_GAMEPAD_BUTTON_INVALID;
    }
    return g_gamepadBindings[index];
}

void SetGamepadBinding(GamepadAction action, SDL_GamepadButton button)
{
    const int index = static_cast<int>(action);
    if (index <= 0 || index >= kGamepadActionCount) {
        return;
    }
    for (int other = 1; other < kGamepadActionCount; ++other) {
        if (other == index) {
            continue;
        }
        if (g_gamepadBindings[other] == button) {
            g_gamepadBindings[other] = SDL_GAMEPAD_BUTTON_INVALID;
        }
    }
    g_gamepadBindings[index] = button;
}

std::string FormatGamepadBinding(GamepadAction action)
{
    const SDL_GamepadButton button = GetGamepadButton(action);
    const char* text = SDL_GetGamepadStringForButton(button);
    return text ? text : "(unbound)";
}
#endif

}  // namespace bindings

}  // namespace hotkeys
