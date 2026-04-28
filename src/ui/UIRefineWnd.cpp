#include "UIRefineWnd.h"

#include "UIWindowMgr.h"
#include "item/Item.h"
#include "main/WinMain.h"
#include "session/Session.h"

#include <cstdio>
#include <windows.h>

bool SendWeaponRefineRequest(unsigned int inventoryIndex);

namespace {

constexpr int kFullWidth = 320;
constexpr int kFullHeight = 280;
constexpr int kTitleBarHeight = 17;
constexpr int kListTop = 26;
constexpr int kListLeft = 8;
constexpr int kListRight = 8;
constexpr int kRowHeight = 18;
constexpr int kCloseButtonAreaTop = 248;
constexpr int kButtonWidth = 120;
constexpr int kButtonHeight = 22;

void FillSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void FrameSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawLabel(HDC hdc, int x, int y, COLORREF color, const std::string& text,
               UINT format = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, format);
}

unsigned long long HashU64(unsigned long long seed, unsigned long long value)
{
    seed ^= value + 0x9E3779B97F4A7C15ull + (seed << 6) + (seed >> 2);
    return seed;
}

std::string ResolveItemName(unsigned int itemId)
{
    std::string n = g_ttemmgr.GetDisplayName(itemId, true);
    if (n.empty()) {
        char buf[24] = {};
        std::snprintf(buf, sizeof(buf), "Item #%u", itemId);
        return buf;
    }
    return n;
}

}  // namespace

UIRefineWnd::UIRefineWnd() = default;
UIRefineWnd::~UIRefineWnd() = default;

void UIRefineWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

void UIRefineWnd::SetCandidates(std::vector<Candidate> candidates)
{
    m_candidates = std::move(candidates);
    m_active = !m_candidates.empty();
    m_hoverRow = -1;
    m_pressedRow = -1;
    m_pressedButtonId = 0;
    if (m_active) {
        SetShow(1);
    } else {
        SetShow(0);
    }
    Invalidate();
}

void UIRefineWnd::ClearCandidates()
{
    m_candidates.clear();
    m_active = false;
    m_hoverRow = -1;
    m_pressedRow = -1;
    m_pressedButtonId = 0;
    SetShow(0);
    Invalidate();
}

bool UIRefineWnd::IsActive() const
{
    return m_active;
}

const std::vector<UIRefineWnd::Candidate>& UIRefineWnd::GetCandidates() const
{
    return m_candidates;
}

bool UIRefineWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) return true;
    return BuildVisualStateToken() != m_lastVisualStateToken;
}

void UIRefineWnd::OnDraw()
{
    if (m_show == 0 || !m_active) return;
    if (!g_hMainWnd) return;
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));

    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    DrawLabel(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "Refine Weapon");

    DrawLabel(hdc, m_x + kListLeft + 4, m_y + kListTop, RGB(70, 50, 20), "Item");
    DrawLabel(hdc, m_x + kListLeft + 220, m_y + kListTop, RGB(70, 50, 20), "+R");

    int rowY = m_y + kListTop + 18;
    for (size_t i = 0; i < m_candidates.size(); ++i) {
        const Candidate& c = m_candidates[i];
        const RECT row{ m_x + kListLeft, rowY, m_x + m_w - kListRight, rowY + kRowHeight - 2 };
        const COLORREF rowColor = (static_cast<int>(i) == m_pressedRow)
            ? RGB(190, 170, 110)
            : ((static_cast<int>(i) == m_hoverRow) ? RGB(220, 210, 170)
                                                   : ((i & 1) ? RGB(244, 240, 226) : RGB(232, 220, 192)));
        FillSolid(hdc, row, rowColor);
        FrameSolid(hdc, row, RGB(180, 165, 120));
        DrawLabel(hdc, row.left + 4, row.top + 2, RGB(20, 20, 20), ResolveItemName(c.itemId));
        char ref[16] = {};
        std::snprintf(ref, sizeof(ref), "+%u", static_cast<unsigned int>(c.refine));
        DrawLabel(hdc, row.left + 220, row.top + 2, RGB(160, 100, 20), ref);
        rowY += kRowHeight;
        if (rowY + kRowHeight > m_y + kCloseButtonAreaTop - 6) break;
    }

    const RECT closeRc = GetCloseButtonRect();
    const COLORREF fill = (m_pressedButtonId == ButtonClose) ? RGB(170, 145, 75) : RGB(210, 195, 145);
    FillSolid(hdc, closeRc, fill);
    FrameSolid(hdc, closeRc, RGB(80, 60, 30));
    DrawLabel(hdc, closeRc.left + 8, closeRc.top + 5, RGB(20, 20, 20), "Cancel");

    ReleaseDrawTarget(hdc);
    m_lastVisualStateToken = BuildVisualStateToken();
    m_isDirty = 0;
}

