#include "UIEmotionWnd.h"

#include "gamemode/GameMode.h"
#include "gamemode/Mode.h"
#include "main/WinMain.h"
#include "qtui/QtUiRuntime.h"
#include "ui/UIWindowMgr.h"

#include <windows.h>

#include <cstddef>
#include <array>

namespace {

constexpr int kWindowWidth = 384;
constexpr int kWindowHeight = 186;
constexpr int kTitleBarHeight = 17;
constexpr int kButtonWidth = 64;
constexpr int kButtonHeight = 40;
constexpr int kButtonSpacingX = 6;
constexpr int kButtonSpacingY = 8;
constexpr int kButtonStartY = 44;
constexpr int kButtonColumns = 5;
constexpr int kFooterHeight = 28;

struct EmotionSlot {
    int emotionType;
    const char* label;
    const char* hotkey;
};

constexpr std::array<EmotionSlot, 10> kEmotionSlots = {{
    { 0, "!",   "Alt+1" },
    { 1, "?",   "Alt+2" },
    { 2, "ho",  "Alt+3" },
    { 3, "lv",  "Alt+4" },
    { 4, "swt", "Alt+5" },
    { 5, "ic",  "Alt+6" },
    { 6, "an",  "Alt+7" },
    { 7, "ag",  "Alt+8" },
    { 8, "$",   "Alt+9" },
    { 9, "...", "Alt+0" },
}};

RECT MakeRect(int x, int y, int w, int h)
{
    RECT rect{ x, y, x + w, y + h };
    return rect;
}

void FillRectColor(HDC hdc, const RECT& rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void FrameRectColor(HDC hdc, const RECT& rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void DrawTextCentered(HDC hdc, const RECT& rect, const char* text, COLORREF color, UINT format = DT_CENTER | DT_VCENTER | DT_SINGLELINE)
{
    if (!hdc || !text || !*text) {
        return;
    }
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT copy = rect;
    DrawTextA(hdc, text, -1, &copy, format);
}

void DrawButtonText(HDC hdc, const RECT& rect, const char* label, const char* hotkey, bool hovered, bool pressed)
{
    const COLORREF primaryColor = pressed ? RGB(255, 255, 255) : hovered ? RGB(42, 52, 79) : RGB(34, 44, 69);
    const COLORREF secondaryColor = pressed ? RGB(245, 248, 255) : RGB(92, 103, 133);

    RECT labelRect = rect;
    labelRect.top += 5;
    labelRect.bottom = labelRect.top + 18;
    DrawTextCentered(hdc, labelRect, label, primaryColor, DT_CENTER | DT_TOP | DT_SINGLELINE);

    RECT hotkeyRect = rect;
    hotkeyRect.top += 22;
    hotkeyRect.bottom = hotkeyRect.top + 14;
    DrawTextCentered(hdc, hotkeyRect, hotkey, secondaryColor, DT_CENTER | DT_TOP | DT_SINGLELINE);
}

} // namespace

UIEmotionWnd::UIEmotionWnd()
    : m_controlsCreated(false),
      m_hoveredButton(-1),
      m_pressedButton(-1)
{
    Create(kWindowWidth, kWindowHeight);
    int defaultX = 180;
    int defaultY = 220;
    LoadUiWindowPlacement("EmotionWnd", &defaultX, &defaultY);
    UIWindow::Move(defaultX, defaultY);
    m_defCancelPushId = ButtonClose;
    UIWindow::SetShow(0);
}

UIEmotionWnd::~UIEmotionWnd() = default;

void UIEmotionWnd::SetShow(int show)
{
    UIWindow::SetShow(show);
    if (show == 0) {
        m_hoveredButton = -1;
        m_pressedButton = -1;
        return;
    }

    EnsureCreated();
    Move(m_x, m_y);
    Invalidate();
}

void UIEmotionWnd::Move(int x, int y)
{
    g_windowMgr.ClampWindowToClient(&x, &y, m_w, m_h);
    UIWindow::Move(x, y);
}

void UIEmotionWnd::OnDraw()
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
    const RECT titleBar = MakeRect(m_x, m_y, m_w, kTitleBarHeight);
    const RECT footerBar = MakeRect(m_x, m_y + m_h - kFooterHeight, m_w, kFooterHeight);

    FillRectColor(hdc, panel, RGB(242, 245, 250));
    FrameRectColor(hdc, panel, RGB(66, 76, 98));
    FillRectColor(hdc, titleBar, RGB(96, 112, 152));

    DrawTextCentered(hdc, MakeRect(m_x + 10, m_y + 1, m_w - 54, kTitleBarHeight - 2), "Emotion Picker", RGB(255, 255, 255), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextCentered(hdc, MakeRect(m_x + 10, m_y + 22, m_w - 20, 16), "Alt+E opens this panel   •   Alt+1..0 sends emotes", RGB(44, 54, 76), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    const RECT closeRect = GetCloseRect();
    FillRectColor(hdc, closeRect, RGB(171, 74, 63));
    FrameRectColor(hdc, closeRect, RGB(103, 34, 28));
    DrawTextCentered(hdc, closeRect, "X", RGB(255, 255, 255));

    for (int index = 0; index < kButtonCount; ++index) {
        const RECT buttonRect = GetButtonRect(index);
        const bool hovered = index == m_hoveredButton;
        const bool pressed = index == m_pressedButton;
        COLORREF fillColor = RGB(223, 230, 242);
        if (pressed) {
            fillColor = RGB(164, 186, 225);
        } else if (hovered) {
            fillColor = RGB(206, 218, 238);
        }
        FillRectColor(hdc, buttonRect, fillColor);
        FrameRectColor(hdc, buttonRect, RGB(95, 107, 136));
        DrawButtonText(hdc, buttonRect, kEmotionSlots[static_cast<size_t>(index)].label, kEmotionSlots[static_cast<size_t>(index)].hotkey, hovered, pressed);
    }

    FillRectColor(hdc, footerBar, RGB(232, 237, 246));
    FrameRectColor(hdc, footerBar, RGB(193, 202, 219));
    DrawTextCentered(hdc, MakeRect(m_x + 10, m_y + m_h - kFooterHeight + 6, m_w - 20, 16),
        "/emotion N supports all 0-87 emotes",
        RGB(79, 88, 110),
        DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIEmotionWnd::OnLBtnDown(int x, int y)
{
    m_pressedButton = -1;

    if (HitTestClose(x, y)) {
        m_pressedButton = ButtonClose;
        PlayUiButtonSound();
        Invalidate();
        return;
    }

    const int buttonIndex = HitTestButton(x, y);
    if (buttonIndex >= 0) {
        m_pressedButton = buttonIndex;
        PlayUiButtonSound();
        Invalidate();
        return;
    }

    if (y >= m_y && y < m_y + kTitleBarHeight) {
        UIFrameWnd::OnLBtnDown(x, y);
    }
}

void UIEmotionWnd::OnLBtnUp(int x, int y)
{
    const bool wasDragging = m_isDragging != 0;
    UIFrameWnd::OnLBtnUp(x, y);
    if (wasDragging) {
        m_pressedButton = -1;
        return;
    }

    const int pressedButton = m_pressedButton;
    m_pressedButton = -1;

    if (pressedButton == ButtonClose && HitTestClose(x, y)) {
        CloseWindow();
        return;
    }

    if (pressedButton >= 0 && pressedButton < kButtonCount && HitTestButton(x, y) == pressedButton) {
        ActivateEmotion(kEmotionSlots[static_cast<size_t>(pressedButton)].emotionType);
        return;
    }

    Invalidate();
}

void UIEmotionWnd::OnMouseMove(int x, int y)
{
    UIFrameWnd::OnMouseMove(x, y);

    const int hovered = HitTestButton(x, y);
    if (hovered != m_hoveredButton) {
        m_hoveredButton = hovered;
        Invalidate();
    }
}

void UIEmotionWnd::OnKeyDown(int virtualKey)
{
    if (virtualKey == VK_ESCAPE) {
        CloseWindow();
        return;
    }

    int slot = -1;
    if (virtualKey >= '1' && virtualKey <= '9') {
        slot = virtualKey - '1';
    } else if (virtualKey == '0') {
        slot = 9;
    }

    if (slot >= 0) {
        ActivateEmotion(kEmotionSlots[static_cast<size_t>(slot)].emotionType);
    }
}

void UIEmotionWnd::StoreInfo()
{
    SaveUiWindowPlacement("EmotionWnd", m_x, m_y);
}

bool UIEmotionWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) {
        return false;
    }

    outData->x = m_x;
    outData->y = m_y;
    outData->width = m_w;
    outData->height = m_h;
    outData->title = "Emotion Picker";
    outData->footer = "/emotion N supports all 0-87 emotes";
    outData->buttons.clear();
    outData->buttons.reserve(kButtonCount);
    for (int index = 0; index < kButtonCount; ++index) {
        EmotionButton button{};
        button.emotionType = kEmotionSlots[static_cast<size_t>(index)].emotionType;
        button.label = kEmotionSlots[static_cast<size_t>(index)].label;
        button.hotkey = kEmotionSlots[static_cast<size_t>(index)].hotkey;
        const RECT rect = GetButtonRect(index);
        button.x = rect.left;
        button.y = rect.top;
        button.width = rect.right - rect.left;
        button.height = rect.bottom - rect.top;
        button.hovered = index == m_hoveredButton;
        button.pressed = index == m_pressedButton;
        outData->buttons.push_back(button);
    }

    return true;
}

void UIEmotionWnd::EnsureCreated()
{
    if (m_controlsCreated) {
        return;
    }
    m_controlsCreated = true;
}

void UIEmotionWnd::CloseWindow()
{
    SetShow(0);
}

bool UIEmotionWnd::ActivateEmotion(int emotionType)
{
    if (emotionType < 0 || emotionType > 87) {
        return false;
    }

    if (g_modeMgr.SendMsg(CGameMode::GameMsg_RequestEmotion, emotionType, 0, 0) != 0) {
        CloseWindow();
        return true;
    }

    return false;
}

int UIEmotionWnd::HitTestButton(int x, int y) const
{
    for (int index = 0; index < kButtonCount; ++index) {
        if (IsPointInRect(GetButtonRect(index), x, y)) {
            return index;
        }
    }
    return -1;
}

bool UIEmotionWnd::HitTestClose(int x, int y) const
{
    return IsPointInRect(GetCloseRect(), x, y);
}

RECT UIEmotionWnd::GetButtonRect(int index) const
{
    const int row = index / kButtonColumns;
    const int col = index % kButtonColumns;
    const int buttonAreaWidth = kButtonColumns * kButtonWidth + (kButtonColumns - 1) * kButtonSpacingX;
    const int startX = m_x + (m_w - buttonAreaWidth) / 2;
    const int x = startX + col * (kButtonWidth + kButtonSpacingX);
    const int y = m_y + kButtonStartY + row * (kButtonHeight + kButtonSpacingY);
    return MakeRect(x, y, kButtonWidth, kButtonHeight);
}

RECT UIEmotionWnd::GetCloseRect() const
{
    return MakeRect(m_x + m_w - 22, m_y + 2, 18, 13);
}

bool UIEmotionWnd::IsPointInRect(const RECT& rect, int x, int y) const
{
    return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
}
