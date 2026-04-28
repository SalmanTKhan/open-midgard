#include "UIVendingShopWnd.h"

#include "UIWindowMgr.h"
#include "item/Item.h"
#include "main/WinMain.h"
#include "session/Session.h"

#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>

bool SendVendingPurchase(u32 sellerAid,
                         const u16* itemAmounts,
                         const u16* itemIndices,
                         int pickCount);

namespace {

constexpr int kFullWidth = 380;
constexpr int kFullHeight = 320;
constexpr int kTitleBarHeight = 17;
constexpr int kListTop = 26;
constexpr int kListLeft = 8;
constexpr int kListRight = 8;
constexpr int kRowHeight = 18;
constexpr int kButtonAreaTop = 286;
constexpr int kButtonWidth = 110;
constexpr int kButtonHeight = 22;
constexpr int kButtonGap = 12;

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

void DrawLabel(HDC hdc, int x, int y, COLORREF color, const std::string& text,
               UINT format = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) {
        return;
    }
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, format);
}

void DrawButton(HDC hdc, const RECT& rc, const char* label, bool enabled, bool pressed)
{
    const COLORREF fill = enabled
        ? (pressed ? RGB(170, 145, 75) : RGB(210, 195, 145))
        : RGB(170, 170, 170);
    FillSolid(hdc, rc, fill);
    FrameSolid(hdc, rc, RGB(80, 60, 30));
    DrawLabel(hdc, rc.left + 8, rc.top + 5, enabled ? RGB(20, 20, 20) : RGB(90, 90, 90), label);
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

std::string ResolveItemName(const VENDING_ITEM& item)
{
    std::string n = g_ttemmgr.GetDisplayName(item.itemId, item.identified != 0);
    if (n.empty()) {
        char buf[24] = {};
        std::snprintf(buf, sizeof(buf), "Item #%u", static_cast<unsigned int>(item.itemId));
        return buf;
    }
    return n;
}

} // namespace

UIVendingShopWnd::UIVendingShopWnd() = default;
UIVendingShopWnd::~UIVendingShopWnd() = default;

void UIVendingShopWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

bool UIVendingShopWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) {
        return true;
    }
    return BuildVisualStateToken() != m_lastVisualStateToken;
}

void UIVendingShopWnd::OnDraw()
{
    if (!g_session.IsVendingBrowseActive()) {
        if (!m_endedDueToInactive) {
            m_endedDueToInactive = true;
            SetShow(0);
            m_selected.clear();
            m_lastBrowseSessionToken = 0;
        }
        return;
    }
    m_endedDueToInactive = false;

    const VENDING_SHOP_BROWSE_STATE& state = g_session.GetVendingBrowseState();
    // Reset selection vector when the partner changes.
    const int sessionToken = static_cast<int>(state.partnerAid)
        ^ static_cast<int>(state.items.size() << 16);
    if (sessionToken != m_lastBrowseSessionToken
        || m_selected.size() != state.items.size()) {
        m_selected.assign(state.items.size(), false);
        m_lastBrowseSessionToken = sessionToken;
    }

    if (!g_hMainWnd || m_show == 0) {
        return;
    }
    HDC hdc = AcquireDrawTarget();
    if (!hdc) {
        return;
    }

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));
    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    const std::string heading = std::string("Shop: ")
        + (state.partnerShopTitle.empty() ? std::string("(unnamed)") : state.partnerShopTitle);
    DrawLabel(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), heading);

    // Column headers
    DrawLabel(hdc, m_x + kListLeft + 4, m_y + kListTop, RGB(70, 50, 20), "Item");
    DrawLabel(hdc, m_x + kListLeft + 200, m_y + kListTop, RGB(70, 50, 20), "Qty");
    DrawLabel(hdc, m_x + kListLeft + 240, m_y + kListTop, RGB(70, 50, 20), "Price");

    int rowY = m_y + kListTop + 18;
    for (size_t i = 0; i < state.items.size(); ++i) {
        const VENDING_ITEM& item = state.items[i];
        const RECT row{ m_x + kListLeft, rowY, m_x + m_w - kListRight, rowY + kRowHeight - 2 };
        const COLORREF rowFill = m_selected[i]
            ? RGB(220, 230, 200)
            : ((i & 1) ? RGB(244, 240, 226) : RGB(232, 220, 192));
        FillSolid(hdc, row, rowFill);
        FrameSolid(hdc, row, RGB(180, 165, 120));

        const std::string name = ResolveItemName(item);
        DrawLabel(hdc, row.left + 4, row.top + 2, RGB(20, 20, 20), name);
        char qtyText[16] = {};
        std::snprintf(qtyText, sizeof(qtyText), "%u", static_cast<unsigned int>(item.amount));
        DrawLabel(hdc, row.left + 200, row.top + 2, RGB(20, 20, 20), qtyText);
        char priceText[24] = {};
        std::snprintf(priceText, sizeof(priceText), "%u z", static_cast<unsigned int>(item.price));
        DrawLabel(hdc, row.left + 240, row.top + 2, RGB(160, 100, 20), priceText);

        rowY += kRowHeight;
        if (rowY + kRowHeight > m_y + kButtonAreaTop - 6) {
            break;
        }
    }

    // Total + buttons
    char totalText[64] = {};
    std::snprintf(totalText, sizeof(totalText), "Total: %lld z",
                  static_cast<long long>(ComputeTotalZeny()));
    DrawLabel(hdc, m_x + kListLeft, m_y + kButtonAreaTop - 18, RGB(160, 100, 20), totalText);

    const RECT buyRc = GetButtonRect(ButtonBuy);
    const RECT closeRc = GetButtonRect(ButtonClose);
    const bool hasSelection = ComputeTotalZeny() > 0;
    DrawButton(hdc, buyRc, "Buy Selected", hasSelection, m_pressedButtonId == ButtonBuy);
    DrawButton(hdc, closeRc, "Close", true, m_pressedButtonId == ButtonClose);

    ReleaseDrawTarget(hdc);
    m_lastVisualStateToken = BuildVisualStateToken();
    m_isDirty = 0;
}

