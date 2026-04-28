#include "UIVendingWnd.h"

#include "UIWindowMgr.h"
#include "UINpcInputWnd.h"
#include "gamemode/GameMode.h"
#include "item/Item.h"
#include "main/WinMain.h"
#include "session/Session.h"

#include <cstdio>
#include <string>
#include <vector>
#include <windows.h>

bool SendVendingOpen(const char* shopTitle,
                     const u16* itemIndices,
                     const u16* itemAmounts,
                     const u32* itemPrices,
                     int itemCount);
bool SendVendingClose();

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

constexpr const char* kDefaultShopTitle = "My Shop";

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

std::string ResolveItemName(unsigned int itemId, bool identified)
{
    std::string n = g_ttemmgr.GetDisplayName(itemId, identified);
    if (n.empty()) {
        char buf[24] = {};
        std::snprintf(buf, sizeof(buf), "Item #%u", itemId);
        return buf;
    }
    return n;
}

} // namespace

UIVendingWnd::UIVendingWnd() : m_pendingShopTitle(kDefaultShopTitle) {}
UIVendingWnd::~UIVendingWnd() = default;

void UIVendingWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

void UIVendingWnd::SetPendingShopTitle(const char* title)
{
    if (!title) {
        return;
    }
    m_pendingShopTitle = title;
    Invalidate();
}

void UIVendingWnd::SetPendingItemPrice(unsigned int inventoryIndex, unsigned int price)
{
    if (price == 0) {
        m_pendingPrices.erase(inventoryIndex);
    } else {
        m_pendingPrices[inventoryIndex] = price;
    }
    Invalidate();
}

bool UIVendingWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) {
        return true;
    }
    return BuildVisualStateToken() != m_lastVisualStateToken;
}

void UIVendingWnd::OnDraw()
{
    const bool active = g_session.IsVendingActive();
    const bool hasCart = !g_session.GetCartItems().empty();

    // Hide when neither setup (cart empty) nor active.
    if (!active && !hasCart) {
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

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));
    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));

    const std::string heading = active
        ? std::string("Active: ") + g_session.GetVendingState().shopTitle
        : std::string("Setup: ") + m_pendingShopTitle + "  [click here to rename]";
    DrawLabel(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), heading);

    DrawLabel(hdc, m_x + kListLeft + 4, m_y + kListTop, RGB(70, 50, 20), "Item");
    DrawLabel(hdc, m_x + kListLeft + 200, m_y + kListTop, RGB(70, 50, 20), "Qty");
    DrawLabel(hdc, m_x + kListLeft + 240, m_y + kListTop, RGB(70, 50, 20), "Price");

    int rowY = m_y + kListTop + 18;

    if (active) {
        // Render shop items from VENDING_STATE.
        const VENDING_STATE& vs = g_session.GetVendingState();
        for (size_t i = 0; i < vs.items.size(); ++i) {
            const VENDING_ITEM& item = vs.items[i];
            const RECT row{ m_x + kListLeft, rowY, m_x + m_w - kListRight, rowY + kRowHeight - 2 };
            FillSolid(hdc, row, (i & 1) ? RGB(244, 240, 226) : RGB(232, 220, 192));
            FrameSolid(hdc, row, RGB(180, 165, 120));
            DrawLabel(hdc, row.left + 4, row.top + 2, RGB(20, 20, 20),
                      ResolveItemName(item.itemId, item.identified != 0));
            char qty[16] = {};
            std::snprintf(qty, sizeof(qty), "%u", static_cast<unsigned int>(item.amount));
            DrawLabel(hdc, row.left + 200, row.top + 2, RGB(20, 20, 20), qty);
            char price[24] = {};
            std::snprintf(price, sizeof(price), "%u z", static_cast<unsigned int>(item.price));
            DrawLabel(hdc, row.left + 240, row.top + 2, RGB(160, 100, 20), price);
            rowY += kRowHeight;
            if (rowY + kRowHeight > m_y + kButtonAreaTop - 6) break;
        }
    } else {
        // Setup mode: show cart contents with item->m_price as default.
        const std::list<ITEM_INFO>& cart = g_session.GetCartItems();
        for (const ITEM_INFO& item : cart) {
            const RECT row{ m_x + kListLeft, rowY, m_x + m_w - kListRight, rowY + kRowHeight - 2 };
            FillSolid(hdc, row, RGB(244, 240, 226));
            FrameSolid(hdc, row, RGB(180, 165, 120));
            DrawLabel(hdc, row.left + 4, row.top + 2, RGB(20, 20, 20),
                      ResolveItemName(item.GetItemId(), item.m_isIdentified != 0));
            char qty[16] = {};
            std::snprintf(qty, sizeof(qty), "%d", item.m_num);
            DrawLabel(hdc, row.left + 200, row.top + 2, RGB(20, 20, 20), qty);
            char price[32] = {};
            auto pit = m_pendingPrices.find(static_cast<unsigned int>(item.m_itemIndex));
            const int finalPrice = (pit != m_pendingPrices.end())
                ? static_cast<int>(pit->second)
                : (item.m_price > 0 ? item.m_price : 1);
            std::snprintf(price, sizeof(price), "%d z [edit]", finalPrice);
            DrawLabel(hdc, row.left + 240, row.top + 2, RGB(160, 100, 20), price);
            rowY += kRowHeight;
            if (rowY + kRowHeight > m_y + kButtonAreaTop - 6) break;
        }
    }

    const RECT primaryRc = GetButtonRect(ButtonPrimary);
    const RECT cancelRc = GetButtonRect(ButtonCancel);
    DrawButton(hdc, primaryRc, active ? "Close Shop" : "Open Shop",
               true, m_pressedButtonId == ButtonPrimary);
    DrawButton(hdc, cancelRc, "Cancel", true, m_pressedButtonId == ButtonCancel);

    ReleaseDrawTarget(hdc);
    m_lastVisualStateToken = BuildVisualStateToken();
    m_isDirty = 0;
}

