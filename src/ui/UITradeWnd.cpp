#include "UITradeWnd.h"

#include "UIWindowMgr.h"
#include "main/WinMain.h"
#include "session/Session.h"

#include <cstdio>
#include <string>
#include <windows.h>

// Forward declarations of CZ trade senders defined in src/gamemode/GameMode.cpp.
// Kept file-local rather than in a public header to avoid coupling the UI
// translation unit to gamemode internals beyond what it actually consumes.
bool SendTradeAddItem(u16 inventoryIndex, u32 amount);
bool SendTradeOk();
bool SendTradeCancel();
bool SendTradeCommit();

namespace {

constexpr int kFullWidth = 360;
constexpr int kFullHeight = 280;
constexpr int kTitleBarHeight = 17;
constexpr int kPaneWidth = 168;
constexpr int kPaneTop = 24;
constexpr int kPaneHeight = 196;
constexpr int kRowHeight = 14;
constexpr int kButtonAreaTop = 230;
constexpr int kButtonWidth = 100;
constexpr int kButtonHeight = 22;
constexpr int kButtonGap = 8;

void FillSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void FrameSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawLabel(HDC hdc, int x, int y, COLORREF color, const std::string& text, UINT format = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) {
        return;
    }
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, format);
}

unsigned long long HashU64(unsigned long long seed, unsigned long long value)
{
    seed ^= value + 0x9E3779B97F4A7C15ull + (seed << 6) + (seed >> 2);
    return seed;
}

unsigned long long HashStr(unsigned long long seed, const std::string& s)
{
    for (unsigned char ch : s) {
        seed = HashU64(seed, ch);
    }
    return seed;
}

void DrawPane(HDC hdc, int x, int y, const std::string& heading, const TRADE_PANE& pane, bool isReady)
{
    const RECT outer{ x, y, x + kPaneWidth, y + kPaneHeight };
    FillSolid(hdc, outer, RGB(244, 240, 226));
    FrameSolid(hdc, outer, RGB(120, 100, 70));

    DrawLabel(hdc, x + 6, y + 4, RGB(60, 40, 20), heading);
    if (isReady) {
        DrawLabel(hdc, x + kPaneWidth - 60, y + 4, RGB(40, 130, 50), "Ready");
    }

    int rowY = y + 22;
    int rowsLeft = (kPaneHeight - 30 - 18) / kRowHeight;
    for (const ITEM_INFO& item : pane.items) {
        if (rowsLeft-- <= 0) break;
        char label[96] = {};
        std::string name = item.GetDisplayName();
        if (name.empty()) {
            std::snprintf(label, sizeof(label), "Item #%u  x%d", item.GetItemId(), item.m_num);
        } else {
            std::snprintf(label, sizeof(label), "%.40s  x%d", name.c_str(), item.m_num);
        }
        DrawLabel(hdc, x + 6, rowY, RGB(20, 20, 20), label);
        rowY += kRowHeight;
    }

    char zenyLine[64] = {};
    std::snprintf(zenyLine, sizeof(zenyLine), "Zeny: %d", pane.zeny);
    DrawLabel(hdc, x + 6, y + kPaneHeight - 16, RGB(160, 100, 20), zenyLine);
}

void DrawButton(HDC hdc, const RECT& rc, const char* label, bool enabled, bool pressed)
{
    const COLORREF fill = enabled
        ? (pressed ? RGB(170, 145, 75) : RGB(210, 195, 145))
        : RGB(170, 170, 170);
    FillSolid(hdc, rc, fill);
    FrameSolid(hdc, rc, RGB(80, 60, 30));
    DrawLabel(hdc, rc.left + 6, rc.top + 5, enabled ? RGB(20, 20, 20) : RGB(90, 90, 90), label);
}

} // namespace

UITradeWnd::UITradeWnd() = default;
UITradeWnd::~UITradeWnd() = default;

void UITradeWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

bool UITradeWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) {
        return true;
    }
    return BuildVisualStateToken() != m_lastVisualStateToken;
}

