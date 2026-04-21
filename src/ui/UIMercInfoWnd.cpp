#include "UIMercInfoWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

UIMercInfoWnd::UIMercInfoWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("MercInfoWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIMercInfoWnd::~UIMercInfoWnd() = default;

void UIMercInfoWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Mercenary)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UIMercInfoWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) return;
}

void UIMercInfoWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIMercInfoWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{}; close.id = 1; close.x = m_x + m_w - 17; close.y = m_y + 3; out->push_back(close);
}

void UIMercInfoWnd::OnLBtnDown(int x, int y)
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

void UIMercInfoWnd::StoreInfo()
{
    SaveUiWindowPlacement("MercInfoWnd", m_x, m_y);
}

msgresult_t UIMercInfoWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UIMercInfoWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) return false;
    if (!g_session.IsMercActive()) return false;
    outData->title = "Mercenary";
    outData->mercName = g_session.m_mercName;
    outData->level = g_session.m_mercLevel;
    outData->hp = g_session.m_mercHp;
    outData->maxHp = g_session.m_mercMaxHp;
    outData->sp = g_session.m_mercSp;
    outData->maxSp = g_session.m_mercMaxSp;
    outData->faith = g_session.m_mercFaith;
    outData->calls = g_session.m_mercCalls;
    outData->expireTime = g_session.m_mercExpireTime;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
