#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <unordered_map>

// Seller-side vending window. Two modes driven by g_session.IsVendingActive():
//
//   Setup mode (active==false): displays the player's cart items with the
//   server-known item price as a default. Open Shop button collects every
//   non-zero-quantity row and sends CZ_REQ_OPENSTORE (0x012F). Cancel hides
//   the window without sending.
//
//   Active mode (active==true): displays current shop stock from
//   VENDING_STATE.items (kept in sync by ZC_PC_PURCHASE_MYITEMLIST 0x0136 and
//   ZC_DELETEITEM_FROMMC 0x0137). Close Shop button sends CZ_REQ_CLOSESTORE
//   (0x012E).
//
// Per-row price editing and a custom shop title field require a generic text
// input widget which the project doesn't have a clean reusable form of yet;
// the first cut uses the item's m_price from cart metadata as the listed
// price and a hardcoded "My Shop" title. Polish is a follow-up.
class UIVendingWnd : public UIFrameWnd {
public:
    UIVendingWnd();
    ~UIVendingWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    bool IsUpdateNeed() override;

    // Setter hooks called from CGameMode::SendMsg dispatch when the shared
    // UINpcInputWnd prompt submits its result.
    void SetPendingShopTitle(const char* title);
    void SetPendingItemPrice(unsigned int inventoryIndex, unsigned int price);

private:
    enum ButtonId {
        ButtonPrimary = 1,   // Open in setup, Close in active
        ButtonCancel = 2,
    };

    RECT GetButtonRect(int buttonId) const;
    int HitTestButton(int x, int y) const;
    unsigned long long BuildVisualStateToken() const;

    int m_pressedButtonId = 0;
    bool m_endedDueToInactive = false;
    unsigned long long m_lastVisualStateToken = 0;
    std::string m_pendingShopTitle;
    std::unordered_map<unsigned int, unsigned int> m_pendingPrices;  // invIndex -> price
};
