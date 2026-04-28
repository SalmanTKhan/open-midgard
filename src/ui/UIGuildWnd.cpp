#include "UIGuildWnd.h"

#include "UIWindowMgr.h"
#include "UINpcInputWnd.h"
#include "core/ClientFeature.h"
#include "gamemode/GameMode.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <windows.h>

bool RequestGuildLeave(int guildId, const char* reason);
bool RequestGuildExpel(int guildId, unsigned int accountId, unsigned int charId, const char* reason);

namespace {

constexpr int kPad = 8;
constexpr int kTabBarTop = 22;
constexpr int kTabHeight = 18;
constexpr int kTabWidth = 38;
constexpr int kMemberRowHeight = 18;
constexpr int kActionBarHeight = 26;

const char* TabLabel(int tab)
{
    switch (tab) {
    case UIGuildWnd::TabInfo:     return "Info";
    case UIGuildWnd::TabMembers:  return "Member";
    case UIGuildWnd::TabPosition: return "Pos";
    case UIGuildWnd::TabSkill:    return "Skill";
    case UIGuildWnd::TabBanish:   return "Ban";
    case UIGuildWnd::TabNotice:   return "Notice";
    case UIGuildWnd::TabAlliance: return "Ally";
    default: return "?";
    }
}

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

UIGuildWnd::UIGuildWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("GuildWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIGuildWnd::~UIGuildWnd() = default;

void UIGuildWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::Guild)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

UIGuildWnd::InteractRect UIGuildWnd::GetTabRect(int tabIndex) const
{
    const int x = m_x + kPad + tabIndex * (kTabWidth + 2);
    return InteractRect{ x, m_y + kTabBarTop, kTabWidth, kTabHeight };
}

UIGuildWnd::InteractRect UIGuildWnd::GetMemberRowRect(int rowIndex) const
{
    const int top = m_y + kTabBarTop + kTabHeight + 4;
    return InteractRect{
        m_x + kPad,
        top + rowIndex * kMemberRowHeight,
        m_w - kPad * 2,
        kMemberRowHeight - 2
    };
}

UIGuildWnd::InteractRect UIGuildWnd::GetActionButtonRect(int buttonId) const
{
    const int y = m_y + m_h - kActionBarHeight + 2;
    const int w = 90;
    int slotFromRight = 0;  // 0 = right-most
    switch (buttonId) {
    case ButtonLeaveGuild:  slotFromRight = 0; break;
    case ButtonExpelMember: slotFromRight = 1; break;
    case ButtonEditNotice:  slotFromRight = 1; break;  // Same slot as Expel; only one shown per tab
    default: break;
    }
    const int x = m_x + m_w - kPad - w - slotFromRight * (w + 6);
    return InteractRect{ x, y, w, kActionBarHeight - 6 };
}

int UIGuildWnd::HitTestTab(int x, int y) const
{
    for (int i = 0; i < TabCount; ++i) {
        const InteractRect r = GetTabRect(i);
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) return i;
    }
    return -1;
}

int UIGuildWnd::HitTestMemberRow(int x, int y) const
{
    const auto& members = g_session.GetGuildMembers();
    const int rowsFit = std::max(1, (m_h - kTabBarTop - kTabHeight - 8 - kActionBarHeight) / kMemberRowHeight);
    const int last = std::min(static_cast<int>(members.size()), rowsFit);
    for (int i = 0; i < last; ++i) {
        const InteractRect r = GetMemberRowRect(i);
        if (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h) return i;
    }
    return -1;
}

int UIGuildWnd::HitTestActionButton(int x, int y) const
{
    auto inRect = [&](const InteractRect& r) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    };
    if (inRect(GetActionButtonRect(ButtonLeaveGuild))) return ButtonLeaveGuild;
    if (m_activeTab == TabMembers && inRect(GetActionButtonRect(ButtonExpelMember))) return ButtonExpelMember;
    if (m_activeTab == TabNotice && inRect(GetActionButtonRect(ButtonEditNotice))) return ButtonEditNotice;
    return 0;
}

void UIGuildWnd::SetPendingNoticeSubject(const char* s)
{
    if (!s) return;
    m_pendingNoticeSubject = s;
    Invalidate();
}

