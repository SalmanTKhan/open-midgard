#pragma once

#include "UIFrameWnd.h"
#include "input/HotkeyBindings.h"

#include <string>
#include <vector>

// Controller + keyboard hotkey manager window. Shows live controller state,
// a movement-mode toggle, and editable keyboard/gamepad binding lists.
class UIControllerWnd : public UIFrameWnd {
public:
    struct DisplayTab {
        int id = 0;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool active = false;
        std::string label;
    };

    struct DisplayRow {
        int id = 0;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool selected = false;
        bool rebinding = false;
        std::string label;
        std::string bindingText;
    };

    struct DisplayData {
        int activeTab = 0;
        bool connected = false;
        std::string controllerName;
        std::string controllerType;
        std::string moveModeText;
        std::vector<DisplayTab> tabs;
        std::vector<DisplayRow> rows;
    };

    enum class BindingTab {
        Keyboard = 0,
        Gamepad = 1,
    };

    UIControllerWnd();
    ~UIControllerWnd() override;

    void OnCreate(int cx, int cy) override;
    void OnDraw() override;
    void OnProcess() override;
    void SetShow(int show) override;

    void OnLBtnDown(int x, int y) override;
    void OnMouseMove(int x, int y) override;
    void OnWheel(int delta) override;
    void OnKeyDown(int virtualKey) override;

    void PositionCentered(int clientWidth, int clientHeight);
    bool GetDisplayDataForQt(DisplayData* outData) const;
    bool IsRebinding() const;

private:
    int RowAtPoint(int x, int y) const;
    BindingTab CurrentTab() const;
    int RowCountForTab(BindingTab tab) const;
    int VisibleRowCount() const;
    int MaxScrollOffset() const;
    void ClampScrollOffset();
    void EnsureSelectedRowVisible();
    bool IsInTitleBar(int x, int y) const;
    hotkeys::KeyboardAction KeyboardActionAtRow(int row) const;
    hotkeys::GamepadAction GamepadActionAtRow(int row) const;

    void BeginKeyboardRebind(hotkeys::KeyboardAction action);
    void BeginGamepadRebind(hotkeys::GamepadAction action);
    void CancelRebind();

    void DrawTabs(HDC hdc);
    void DrawBindings(HDC hdc);

    BindingTab m_activeTab;
    int m_selectedRow;
    int m_listScrollOffset;
    bool m_keyboardRebinding;
    hotkeys::KeyboardAction m_keyboardRebindAction;
};