RECT UIVendingWnd::GetButtonRect(int buttonId) const
{
    const int totalWidth = kButtonWidth * 2 + kButtonGap;
    const int x0 = m_x + (m_w - totalWidth) / 2;
    const int y0 = m_y + kButtonAreaTop;
    const int idx = (buttonId == ButtonPrimary) ? 0 : 1;
    const int left = x0 + idx * (kButtonWidth + kButtonGap);
    return RECT{ left, y0, left + kButtonWidth, y0 + kButtonHeight };
}

int UIVendingWnd::HitTestButton(int x, int y) const
{
    auto inRect = [](const RECT& rc, int px, int py) {
        return px >= rc.left && px < rc.right && py >= rc.top && py < rc.bottom;
    };
    if (inRect(GetButtonRect(ButtonPrimary), x, y)) return ButtonPrimary;
    if (inRect(GetButtonRect(ButtonCancel), x, y)) return ButtonCancel;
    return 0;
}

void UIVendingWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != 0) {
        m_pressedButtonId = btn;
        Invalidate();
        return;
    }
    m_pressedButtonId = 0;

    if (!g_session.IsVendingActive()) {
        // Title-bar text area in setup mode opens a string prompt.
        const RECT titleRowRc{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
        if (x >= titleRowRc.left && x < titleRowRc.right
            && y >= titleRowRc.top && y < titleRowRc.bottom) {
            if (auto* inputWnd = static_cast<UINpcInputWnd*>(
                    g_windowMgr.MakeWindow(UIWindowMgr::WID_NPCINPUTWND))) {
                inputWnd->OpenGameStringPrompt(
                    "Enter shop title",
                    CGameMode::GameMsg_RequestVendingShopTitle,
                    0);
            }
            return;
        }

        // Price cell click: any cart-row hit in the price column.
        const std::list<ITEM_INFO>& cart = g_session.GetCartItems();
        int rowY = m_y + kListTop + 18;
        for (const ITEM_INFO& item : cart) {
            const RECT priceRc{ m_x + kListLeft + 240, rowY,
                                m_x + m_w - kListRight, rowY + kRowHeight - 2 };
            if (x >= priceRc.left && x < priceRc.right
                && y >= priceRc.top && y < priceRc.bottom) {
                auto pit = m_pendingPrices.find(static_cast<unsigned int>(item.m_itemIndex));
                const u32 currentPrice = (pit != m_pendingPrices.end())
                    ? pit->second
                    : static_cast<u32>(item.m_price > 0 ? item.m_price : 1);
                if (auto* inputWnd = static_cast<UINpcInputWnd*>(
                        g_windowMgr.MakeWindow(UIWindowMgr::WID_NPCINPUTWND))) {
                    inputWnd->OpenGameNumberPrompt(
                        "Enter unit price",
                        CGameMode::GameMsg_RequestVendingItemPrice,
                        static_cast<msgparam_t>(item.m_itemIndex),
                        currentPrice,
                        99000000u);  // Vending zeny cap is per-row
                }
                return;
            }
            rowY += kRowHeight;
            if (rowY + kRowHeight > m_y + kButtonAreaTop - 6) break;
        }
    }

    UIFrameWnd::OnLBtnDown(x, y);
}