void UITradeWnd::OnDraw()
{
    // Auto-hide when the trade state ends from a packet (ZC_CANCEL or
    // ZC_EXEC). Keep the window object alive for the next trade.
    if (!g_session.IsTradeActive()) {
        if (!m_endedDueToInactive) {
            m_endedDueToInactive = true;
            SetShow(0);
        }
        return;
    }
    m_endedDueToInactive = false;

    if (!g_hMainWnd || m_show == 0) {
        return;
    }

    HDC hdc = AcquireDrawTarget();
    if (!hdc) {
        return;
    }

    const TRADE_STATE& state = g_session.GetTradeState();

    // Window background + title bar.
    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));
    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    DrawLabel(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "Trade");

    const std::string partner = state.partnerName.empty() ? std::string("Partner") : state.partnerName;

    DrawPane(hdc, m_x + 8, m_y + kPaneTop, "You", state.mine, state.mine.ready);
    DrawPane(hdc, m_x + 8 + kPaneWidth + 8, m_y + kPaneTop, partner, state.theirs, state.theirs.ready);

    const RECT readyRc = GetButtonRect(ButtonReady);
    const RECT cancelRc = GetButtonRect(ButtonCancel);
    const RECT confirmRc = GetButtonRect(ButtonConfirm);
    DrawButton(hdc, readyRc, state.mine.ready ? "Ready (set)" : "Ready", !state.mine.ready, m_pressedButtonId == ButtonReady);
    DrawButton(hdc, cancelRc, "Cancel", true, m_pressedButtonId == ButtonCancel);
    DrawButton(hdc, confirmRc, "Confirm", IsConfirmEnabled(), m_pressedButtonId == ButtonConfirm);

    ReleaseDrawTarget(hdc);

    m_lastVisualStateToken = BuildVisualStateToken();
    m_hasVisualStateToken = true;
    m_isDirty = 0;
}

RECT UITradeWnd::GetButtonRect(int buttonId) const
{
    const int totalWidth = kButtonWidth * 3 + kButtonGap * 2;
    const int x0 = m_x + (m_w - totalWidth) / 2;
    const int y0 = m_y + kButtonAreaTop;
    const int idx = (buttonId == ButtonReady) ? 0 : (buttonId == ButtonCancel) ? 1 : 2;
    const int left = x0 + idx * (kButtonWidth + kButtonGap);
    return RECT{ left, y0, left + kButtonWidth, y0 + kButtonHeight };
}

int UITradeWnd::HitTestButton(int x, int y) const
{
    auto inRect = [](const RECT& rc, int px, int py) {
        return px >= rc.left && px < rc.right && py >= rc.top && py < rc.bottom;
    };
    if (inRect(GetButtonRect(ButtonReady), x, y)) return ButtonReady;
    if (inRect(GetButtonRect(ButtonCancel), x, y)) return ButtonCancel;
    if (inRect(GetButtonRect(ButtonConfirm), x, y)) return ButtonConfirm;
    return 0;
}

bool UITradeWnd::IsConfirmEnabled() const
{
    const TRADE_STATE& state = g_session.GetTradeState();
    return state.finalConfirmAvailable && state.mine.ready && state.theirs.ready;
}

void UITradeWnd::OnLBtnDown(int x, int y)
{
    const int hit = HitTestButton(x, y);
    if (hit != 0) {
        m_pressedButtonId = hit;
        Invalidate();
        return;
    }
    m_pressedButtonId = 0;
    UIFrameWnd::OnLBtnDown(x, y);
}

void UITradeWnd::OnLBtnUp(int x, int y)
{
    const int wasPressed = m_pressedButtonId;
    m_pressedButtonId = 0;
    if (wasPressed != 0) {
        const int hit = HitTestButton(x, y);
        if (hit == wasPressed) {
            switch (hit) {
            case ButtonReady:
                if (!g_session.GetTradeState().mine.ready) {
                    SendTradeOk();
                }
                break;
            case ButtonCancel:
                SendTradeCancel();
                break;
            case ButtonConfirm:
                if (IsConfirmEnabled()) {
                    SendTradeCommit();
                }
                break;
            }
        }
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnUp(x, y);
}

bool UITradeWnd::TryAddItemFromInventory(unsigned int inventoryIndex, int quantity)
{
    if (!g_session.IsTradeActive()) {
        return false;
    }
    if (g_session.GetTradeState().mine.ready) {
        // Once we've concluded our side, the server rejects further adds.
        return false;
    }
    return SendTradeAddItem(static_cast<u16>(inventoryIndex), static_cast<u32>(quantity));
}

unsigned long long UITradeWnd::BuildVisualStateToken() const
{
    unsigned long long h = 14695981039346656037ull;
    const TRADE_STATE& s = g_session.GetTradeState();
    h = HashU64(h, s.active ? 1u : 0u);
    h = HashU64(h, s.finalConfirmAvailable ? 1u : 0u);
    h = HashStr(h, s.partnerName);
    auto hashPane = [&](const TRADE_PANE& p) {
        h = HashU64(h, static_cast<unsigned long long>(p.zeny));
        h = HashU64(h, p.ready ? 1u : 0u);
        h = HashU64(h, p.items.size());
        for (const ITEM_INFO& it : p.items) {
            h = HashU64(h, it.GetItemId());
            h = HashU64(h, static_cast<unsigned long long>(it.m_num));
            h = HashU64(h, static_cast<unsigned long long>(it.m_refiningLevel));
        }
    };
    hashPane(s.mine);
    hashPane(s.theirs);
    h = HashU64(h, static_cast<unsigned long long>(m_pressedButtonId));
    h = HashU64(h, static_cast<unsigned long long>(m_x));
    h = HashU64(h, static_cast<unsigned long long>(m_y));
    h = HashU64(h, static_cast<unsigned long long>(m_show));
    return h;
}
