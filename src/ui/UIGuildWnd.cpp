#include "UIGuildWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

UIGuildWnd::UIGuildWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("GuildWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIGuildWnd::~UIGuildWnd() = default;

void UIGuildWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Guild)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UIGuildWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) {
        return;
    }
}

void UIGuildWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIGuildWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{};
    minimize.id = 0;
    minimize.x = m_x + m_w - 34;
    minimize.y = m_y + 3;
    out->push_back(minimize);
    SystemButton close{};
    close.id = 1;
    close.x = m_x + m_w - 17;
    close.y = m_y + 3;
    out->push_back(close);
}

void UIGuildWnd::OnLBtnDown(int x, int y)
{
    if (m_show == 0) {
        return;
    }
    std::vector<SystemButton> buttons;
    BuildSystemButtons(&buttons);
    for (const SystemButton& b : buttons) {
        if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) {
            if (b.id == 1) {
                SetShow(0);
            } else if (b.id == 0) {
                ToggleMinimized();
            }
            return;
        }
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIGuildWnd::StoreInfo()
{
    SaveUiWindowPlacement("GuildWnd", m_x, m_y);
}

msgresult_t UIGuildWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

void UIGuildWnd::SetActiveTab(int tab)
{
    if (tab < 0 || tab >= TabCount) {
        return;
    }
    m_activeTab = tab;
    Invalidate();
}

bool UIGuildWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) {
        return false;
    }
    outData->title = "Guild";
    outData->guildName = g_session.m_guildName;
    outData->masterName = g_session.m_guildMasterName;
    outData->guildId = g_session.m_guildId;
    outData->emblemId = g_session.m_guildEmblemId;
    outData->activeTab = m_activeTab;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
