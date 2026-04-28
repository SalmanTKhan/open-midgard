#include "UIWhisperWnd.h"

#include "UIWindowMgr.h"
#include "UINpcInputWnd.h"
#include "gamemode/GameMode.h"
#include "main/WinMain.h"

#include <cstdio>
#include <windows.h>

namespace {

constexpr int kFullWidth = 320;
constexpr int kFullHeight = 220;
constexpr int kTitleBarHeight = 17;
constexpr int kPad = 8;
constexpr int kButtonHeight = 22;
constexpr int kButtonWidth = 70;
constexpr int kBufferTop = 24;
constexpr int kLineHeight = 14;
constexpr int kMaxStoredLines = 200;

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

void DrawTextLine(HDC hdc, int x, int y, COLORREF color, const std::string& text)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 1024, y + kLineHeight + 2 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc,
                DT_LEFT | DT_TOP | DT_SINGLELINE);
}

}  // namespace

UIWhisperWnd::UIWhisperWnd() = default;
UIWhisperWnd::~UIWhisperWnd() = default;

void UIWhisperWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

void UIWhisperWnd::SetTarget(const std::string& displayName)
{
    m_targetName = displayName;
    Invalidate();
}

void UIWhisperWnd::AppendLine(const std::string& text, unsigned int color)
{
    Line line;
    line.text = text;
    line.color = color;
    m_lines.push_back(std::move(line));
    if (m_lines.size() > kMaxStoredLines) {
        m_lines.pop_front();
    }
    m_scrollOffset = 0;
    Invalidate();
}

void UIWhisperWnd::AppendIncoming(const std::string& text)
{
    AppendLine(std::string("(From ") + m_targetName + ") " + text, 0x00FFFF00);
}

void UIWhisperWnd::AppendOutgoing(const std::string& text)
{
    AppendLine(std::string("(To ") + m_targetName + ") " + text, 0x00C0FFC0);
}

void UIWhisperWnd::AppendSystem(const std::string& text)
{
    AppendLine(text, 0x00C0C0C0);
}

RECT UIWhisperWnd::GetButtonRect(int buttonId) const
{
    const int rowY = m_y + m_h - kButtonHeight - kPad;
    if (buttonId == ButtonReply) {
        const int x = m_x + kPad;
        return RECT{ x, rowY, x + kButtonWidth, rowY + kButtonHeight };
    }
    if (buttonId == ButtonClose) {
        const int x = m_x + m_w - kPad - kButtonWidth;
        return RECT{ x, rowY, x + kButtonWidth, rowY + kButtonHeight };
    }
    return RECT{ 0, 0, 0, 0 };
}

int UIWhisperWnd::HitTestButton(int x, int y) const
{
    for (int id : { ButtonReply, ButtonClose }) {
        const RECT r = GetButtonRect(id);
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return id;
    }
    return ButtonNone;
}

void UIWhisperWnd::OnDraw()
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

    const std::string titleText = std::string("Whisper - ")
        + (m_targetName.empty() ? std::string("(unset)") : m_targetName);
    DrawTextLine(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), titleText);

    // Buffer area.
    const int bufLeft = m_x + kPad;
    const int bufTop = m_y + kBufferTop;
    const int bufRight = m_x + m_w - kPad;
    const int bufBottom = m_y + m_h - kButtonHeight - 2 * kPad;
    const RECT bufRc{ bufLeft, bufTop, bufRight, bufBottom };
    FillSolid(hdc, bufRc, RGB(248, 240, 216));
    FrameSolid(hdc, bufRc, RGB(120, 90, 30));

    const int visibleLines = (bufBottom - bufTop - 4) / kLineHeight;
    const int total = static_cast<int>(m_lines.size());
    const int firstVisible = (total > visibleLines + m_scrollOffset)
                                 ? (total - visibleLines - m_scrollOffset)
                                 : 0;
    int rowY = bufTop + 2;
    for (int i = firstVisible;
         i < total && rowY + kLineHeight <= bufBottom;
         ++i) {
        const Line& ln = m_lines[i];
        const COLORREF rgbColor = RGB((ln.color >> 16) & 0xFF,
                                      (ln.color >> 8) & 0xFF,
                                      ln.color & 0xFF);
        DrawTextLine(hdc, bufLeft + 4, rowY, rgbColor, ln.text);
        rowY += kLineHeight;
    }

    // Buttons.
    auto drawButton = [&](int id, const char* label) {
        const RECT br = GetButtonRect(id);
        const COLORREF fill = (m_pressedButton == id)
            ? RGB(170, 145, 75) : RGB(210, 195, 145);
        FillSolid(hdc, br, fill);
        FrameSolid(hdc, br, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20, 20, 20));
        RECT bt = br;
        ::DrawTextA(hdc, label, -1, &bt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    };
    drawButton(ButtonReply, "Reply");
    drawButton(ButtonClose, "Close");

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIWhisperWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != ButtonNone) {
        m_pressedButton = btn;
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIWhisperWnd::OnLBtnUp(int x, int y)
{
    const int wasButton = m_pressedButton;
    m_pressedButton = ButtonNone;
    if (wasButton != ButtonNone && HitTestButton(x, y) == wasButton) {
        if (wasButton == ButtonClose) {
            SetShow(0);
            Invalidate();
            return;
        }
        if (wasButton == ButtonReply && !m_targetName.empty()) {
            char label[80] = {};
            std::snprintf(label, sizeof(label), "Whisper to %s",
                          m_targetName.c_str());
            if (auto* input = static_cast<UINpcInputWnd*>(
                    g_windowMgr.MakeWindow(UIWindowMgr::WID_NPCINPUTWND))) {
                input->OpenGameStringPrompt(
                    label,
                    CGameMode::GameMsg_RequestWhisperSendMessage,
                    reinterpret_cast<msgparam_t>(this));
            }
            Invalidate();
            return;
        }
    }
    UIFrameWnd::OnLBtnUp(x, y);
}

void UIWhisperWnd::OnWheel(int delta)
{
    const int step = delta > 0 ? 1 : -1;
    m_scrollOffset += step;
    if (m_scrollOffset < 0) m_scrollOffset = 0;
    const int maxOffset = (std::max)(0, static_cast<int>(m_lines.size()) - 1);
    if (m_scrollOffset > maxOffset) m_scrollOffset = maxOffset;
    Invalidate();
}
