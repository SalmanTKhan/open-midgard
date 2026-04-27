#pragma once

#include "UIYesNoDialogWnd.h"

class UIFriendRequestAcceptWnd : public UIYesNoDialogWnd {
public:
    void OpenInvite(u32 inviterAid, u32 inviterCid, const char* inviterName);

protected:
    int GetWindowId() const override;
    const char* GetTitleText() const override { return "Friend Request"; }
    void GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const override;
    std::string ComposeBodyText() const override;
    void ComposeMessageLines(std::vector<std::string>& outLines) const override;
    bool OnSubmit(bool accept) override;
    unsigned long long HashSubclassState() const override;

private:
    u32 m_inviterAid = 0;
    u32 m_inviterCid = 0;
    std::string m_inviterName;
};
