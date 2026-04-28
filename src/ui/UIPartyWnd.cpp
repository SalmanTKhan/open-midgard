#include "UIPartyWnd.h"

#include "UIWindowMgr.h"
#include "main/WinMain.h"
#include "session/Session.h"
#include "session/MapNameResolver.h"

#include <algorithm>
#include <cstdio>
#include <windows.h>

// File-scope party request senders defined in src/gamemode/GameMode.cpp.
extern bool RequestPartyLeave();
extern bool RequestPartyExpel(unsigned int accountId, const char* characterName);

namespace {

constexpr int kFullWidth = 360;
constexpr int kFullHeight = 320;
constexpr int kTitleBarHeight = 17;
constexpr int kPad = 10;
constexpr int kRowsTop = 56;
constexpr int kRowHeight = 28;
constexpr int kMaxRows = 7;
constexpr int kButtonHeight = 22;
constexpr int kButtonGap = 6;
constexpr int kCloseButtonWidth = 70;
constexpr int kActionButtonWidth = 72;

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

void DrawTextLine(HDC hdc, int x, int y, COLORREF color, const std::string& text,
                  UINT fmt = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 800, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, fmt);
}

const FRIEND_INFO* GetMemberByVisibleIndex(int index)
{
    int i = 0;
    for (const FRIEND_INFO& info : g_session.GetPartyList()) {
        if (info.characterName.empty()) continue;
        if (i == index) return &info;
        ++i;
    }
    return nullptr;
}

}  // namespace

UIPartyWnd::UIPartyWnd() = default;
UIPartyWnd::~UIPartyWnd() = default;

void UIPartyWnd::OnCreate(int x, int y)
{
    Create(kFullWidth, kFullHeight);
    Move(x, y);
    m_isDirty = 1;
}

int UIPartyWnd::CountVisibleMembers() const
{
    int n = 0;
    for (const FRIEND_INFO& info : g_session.GetPartyList()) {
        if (!info.characterName.empty()) ++n;
    }
    return n;
}

RECT UIPartyWnd::GetRowRect(int index) const
{
    const int x0 = m_x + kPad;
    const int y0 = m_y + kRowsTop + index * kRowHeight;
    return RECT{ x0, y0, x0 + (m_w - 2 * kPad), y0 + kRowHeight - 2 };
}

RECT UIPartyWnd::GetButtonRect(int buttonId) const
{
    const int rowY = m_y + m_h - kButtonHeight - kPad;
    switch (buttonId) {
        case ButtonOptions: {
            const int x = m_x + kPad;
            return RECT{ x, rowY, x + kActionButtonWidth, rowY + kButtonHeight };
        }
        case ButtonExpel: {
            const int x = m_x + kPad + kActionButtonWidth + kButtonGap;
            return RECT{ x, rowY, x + kActionButtonWidth, rowY + kButtonHeight };
        }
        case ButtonLeave: {
            const int x = m_x + kPad + 2 * (kActionButtonWidth + kButtonGap);
            return RECT{ x, rowY, x + kActionButtonWidth, rowY + kButtonHeight };
        }
        case ButtonClose: {
            const int x = m_x + m_w - kPad - kCloseButtonWidth;
            return RECT{ x, rowY, x + kCloseButtonWidth, rowY + kButtonHeight };
        }
    }
    return RECT{ 0, 0, 0, 0 };
}

int UIPartyWnd::HitTestRow(int x, int y) const
{
    const int visible = (std::min)(CountVisibleMembers(), kMaxRows);
    for (int i = 0; i < visible; ++i) {
        const RECT r = GetRowRect(i);
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return i;
    }
    return -1;
}

int UIPartyWnd::HitTestButton(int x, int y) const
{
    static const int kIds[] = { ButtonOptions, ButtonExpel, ButtonLeave, ButtonClose };
    for (int id : kIds) {
        const RECT r = GetButtonRect(id);
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return id;
    }
    return ButtonNone;
}

