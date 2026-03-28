#pragma once

#include "UIFrameWnd.h"

class UIBitmapButton;

class UINotifyLevelUpWnd : public UIFrameWnd {
public:
    UINotifyLevelUpWnd();
    ~UINotifyLevelUpWnd() override;

    void SetShow(int show) override;
    bool IsUpdateNeed() override;
    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnProcess() override;
    msgresult_t SendMsg(UIWindow* sender, int msg, msgparam_t wparam, msgparam_t lparam, msgparam_t extra) override;

protected:
    virtual int GetTargetWindowId() const;

private:
    void EnsureCreated();
    void UpdateAnchor();

    bool m_controlsCreated;
    UIBitmapButton* m_button;
};

class UINotifyJobLevelUpWnd : public UINotifyLevelUpWnd {
public:
    UINotifyJobLevelUpWnd();
    ~UINotifyJobLevelUpWnd() override;

protected:
    int GetTargetWindowId() const override;
};