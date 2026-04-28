#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

// Dedicated party roster window. Shows party name, leader, member list with
// per-row map name and HP bar, plus footer buttons for Options / Expel /
// Leave / Close. Backed entirely by Session::m_partyList; no extra packet
// plumbing beyond the existing party handlers.
class UIPartyWnd : public UIFrameWnd {
public:
    UIPartyWnd();
    ~UIPartyWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;

private:
    enum ButtonId {
        ButtonNone = 0,
        ButtonClose = 1,
        ButtonOptions = 2,
        ButtonExpel = 3,
        ButtonLeave = 4,
    };

    RECT GetRowRect(int index) const;
    RECT GetButtonRect(int buttonId) const;
    int HitTestRow(int x, int y) const;
    int HitTestButton(int x, int y) const;
    int CountVisibleMembers() const;

    int m_selectedRow = -1;
    int m_pressedRow = -1;
    int m_pressedButton = ButtonNone;
};