void UIGuildWnd::SetPendingNoticeBody(const char* s)
{
    if (!s) return;
    m_pendingNoticeBody = s;
    Invalidate();
}

void UIGuildWnd::OnDraw()
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
    DrawText(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "Guild");

    if (m_minimized) {
        ReleaseDrawTarget(hdc);
        m_isDirty = 0;
        return;
    }

    // Tab strip
    for (int i = 0; i < TabCount; ++i) {
        const InteractRect r = GetTabRect(i);
        const RECT rc{ r.x, r.y, r.x + r.w, r.y + r.h };
        const COLORREF fill = (i == m_activeTab) ? RGB(176, 152, 92) : RGB(204, 192, 158);
        FillSolid(hdc, rc, fill);
        FrameSolid(hdc, rc, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, (i == m_activeTab) ? RGB(255, 248, 220) : RGB(20, 20, 20));
        RECT textRc = rc;
        ::DrawTextA(hdc, TabLabel(i), -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    // Tab content
    if (m_activeTab == TabInfo) {
        const int y0 = m_y + kTabBarTop + kTabHeight + 8;
        char buf[128] = {};
        DrawText(hdc, m_x + kPad, y0,        RGB(60, 40, 10), "Name:");
        DrawText(hdc, m_x + kPad + 60, y0,   RGB(20, 20, 20), g_session.m_guildName);
        DrawText(hdc, m_x + kPad, y0 + 18,   RGB(60, 40, 10), "Master:");
        DrawText(hdc, m_x + kPad + 60, y0+18,RGB(20, 20, 20), g_session.m_guildMasterName);
        std::snprintf(buf, sizeof(buf), "%d", g_session.m_guildId);
        DrawText(hdc, m_x + kPad, y0 + 36,   RGB(60, 40, 10), "Guild ID:");
        DrawText(hdc, m_x + kPad + 60, y0+36,RGB(20, 20, 20), buf);
        std::snprintf(buf, sizeof(buf), "%d", g_session.m_guildEmblemId);
        DrawText(hdc, m_x + kPad, y0 + 54,   RGB(60, 40, 10), "Emblem:");
        DrawText(hdc, m_x + kPad + 60, y0+54,RGB(20, 20, 20), buf);
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned int>(g_session.GetGuildMembers().size()));
        DrawText(hdc, m_x + kPad, y0 + 72,   RGB(60, 40, 10), "Members:");
        DrawText(hdc, m_x + kPad + 60, y0+72,RGB(20, 20, 20), buf);
    } else if (m_activeTab == TabMembers) {
        const auto& members = g_session.GetGuildMembers();
        const int rowsFit = std::max(1, (m_h - kTabBarTop - kTabHeight - 8 - kActionBarHeight) / kMemberRowHeight);
        const int last = std::min(static_cast<int>(members.size()), rowsFit);
        for (int i = 0; i < last; ++i) {
            const InteractRect r = GetMemberRowRect(i);
            const RECT row{ r.x, r.y, r.x + r.w, r.y + r.h };
            const bool selected = (i == m_selectedMemberIndex);
            const COLORREF fill = selected
                ? RGB(180, 165, 100)
                : ((i & 1) ? RGB(244, 240, 226) : RGB(232, 220, 192));
            FillSolid(hdc, row, fill);
            FrameSolid(hdc, row, RGB(180, 165, 120));
            const GUILD_MEMBER& m = members[i];
            char line[160] = {};
            std::snprintf(line, sizeof(line), "%s   Lv%d   job%d   %s",
                          m.name,
                          m.level,
                          m.job,
                          m.currentState == 0 ? "Online" : "Offline");
            DrawText(hdc, row.left + 4, row.top + 2,
                     selected ? RGB(255, 248, 220) : RGB(20, 20, 20), line);
        }
    } else if (m_activeTab == TabNotice) {
        const int y0 = m_y + kTabBarTop + kTabHeight + 8;
        DrawText(hdc, m_x + kPad, y0, RGB(60, 40, 10), "Subject:");
        DrawText(hdc, m_x + kPad + 60, y0, RGB(20, 20, 20), g_session.m_guildNoticeSubject);
        DrawText(hdc, m_x + kPad, y0 + 22, RGB(60, 40, 10), "Body:");
        // Body wrapping: simple manual word-line breaks at ~32 chars per line.
        const std::string body = g_session.m_guildNoticeBody;
        constexpr size_t kCharsPerLine = 32;
        constexpr int kBodyLines = 7;
        for (int line = 0; line < kBodyLines; ++line) {
            const size_t start = static_cast<size_t>(line) * kCharsPerLine;
            if (start >= body.size()) break;
            DrawText(hdc, m_x + kPad + 60, y0 + 22 + line * 14, RGB(20, 20, 20),
                     body.substr(start, kCharsPerLine));
        }
    } else {
        const int y0 = m_y + kTabBarTop + kTabHeight + 8;
        DrawText(hdc, m_x + kPad, y0, RGB(120, 90, 30),
                 "(Tab not yet implemented in legacy UI.)");
    }

    // Action buttons (bottom-right)
    auto drawBtn = [&](int id, const char* label) {
        const InteractRect r = GetActionButtonRect(id);
        const RECT rc{ r.x, r.y, r.x + r.w, r.y + r.h };
        FillSolid(hdc, rc, RGB(210, 195, 145));
        FrameSolid(hdc, rc, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20, 20, 20));
        RECT textRc = rc;
        ::DrawTextA(hdc, label, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    };
    drawBtn(ButtonLeaveGuild, "Leave Guild");
    if (m_activeTab == TabMembers) {
        drawBtn(ButtonExpelMember, "Expel");
    }
    if (m_activeTab == TabNotice) {
        drawBtn(ButtonEditNotice, "Edit Notice");
    }

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIGuildWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIGuildWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{};    close.id = 1;    close.x = m_x + m_w - 17;    close.y = m_y + 3;    out->push_back(close);
}

