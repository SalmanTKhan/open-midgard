#pragma once

#include "UIYesNoDialogWnd.h"

#include <string>

// Confirmation dialog for skill point allocation. UISkillListWnd opens this
// when the player clicks the upgrade affordance instead of sending the CZ
// packet directly — gives a chance to back out of an accidental click.
class UISkillUpConfirmWnd : public UIYesNoDialogWnd {
public:
    void OpenConfirm(int skillId, const char* skillName, int currentLevel, int maxLevel);

protected:
    int GetWindowId() const override;
    const char* GetTitleText() const override { return "Confirm Skill Up"; }
    void GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const override;
    std::string ComposeBodyText() const override;
    void ComposeMessageLines(std::vector<std::string>& outLines) const override;
    bool OnSubmit(bool accept) override;
    unsigned long long HashSubclassState() const override;

private:
    int m_skillId = 0;
    int m_currentLevel = 0;
    int m_maxLevel = 0;
    std::string m_skillName;
};
