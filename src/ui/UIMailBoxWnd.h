#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIMailBoxWnd : public UIFrameWnd {
public:
    struct SystemButton { int id = 0; int x = 0; int y = 0; int w = 13; int h = 13; };

    struct DisplayEntry {
        unsigned int mailId = 0;
        std::string title;
        std::string sender;
        bool isRead = false;
    };

    struct DisplayData {
        std::string title;
        std::vector<DisplayEntry> entries;
        int selectedIndex = -1;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIMailBoxWnd();
    ~UIMailBoxWnd() override;

    void SetShow(int show) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void StoreInfo() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

    void SetSelectedIndex(int index) { m_selectedIndex = index; }
    int GetSelectedIndex() const { return m_selectedIndex; }
    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    void ToggleMinimized();
    void BuildSystemButtons(std::vector<SystemButton>* out) const;

    int m_selectedIndex = -1;
    bool m_minimized = false;
    static constexpr int kTitleHeight = 18;
    static constexpr int kFullWidth = 320;
    static constexpr int kFullHeight = 240;
    static constexpr int kDefaultX = 260;
    static constexpr int kDefaultY = 90;
};
