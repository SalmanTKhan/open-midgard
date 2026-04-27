#pragma once

#include "UIYesNoDialogWnd.h"

class UIDealRequestAcceptWnd : public UIYesNoDialogWnd {
public:
    void OpenInvite(const char* inviterName);

protected:
    int GetWindowId() const override;
    const char* GetTitleText() const override { return "Trade Request"; }
    void GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const override;
    std::string ComposeBodyText() const override;
    void ComposeMessageLines(std::vector<std::string>& outLines) const override;
    bool OnSubmit(bool accept) override;
    unsigned long long HashSubclassState() const override;

private:
    std::string m_inviterName;
};
