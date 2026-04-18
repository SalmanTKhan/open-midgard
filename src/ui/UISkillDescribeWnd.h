#pragma once

#include "session/Session.h"
#include "UIFrameWnd.h"
#include "UIShopCommon.h"

#include <string>
#include <vector>

class UISkillDescribeWnd : public UIFrameWnd {
public:
    struct DisplayButton {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool hovered = false;
        bool pressed = false;
        std::string label;
    };

    struct DisplayData {
        std::string title;
        int skillId = 0;
        std::string name;
        std::vector<std::string> detailLines;
        std::string description;
        DisplayButton closeButton;
    };

    UISkillDescribeWnd();
    ~UISkillDescribeWnd() override = default;

    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnMouseHover(int x, int y) override;
    void StoreInfo() override;
    void SetSkillInfo(const PLAYER_SKILL_INFO& skillInfo, int preferredX, int preferredY);
    bool GetDisplayDataForQt(DisplayData* outData) const;
    bool HasSkill() const;

private:
    RECT GetCloseButtonRect() const;
    void UpdateInteraction(int x, int y);
    std::string BuildDescriptionText() const;

    PLAYER_SKILL_INFO m_skillInfo;
    bool m_hasSkill;
    bool m_closeHovered;
    bool m_closePressed;
    bool m_hasStoredPlacement;
    shopui::BitmapPixels m_iconBitmap;
};