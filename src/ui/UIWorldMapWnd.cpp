#include "UIWorldMapWnd.h"

#include "UIWindowMgr.h"
#include "main/WinMain.h"
#include "session/MapNameResolver.h"

#include <cstdio>
#include <windows.h>

namespace {

constexpr int kFullWidth = 380;
constexpr int kFullHeight = 320;
constexpr int kTitleBarHeight = 17;
constexpr int kPad = 10;
constexpr int kColumns = 4;
constexpr int kTileWidth = 80;
constexpr int kTileHeight = 28;
constexpr int kTileGapX = 8;
constexpr int kTileGapY = 8;
constexpr int kTilesTop = 26;
constexpr int kStatusBarHeight = 22;
constexpr int kCloseButtonWidth = 80;

// Canonical pre-Renewal Rune-Midgard towns + Izlude port.
const char* const kCanonicalTowns[] = {
    "prontera",
    "morocc",
    "geffen",
    "payon",
    "alberta",
    "izlude",
    "aldebaran",
    "yuno",
    "lighthalzen",
    "hugel",
    "einbroch",
    "einbech",
    "comodo",
    "umbala",
    "gonryun",
    "amatsu",
    "louyang",
    "xmas",
    "moscovia",
    "ayothaya",
};

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

void DrawText(HDC hdc, int x, int y, COLORREF color, const std::string& text,
              UINT fmt = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, fmt);
}

}  // namespace

UIWorldMapWnd::UIWorldMapWnd() = default;
UIWorldMapWnd::~UIWorldMapWnd() = default;

void UIWorldMapWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    EnsureTiles();
    m_isDirty = 1;
}

void UIWorldMapWnd::EnsureTiles()
{
    if (!m_tiles.empty()) return;
    for (const char* id : kCanonicalTowns) {
        Tile t;
        t.mapId = id;
        t.displayName = mapname::ResolveMapDisplayName(id);
        m_tiles.push_back(std::move(t));
    }
}

RECT UIWorldMapWnd::GetTileRect(int index) const
{
    const int col = index % kColumns;
    const int row = index / kColumns;
    const int x0 = m_x + kPad + col * (kTileWidth + kTileGapX);
    const int y0 = m_y + kTilesTop + row * (kTileHeight + kTileGapY);
    return RECT{ x0, y0, x0 + kTileWidth, y0 + kTileHeight };
}

RECT UIWorldMapWnd::GetCloseButtonRect() const
{
    const int y = m_y + m_h - kStatusBarHeight - 4;
    const int x = m_x + m_w - kPad - kCloseButtonWidth;
    return RECT{ x, y, x + kCloseButtonWidth, y + 20 };
}

int UIWorldMapWnd::HitTestTile(int x, int y) const
{
    for (size_t i = 0; i < m_tiles.size(); ++i) {
        const RECT r = GetTileRect(static_cast<int>(i));
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int UIWorldMapWnd::HitTestButton(int x, int y) const
{
    const RECT r = GetCloseButtonRect();
    if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return ButtonClose;
    return 0;
}

void UIWorldMapWnd::OnDraw()
{
    if (m_show == 0) return;
    if (!g_hMainWnd) return;
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));

    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleBarHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    DrawText(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "World Map");

    for (size_t i = 0; i < m_tiles.size(); ++i) {
        const RECT r = GetTileRect(static_cast<int>(i));
        const bool selected = (static_cast<int>(i) == m_selectedTile);
        const bool pressed = (static_cast<int>(i) == m_pressedTile);
        const COLORREF fill = selected ? RGB(176, 152, 92)
                                       : (pressed ? RGB(160, 142, 82)
                                                  : RGB(210, 195, 145));
        FillSolid(hdc, r, fill);
        FrameSolid(hdc, r, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, selected ? RGB(255, 248, 220) : RGB(20, 20, 20));
        RECT textRc = r;
        ::DrawTextA(hdc, m_tiles[i].displayName.c_str(),
                    static_cast<int>(m_tiles[i].displayName.size()),
                    &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Status line at the bottom-left.
    const int statusY = m_y + m_h - kStatusBarHeight;
    if (m_selectedTile >= 0 && m_selectedTile < static_cast<int>(m_tiles.size())) {
        char buf[160] = {};
        std::snprintf(buf, sizeof(buf), "Selected: %s  (%s)",
                      m_tiles[m_selectedTile].displayName.c_str(),
                      m_tiles[m_selectedTile].mapId.c_str());
        DrawText(hdc, m_x + kPad, statusY + 2, RGB(60, 40, 10), buf);
    } else {
        DrawText(hdc, m_x + kPad, statusY + 2, RGB(120, 90, 30),
                 "Click a town to inspect.");
    }

    // Close button (bottom-right).
    const RECT closeRc = GetCloseButtonRect();
    FillSolid(hdc, closeRc,
              m_pressedButton == ButtonClose ? RGB(170, 145, 75) : RGB(210, 195, 145));
    FrameSolid(hdc, closeRc, RGB(80, 60, 30));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(20, 20, 20));
    RECT closeText = closeRc;
    ::DrawTextA(hdc, "Close", -1, &closeText, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIWorldMapWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != 0) {
        m_pressedButton = btn;
        m_pressedTile = -1;
        Invalidate();
        return;
    }
    const int tile = HitTestTile(x, y);
    if (tile >= 0) {
        m_pressedTile = tile;
        m_pressedButton = 0;
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIWorldMapWnd::OnLBtnUp(int x, int y)
{
    const int wasButton = m_pressedButton;
    const int wasTile = m_pressedTile;
    m_pressedButton = 0;
    m_pressedTile = -1;

    if (wasButton == ButtonClose && HitTestButton(x, y) == ButtonClose) {
        SetShow(0);
        Invalidate();
        return;
    }
    if (wasTile >= 0) {
        const int hit = HitTestTile(x, y);
        if (hit == wasTile) {
            m_selectedTile = wasTile;
        }
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnUp(x, y);
}
