#include "UISkillUpConfirmWnd.h"

#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"
#include "gamemode/Mode.h"

#include <cstdio>
#include <windows.h>

void UISkillUpConfirmWnd::OpenConfirm(int skillId, const char* skillName, int currentLevel, int maxLevel)
{
    m_skillId = skillId;
    m_skillName = skillName ? skillName : "";
    m_currentLevel = currentLevel;
    m_maxLevel = maxLevel;
    MarkPendingAndShow();
}

int UISkillUpConfirmWnd::GetWindowId() const
{
    return UIWindowMgr::WID_SKILLUPCONFIRMWND;
}

void UISkillUpConfirmWnd::GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const
{
    fillRgb = RGB(70, 100, 60);
    frameRgb = RGB(40, 60, 30);
}

std::string UISkillUpConfirmWnd::ComposeBodyText() const
{
    const std::string name = m_skillName.empty() ? std::string("this skill") : m_skillName;
    char buf[160] = {};
    if (m_maxLevel > 0) {
        std::snprintf(buf, sizeof(buf), "Raise %s to Lv %d / %d?",
                      name.c_str(), m_currentLevel + 1, m_maxLevel);
    } else {
        std::snprintf(buf, sizeof(buf), "Raise %s to Lv %d?",
                      name.c_str(), m_currentLevel + 1);
    }
    return buf;
}

void UISkillUpConfirmWnd::ComposeMessageLines(std::vector<std::string>& outLines) const
{
    const std::string name = m_skillName.empty() ? std::string("this skill") : m_skillName;
    outLines.push_back(name);
    if (m_maxLevel > 0) {
        outLines.push_back("Lv " + std::to_string(m_currentLevel) + " -> Lv "
            + std::to_string(m_currentLevel + 1) + " / " + std::to_string(m_maxLevel));
    } else {
        outLines.push_back("Lv " + std::to_string(m_currentLevel) + " -> Lv "
            + std::to_string(m_currentLevel + 1));
    }
}

bool UISkillUpConfirmWnd::OnSubmit(bool accept)
{
    const int submittedSkillId = m_skillId;
    m_skillId = 0;
    m_currentLevel = 0;
    m_maxLevel = 0;
    m_skillName.clear();
    if (!accept || submittedSkillId == 0) {
        return false;
    }
    return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestUpgradeSkillLevel,
                              submittedSkillId, 0, 0) != 0;
}

unsigned long long UISkillUpConfirmWnd::HashSubclassState() const
{
    unsigned long long h = static_cast<unsigned long long>(m_skillId);
    h ^= static_cast<unsigned long long>(m_currentLevel) << 16;
    h ^= static_cast<unsigned long long>(m_maxLevel) << 24;
    for (unsigned char ch : m_skillName) {
        h ^= ch;
        h *= 1099511628211ull;
    }
    return h;
}
