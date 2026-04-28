#pragma once

#include "UIFrameWnd.h"

#include <string>

// HUD banner shown while a ground-AoE skill is "armed" (the player picked the
// skill from UISkillListWnd or via shortcut, but hasn't clicked a target cell
// yet). Polls `g_modeMgr.GetCurrentGameMode()->m_skillUseInfo` every frame —
// hides itself once the skill is fired or cancelled.
//
// First-cut scope is a screen-space banner rather than a true world-space
// floor reticle. The banner solves the UX problem of "player armed an AoE
// and didn't notice" without the render-pipeline refactor a true reticle
// would require.
class UISkillCastIndicatorWnd : public UIFrameWnd {
public:
    UISkillCastIndicatorWnd();
    ~UISkillCastIndicatorWnd() override;

    void OnDraw() override;
    bool IsUpdateNeed() override;

private:
    void Reposition();
    int m_lastShownSkillId = 0;
};