int UIVendingShopWnd::RowIndexAt(int x, int y) const
{
    const VENDING_SHOP_BROWSE_STATE& state = g_session.GetVendingBrowseState();
    const int rowsTop = m_y + kListTop + 18;
    if (x < m_x + kListLeft || x >= m_x + m_w - kListRight) {
        return -1;
    }
    if (y < rowsTop) {
        return -1;
    }
    const int idx = (y - rowsTop) / kRowHeight;
    if (idx < 0 || idx >= static_cast<int>(state.items.size())) {
        return -1;
    }
    return idx;
}

RECT UIVendingShopWnd::GetRowRect(int rowIndex) const
{
    const int rowsTop = m_y + kListTop + 18;
    return RECT{
        m_x + kListLeft,
        rowsTop + rowIndex * kRowHeight,
        m_x + m_w - kListRight,
        rowsTop + rowIndex * kRowHeight + kRowHeight - 2,
    };
}

RECT UIVendingShopWnd::GetButtonRect(int buttonId) const
{
    const int totalWidth = kButtonWidth * 2 + kButtonGap;
    const int x0 = m_x + (m_w - totalWidth) / 2;
    const int y0 = m_y + kButtonAreaTop;
    const int idx = (buttonId == ButtonBuy) ? 0 : 1;
    const int left = x0 + idx * (kButtonWidth + kButtonGap);
    return RECT{ left, y0, left + kButtonWidth, y0 + kButtonHeight };
}

int UIVendingShopWnd::HitTestButton(int x, int y) const
{
    auto inRect = [](const RECT& rc, int px, int py) {
        return px >= rc.left && px < rc.right && py >= rc.top && py < rc.bottom;
    };
    if (inRect(GetButtonRect(ButtonBuy), x, y)) return ButtonBuy;
    if (inRect(GetButtonRect(ButtonClose), x, y)) return ButtonClose;
    return 0;
}

long long UIVendingShopWnd::ComputeTotalZeny() const
{
    const VENDING_SHOP_BROWSE_STATE& state = g_session.GetVendingBrowseState();
    long long total = 0;
    for (size_t i = 0; i < state.items.size() && i < m_selected.size(); ++i) {
        if (m_selected[i]) {
            total += static_cast<long long>(state.items[i].amount)
                   * static_cast<long long>(state.items[i].price);
        }
    }
    return total;
}

void UIVendingShopWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != 0) {
        m_pressedButtonId = btn;
        Invalidate();
        return;
    }
    const int row = RowIndexAt(x, y);
    if (row >= 0 && row < static_cast<int>(m_selected.size())) {
        m_selected[row] = !m_selected[row];
        Invalidate();
        return;
    }
    m_pressedButtonId = 0;
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIVendingShopWnd::OnLBtnUp(int x, int y)
{
    const int wasPressed = m_pressedButtonId;
    m_pressedButtonId = 0;
    if (wasPressed != 0) {
        const int hit = HitTestButton(x, y);
        if (hit == wasPressed) {
            switch (hit) {
            case ButtonBuy: {
                const VENDING_SHOP_BROWSE_STATE& state = g_session.GetVendingBrowseState();
                std::vector<u16> indices;
                std::vector<u16> amounts;
                for (size_t i = 0; i < state.items.size() && i < m_selected.size(); ++i) {
                    if (m_selected[i]) {
                        indices.push_back(state.items[i].inventoryIndex);
                        amounts.push_back(static_cast<u16>(state.items[i].amount));
                    }
                }
                if (!indices.empty()) {
                    SendVendingPurchase(state.partnerAid,
                                        amounts.data(),
                                        indices.data(),
                                        static_cast<int>(indices.size()));
                }
                break;
            }
            case ButtonClose:
                g_session.EndVendingBrowse();
                break;
            }
        }
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnUp(x, y);
}

unsigned long long UIVendingShopWnd::BuildVisualStateToken() const
{
    unsigned long long h = 14695981039346656037ull;
    const VENDING_SHOP_BROWSE_STATE& s = g_session.GetVendingBrowseState();
    h = HashU64(h, s.active ? 1u : 0u);
    h = HashU64(h, s.partnerAid);
    h = HashStr(h, s.partnerShopTitle);
    h = HashU64(h, s.items.size());
    for (size_t i = 0; i < s.items.size(); ++i) {
        const VENDING_ITEM& it = s.items[i];
        h = HashU64(h, it.itemId);
        h = HashU64(h, it.amount);
        h = HashU64(h, it.price);
        h = HashU64(h, it.inventoryIndex);
        const bool sel = i < m_selected.size() && m_selected[i];
        h = HashU64(h, sel ? 1u : 0u);
    }
    h = HashU64(h, static_cast<unsigned long long>(m_pressedButtonId));
    h = HashU64(h, static_cast<unsigned long long>(m_x));
    h = HashU64(h, static_cast<unsigned long long>(m_y));
    h = HashU64(h, static_cast<unsigned long long>(m_show));
    return h;
}
