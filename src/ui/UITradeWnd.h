#pragma once

#include "UIFrameWnd.h"

// Two-pane player-to-player trade window. Reads its state directly from
// g_session.GetTradeState(); the window is opened by HandleDealRequestResponse
// (result==3, both sides accepted) and closed by ZC_CANCEL_EXCHANGE_ITEM /
// ZC_EXEC_EXCHANGE_ITEM packet handlers via g_session.EndTrade().
//
// Buttons: Ready, Cancel, Confirm (only enabled once both sides hit Ready).
// Add-item is driven externally — UIItemWnd or the player context menu hands
// in an inventory index via TryAddItemFromInventory().
class UITradeWnd : public UIFrameWnd {
public:
    UITradeWnd();
    ~UITradeWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    bool IsUpdateNeed() override;

    bool TryAddItemFromInventory(unsigned int inventoryIndex, int quantity);

private:
    enum ButtonId {
        ButtonReady = 1,
        ButtonCancel = 2,
        ButtonConfirm = 3,
    };

    RECT GetButtonRect(int buttonId) const;
    int HitTestButton(int x, int y) const;
    bool IsConfirmEnabled() const;
    unsigned long long BuildVisualStateToken() const;

    int m_pressedButtonId = 0;
    bool m_endedDueToInactive = false;
    unsigned long long m_lastVisualStateToken = 0;
    bool m_hasVisualStateToken = false;
};
