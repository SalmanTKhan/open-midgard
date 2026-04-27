#include "UIFriendRequestAcceptWnd.h"

#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"

#include <windows.h>

void UIFriendRequestAcceptWnd::OpenInvite(u32 inviterAid, u32 inviterCid, const char* inviterName)
{
    m_inviterAid = inviterAid;
    m_inviterCid = inviterCid;
    m_inviterName = inviterName ? inviterName : "";
    MarkPendingAndShow();
}

int UIFriendRequestAcceptWnd::GetWindowId() const
{
    return UIWindowMgr::WID_FRIENDREQUESTACCEPTWND;
}

void UIFriendRequestAcceptWnd::GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const
{
    fillRgb = RGB(92, 96, 111);
    frameRgb = RGB(60, 62, 74);
}

std::string UIFriendRequestAcceptWnd::ComposeBodyText() const
{
    const std::string nameLine = m_inviterName.empty() ? std::string("Unknown player") : m_inviterName;
    return nameLine + "\nWants to add you as a friend";
}

void UIFriendRequestAcceptWnd::ComposeMessageLines(std::vector<std::string>& outLines) const
{
    outLines.push_back(m_inviterName.empty() ? "Unknown player" : m_inviterName);
    outLines.push_back("Wants to add you as a friend");
}

bool UIFriendRequestAcceptWnd::OnSubmit(bool accept)
{
    const u32 inviterAid = m_inviterAid;
    const u32 inviterCid = m_inviterCid;
    m_inviterAid = 0;
    m_inviterCid = 0;
    m_inviterName.clear();

    if (inviterAid == 0 || inviterCid == 0) {
        return false;
    }
    // CZ_ACK_REQ_ADD_FRIENDS needs both aid and cid; carry cid in `extra` so the
    // dispatcher can build the 14-byte packet without storing pending state on
    // the mode object.
    return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestFriendReply,
        inviterAid,
        accept ? 1 : 0,
        static_cast<msgparam_t>(inviterCid)) != 0;
}

unsigned long long UIFriendRequestAcceptWnd::HashSubclassState() const
{
    unsigned long long token = static_cast<unsigned long long>(m_inviterAid);
    token ^= static_cast<unsigned long long>(m_inviterCid) << 1;
    for (unsigned char ch : m_inviterName) {
        token ^= static_cast<unsigned long long>(ch);
        token *= 1099511628211ull;
    }
    return token;
}