void UIVendingWnd::OnLBtnUp(int x, int y)
{
    const int wasPressed = m_pressedButtonId;
    m_pressedButtonId = 0;
    if (wasPressed != 0) {
        const int hit = HitTestButton(x, y);
        if (hit == wasPressed) {
            const bool active = g_session.IsVendingActive();
            if (hit == ButtonPrimary) {
                if (active) {
                    SendVendingClose();
                } else {
                    // Setup → collect cart items, send CZ_REQ_OPENSTORE.
                    const std::list<ITEM_INFO>& cart = g_session.GetCartItems();
                    std::vector<u16> indices;
                    std::vector<u16> amounts;
                    std::vector<u32> prices;
                    for (const ITEM_INFO& it : cart) {
                        if (it.m_num <= 0) continue;
                        indices.push_back(static_cast<u16>(it.m_itemIndex));
                        amounts.push_back(static_cast<u16>(it.m_num));
                        auto pit = m_pendingPrices.find(static_cast<unsigned int>(it.m_itemIndex));
                        const u32 finalPrice = (pit != m_pendingPrices.end())
                            ? pit->second
                            : static_cast<u32>(it.m_price > 0 ? it.m_price : 1);
                        prices.push_back(finalPrice);
                    }
                    if (!indices.empty()) {
                        const std::string& title = m_pendingShopTitle.empty()
                            ? std::string(kDefaultShopTitle) : m_pendingShopTitle;
                        // Pre-set the seller state so HandleVendingOpenAck has
                        // the title to keep when it flips active=true.
                        g_session.BeginVending(title);
                        // BeginVending wipes items; the server's ZC_PC_PURCHASE_MYITEMLIST
                        // (0x0136) will repopulate. Send the open packet now.
                        SendVendingOpen(title.c_str(),
                                        indices.data(),
                                        amounts.data(),
                                        prices.data(),
                                        static_cast<int>(indices.size()));
                    }
                }
            } else if (hit == ButtonCancel) {
                if (active) {
                    SendVendingClose();
                } else {
                    SetShow(0);
                }
            }
        }
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnUp(x, y);
}

unsigned long long UIVendingWnd::BuildVisualStateToken() const
{
    unsigned long long h = 14695981039346656037ull;
    const VENDING_STATE& s = g_session.GetVendingState();
    h = HashU64(h, s.active ? 1u : 0u);
    h = HashStr(h, s.shopTitle);
    h = HashU64(h, s.items.size());
    for (const VENDING_ITEM& it : s.items) {
        h = HashU64(h, it.itemId);
        h = HashU64(h, it.amount);
        h = HashU64(h, it.price);
    }
    if (!s.active) {
        const std::list<ITEM_INFO>& cart = g_session.GetCartItems();
        h = HashU64(h, cart.size());
        for (const ITEM_INFO& it : cart) {
            h = HashU64(h, it.GetItemId());
            h = HashU64(h, static_cast<unsigned long long>(it.m_num));
            h = HashU64(h, static_cast<unsigned long long>(it.m_price));
        }
    }
    h = HashStr(h, m_pendingShopTitle);
    h = HashU64(h, m_pendingPrices.size());
    for (const auto& kv : m_pendingPrices) {
        h = HashU64(h, kv.first);
        h = HashU64(h, kv.second);
    }
    h = HashU64(h, static_cast<unsigned long long>(m_pressedButtonId));
    h = HashU64(h, static_cast<unsigned long long>(m_x));
    h = HashU64(h, static_cast<unsigned long long>(m_y));
    h = HashU64(h, static_cast<unsigned long long>(m_show));
    return h;
}
