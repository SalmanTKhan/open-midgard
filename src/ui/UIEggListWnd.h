#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIEggListWnd : public UIFrameWnd {
public:
    struct SystemButton { int id = 0; int x = 0; int y = 0; int w = 13; int h = 13; };

    struct DisplayData {
        std::string title;
        std::vector<int> eggItemIds;
        int selectedIndex = -1;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIEggListWnd();
    ~UIEggListWnd() override;

    void SetShow(int show) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void StoreInfo() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

    void SetSelectedIndex(int index) { m_selectedIndex = index; }
    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    void ToggleMinimized();
    void BuildSystemButtons(std::vector<SystemButton>* out) const;
    int HitTestRow(int x, int y) const;

    int m_selectedIndex = -1;
    bool m_minimized = false;
    static constexpr int kRowTop = 24;
    static constexpr int kRowHeight = 22;
    static constexpr int kTitleHeight = 18;
    static constexpr int kFullWidth = 200;
    static constexpr int kFullHeight = 240;
    static constexpr int kDefaultX = 430;
    static constexpr int kDefaultY = 90;
};
