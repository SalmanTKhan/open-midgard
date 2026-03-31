#pragma once

#include "UIFrameWnd.h"

class UIChooseSellBuyWnd : public UIFrameWnd {
public:
    enum ButtonId {
        ButtonNone = -1,
        ButtonBuy = 0,
        ButtonSell = 1,
        ButtonCancel = 2,
    };

    UIChooseSellBuyWnd();
    ~UIChooseSellBuyWnd() override;

    void SetShow(int show) override;
    bool IsUpdateNeed() override;
    void StoreInfo() override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void HandleKeyDown(int virtualKey);
    ButtonId GetHoverButton() const;
    ButtonId GetPressedButton() const;

private:
    RECT GetButtonRect(ButtonId buttonId) const;
    ButtonId HitTestButton(int x, int y) const;
    void ActivateButton(ButtonId buttonId);
    unsigned long long BuildDisplayStateToken() const;

    ButtonId m_hoverButton;
    ButtonId m_pressedButton;
    unsigned long long m_lastDrawStateToken;
    bool m_hasDrawStateToken;
};
