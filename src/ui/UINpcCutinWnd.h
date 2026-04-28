#pragma once

#include "UIFrameWnd.h"
#include "UIShopCommon.h"

#include <string>

// Renders a single full-image NPC "cutin" overlay driven by the server packets
// ZC_SHOW_IMAGE (0x0145) and ZC_SHOW_IMAGE2 (0x01B3). Image lives at
// data/illust/<name>.bmp; position is one of the classic 2008 anchors.
//
// The window auto-clears when the server sends an empty filename or the
// special hide marker (position == 0xFF). It is non-interactive: clicks pass
// through to whatever is underneath.
class UINpcCutinWnd : public UIFrameWnd {
public:
    enum CutinPosition : unsigned char {
        Pos_BottomLeft   = 0,
        Pos_BottomCenter = 1,
        Pos_BottomRight  = 2,
        Pos_MiddleExt    = 3,  // roBrowser falls through to MiddleCenter
        Pos_MiddleCenter = 4,
        Pos_Hide         = 0xFF,
    };

    UINpcCutinWnd();
    ~UINpcCutinWnd() override;

    void OnDraw() override;
    bool IsUpdateNeed() override;

    // Driven by packet handlers in GameModePacket.cpp. Empty filename or
    // position == Pos_Hide hides the overlay.
    void SetCutin(const std::string& imageName, unsigned char position);
    void ClearCutin();

    bool HasCutin() const;
    const std::string& GetImageName() const;
    unsigned char GetPosition() const;

private:
    void Reposition();
    bool LoadBitmapForCurrentName();

    std::string m_imageName;
    unsigned char m_position = Pos_Hide;
    shopui::BitmapPixels m_bitmap;
};
