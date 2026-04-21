#include "UIMailReadWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

UIMailReadWnd::UIMailReadWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("MailReadWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIMailReadWnd::~UIMailReadWnd() = default;

void UIMailReadWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::MailLegacy)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

void UIMailReadWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) return;
}

void UIMailReadWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIMailReadWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{}; close.id = 1; close.x = m_x + m_w - 17; close.y = m_y + 3; out->push_back(close);
}

void UIMailReadWnd::OnLBtnDown(int x, int y)
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

void UIMailReadWnd::StoreInfo()
{
    SaveUiWindowPlacement("MailReadWnd", m_x, m_y);
}

msgresult_t UIMailReadWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UIMailReadWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) return false;
    const MAIL_BODY& body = g_session.GetMailReadBody();
    outData->title = "Read Mail";
    outData->subject = body.title;
    outData->sender = body.sender;
    outData->body = body.body;
    outData->mailId = body.mailId;
    outData->zeny = body.zeny;
    outData->attachItemId = body.attachItemId;
    outData->attachAmount = static_cast<int>(body.attachAmount);
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
