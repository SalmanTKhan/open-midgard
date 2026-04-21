#pragma once

#include "UIFrameWnd.h"
#include "UIShopCommon.h"
#include "item/Item.h"

#include <unordered_map>

class UIItemShopWnd : public UIFrameWnd {
public:
    UIItemShopWnd();
    ~UIItemShopWnd() override;

    void SetShow(int show) override;
    bool IsUpdateNeed() override;
    void StoreInfo() override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnDblClk(int x, int y) override;
    void OnRBtnDown(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnWheel(int delta) override;
    void HandleKeyDown(int virtualKey);
    int GetViewOffset() const;
    int GetHoverRow() const;
    int GetVisibleRowCountForQt() const;

private:
    void PromptShopAmount(int rowIndex);
    int GetVisibleRowCount() const;
    int GetMaxViewOffset() const;
    RECT GetListRect() const;
    int HitTestSourceRow(int x, int y) const;
    const shopui::BitmapPixels* GetItemIcon(const ITEM_INFO& item);
    unsigned long long BuildDisplayStateToken() const;

    int m_viewOffset;
    int m_hoverRow;
    std::unordered_map<unsigned int, shopui::BitmapPixels> m_iconCache;
    unsigned long long m_lastDrawStateToken;
    bool m_hasDrawStateToken;
};
