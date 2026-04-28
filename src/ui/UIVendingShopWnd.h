#pragma once

#include "UIFrameWnd.h"

#include <vector>

// Buyer-side vending window. Reads g_session.GetVendingBrowseState() and
// auto-hides when no browse is active. Each row in the list represents an
// item the seller has on offer. Clicking a row toggles whether the buyer
// wants to purchase that row's full listed amount; the Buy button sends a
// CZ_PC_PURCHASE_ITEMLIST_FROMMC (0x0134) carrying every selected row.
//
// Partial-quantity prompts are out of scope for the first cut: a row is
// either purchased at the full listed amount or not at all.
class UIVendingShopWnd : public UIFrameWnd {
public:
    UIVendingShopWnd();
    ~UIVendingShopWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    bool IsUpdateNeed() override;

private:
    enum ButtonId {
        ButtonBuy = 1,
        ButtonClose = 2,
    };

    int RowIndexAt(int x, int y) const;
    RECT GetRowRect(int rowIndex) const;
    RECT GetButtonRect(int buttonId) const;
    int HitTestButton(int x, int y) const;
    unsigned long long BuildVisualStateToken() const;
    long long ComputeTotalZeny() const;

    int m_pressedButtonId = 0;
    bool m_endedDueToInactive = false;
    std::vector<bool> m_selected;       // parallel to GetVendingBrowseState().items
    int m_lastBrowseSessionToken = 0;   // resets m_selected on shop change
    unsigned long long m_lastVisualStateToken = 0;
};
