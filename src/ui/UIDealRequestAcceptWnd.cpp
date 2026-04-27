#include "UIDealRequestAcceptWnd.h"

#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"

#include <windows.h>

void UIDealRequestAcceptWnd::OpenInvite(const char* inviterName)
{
    m_inviterName = inviterName ? inviterName : "";
    MarkPendingAndShow();
}

int UIDealRequestAcceptWnd::GetWindowId() const
{
    return UIWindowMgr::WID_DEALREQUESTACCEPTWND;
}

void UIDealRequestAcceptWnd::GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const
{
    fillRgb = RGB(126, 96, 62);
    frameRgb = RGB(80, 60, 38);
}

std::string UIDealRequestAcceptWnd::ComposeBodyText() const
{
    const std::string nameLine = m_inviterName.empty() ? std::string("Unknown player") : m_inviterName;
    return nameLine + "\nWants to trade with you";
}

void UIDealRequestAcceptWnd::ComposeMessageLines(std::vector<std::string>& outLines) const
{
    outLines.push_back(m_inviterName.empty() ? "Unknown player" : m_inviterName);
    outLines.push_back("Wants to trade with you");
}

bool UIDealRequestAcceptWnd::OnSubmit(bool accept)
{
    m_inviterName.clear();
    // CZ_ACK_EXCHANGE_ITEM (0x00E6) result codes per rAthena trade.hpp e_ack_trade_response:
    // 3 = ACCEPT, 4 = CANCEL (reject). Server tracks the trade partner via session
    // state, so the ack carries no identity — just the result byte.
    return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestDealReply,
        accept ? 1 : 0, 0, 0) != 0;
}

unsigned long long UIDealRequestAcceptWnd::HashSubclassState() const
{
    unsigned long long token = 0;
    for (unsigned char ch : m_inviterName) {
        token ^= static_cast<unsigned long long>(ch);
        token *= 1099511628211ull;
    }
    return token;
}
