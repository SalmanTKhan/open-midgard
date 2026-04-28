#pragma once
#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIGuildWnd : public UIFrameWnd {
public:
    enum Tab {
        TabInfo = 0,
        TabMembers,
        TabPosition,
        TabSkill,
        TabBanish,
        TabNotice,
        TabAlliance,
        TabCount
    };

    struct SystemButton {
        int id = 0;
        int x = 0;
        int y = 0;
        int w = 13;
        int h = 13;
    };

    struct DisplayData {
        std::string title;
        std::string guildName;
        std::string masterName;
        int guildId = 0;
        int emblemId = 0;
        int activeTab = 0;
        bool minimized = false;
        std::vector<SystemButton> systemButtons;
    };

    UIGuildWnd();
    ~UIGuildWnd() override;

    void SetShow(int show) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void StoreInfo() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

    void SetActiveTab(int tab);
    int GetActiveTab() const { return m_activeTab; }
    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    void ToggleMinimized();
    void BuildSystemButtons(std::vector<SystemButton>* out) const;

    enum ActionButtonId {
        ButtonLeaveGuild = 1,
        ButtonExpelMember = 2,
        ButtonEditNotice = 3,
    };

public:
    void SetPendingNoticeSubject(const char* s);
    void SetPendingNoticeBody(const char* s);
    const std::string& GetPendingNoticeSubject() const { return m_pendingNoticeSubject; }
    const std::string& GetPendingNoticeBody() const { return m_pendingNoticeBody; }
private:

    struct InteractRect { int x; int y; int w; int h; };
    InteractRect GetTabRect(int tabIndex) const;
    InteractRect GetMemberRowRect(int rowIndex) const;
    InteractRect GetActionButtonRect(int buttonId) const;
    int HitTestTab(int x, int y) const;
    int HitTestMemberRow(int x, int y) const;
    int HitTestActionButton(int x, int y) const;

    int m_activeTab = TabInfo;
    bool m_minimized = false;
    int m_selectedMemberIndex = -1;
    std::string m_pendingNoticeSubject;
    std::string m_pendingNoticeBody;
    static constexpr int kTitleHeight = 18;
    static constexpr int kFullWidth = 300;
    static constexpr int kFullHeight = 280;
    static constexpr int kDefaultX = 220;
    static constexpr int kDefaultY = 60;
};
