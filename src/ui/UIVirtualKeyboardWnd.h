#pragma once

#include "UIFrameWnd.h"

// Gamepad-driven on-screen keyboard. Rendered as an ordinary legacy UI window
// but takes no mouse input — typing is driven by the gamepad. Typed characters
// are routed back through g_windowMgr.OnChar so the focused UIEditCtrl /
// UINewChatWnd field consumes them via the existing WM_CHAR pipeline.
class UIVirtualKeyboardWnd : public UIFrameWnd {
public:
    enum Page {
        PAGE_LOWER   = 0,
        PAGE_UPPER   = 1,
        PAGE_SYMBOLS = 2,
        PAGE_COUNT   = 3,
    };

    static constexpr int kRows = 4;
    static constexpr int kCols = 10;

    UIVirtualKeyboardWnd();
    ~UIVirtualKeyboardWnd() override;

    void OnCreate(int cx, int cy) override;
    void OnDraw() override;
    void SetShow(int show) override;

    // Gamepad-driven API — all no-ops when m_show == 0.
    void MoveHighlight(int dx, int dy);
    void ActivateHighlight();
    void ToggleShift();
    void CyclePage(int direction);
    void SendBackspace();
    void SendSpace();
    void Submit();
    void Cancel();

    // Layout helper so UIWindowMgr can re-center on client resize.
    void PositionBottomCenter(int clientWidth, int clientHeight);

private:
    char KeyAt(int row, int col) const;

    Page m_page;
    int  m_row;
    int  m_col;
    int  m_pressedFrames;  // brief highlight flash after ActivateHighlight
    bool m_everShownLogged;
};
