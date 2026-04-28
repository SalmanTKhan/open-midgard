#pragma once

#include "UIFrameWnd.h"

#include <string>
#include <vector>

// Minimal first-cut WorldMap window: a grid of canonical Rune-Midgard town
// tiles. Click a tile to select it; the bottom status line shows the friendly
// name + map ID. Asset-driven background and warp packets are out of scope
// for this first cut.
class UIWorldMapWnd : public UIFrameWnd {
public:
    UIWorldMapWnd();
    ~UIWorldMapWnd() override;

    void OnCreate(int x, int y) override;
    void OnDraw() override;
    void OnLBtnDown(int x, int y) override;
    void OnLBtnUp(int x, int y) override;

private:
    enum ButtonId {
        ButtonClose = 1,
    };

    struct Tile {
        std::string mapId;       // e.g. "prontera"
        std::string displayName; // resolved on construction
    };

    void EnsureTiles();
    RECT GetTileRect(int index) const;
    RECT GetCloseButtonRect() const;
    int HitTestTile(int x, int y) const;
    int HitTestButton(int x, int y) const;

    std::vector<Tile> m_tiles;
    int m_selectedTile = -1;
    int m_pressedTile = -1;
    int m_pressedButton = 0;
};
