#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

// Lists inventory items eligible for refining (driven by
// ZC_NOTIFY_WEAPONITEMLIST / 0x0221) and lets the player pick one to send
// CZ_REQ_WEAPONREFINE (0x0222). The result is reported by the existing chat
// pipeline from ZC_ACK_WEAPONREFINE (0x0223).
class UIRefineWnd : public UIFrameWnd {
public:
    struct Candidate {
        unsigned int inventoryIndex = 0;  // server-emitted, may include offset
        unsigned int itemId = 0;
        unsigned char refine = 0;
        unsigned short cards[4] = { 0, 0, 0, 0 };
    };

    UIRefineWnd();
    ~UIRefineWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    bool IsUpdateNeed() override;

    void SetCandidates(std::vector<Candidate> candidates);
    void ClearCandidates();
    bool IsActive() const;
    const std::vector<Candidate>& GetCandidates() const;

private:
    enum ButtonId {
        ButtonClose = 1,
    };

    int HitTestRow(int x, int y) const;
    int HitTestButton(int x, int y) const;
    RECT GetCloseButtonRect() const;
    unsigned long long BuildVisualStateToken() const;

    std::vector<Candidate> m_candidates;
    bool m_active = false;
    int m_hoverRow = -1;
    int m_pressedRow = -1;
    int m_pressedButtonId = 0;
    unsigned long long m_lastVisualStateToken = 0;
};
