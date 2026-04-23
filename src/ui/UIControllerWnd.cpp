#include "UIControllerWnd.h"

#include "input/Gamepad.h"
#include "main/WinMain.h"
#include "qtui/QtUiRuntime.h"
#include "ui/UIWindowMgr.h"

#include <windows.h>

#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>

extern UIWindowMgr g_windowMgr;

namespace {

constexpr int kPanelW = 720;
constexpr int kPanelH = 760;
constexpr int kMargin = 16;
constexpr int kHeaderH = 42;
constexpr int kLiveH = 150;
constexpr int kMoveRowH = 24;
constexpr int kTabH = 22;
constexpr int kRowH = 20;
constexpr int kFooterH = 24;
constexpr int kCornerRadius = 10;

constexpr std::array<hotkeys::KeyboardAction, 22> kKeyboardActions = {
    hotkeys::KeyboardAction::Quickslot1,
    hotkeys::KeyboardAction::Quickslot2,
    hotkeys::KeyboardAction::Quickslot3,
    hotkeys::KeyboardAction::Quickslot4,
    hotkeys::KeyboardAction::Quickslot5,
    hotkeys::KeyboardAction::Quickslot6,
    hotkeys::KeyboardAction::Quickslot7,
    hotkeys::KeyboardAction::Quickslot8,
    hotkeys::KeyboardAction::Quickslot9,
    hotkeys::KeyboardAction::ToggleOptionsWindow,
    hotkeys::KeyboardAction::ToggleControllerWindow,
    hotkeys::KeyboardAction::ResetCamera,
    hotkeys::KeyboardAction::SitStand,
    hotkeys::KeyboardAction::ChatToggle,
    hotkeys::KeyboardAction::InventoryToggle,
    hotkeys::KeyboardAction::EquipToggle,
    hotkeys::KeyboardAction::SkillToggle,
    hotkeys::KeyboardAction::StatusToggle,
    hotkeys::KeyboardAction::MapToggle,
    hotkeys::KeyboardAction::QuestToggle,
    hotkeys::KeyboardAction::HotbarPagePrev,
    hotkeys::KeyboardAction::HotbarPageNext,
};

#if RO_HAS_GAMEPAD
constexpr std::array<hotkeys::GamepadAction, 14> kGamepadActions = {
    hotkeys::GamepadAction::Confirm,
    hotkeys::GamepadAction::Cancel,
    hotkeys::GamepadAction::ToggleOptionsWindow,
    hotkeys::GamepadAction::ChatToggle,
    hotkeys::GamepadAction::InventoryToggle,
    hotkeys::GamepadAction::EquipToggle,
    hotkeys::GamepadAction::SkillToggle,
    hotkeys::GamepadAction::StatusToggle,
    hotkeys::GamepadAction::MapToggle,
    hotkeys::GamepadAction::QuestToggle,
    hotkeys::GamepadAction::HotbarPagePrev,
    hotkeys::GamepadAction::HotbarPageNext,
    hotkeys::GamepadAction::SitStand,
    hotkeys::GamepadAction::ResetCamera,
};
#endif

RECT MakeRect(int x, int y, int w, int h)
{
    return RECT{ x, y, x + w, y + h };
}

void FillRectColor(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawRectFrame(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void FillRoundedRectColor(HDC hdc, const RECT& rc, COLORREF color, int radius)
{
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void FillEllipseColor(HDC hdc, int cx, int cy, int radius, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}

void DrawTextAt(HDC hdc, int x, int y, int w, int h, const char* text,
    COLORREF color, int heightPx, int weight, UINT flags)
{
    HFONT font = CreateFontA(heightPx, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc = MakeRect(x, y, w, h);
    DrawTextA(hdc, text, -1, &rc, flags | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

int KeyboardRowCount()
{
    return static_cast<int>(kKeyboardActions.size());
}

int GamepadRowCount()
{
#if RO_HAS_GAMEPAD
    return static_cast<int>(kGamepadActions.size());
#else
    return 0;
#endif
}

int TitleBarHeight()
{
    return kHeaderH;
}

}  // namespace

UIControllerWnd::UIControllerWnd()
    : m_activeTab(BindingTab::Keyboard),
      m_selectedRow(0),
      m_listScrollOffset(0),
      m_keyboardRebinding(false),
      m_keyboardRebindAction(hotkeys::KeyboardAction::Invalid)
{
    m_defPushId = 0;
    m_defCancelPushId = 0;
    Create(kPanelW, kPanelH);
}

UIControllerWnd::~UIControllerWnd() = default;

void UIControllerWnd::OnCreate(int cx, int cy)
{
    (void)cx;
    (void)cy;
    m_w = kPanelW;
    m_h = kPanelH;
}

void UIControllerWnd::SetShow(int show)
{
    UIWindow::SetShow(show);
    if (show == 0) {
        CancelRebind();
    }
}

void UIControllerWnd::PositionCentered(int clientWidth, int clientHeight)
{
    if (clientWidth <= 0) clientWidth = 1920;
    if (clientHeight <= 0) clientHeight = 1080;
    m_x = std::max(0, (clientWidth - m_w) / 2);
    m_y = std::max(0, (clientHeight - m_h) / 2);
}

UIControllerWnd::BindingTab UIControllerWnd::CurrentTab() const
{
    return m_activeTab;
}

int UIControllerWnd::RowCountForTab(BindingTab tab) const
{
    switch (tab) {
    case BindingTab::Keyboard: return KeyboardRowCount();
    case BindingTab::Gamepad:  return GamepadRowCount();
    default: return 0;
    }
}

int UIControllerWnd::VisibleRowCount() const
{
    const int listTopY = m_y + kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4 + kTabH + 4;
    const int listBottomY = m_y + m_h - kMargin - kFooterH;
    const int listHeight = (std::max)(0, listBottomY - listTopY);
    return (std::max)(1, listHeight / kRowH);
}

int UIControllerWnd::MaxScrollOffset() const
{
    const int rowCount = RowCountForTab(m_activeTab);
    return (std::max)(0, rowCount - VisibleRowCount());
}

void UIControllerWnd::ClampScrollOffset()
{
    m_listScrollOffset = std::clamp(m_listScrollOffset, 0, MaxScrollOffset());
}

void UIControllerWnd::EnsureSelectedRowVisible()
{
    ClampScrollOffset();
    const int visibleRows = VisibleRowCount();
    const int firstVisibleRow = m_listScrollOffset;
    const int lastVisibleRow = firstVisibleRow + visibleRows - 1;
    if (m_selectedRow < firstVisibleRow) {
        m_listScrollOffset = m_selectedRow;
    } else if (m_selectedRow > lastVisibleRow) {
        m_listScrollOffset = m_selectedRow - visibleRows + 1;
    }
    ClampScrollOffset();
}

bool UIControllerWnd::IsInTitleBar(int x, int y) const
{
    return x >= m_x && x < m_x + m_w && y >= m_y && y < m_y + TitleBarHeight();
}

hotkeys::KeyboardAction UIControllerWnd::KeyboardActionAtRow(int row) const
{
    if (row < 0 || row >= static_cast<int>(kKeyboardActions.size())) {
        return hotkeys::KeyboardAction::Invalid;
    }
    return kKeyboardActions[static_cast<size_t>(row)];
}

hotkeys::GamepadAction UIControllerWnd::GamepadActionAtRow(int row) const
{
#if RO_HAS_GAMEPAD
    if (row < 0 || row >= static_cast<int>(kGamepadActions.size())) {
        return hotkeys::GamepadAction::Invalid;
    }
    return kGamepadActions[static_cast<size_t>(row)];
#else
    (void)row;
    return hotkeys::GamepadAction::Invalid;
#endif
}

void UIControllerWnd::BeginKeyboardRebind(hotkeys::KeyboardAction action)
{
    m_keyboardRebinding = true;
    m_keyboardRebindAction = action;
}

void UIControllerWnd::BeginGamepadRebind(hotkeys::GamepadAction action)
{
#if RO_HAS_GAMEPAD
    gamepad::g_gamepad.BeginRebind(action);
#else
    (void)action;
#endif
}

void UIControllerWnd::CancelRebind()
{
    if (m_keyboardRebinding) {
        m_keyboardRebinding = false;
        m_keyboardRebindAction = hotkeys::KeyboardAction::Invalid;
    }
#if RO_HAS_GAMEPAD
    if (gamepad::g_gamepad.IsRebinding()) {
        gamepad::g_gamepad.CancelRebind();
    }
#endif
}

void UIControllerWnd::OnProcess()
{
    if (m_show == 0) {
        return;
    }
    ClampScrollOffset();
    Invalidate();
}

int UIControllerWnd::RowAtPoint(int x, int y) const
{
    const int listTopY = m_y + kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4 + kTabH + 4;
    const int listX = m_x + kMargin;
    const int listW = m_w - kMargin * 2;
    if (x < listX || x >= listX + listW) return -1;
    const int offset = y - listTopY;
    if (offset < 0) return -1;
    const int row = m_listScrollOffset + (offset / kRowH);
    const int rowCount = RowCountForTab(CurrentTab());
    if (row < 0 || row >= rowCount) return -1;
    return row;
}

void UIControllerWnd::OnLBtnDown(int x, int y)
{
    if (IsInTitleBar(x, y)) {
        m_isDragging = 1;
        m_startGlobalX = x;
        m_startGlobalY = y;
        m_orgX = m_x;
        m_orgY = m_y;
        return;
    }

    const int tabY = m_y + kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4;
    const int tabX = m_x + kMargin;
    const int tabW = (m_w - kMargin * 2) / 2;
    const RECT keyboardTab = MakeRect(tabX, tabY, tabW, kTabH);
    const RECT gamepadTab = MakeRect(tabX + tabW, tabY, tabW, kTabH);
    if (x >= keyboardTab.left && x < keyboardTab.right && y >= keyboardTab.top && y < keyboardTab.bottom) {
        m_activeTab = BindingTab::Keyboard;
        m_selectedRow = std::min(m_selectedRow, std::max(0, RowCountForTab(m_activeTab) - 1));
        ClampScrollOffset();
        EnsureSelectedRowVisible();
        Invalidate();
        return;
    }
    if (x >= gamepadTab.left && x < gamepadTab.right && y >= gamepadTab.top && y < gamepadTab.bottom) {
        m_activeTab = BindingTab::Gamepad;
        m_selectedRow = std::min(m_selectedRow, std::max(0, RowCountForTab(m_activeTab) - 1));
        ClampScrollOffset();
        EnsureSelectedRowVisible();
        Invalidate();
        return;
    }

    const int moveRowY = m_y + kMargin + kHeaderH + kLiveH + 4;
    const int moveRowX = m_x + kMargin;
    const int moveRowW = m_w - kMargin * 2;
    if (x >= moveRowX && x < moveRowX + moveRowW && y >= moveRowY && y < moveRowY + kMoveRowH) {
#if RO_HAS_GAMEPAD
        const gamepad::MoveMode cur = gamepad::g_gamepad.GetMoveMode();
        gamepad::g_gamepad.SetMoveMode(cur == gamepad::MoveMode::Cursor
            ? gamepad::MoveMode::Character
            : gamepad::MoveMode::Cursor);
#endif
        Invalidate();
        return;
    }

    const int row = RowAtPoint(x, y);
    if (row < 0) {
        return;
    }

    m_selectedRow = row;
    EnsureSelectedRowVisible();
    if (m_activeTab == BindingTab::Keyboard) {
        BeginKeyboardRebind(KeyboardActionAtRow(row));
    } else {
        BeginGamepadRebind(GamepadActionAtRow(row));
    }
    Invalidate();
}

void UIControllerWnd::OnMouseMove(int x, int y)
{
    if (m_isDragging != 0) {
        UIFrameWnd::OnMouseMove(x, y);
        return;
    }

    const int row = RowAtPoint(x, y);
    if (row >= 0 && row != m_selectedRow) {
        m_selectedRow = row;
        EnsureSelectedRowVisible();
        Invalidate();
    }
}

void UIControllerWnd::OnWheel(int delta)
{
    const int oldOffset = m_listScrollOffset;
    if (delta > 0) {
        m_listScrollOffset = std::max(0, m_listScrollOffset - 1);
    } else if (delta < 0) {
        m_listScrollOffset = std::min(MaxScrollOffset(), m_listScrollOffset + 1);
    }
    if (m_listScrollOffset != oldOffset) {
        Invalidate();
    }
}

void UIControllerWnd::OnKeyDown(int virtualKey)
{
    if (m_keyboardRebinding) {
        if (virtualKey == VK_ESCAPE) {
            m_keyboardRebinding = false;
            m_keyboardRebindAction = hotkeys::KeyboardAction::Invalid;
            Invalidate();
            return;
        }

        const bool isAltDown = (GetKeyState(VK_MENU) & 0x8000) != 0;
        const bool isCtrlDown = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        const bool isShiftDown = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        hotkeys::KeyboardBinding binding{};
        binding.virtualKey = virtualKey;
        binding.alt = isAltDown;
        binding.ctrl = isCtrlDown;
        binding.shift = isShiftDown;
        hotkeys::bindings::SetKeyboardBinding(m_keyboardRebindAction, binding);
        hotkeys::bindings::Save();
        m_keyboardRebinding = false;
        m_keyboardRebindAction = hotkeys::KeyboardAction::Invalid;
        Invalidate();
        return;
    }

    if (virtualKey == VK_ESCAPE) {
        SetShow(0);
        return;
    }
    if (virtualKey == VK_TAB) {
        m_activeTab = (m_activeTab == BindingTab::Keyboard) ? BindingTab::Gamepad : BindingTab::Keyboard;
        m_selectedRow = std::min(m_selectedRow, std::max(0, RowCountForTab(m_activeTab) - 1));
        ClampScrollOffset();
        EnsureSelectedRowVisible();
        Invalidate();
        return;
    }
    if (virtualKey == VK_UP) {
        if (m_selectedRow > 0) --m_selectedRow;
        EnsureSelectedRowVisible();
        Invalidate();
        return;
    }
    if (virtualKey == VK_DOWN) {
        if (m_selectedRow < RowCountForTab(m_activeTab) - 1) ++m_selectedRow;
        EnsureSelectedRowVisible();
        Invalidate();
        return;
    }
    if (virtualKey == VK_PRIOR) {
        m_listScrollOffset = std::max(0, m_listScrollOffset - VisibleRowCount());
        Invalidate();
        return;
    }
    if (virtualKey == VK_NEXT) {
        m_listScrollOffset = std::min(MaxScrollOffset(), m_listScrollOffset + VisibleRowCount());
        Invalidate();
        return;
    }
    if (virtualKey == VK_RETURN) {
        if (m_activeTab == BindingTab::Keyboard) {
            BeginKeyboardRebind(KeyboardActionAtRow(m_selectedRow));
        } else {
#if RO_HAS_GAMEPAD
            BeginGamepadRebind(GamepadActionAtRow(m_selectedRow));
#endif
        }
        Invalidate();
        return;
    }
    if (virtualKey == VK_DELETE) {
        hotkeys::bindings::ResetDefaults();
        hotkeys::bindings::Save();
        Invalidate();
        return;
    }
}

void UIControllerWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) {
        return;
    }
    if (!g_hMainWnd || m_show == 0) {
        return;
    }

    HDC hdc = AcquireDrawTarget();
    if (!hdc) {
        return;
    }

    const RECT panel = MakeRect(m_x, m_y, m_w, m_h);
    FillRoundedRectColor(hdc, panel, RGB(245, 245, 250), kCornerRadius);
    DrawRectFrame(hdc, panel, RGB(90, 90, 110));

#if RO_HAS_GAMEPAD
    const gamepad::LiveState live = gamepad::g_gamepad.GetLiveState();
    char header[256];
    if (live.connected) {
        snprintf(header, sizeof(header), "Controller: %s (connected)", live.name ? live.name : "(unknown)");
    } else {
        snprintf(header, sizeof(header), "Controller: no controller connected");
    }
#else
    const char* header = "Controller: built without gamepad support";
#endif
    DrawTextAt(hdc, m_x + kMargin, m_y + kMargin, m_w - kMargin * 2, 22,
        header, RGB(20, 20, 40), 20, FW_BOLD, DT_LEFT | DT_VCENTER);

    const int liveX = m_x + kMargin;
    const int liveY = m_y + kMargin + kHeaderH;
    const RECT liveBg = MakeRect(liveX, liveY, m_w - kMargin * 2, kLiveH);
    FillRoundedRectColor(hdc, liveBg, RGB(232, 232, 240), 6);
    DrawRectFrame(hdc, liveBg, RGB(180, 180, 200));

#if RO_HAS_GAMEPAD
    static const struct {
        SDL_GamepadButton button;
        const char* label;
        int x;
        int y;
    } kDots[] = {
        { SDL_GAMEPAD_BUTTON_DPAD_UP,    "U",  60,  28 },
        { SDL_GAMEPAD_BUTTON_DPAD_LEFT,  "L",  28,  64 },
        { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, "R",  92,  64 },
        { SDL_GAMEPAD_BUTTON_DPAD_DOWN,  "D",  60, 100 },
        { SDL_GAMEPAD_BUTTON_NORTH,      "N", 620,  28 },
        { SDL_GAMEPAD_BUTTON_WEST,       "W", 588,  64 },
        { SDL_GAMEPAD_BUTTON_EAST,       "E", 652,  64 },
        { SDL_GAMEPAD_BUTTON_SOUTH,      "S", 620, 100 },
        { SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,  "LB",  60,   8 },
        { SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, "RB", 620,   8 },
        { SDL_GAMEPAD_BUTTON_BACK,       "BK", 300,  12 },
        { SDL_GAMEPAD_BUTTON_START,      "ST", 380,  12 },
        { SDL_GAMEPAD_BUTTON_LEFT_STICK,  "L3", 220, 118 },
        { SDL_GAMEPAD_BUTTON_RIGHT_STICK, "R3", 460, 118 },
    };

    for (const auto& dot : kDots) {
        const int cx = liveX + dot.x;
        const int cy = liveY + dot.y;
        const bool on = live.buttons[dot.button];
        FillEllipseColor(hdc, cx, cy, 11, on ? RGB(80, 200, 90) : RGB(200, 200, 210));
        DrawTextAt(hdc, cx - 14, cy + 12, 28, 14, dot.label,
            RGB(40, 40, 60), 12, FW_NORMAL, DT_CENTER);
    }

    FillEllipseColor(hdc, liveX + 220, liveY + 70, 32, RGB(210, 210, 220));
    DrawRectFrame(hdc, MakeRect(liveX + 188, liveY + 38, 64, 64), RGB(140, 140, 160));
    FillEllipseColor(hdc, liveX + 460, liveY + 70, 32, RGB(210, 210, 220));
    DrawRectFrame(hdc, MakeRect(liveX + 428, liveY + 38, 64, 64), RGB(140, 140, 160));

    DrawTextAt(hdc, liveX + 120, liveY + 128, 180, 14, "Left stick", RGB(60, 60, 80), 12, FW_NORMAL, DT_CENTER);
    DrawTextAt(hdc, liveX + 360, liveY + 128, 180, 14, "Right stick", RGB(60, 60, 80), 12, FW_NORMAL, DT_CENTER);
#else
    DrawTextAt(hdc, liveX + 20, liveY + 20, liveBg.right - liveBg.left - 40, 20,
        "Gamepad support not compiled in.", RGB(120, 40, 40), 14, FW_NORMAL, DT_LEFT);
#endif

    const int moveRowX = liveX;
    const int moveRowY = liveY + kLiveH + 4;
    const int moveRowW = m_w - kMargin * 2;
    const RECT moveRowRc = MakeRect(moveRowX, moveRowY, moveRowW, kMoveRowH);
    FillRoundedRectColor(hdc, moveRowRc, RGB(228, 236, 248), 6);
    DrawRectFrame(hdc, moveRowRc, RGB(150, 165, 195));
#if RO_HAS_GAMEPAD
    const gamepad::MoveMode moveMode = gamepad::g_gamepad.GetMoveMode();
    const char* moveLabel = (moveMode == gamepad::MoveMode::Character)
        ? "Movement mode: Character (stick / D-pad walks the player)   [click to switch to Cursor]"
        : "Movement mode: Cursor (stick / D-pad nudges the virtual cursor)   [click to switch to Character]";
#else
    const char* moveLabel = "Movement mode: (gamepad support not built)";
#endif
    DrawTextAt(hdc, moveRowX + 8, moveRowY + 3, moveRowW - 16, kMoveRowH - 6,
        moveLabel, RGB(30, 40, 70), 13, FW_NORMAL, DT_LEFT | DT_VCENTER);

    DrawTabs(hdc);
    DrawBindings(hdc);

    const int footerY = m_y + m_h - kMargin - kFooterH + 2;
    DrawTextAt(hdc, m_x + kMargin, footerY, m_w - kMargin * 2, kFooterH,
        "Enter/click: rebind   Esc: close/cancel   Del: reset defaults   Tab: switch tab   Wheel/PgUp/PgDn: scroll",
        RGB(60, 60, 80), 13, FW_NORMAL, DT_LEFT | DT_VCENTER);

    ReleaseDrawTarget(hdc);
}

void UIControllerWnd::DrawTabs(HDC hdc)
{
    const int tabY = m_y + kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4;
    const int tabX = m_x + kMargin;
    const int tabW = (m_w - kMargin * 2) / 2;

    const RECT keyboardTab = MakeRect(tabX, tabY, tabW, kTabH);
    const RECT gamepadTab = MakeRect(tabX + tabW, tabY, tabW, kTabH);

    const bool keyboardActive = m_activeTab == BindingTab::Keyboard;
    FillRoundedRectColor(hdc, keyboardTab, keyboardActive ? RGB(255, 255, 255) : RGB(222, 228, 240), 4);
    FillRoundedRectColor(hdc, gamepadTab, !keyboardActive ? RGB(255, 255, 255) : RGB(222, 228, 240), 4);
    DrawRectFrame(hdc, keyboardTab, RGB(130, 140, 160));
    DrawRectFrame(hdc, gamepadTab, RGB(130, 140, 160));
    DrawTextAt(hdc, keyboardTab.left + 8, keyboardTab.top + 3, tabW - 16, kTabH - 6,
        "Keyboard", RGB(30, 40, 70), 13, keyboardActive ? FW_BOLD : FW_NORMAL, DT_CENTER);
    DrawTextAt(hdc, gamepadTab.left + 8, gamepadTab.top + 3, tabW - 16, kTabH - 6,
        "Controller", RGB(30, 40, 70), 13, !keyboardActive ? FW_BOLD : FW_NORMAL, DT_CENTER);
}

void UIControllerWnd::DrawBindings(HDC hdc)
{
    const int listX = m_x + kMargin;
    const int listY = m_y + kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4 + kTabH + 4;
    const int listW = m_w - kMargin * 2;
    const int rowCount = RowCountForTab(m_activeTab);
    const int visibleRows = VisibleRowCount();
    const int firstRow = m_listScrollOffset;
    const int lastRow = (std::min)(rowCount, firstRow + visibleRows);

    DrawTextAt(hdc, listX, listY - 16, listW, 14,
        m_activeTab == BindingTab::Keyboard ? "Keyboard bindings" : "Controller bindings",
        RGB(40, 40, 60), 13, FW_BOLD, DT_LEFT);

    SaveDC(hdc);
    IntersectClipRect(hdc, listX, listY, listX + listW, listY + visibleRows * kRowH);

    for (int row = firstRow; row < lastRow; ++row) {
        const int rowY = listY + (row - firstRow) * kRowH;
        const bool selected = (row == m_selectedRow);
        const RECT rowRc = MakeRect(listX, rowY, listW, kRowH);
        if (selected) {
            FillRectColor(hdc, rowRc, RGB(210, 225, 255));
        }

        const char* label = "";
        std::string bindingText;
#if RO_HAS_GAMEPAD
        const bool rebinding = !m_keyboardRebinding && gamepad::g_gamepad.IsRebinding();
#else
        const bool rebinding = false;
#endif

        if (m_activeTab == BindingTab::Keyboard) {
            const hotkeys::KeyboardAction action = KeyboardActionAtRow(row);
            label = hotkeys::KeyboardActionName(action);
            bindingText = m_keyboardRebinding && m_keyboardRebindAction == action
                ? "press a key..."
                : hotkeys::bindings::FormatKeyboardBinding(action);
        } else {
#if RO_HAS_GAMEPAD
            const hotkeys::GamepadAction action = GamepadActionAtRow(row);
            label = hotkeys::GamepadActionName(action);
            bindingText = rebinding && gamepad::g_gamepad.RebindAction() == action
                ? "press any button..."
                : hotkeys::bindings::FormatGamepadBinding(action);
#else
            label = "(gamepad support not built)";
            bindingText = "(unavailable)";
#endif
        }

        DrawTextAt(hdc, listX + 8, rowY + 2, listW / 2 - 16, kRowH - 4,
            label, RGB(30, 30, 50), 13, FW_NORMAL, DT_LEFT);
        DrawTextAt(hdc, listX + listW / 2, rowY + 2, listW / 2 - 16, kRowH - 4,
            bindingText.c_str(), RGB(40, 60, 120), 13, FW_BOLD, DT_LEFT);
    }

    RestoreDC(hdc, -1);
}

bool UIControllerWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) {
        return false;
    }

    DisplayData data{};
    data.activeTab = static_cast<int>(m_activeTab);

#if RO_HAS_GAMEPAD
    const gamepad::LiveState live = gamepad::g_gamepad.GetLiveState();
    data.connected = live.connected;
    data.controllerName = live.name ? live.name : "";
    data.controllerType = live.connected ? "connected" : "disconnected";
    data.moveModeText = (gamepad::g_gamepad.GetMoveMode() == gamepad::MoveMode::Character)
        ? "Character"
        : "Cursor";
#else
    data.connected = false;
    data.controllerName = "";
    data.controllerType = "unsupported";
    data.moveModeText = "Unsupported";
#endif

    const int tabY = kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4;
    const int tabW = (kPanelW - kMargin * 2) / 2;
    data.tabs.push_back(DisplayTab{ 0, kMargin, tabY, tabW, kTabH, m_activeTab == BindingTab::Keyboard, "Keyboard" });
    data.tabs.push_back(DisplayTab{ 1, kMargin + tabW, tabY, tabW, kTabH, m_activeTab == BindingTab::Gamepad, "Controller" });

    const int listX = kMargin;
    const int listY = kMargin + kHeaderH + kLiveH + 4 + kMoveRowH + 4 + kTabH + 4;
    const int listW = kPanelW - kMargin * 2;
    const int rowCount = RowCountForTab(m_activeTab);
    const int firstRow = m_listScrollOffset;
    for (int row = 0; row < rowCount; ++row) {
        DisplayRow entry{};
        entry.id = row;
        entry.x = listX;
        entry.y = listY + (row - firstRow) * kRowH;
        entry.width = listW;
        entry.height = kRowH;
        entry.selected = (row == m_selectedRow);
        entry.rebinding = (m_activeTab == BindingTab::Keyboard && m_keyboardRebinding && m_keyboardRebindAction == KeyboardActionAtRow(row))
#if RO_HAS_GAMEPAD
            || (m_activeTab == BindingTab::Gamepad && gamepad::g_gamepad.IsRebinding() && gamepad::g_gamepad.RebindAction() == GamepadActionAtRow(row))
#endif
            ;
        if (m_activeTab == BindingTab::Keyboard) {
            const hotkeys::KeyboardAction action = KeyboardActionAtRow(row);
            entry.label = hotkeys::KeyboardActionName(action);
            entry.bindingText = m_keyboardRebinding && m_keyboardRebindAction == action
                ? "press a key..."
                : hotkeys::bindings::FormatKeyboardBinding(action);
        } else {
#if RO_HAS_GAMEPAD
            const hotkeys::GamepadAction action = GamepadActionAtRow(row);
            entry.label = hotkeys::GamepadActionName(action);
            entry.bindingText = gamepad::g_gamepad.IsRebinding() && gamepad::g_gamepad.RebindAction() == action
                ? "press any button..."
                : hotkeys::bindings::FormatGamepadBinding(action);
#else
            entry.label = "(gamepad support not built)";
            entry.bindingText = "(unavailable)";
#endif
        }
        data.rows.push_back(std::move(entry));
    }

    *outData = std::move(data);
    return true;
}

bool UIControllerWnd::IsRebinding() const
{
    if (m_keyboardRebinding) {
        return true;
    }
#if RO_HAS_GAMEPAD
    return gamepad::g_gamepad.IsRebinding();
#else
    return false;
#endif
}
