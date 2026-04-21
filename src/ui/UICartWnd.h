#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

struct ITEM_INFO;

class UICartWnd : public UIFrameWnd {
public:
    struct SystemButton {
        int id = 0; // 0 = minimize, 1 = close
        int x = 0;
        int y = 0;
        int w = 13;
        int h = 13;
    };

    struct DisplaySlot {
        unsigned int itemIndex = 0;
        unsigned int itemId = 0;
        int count = 0;
        std::string label;
    };

    struct DisplayData {
        std::string title;
        int currentCount = 0;
        int maxCount = 0;
        int currentWeight = 0;
        int maxWeight = 0;
        bool minimized = false;
        std::vector<DisplaySlot> entries;
        std::vector<SystemButton> systemButtons;
    };

    UICartWnd();
    ~UICartWnd() override;

    void SetShow(int show) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void StoreInfo() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    void ToggleMinimized();
    void BuildSystemButtons(std::vector<SystemButton>* out) const;

    bool m_minimized = false;
    static constexpr int kTitleHeight = 18;
    static constexpr int kFullWidth = 180;
    static constexpr int kFullHeight = 240;
    static constexpr int kDefaultX = 140;
    static constexpr int kDefaultY = 320;
};
