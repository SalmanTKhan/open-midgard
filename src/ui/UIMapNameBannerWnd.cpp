#include "UIMapNameBannerWnd.h"

#include "UIWindowMgr.h"
#include "main/WinMain.h"
#include "session/MapNameResolver.h"

#include <algorithm>
#include <cstdio>
#include <windows.h>

namespace {

constexpr int kBannerWidth = 360;
constexpr int kBannerHeight = 56;
constexpr int kTopMargin = 64;

// (Friendly-name resolution moved to mapname::ResolveMapDisplayName, which
//  also strips .rsw/.gat/.gnd extensions as a fallback.)

void GetClientSize(int* outW, int* outH)
{
    *outW = 800;
    *outH = 600;
    if (!g_hMainWnd) return;
    RECT rc{};
    if (GetClientRect(g_hMainWnd, &rc)) {
        *outW = rc.right - rc.left;
        *outH = rc.bottom - rc.top;
    }
}

void DrawCenteredText(HDC hdc, const RECT& rc, const std::string& text, COLORREF color)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    HFONT font = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, "Arial");
    HGDIOBJ oldFont = font ? SelectObject(hdc, font) : nullptr;
    RECT mut = rc;
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &mut,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if (font) {
        SelectObject(hdc, oldFont);
        DeleteObject(font);
    }
}

}  // namespace

UIMapNameBannerWnd::UIMapNameBannerWnd() = default;
UIMapNameBannerWnd::~UIMapNameBannerWnd() = default;

void UIMapNameBannerWnd::Trigger(const std::string& mapName, unsigned int durationMs)
{
    if (mapName.empty() || durationMs == 0) {
        Hide();
        return;
    }
    m_displayName = mapname::ResolveMapDisplayName(mapName);
    m_startTickMs = GetTickCount();
    m_durationMs = durationMs;
    m_active = true;

    Create(kBannerWidth, kBannerHeight);
    Reposition();
    SetShow(1);
    Invalidate();
}

void UIMapNameBannerWnd::Hide()
{
    m_active = false;
    m_displayName.clear();
    SetShow(0);
    Invalidate();
}

bool UIMapNameBannerWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) return true;
    if (m_active) return true;  // animating fade
    return false;
}

unsigned int UIMapNameBannerWnd::ElapsedMs() const
{
    return GetTickCount() - m_startTickMs;
}

void UIMapNameBannerWnd::OnDraw()
{
    if (m_show == 0 || !m_active || m_displayName.empty()) {
        return;
    }
    const unsigned int elapsed = ElapsedMs();
    if (elapsed >= m_durationMs) {
        Hide();
        return;
    }

    if (!g_hMainWnd) return;
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    // Two-color drop shadow for readability against any backdrop.
    const RECT outer{ m_x, m_y, m_x + m_w, m_y + m_h };

    // Compute fade alpha as 0..255 (1.0 for first 70%, linear to 0 for last 30%).
    const unsigned int fadeStart = (m_durationMs * 7u) / 10u;
    int alpha255 = 255;
    if (elapsed > fadeStart) {
        const unsigned int range = m_durationMs - fadeStart;
        const unsigned int fadeOff = elapsed - fadeStart;
        alpha255 = 255 - static_cast<int>((fadeOff * 255u) / (range > 0 ? range : 1u));
        if (alpha255 < 0) alpha255 = 0;
    }
    const int textAlpha = alpha255;
    const COLORREF shadow = RGB((255 * (255 - textAlpha)) / 255,
                                (255 * (255 - textAlpha)) / 255,
                                (255 * (255 - textAlpha)) / 255);
    (void)shadow;

    // Shadow pass at +1/+1.
    RECT shadowRect = outer;
    OffsetRect(&shadowRect, 1, 1);
    DrawCenteredText(hdc, shadowRect, m_displayName, RGB(0, 0, 0));
    // Main pass — bright cream-white tint, intensity scaled by alpha.
    const int v = 220 + ((255 - 220) * textAlpha) / 255;
    DrawCenteredText(hdc, outer, m_displayName, RGB(v, v, 200));

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIMapNameBannerWnd::Reposition()
{
    int clientW = 0;
    int clientH = 0;
    GetClientSize(&clientW, &clientH);
    int x = (clientW - m_w) / 2;
    int y = kTopMargin;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    Move(x, y);
}
