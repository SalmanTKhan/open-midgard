#include "UIEggListWnd.h"

#include "UIWindowMgr.h"
#include "core/ClientFeature.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

#include <cstdio>
#include <windows.h>

bool RequestPetSelectEgg(unsigned short inventoryIndex);

namespace {

void FillSolid(HDC hdc, const RECT& rc, COLORREF c)
{
    HBRUSH b = CreateSolidBrush(c);
    FillRect(hdc, &rc, b);
    DeleteObject(b);
}

void FrameSolid(HDC hdc, const RECT& rc, COLORREF c)
{
    HBRUSH b = CreateSolidBrush(c);
    FrameRect(hdc, &rc, b);
    DeleteObject(b);
}

void DrawText(HDC hdc, int x, int y, COLORREF color, const std::string& s)
{
    if (s.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT r{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, s.c_str(), static_cast<int>(s.size()), &r,
                DT_LEFT | DT_TOP | DT_SINGLELINE);
}

}  // namespace

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
    if (m_show == 0) return;
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));
    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    DrawText(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "Pet Eggs");

    if (m_minimized) {
        ReleaseDrawTarget(hdc);
        m_isDirty = 0;
        return;
    }

    const auto& eggs = g_session.m_petEggList;
    if (eggs.empty()) {
        DrawText(hdc, m_x + 12, m_y + kRowTop + 4, RGB(120, 90, 30),
                 "(No eggs in inventory.)");
    } else {
        for (size_t i = 0; i < eggs.size(); ++i) {
            const int rowY = m_y + kRowTop + static_cast<int>(i) * kRowHeight;
            if (rowY + kRowHeight > m_y + m_h - 4) break;
            const RECT row{ m_x + 8, rowY, m_x + m_w - 8, rowY + kRowHeight - 2 };
            const bool selected = (static_cast<int>(i) == m_selectedIndex);
            FillSolid(hdc, row,
                      selected ? RGB(180, 165, 100)
                               : ((i & 1) ? RGB(244, 240, 226) : RGB(232, 220, 192)));
            FrameSolid(hdc, row, RGB(180, 165, 120));
            char buf[32] = {};
            std::snprintf(buf, sizeof(buf), "Egg slot #%d", eggs[i]);
            DrawText(hdc, row.left + 6, row.top + 2,
                     selected ? RGB(255, 248, 220) : RGB(20, 20, 20), buf);
        }
    }

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
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

int UIEggListWnd::HitTestRow(int x, int y) const
{
    const auto& eggs = g_session.m_petEggList;
    for (size_t i = 0; i < eggs.size(); ++i) {
        const int rowY = m_y + kRowTop + static_cast<int>(i) * kRowHeight;
        const RECT row{ m_x + 8, rowY, m_x + m_w - 8, rowY + kRowHeight - 2 };
        if (x >= row.left && x < row.right && y >= row.top && y < row.bottom) {
            return static_cast<int>(i);
        }
    }
    return -1;
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

    if (!m_minimized) {
        const int row = HitTestRow(x, y);
        if (row >= 0) {
            m_selectedIndex = row;
            const auto& eggs = g_session.m_petEggList;
            if (row < static_cast<int>(eggs.size())) {
                const unsigned short invIndex = static_cast<unsigned short>(eggs[row]);
                if (invIndex != 0) {
                    RequestPetSelectEgg(invIndex);
                    SetShow(0);
                }
            }
            Invalidate();
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