void UIGuildWnd::OnLBtnDown(int x, int y)
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

    if (m_minimized) {
        UIFrameWnd::OnLBtnDown(x, y);
        return;
    }

    const int actionId = HitTestActionButton(x, y);
    if (actionId == ButtonLeaveGuild) {
        if (g_session.IsInGuild()) {
            RequestGuildLeave(g_session.m_guildId, "");
        }
        return;
    }
    if (actionId == ButtonExpelMember) {
        const auto& members = g_session.GetGuildMembers();
        if (m_selectedMemberIndex >= 0 &&
            m_selectedMemberIndex < static_cast<int>(members.size())) {
            const GUILD_MEMBER& m = members[m_selectedMemberIndex];
            RequestGuildExpel(g_session.m_guildId, m.aid, m.gid, "");
        }
        return;
    }
    if (actionId == ButtonEditNotice) {
        // Seed the pending fields from current notice so the prompt edits the
        // existing text. The flow runs as: subject prompt → body prompt → submit.
        m_pendingNoticeSubject = g_session.m_guildNoticeSubject;
        m_pendingNoticeBody = g_session.m_guildNoticeBody;
        if (auto* input = static_cast<UINpcInputWnd*>(
                g_windowMgr.MakeWindow(UIWindowMgr::WID_NPCINPUTWND))) {
            input->OpenGameStringPrompt(
                "Guild notice subject",
                CGameMode::GameMsg_RequestGuildNoticeSubject, 0);
        }
        return;
    }

    const int tabId = HitTestTab(x, y);
    if (tabId >= 0) {
        SetActiveTab(tabId);
        m_selectedMemberIndex = -1;
        return;
    }

    if (m_activeTab == TabMembers) {
        const int memberRow = HitTestMemberRow(x, y);
        if (memberRow >= 0) {
            m_selectedMemberIndex = memberRow;
            Invalidate();
            return;
        }
    }

    UIFrameWnd::OnLBtnDown(x, y);
}

void UIGuildWnd::StoreInfo()
{
    SaveUiWindowPlacement("GuildWnd", m_x, m_y);
}

msgresult_t UIGuildWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

void UIGuildWnd::SetActiveTab(int tab)
{
    if (tab < 0 || tab >= TabCount) {
        return;
    }
    m_activeTab = tab;
    Invalidate();
}

bool UIGuildWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) {
        return false;
    }
    outData->title = "Guild";
    outData->guildName = g_session.m_guildName;
    outData->masterName = g_session.m_guildMasterName;
    outData->guildId = g_session.m_guildId;
    outData->emblemId = g_session.m_guildEmblemId;
    outData->activeTab = m_activeTab;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
