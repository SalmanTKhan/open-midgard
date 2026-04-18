#pragma once

#include "item/Item.h"
#include "UIFrameWnd.h"
#include "UIShopCommon.h"

#include <vector>

class UIItemCollectionWnd : public UIFrameWnd {
public:
    struct DisplayButton {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool hovered = false;
        bool pressed = false;
        std::string label;
    };

    struct DisplayTile {
        unsigned int itemId = 0;
        bool usesCollection = false;
    };

    struct DisplayData {
        std::string title;
        unsigned int itemId = 0;
        bool mainUsesIllust = false;
        bool mainUsesCollection = false;
        DisplayButton closeButton;
    };

    UIItemCollectionWnd();
    ~UIItemCollectionWnd() override = default;

    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseHover(int x, int y) override;
    void StoreInfo() override;
    void SetItemInfo(const ITEM_INFO& item, int preferredX, int preferredY);
    bool GetDisplayDataForQt(DisplayData* outData) const;
    bool HasItem() const;

private:
    RECT GetCloseButtonRect() const;
    void UpdateInteraction(int x, int y);
    void RefreshBitmaps();

    ITEM_INFO m_item;
    bool m_hasItem;
    bool m_closeHovered;
    bool m_closePressed;
    bool m_hasStoredPlacement;
    bool m_mainUsesIllust;
    bool m_mainUsesCollection;
    shopui::BitmapPixels m_mainBitmap;
};