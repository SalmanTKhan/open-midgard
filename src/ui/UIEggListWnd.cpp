#include "UIEggListWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

UIEggListWnd::UIEggListWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("EggListWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIEggListWnd::~UIEggListWnd() = default;

void UIEggListWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Pet)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UIEggListWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) return;
}

void UIEggListWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIEggListWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{}; close.id = 1; close.x = m_x + m_w - 17; close.y = m_y + 3; out->push_back(close);
}

void UIEggListWnd::OnLBtnDown(int x, int y)
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

void UIEggListWnd::StoreInfo()
{
    SaveUiWindowPlacement("EggListWnd", m_x, m_y);
}

msgresult_t UIEggListWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UIEggListWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) return false;
    outData->title = "Pet Eggs";
    outData->eggItemIds = g_session.m_petEggList;
    outData->selectedIndex = m_selectedIndex;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
