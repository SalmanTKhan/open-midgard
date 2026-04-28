#include "UISkillCastIndicatorWnd.h"

#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"
#include "main/WinMain.h"
#include "session/Session.h"

#include <cstdio>
#include <windows.h>

namespace {

constexpr int kBannerWidth = 380;
constexpr int kBannerHeight = 30;
constexpr int kSkillInfGroundSkill = 0x02;
constexpr int kTopMargin = 36;  // Sit above UIMapNameBannerWnd's anchor (64).

// Returns AoE radius hint for a few well-known 2008 ground skills. Used only
// for the banner label, not for the actual cast. Unknown skills return -1
// and the banner omits the radius. Loading skill_db.txt for full coverage
// is a follow-up.
int LookupAoeRadius(int skillId)
{
    switch (skillId) {
    case 18:   return 1;  // MG_FIREBALL
    case 21:   return 1;  // MG_FROSTDIVER
    case 88:   return 1;  // MG_SAFETYWALL
    case 89:   return 1;  // MG_FIREWALL
    case 70:   return 1;  // AL_PNEUMA
    case 17:   return 3;  // MG_THUNDERSTORM
    case 79:   return 3;  // WZ_HEAVENDRIVE
    case 80:   return 3;  // WZ_QUAGMIRE
    case 83:   return 3;  // SA_LANDPROTECTOR
    case 84:   return 3;  // SA_VIOLENTGALE
    case 85:   return 5;  // WZ_STORMGUST
    case 81:   return 5;  // WZ_VERMILION (Lord of Vermilion)
    case 78:   return 5;  // WZ_SIGHTRASHER
    case 86:   return 3;  // WZ_FROSTNOVA
    case 92:   return 5;  // PR_SANCTUARY
    // (PR_MAGNUSEXORCISMUS id varies by version; omit to avoid collision)
    default:   return -1;
    }
}

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

}  // namespace

UISkillCastIndicatorWnd::UISkillCastIndicatorWnd()
{
    Create(kBannerWidth, kBannerHeight);
    Reposition();
}

UISkillCastIndicatorWnd::~UISkillCastIndicatorWnd() = default;

bool UISkillCastIndicatorWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) return true;
    return true;  // Always re-evaluate so we hide promptly when disarmed.
}

void UISkillCastIndicatorWnd::OnDraw()
{
    if (!g_hMainWnd) return;
    CGameMode* gm = g_modeMgr.GetCurrentGameMode();
    if (!gm) {
        if (m_show != 0) SetShow(0);
        return;
    }
    const int armedId = gm->m_skillUseInfo.id;
    const int armedLevel = gm->m_skillUseInfo.level;
    if (armedId <= 0 || armedLevel <= 0) {
        if (m_show != 0) SetShow(0);
        m_lastShownSkillId = 0;
        return;
    }
    const PLAYER_SKILL_INFO* skill = g_session.GetSkillItemBySkillId(armedId);
    const bool isGround = skill && (skill->type & kSkillInfGroundSkill) != 0;
    if (!isGround) {
        if (m_show != 0) SetShow(0);
        m_lastShownSkillId = 0;
        return;
    }

    if (m_show == 0) {
        SetShow(1);
        Reposition();
    }
    m_lastShownSkillId = armedId;

    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(40, 28, 16));
    FrameSolid(hdc, bg, RGB(220, 200, 140));

    char buf[160] = {};
    const std::string skillName = skill->skillName.empty()
        ? skill->skillIdName : skill->skillName;
    const int radius = LookupAoeRadius(armedId);
    if (radius > 0) {
        std::snprintf(buf, sizeof(buf),
                      "Casting %s [Lv %d]  -  ~%dx%d AoE  -  click ground to fire (RMB to cancel)",
                      skillName.c_str(), armedLevel,
                      radius * 2 + 1, radius * 2 + 1);
    } else {
        std::snprintf(buf, sizeof(buf),
                      "Casting %s [Lv %d]  -  click ground to fire (RMB to cancel)",
                      skillName.c_str(), armedLevel);
    }

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 220, 140));
    RECT textRc = bg;
    ::DrawTextA(hdc, buf, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UISkillCastIndicatorWnd::Reposition()
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
