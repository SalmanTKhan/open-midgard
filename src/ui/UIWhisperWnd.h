#pragma once

#include "UIFrameWnd.h"

#include <deque>
#include <string>

// Per-target private chat window. UIWindowMgr keeps an instance per
// counterparty (keyed by lowercase name). Incoming whispers from a known
// counterparty are routed here by GameModePacket; outgoing whispers are
// composed via UINpcInputWnd::OpenGameStringPrompt and echoed locally.
class UIWhisperWnd : public UIFrameWnd {
public:
    UIWhisperWnd();
    ~UIWhisperWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnWheel(int delta) override;

    void SetTarget(const std::string& displayName);
    const std::string& GetTargetName() const { return m_targetName; }

    void AppendIncoming(const std::string& text);
    void AppendOutgoing(const std::string& text);
    void AppendSystem(const std::string& text);

private:
    enum ButtonId {
        ButtonNone = 0,
        ButtonClose = 1,
        ButtonReply = 2,
    };

    struct Line {
        std::string text;
        unsigned int color = 0x00FFFFFF;
    };

    void AppendLine(const std::string& text, unsigned int color);
    RECT GetButtonRect(int buttonId) const;
    int HitTestButton(int x, int y) const;

    std::string m_targetName;
    std::deque<Line> m_lines;
    int m_scrollOffset = 0;
    int m_pressedButton = ButtonNone;
};
