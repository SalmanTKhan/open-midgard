#include "UICartWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

#include <cstdio>

UICartWnd::UICartWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("CartWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UICartWnd::~UICartWnd() = default;

void UICartWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Cart)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UICartWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) {
        return;
    }
}

void UICartWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UICartWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{};
    minimize.id = 0;
    minimize.x = m_x + m_w - 34;
    minimize.y = m_y + 3;
    minimize.w = 13;
    minimize.h = 13;
    out->push_back(minimize);
    SystemButton close{};
    close.id = 1;
    close.x = m_x + m_w - 17;
    close.y = m_y + 3;
    close.w = 13;
    close.h = 13;
    out->push_back(close);
}

void UICartWnd::OnLBtnDown(int x, int y)
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

void UICartWnd::StoreInfo()
{
    SaveUiWindowPlacement("CartWnd", m_x, m_y);
}

msgresult_t UICartWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UICartWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) {
        return false;
    }
    if (!g_session.IsCartActive()) {
        return false;
    }
    outData->title = "Cart";
    outData->currentCount = g_session.GetCartCurrentCount();
    outData->maxCount = g_session.GetCartMaxCount();
    outData->currentWeight = g_session.GetCartCurrentWeight();
    outData->maxWeight = g_session.GetCartMaxWeight();
    outData->minimized = m_minimized;
    outData->entries.clear();
    for (const ITEM_INFO& item : g_session.GetCartItems()) {
        DisplaySlot slot;
        slot.itemIndex = item.m_itemIndex;
        slot.itemId = item.GetItemId();
        slot.count = item.m_num;
        char buf[64];
        std::snprintf(buf, sizeof(buf), "#%u x%d", slot.itemId, slot.count);
        slot.label = buf;
        outData->entries.push_back(slot);
    }
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
