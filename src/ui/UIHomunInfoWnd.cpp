#include "UIHomunInfoWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

UIHomunInfoWnd::UIHomunInfoWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("HomunInfoWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIHomunInfoWnd::~UIHomunInfoWnd() = default;

void UIHomunInfoWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Homunculus)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UIHomunInfoWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) return;
}

void UIHomunInfoWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIHomunInfoWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{}; close.id = 1; close.x = m_x + m_w - 17; close.y = m_y + 3; out->push_back(close);
}

void UIHomunInfoWnd::OnLBtnDown(int x, int y)
{
    if (m_show == 0) return;
    std::vector<SystemButton> buttons;
    BuildSystemButtons(&buttons);
    for (const SystemButton& b : buttons) {
        if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) {
            if (b.id == 1) SetShow(0);
            else if (b.id == 0) ToggleMinimized();
            return;
        }
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIHomunInfoWnd::StoreInfo()
{
    SaveUiWindowPlacement("HomunInfoWnd", m_x, m_y);
}

msgresult_t UIHomunInfoWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UIHomunInfoWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) return false;
    if (!g_session.IsHomunActive()) return false;
    outData->title = "Homunculus";
    outData->homunName = g_session.m_homunName;
    outData->level = g_session.m_homunLevel;
    outData->hp = g_session.m_homunHp;
    outData->maxHp = g_session.m_homunMaxHp;
    outData->sp = g_session.m_homunSp;
    outData->maxSp = g_session.m_homunMaxSp;
    outData->hunger = g_session.m_homunHunger;
    outData->intimacy = g_session.m_homunIntimacy;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
