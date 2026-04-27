#include "UIJoinPartyAcceptWnd.h"

#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"

#include <windows.h>

void UIJoinPartyAcceptWnd::OpenInvite(u32 partyId, const char* partyName)
{
    m_partyId = partyId;
    m_partyName = partyName ? partyName : "";
    MarkPendingAndShow();
}

int UIJoinPartyAcceptWnd::GetWindowId() const
{
    return UIWindowMgr::WID_JOINPARTYACCEPTWND;
}

void UIJoinPartyAcceptWnd::GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const
{
    fillRgb = RGB(92, 111, 96);
    frameRgb = RGB(60, 74, 62);
}

std::string UIJoinPartyAcceptWnd::ComposeBodyText() const
{
    return (m_partyName.empty() ? std::string("Party") : m_partyName) + "\nSuggest Join Party";
}

void UIJoinPartyAcceptWnd::ComposeMessageLines(std::vector<std::string>& outLines) const
{
    outLines.push_back(m_partyName.empty() ? "Party" : m_partyName);
    outLines.push_back("Suggest Join Party");
}

bool UIJoinPartyAcceptWnd::OnSubmit(bool accept)
{
    const u32 partyId = m_partyId;
    m_partyId = 0;
    m_partyName.clear();

    if (partyId == 0) {
        return false;
    }
    return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestPartyInviteReply, partyId, accept ? 1 : 0, 0) != 0;
}

unsigned long long UIJoinPartyAcceptWnd::HashSubclassState() const
{
    unsigned long long token = static_cast<unsigned long long>(m_partyId);
    for (unsigned char ch : m_partyName) {
        token ^= static_cast<unsigned long long>(ch);
        token *= 1099511628211ull;
    }
    return token;
}
