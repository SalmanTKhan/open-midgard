#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIMercInfoWnd : public UIFrameWnd {
public:
    struct SystemButton { int id = 0; int x = 0; int y = 0; int w = 13; int h = 13; };

    struct DisplayData {
        std::string title;
        std::string mercName;
        int level = 0;
        int hp = 0;
        int maxHp = 0;
        int sp = 0;
        int maxSp = 0;
        int faith = 0;
        int calls = 0;
        unsigned int expireTime = 0;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIMercInfoWnd();
    ~UIMercInfoWnd() override;

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
    static constexpr int kFullWidth = 220;
    static constexpr int kFullHeight = 220;
    static constexpr int kDefaultX = 400;
    static constexpr int kDefaultY = 60;
};
