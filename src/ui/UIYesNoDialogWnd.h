#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

// Shared base for small modal Accept / Decline confirmation dialogs (party
// invite, friend request, deal request, ...). Subclasses provide title text,
// title bar colors, body text composition, and the SendMsg dispatch performed
// when the user accepts or declines. Layout, drawing, hit-testing, key
// handling, and Qt mirror data are owned here.
class UIYesNoDialogWnd : public UIFrameWnd {
public:
    enum ButtonId {
        ButtonClose = 1,
        ButtonAccept = 2,
        ButtonDecline = 3,
    };

    struct QtButtonDisplay {
        int id = 0;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        std::string label;
        bool active = false;
    };

    struct DisplayData {
        std::string title;
        std::string bodyText;
        std::vector<std::string> messageLines;
        std::vector<QtButtonDisplay> buttons;
    };

    UIYesNoDialogWnd();
    ~UIYesNoDialogWnd() override;

    void SetShow(int show) override;
    bool IsUpdateNeed() override;
    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnKeyDown(int virtualKey) override;
    bool HandleKeyDown(int virtualKey);

    bool GetDisplayDataForQt(DisplayData* outData) const;

protected:
    // Subclass-supplied configuration.
    virtual int GetWindowId() const = 0;
    virtual const char* GetTitleText() const = 0;
    virtual void GetTitleColors(COLORREF& fillRgb, COLORREF& frameRgb) const = 0;
    virtual std::string ComposeBodyText() const = 0;
    virtual void ComposeMessageLines(std::vector<std::string>& outLines) const = 0;
    virtual bool OnSubmit(bool accept) = 0;
    virtual unsigned long long HashSubclassState() const { return 0; }

    // Center the dialog under the main window, mark a pending invite, reset
    // button state, show, and invalidate. Subclasses call this from their
    // typed Open* method after storing their state.
    void MarkPendingAndShow();

private:
    RECT GetCloseButtonRect() const;
    RECT GetButtonRect(int buttonId) const;
    int HitTestButton(int x, int y) const;
    bool SubmitReply(bool accept);
    unsigned long long BuildVisualStateToken() const;

    int m_hoverButtonId;
    int m_pressedButtonId;
    unsigned long long m_lastVisualStateToken;
    bool m_hasVisualStateToken;
    bool m_controlsCreated;
    bool m_hasPending;
};
