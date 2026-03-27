#pragma once
#include "UIWindow.h"

//===========================================================================
// UIFrameWnd  –  Base class for draggable windows
//===========================================================================
class UIFrameWnd : public UIWindow {
public:
    UIFrameWnd();
    virtual ~UIFrameWnd();

    virtual bool IsFrameWnd() override { return true; }
    virtual int SendMsg(UIWindow* sender, int msg, int wparam, int lparam, int extra) override;
    virtual void OnLBtnDown(int x, int y) override;
    virtual void OnMouseMove(int x, int y) override;
    virtual void OnLBtnUp(int x, int y) override;

    // Memory layout from HighPriest.exe.h:10567
    int m_startGlobalX;
    int m_startGlobalY;
    int m_orgX;
    int m_orgY;
    int m_defPushId;
    int m_defCancelPushId;
    int m_isDragging;
};

