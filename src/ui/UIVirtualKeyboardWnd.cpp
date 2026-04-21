#include "UIVirtualKeyboardWnd.h"

#include "DebugLog.h"
#include "main/WinMain.h"
#include "ui/UIWindowMgr.h"

#include <windows.h>

#include <algorithm>

extern UIWindowMgr g_windowMgr;

namespace {

constexpr int kKeyCellW = 36;
constexpr int kKeyCellH = 36;
constexpr int kMarginX  = 12;
constexpr int kMarginY  = 36;  // extra space above grid for page label
constexpr int kCornerRadius = 10;

constexpr int kPanelW = kMarginX * 2 + UIVirtualKeyboardWnd::kCols * kKeyCellW;
constexpr int kPanelH = kMarginY + kKeyCellH * UIVirtualKeyboardWnd::kRows + kMarginX;

// Row-major, indexed by [page][row][col]. All printable ASCII.
const char kKeys[UIVirtualKeyboardWnd::PAGE_COUNT]
                [UIVirtualKeyboardWnd::kRows]
                [UIVirtualKeyboardWnd::kCols] = {
    // PAGE_LOWER
    {
        { '1','2','3','4','5','6','7','8','9','0' },
        { 'q','w','e','r','t','y','u','i','o','p' },
        { 'a','s','d','f','g','h','j','k','l','.' },
        { 'z','x','c','v','b','n','m',',','-','_' },
    },
    // PAGE_UPPER
    {
        { '1','2','3','4','5','6','7','8','9','0' },
        { 'Q','W','E','R','T','Y','U','I','O','P' },
        { 'A','S','D','F','G','H','J','K','L',':' },
        { 'Z','X','C','V','B','N','M',';','-','_' },
    },
    // PAGE_SYMBOLS
    {
        { '!','@','#','$','%','^','&','*','(',')' },
        { '`','~','[',']','{','}','<','>','|','\\' },
        { '/','?','+','=','"','\'',';',':','.',',' },
        { '-','_','.','@','#','$','%','^','&','*' },
    },
};

const char* PageLabel(UIVirtualKeyboardWnd::Page page)
{
    switch (page) {
    case UIVirtualKeyboardWnd::PAGE_LOWER:   return "abc";
    case UIVirtualKeyboardWnd::PAGE_UPPER:   return "ABC";
    case UIVirtualKeyboardWnd::PAGE_SYMBOLS: return "!@#";
    default:                                  return "";
    }
}

RECT MakeRect(int x, int y, int w, int h)
{
    RECT rc = { x, y, x + w, y + h };
    return rc;
}

void FillRectColor(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawRectFrame(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void FillRoundedRectColor(HDC hdc, const RECT& rc, COLORREF color, int radius)
{
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void DrawCenteredText(HDC hdc, const RECT& rc, const char* text, COLORREF color, int heightPx, int weight)
{
    if (!text || !*text) return;
    HFONT font = CreateFontA(heightPx, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Arial");
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT mutRc = rc;
    DrawTextA(hdc, text, -1, &mutRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

}  // namespace

UIVirtualKeyboardWnd::UIVirtualKeyboardWnd()
    : m_page(PAGE_LOWER),
      m_row(1),
      m_col(0),
      m_pressedFrames(0),
      m_everShownLogged(false)
{
    m_defPushId = 0;
    m_defCancelPushId = 0;
    Create(kPanelW, kPanelH);
}

UIVirtualKeyboardWnd::~UIVirtualKeyboardWnd() = default;

void UIVirtualKeyboardWnd::OnCreate(int cx, int cy)
{
    (void)cx; (void)cy;
    m_w = kPanelW;
    m_h = kPanelH;
}

void UIVirtualKeyboardWnd::SetShow(int show)
{
    UIWindow::SetShow(show);
    if (show == 0) {
        m_pressedFrames = 0;
    }
}

void UIVirtualKeyboardWnd::PositionBottomCenter(int clientWidth, int clientHeight)
{
    // Despite the name (kept for ABI), the OSK sits at top-center — the
    // legacy chat window lives at bottom-center and would otherwise cover
    // the grid. Top-center is visible above every other managed window.
    if (clientWidth  <= 0) clientWidth  = 1920;
    if (clientHeight <= 0) clientHeight = 1080;
    m_x = std::max(0, (clientWidth - m_w) / 2);
    m_y = 32;
}

char UIVirtualKeyboardWnd::KeyAt(int row, int col) const
{
    if (row < 0 || row >= kRows || col < 0 || col >= kCols) return 0;
    return kKeys[m_page][row][col];
}

void UIVirtualKeyboardWnd::OnDraw()
{
    if (!g_hMainWnd || m_show == 0) return;

    HDC hdc = AcquireDrawTarget();
    if (!m_everShownLogged) {
        DbgLog("[OSK] OnDraw entered: pos (%d,%d) size %dx%d  show=%d  hdc=%p\n",
            m_x, m_y, m_w, m_h, m_show, static_cast<void*>(hdc));
        m_everShownLogged = true;
    }
    if (!hdc) return;

    // Panel background — use a plain FillRect so we know exactly where
    // this window's pixels landed. Wrap with a thick frame to make the
    // outline unmistakable even if GDI font rendering fails for any reason.
    const RECT panel = MakeRect(m_x, m_y, m_w, m_h);
    FillRectColor(hdc, panel, RGB(245, 245, 250));
    DrawRectFrame(hdc, panel, RGB(40, 40, 70));
    const RECT panelInner = MakeRect(m_x + 1, m_y + 1, m_w - 2, m_h - 2);
    DrawRectFrame(hdc, panelInner, RGB(40, 40, 70));

    // Header: page label + hint row
    const RECT headerRc = MakeRect(m_x + kMarginX, m_y + 6, m_w - kMarginX * 2, kMarginY - 10);
    char header[128];
    wsprintfA(header,
        "Keyboard [%s]    (A)type  (B)del  (X)space  (Y)shift  LB/RB=page  START=OK  BACK=cancel",
        PageLabel(m_page));
    DrawCenteredText(hdc, headerRc, header, RGB(30, 30, 60), 14, FW_NORMAL);

    // Keys.
    const int gridOriginX = m_x + kMarginX;
    const int gridOriginY = m_y + kMarginY;
    for (int r = 0; r < kRows; ++r) {
        for (int c = 0; c < kCols; ++c) {
            const char ch = KeyAt(r, c);
            if (!ch) continue;

            const int kx = gridOriginX + c * kKeyCellW;
            const int ky = gridOriginY + r * kKeyCellH;
            const RECT keyRc = MakeRect(kx + 2, ky + 2, kKeyCellW - 4, kKeyCellH - 4);

            const bool highlighted = (r == m_row && c == m_col);
            const bool flash = highlighted && m_pressedFrames > 0;

            COLORREF fill = highlighted
                ? (flash ? RGB(255, 220, 110) : RGB(90, 140, 230))
                : RGB(255, 255, 255);
            COLORREF text = highlighted ? RGB(20, 20, 40) : RGB(40, 40, 60);

            FillRoundedRectColor(hdc, keyRc, fill, 6);
            DrawRectFrame(hdc, keyRc, RGB(120, 120, 140));

            char label[2] = { ch, 0 };
            DrawCenteredText(hdc, keyRc, label, text, 20, FW_BOLD);
        }
    }

    if (m_pressedFrames > 0) --m_pressedFrames;

    ReleaseDrawTarget(hdc);
}

void UIVirtualKeyboardWnd::MoveHighlight(int dx, int dy)
{
    if (m_show == 0) return;
    m_col = (m_col + dx + kCols) % kCols;
    m_row = (m_row + dy + kRows) % kRows;
    Invalidate();
}

void UIVirtualKeyboardWnd::ActivateHighlight()
{
    if (m_show == 0) return;
    const char ch = KeyAt(m_row, m_col);
    if (!ch) return;
    g_windowMgr.OnChar(ch);
    m_pressedFrames = 4;

    // One-shot shift: after typing an upper-case letter, return to lower.
    if (m_page == PAGE_UPPER) {
        m_page = PAGE_LOWER;
    }
    Invalidate();
}

void UIVirtualKeyboardWnd::ToggleShift()
{
    if (m_show == 0) return;
    m_page = (m_page == PAGE_UPPER) ? PAGE_LOWER : PAGE_UPPER;
    Invalidate();
}

void UIVirtualKeyboardWnd::CyclePage(int direction)
{
    if (m_show == 0) return;
    const int next = (static_cast<int>(m_page) + direction + PAGE_COUNT) % PAGE_COUNT;
    m_page = static_cast<Page>(next);
    Invalidate();
}

void UIVirtualKeyboardWnd::SendBackspace()
{
    if (m_show == 0) return;
    g_windowMgr.OnKeyDown(VK_BACK);
    m_pressedFrames = 4;
    Invalidate();
}

void UIVirtualKeyboardWnd::SendSpace()
{
    if (m_show == 0) return;
    g_windowMgr.OnChar(' ');
    m_pressedFrames = 4;
    Invalidate();
}

void UIVirtualKeyboardWnd::Submit()
{
    if (m_show == 0) return;
    g_windowMgr.OnKeyDown(VK_RETURN);
    // The submit may clear the edit focus; UIWindowMgr polls and hides us.
}

void UIVirtualKeyboardWnd::Cancel()
{
    if (m_show == 0) return;
    g_windowMgr.OnKeyDown(VK_ESCAPE);
}
