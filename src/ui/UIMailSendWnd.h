#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIMailSendWnd : public UIFrameWnd {
public:
    struct SystemButton { int id = 0; int x = 0; int y = 0; int w = 13; int h = 13; };

    struct DisplayData {
        std::string title;
        std::string recipient;
        std::string subject;
        std::string body;
        unsigned int zeny = 0;
        unsigned int attachInventoryIndex = 0;
        int attachAmount = 0;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIMailSendWnd();
    ~UIMailSendWnd() override;

    void SetShow(int show) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void StoreInfo() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

    void SetRecipient(const std::string& name) { m_recipient = name; }
    void SetSubject(const std::string& subject) { m_subject = subject; }
    void SetBody(const std::string& body) { m_body = body; }
    void SetZeny(unsigned int zeny) { m_zeny = zeny; }
    void SetAttach(unsigned int inventoryIndex, int amount) { m_attachInventoryIndex = inventoryIndex; m_attachAmount = amount; }
    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    void ToggleMinimized();
    void BuildSystemButtons(std::vector<SystemButton>* out) const;

    std::string m_recipient;
    std::string m_subject;
    std::string m_body;
    unsigned int m_zeny = 0;
    unsigned int m_attachInventoryIndex = 0;
    int m_attachAmount = 0;
    bool m_minimized = false;
    static constexpr int kTitleHeight = 18;
    static constexpr int kFullWidth = 320;
    static constexpr int kFullHeight = 280;
    static constexpr int kDefaultX = 340;
    static constexpr int kDefaultY = 130;
};
