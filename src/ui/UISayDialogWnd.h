#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

class UISayDialogWnd : public UIFrameWnd {
public:
    UISayDialogWnd();
    ~UISayDialogWnd() override;

    void SetShow(int show) override;
    void Move(int x, int y) override;
    void StoreInfo() override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnLBtnUp(int x, int y) override;
    void OnWheel(int delta) override;

    void AppendText(u32 npcId, const std::string& text);
    void ShowNext(u32 npcId);
    void ShowClose(u32 npcId);
    void ClearAction();
    void HideConversation();
    void SetScrollMetrics(int contentHeight, int viewportHeight);
    void SetScrollContentHeight(int contentHeight);
    int GetScrollOffset() const;
    int GetScrollContentHeight() const;
    bool HandleKeyDown(int virtualKey);
    u32 GetNpcId() const;
    std::string GetDisplayText() const;
    bool HasActionButton() const;
    bool IsNextAction() const;
    bool IsHoveringAction() const;
    bool IsPressingAction() const;
    bool GetActionRectForQt(RECT* outRect) const;

private:
    enum class ActionButton {
        None = 0,
        Next,
        Close,
    };

    RECT GetActionRect() const;
    RECT GetTextRect() const;
    bool IsPointInRect(const RECT& rect, int x, int y) const;
    void StartDragging(int x, int y);
    void StopDragging();
    void ClampScrollOffset();
    void AdjustScrollOffset(int delta);
    void DrawActionButton(HDC hdc, const RECT& rect) const;
    std::string BuildDisplayText() const;

    u32 m_npcId;
    std::vector<std::string> m_textLines;
    ActionButton m_actionButton;
    bool m_clearOnNextText;
    bool m_hoverAction;
    bool m_pressAction;
    int m_scrollOffset = 0;
    int m_scrollContentHeight = 0;
    int m_scrollViewportHeight = 0;
};
