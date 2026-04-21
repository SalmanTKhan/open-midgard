#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UIEmotionWnd : public UIFrameWnd {
public:
    struct EmotionButton {
        int emotionType = 0;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        std::string label;
        std::string hotkey;
        bool hovered = false;
        bool pressed = false;
    };

    struct DisplayData {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        std::string title;
        std::string footer;
        std::vector<EmotionButton> buttons;
    };

    UIEmotionWnd();
    ~UIEmotionWnd() override;

    void SetShow(int show) override;
    void Move(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnKeyDown(int virtualKey) override;
    void StoreInfo() override;

    bool GetDisplayDataForQt(DisplayData* outData) const;

private:
    enum : int {
        ButtonClose = 9000,
    };

    static constexpr int kButtonCount = 10;

    void EnsureCreated();
    void CloseWindow();
    bool ActivateEmotion(int emotionType);
    int HitTestButton(int x, int y) const;
    bool HitTestClose(int x, int y) const;
    RECT GetButtonRect(int index) const;
    RECT GetCloseRect() const;
    bool IsPointInRect(const RECT& rect, int x, int y) const;

    bool m_controlsCreated;
    int m_hoveredButton;
    int m_pressedButton;
};