void UIPartyWnd::OnDraw()
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

    const std::string& partyName = g_session.m_partyName;
    std::string titleText = partyName.empty() ? std::string("Party")
                                              : (std::string("Party - ") + partyName);
    DrawTextLine(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), titleText);

    // Header line: leader name + member count.
    std::string leaderName;
    int total = 0;
    for (const FRIEND_INFO& info : g_session.GetPartyList()) {
        if (info.characterName.empty()) continue;
        ++total;
        if (info.role == 0) leaderName = info.characterName;
    }

    char header[160] = {};
    if (total == 0) {
        std::snprintf(header, sizeof(header), "(no party)");
    } else {
        std::snprintf(header, sizeof(header), "Leader: %s   Members: %d",
                      leaderName.empty() ? "?" : leaderName.c_str(), total);
    }
    DrawTextLine(hdc, m_x + kPad, m_y + kTitleBarHeight + 6, RGB(40, 30, 10), header);

    // Column headers.
    DrawTextLine(hdc, m_x + kPad, m_y + kRowsTop - 18, RGB(80, 60, 30), "Name");
    DrawTextLine(hdc, m_x + kPad + 100, m_y + kRowsTop - 18, RGB(80, 60, 30), "Map");
    DrawTextLine(hdc, m_x + kPad + 220, m_y + kRowsTop - 18, RGB(80, 60, 30), "HP");

    // Member rows.
    const int visible = (std::min)(total, kMaxRows);
    int row = 0;
    for (const FRIEND_INFO& info : g_session.GetPartyList()) {
        if (info.characterName.empty()) continue;
        if (row >= visible) break;

        const RECT r = GetRowRect(row);
        const bool selected = (row == m_selectedRow);
        const bool pressed = (row == m_pressedRow);
        const COLORREF fill = selected ? RGB(176, 152, 92)
                                       : (pressed ? RGB(190, 172, 112)
                                                  : (row % 2 == 0 ? RGB(216, 200, 152)
                                                                  : RGB(208, 192, 144)));
        FillSolid(hdc, r, fill);
        FrameSolid(hdc, r, RGB(120, 90, 30));

        const COLORREF textCol = selected ? RGB(255, 248, 220) : RGB(20, 20, 20);

        std::string nameText = info.characterName;
        if (info.role == 0) nameText = std::string("* ") + nameText;
        DrawTextLine(hdc, r.left + 4, r.top + 6, textCol, nameText);

        const std::string mapDisp = mapname::ResolveMapDisplayName(info.mapName);
        DrawTextLine(hdc, r.left + 100, r.top + 6, textCol, mapDisp);

        // HP bar (server only sends it for nearby members; otherwise zero).
        const int barX = r.left + 220;
        const int barY = r.top + 8;
        const int barW = 110;
        const int barH = 12;
        const RECT barBg{ barX, barY, barX + barW, barY + barH };
        FillSolid(hdc, barBg, RGB(60, 40, 20));
        if (info.partyMaxHp > 0) {
            const int filled = (std::min)(barW,
                static_cast<int>(static_cast<long long>(info.partyHp) * barW
                                 / (std::max)(1, info.partyMaxHp)));
            if (filled > 0) {
                const RECT barFg{ barX, barY, barX + filled, barY + barH };
                const COLORREF hpColor = info.partyHp * 4 < info.partyMaxHp
                                             ? RGB(220, 60, 50)
                                             : RGB(60, 180, 90);
                FillSolid(hdc, barFg, hpColor);
            }
            char hpText[64] = {};
            std::snprintf(hpText, sizeof(hpText), "%d/%d", info.partyHp, info.partyMaxHp);
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 248, 220));
            RECT hpRc{ barX, barY - 1, barX + barW, barY + barH };
            ::DrawTextA(hdc, hpText, -1, &hpRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        FrameSolid(hdc, barBg, RGB(20, 14, 6));

        ++row;
    }

    if (total > kMaxRows) {
        char overflow[48] = {};
        std::snprintf(overflow, sizeof(overflow), "...and %d more", total - kMaxRows);
        DrawTextLine(hdc, m_x + kPad,
                     m_y + kRowsTop + kMaxRows * kRowHeight + 2,
                     RGB(120, 90, 30), overflow);
    }

    // Footer buttons.
    auto drawButton = [&](int id, const char* label, bool enabled) {
        const RECT br = GetButtonRect(id);
        const COLORREF fill = (m_pressedButton == id) ? RGB(170, 145, 75)
                              : enabled              ? RGB(210, 195, 145)
                                                     : RGB(170, 160, 132);
        FillSolid(hdc, br, fill);
        FrameSolid(hdc, br, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, enabled ? RGB(20, 20, 20) : RGB(90, 80, 60));
        RECT bt = br;
        ::DrawTextA(hdc, label, -1, &bt, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    };

    const bool inParty = total > 0;
    const bool isMaster = g_session.m_amIPartyMaster;
    const bool canExpel = inParty && isMaster && m_selectedRow >= 0;

    drawButton(ButtonOptions, "Options", inParty);
    drawButton(ButtonExpel, "Expel", canExpel);
    drawButton(ButtonLeave, "Leave", inParty);
    drawButton(ButtonClose, "Close", true);

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIPartyWnd::OnLBtnDown(int x, int y)
{
    const int btn = HitTestButton(x, y);
    if (btn != ButtonNone) {
        m_pressedButton = btn;
        m_pressedRow = -1;
        Invalidate();
        return;
    }
    const int row = HitTestRow(x, y);
    if (row >= 0) {
        m_pressedRow = row;
        m_pressedButton = ButtonNone;
        Invalidate();
        return;
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIPartyWnd::OnLBtnUp(int x, int y)
{
    const int wasButton = m_pressedButton;
    const int wasRow = m_pressedRow;
    m_pressedButton = ButtonNone;
    m_pressedRow = -1;

    if (wasRow >= 0 && HitTestRow(x, y) == wasRow) {
        m_selectedRow = wasRow;
        Invalidate();
        return;
    }

    if (wasButton != ButtonNone && HitTestButton(x, y) == wasButton) {
        switch (wasButton) {
            case ButtonClose:
                SetShow(0);
                break;
            case ButtonOptions:
                g_windowMgr.MakeWindow(UIWindowMgr::WID_PARTYOPTIONWND);
                break;
            case ButtonLeave:
                RequestPartyLeave();
                break;
            case ButtonExpel: {
                if (g_session.m_amIPartyMaster && m_selectedRow >= 0) {
                    if (const FRIEND_INFO* m = GetMemberByVisibleIndex(m_selectedRow)) {
                        if (m->AID != g_session.m_aid) {
                            RequestPartyExpel(m->AID, m->characterName.c_str());
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
        Invalidate();
        return;
    }

    UIFrameWnd::OnLBtnUp(x, y);
}