RECT UIRefineWnd::GetCloseButtonRect() const
{
    const int x0 = m_x + (m_w - kButtonWidth) / 2;
    const int y0 = m_y + kCloseButtonAreaTop;
    return RECT{ x0, y0, x0 + kButtonWidth, y0 + kButtonHeight };
}

int UIRefineWnd::HitTestRow(int x, int y) const
{
    int rowY = m_y + kListTop + 18;
    for (size_t i = 0; i < m_candidates.size(); ++i) {
        const RECT row{ m_x + kListLeft, rowY, m_x + m_w - kListRight, rowY + kRowHeight - 2 };
        if (x >= row.left && x < row.right && y >= row.top && y < row.bottom) {
            return static_cast<int>(i);
        }
        rowY += kRowHeight;
        if (rowY + kRowHeight > m_y + kCloseButtonAreaTop - 6) break;
    }
    return -1;
}

int UIRefineWnd::HitTestButton(int x, int y) const
{
    const RECT rc = GetCloseButtonRect();
    if (x >= rc.left && x < rc.right && y >= rc.top && y < rc.bottom) {
        return ButtonClose;
    }
    return 0;
}

void UIRefineWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != 0) {
        m_pressedButtonId = btn;
        m_pressedRow = -1;
        Invalidate();
        return;
    }
    const int row = HitTestRow(x, y);
    if (row >= 0) {
        m_pressedRow = row;
        m_pressedButtonId = 0;
        Invalidate();
        return;
    }
    m_pressedRow = -1;
    m_pressedButtonId = 0;
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIRefineWnd::OnLBtnUp(int x, int y)
{
    const int wasButton = m_pressedButtonId;
    const int wasRow = m_pressedRow;
    m_pressedButtonId = 0;
    m_pressedRow = -1;

    if (wasButton == ButtonClose) {
        if (HitTestButton(x, y) == ButtonClose) {
            ClearCandidates();
        }
        Invalidate();
        return;
    }

    if (wasRow >= 0) {
        const int hit = HitTestRow(x, y);
        if (hit == wasRow && hit >= 0 && hit < static_cast<int>(m_candidates.size())) {
            const Candidate& c = m_candidates[hit];
            SendWeaponRefineRequest(c.inventoryIndex);
            ClearCandidates();
        }
        Invalidate();
        return;
    }

    UIFrameWnd::OnLBtnUp(x, y);
}

unsigned long long UIRefineWnd::BuildVisualStateToken() const
{
    unsigned long long h = 14695981039346656037ull;
    h = HashU64(h, m_active ? 1u : 0u);
    h = HashU64(h, m_candidates.size());
    for (const Candidate& c : m_candidates) {
        h = HashU64(h, c.inventoryIndex);
        h = HashU64(h, c.itemId);
        h = HashU64(h, c.refine);
    }
    h = HashU64(h, static_cast<unsigned long long>(m_hoverRow + 1));
    h = HashU64(h, static_cast<unsigned long long>(m_pressedRow + 1));
    h = HashU64(h, static_cast<unsigned long long>(m_pressedButtonId));
    h = HashU64(h, static_cast<unsigned long long>(m_x));
    h = HashU64(h, static_cast<unsigned long long>(m_y));
    h = HashU64(h, static_cast<unsigned long long>(m_show));
    return h;
}
