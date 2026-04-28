#pragma once

#include "UIFrameWnd.h"

#include <string>

// Briefly displays the current map's name centered near the top of the
// screen, fading out after a short duration. Non-interactive.
class UIMapNameBannerWnd : public UIFrameWnd {
public:
    UIMapNameBannerWnd();
    ~UIMapNameBannerWnd() override;

    void OnDraw() override;
    bool IsUpdateNeed() override;

    // Resets the timer and shows the banner for the given map. The mapName is
    // expected to be the bare map ID (e.g. "prontera"); any trailing .rsw/.gat
    // is trimmed. Empty string hides immediately.
    void Trigger(const std::string& mapName, unsigned int durationMs = 3000);

    void Hide();

private:
    void Reposition();
    unsigned int ElapsedMs() const;

    std::string m_displayName;
    unsigned int m_startTickMs = 0;
    unsigned int m_durationMs = 0;
    bool m_active = false;
};
