#pragma once

#include "UIFrameWnd.h"

class UISelectCartWnd : public UIFrameWnd {
public:
    UISelectCartWnd();
    ~UISelectCartWnd() override;

    void SetShow(int show) override;
    void OnCreate(int cx, int cy) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnKeyDown(int virtualKey) override;
    void StoreInfo() override;

    int GetVisibleOptionCount() const;
    int GetHoverIndex() const;
    int GetPressedIndex() const;
    RECT GetOptionRectForRender(int index) const;

private:
    void EnsureCreated();
    int GetAvailableCartCount() const;
    RECT GetOptionRect(int index) const;
    RECT GetCloseRect() const;
    int HitTestOption(int x, int y) const;
    bool HitTestClose(int x, int y) const;
    void CloseWindow();
    bool ActivateOption(int index);

    bool m_controlsCreated;
    int m_hoveredIndex;
    int m_pressedIndex;
};