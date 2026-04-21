#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIMailReadWnd : public UIFrameWnd {
public:
    struct SystemButton { int id = 0; int x = 0; int y = 0; int w = 13; int h = 13; };

    struct DisplayData {
        std::string title;
        std::string subject;
        std::string sender;
        std::string body;
        unsigned int mailId = 0;
        unsigned int zeny = 0;
        unsigned int attachItemId = 0;
        int attachAmount = 0;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIMailReadWnd();
    ~UIMailReadWnd() override;

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
    static constexpr int kFullWidth = 320;
    static constexpr int kFullHeight = 280;
    static constexpr int kDefaultX = 300;
    static constexpr int kDefaultY = 110;
};
