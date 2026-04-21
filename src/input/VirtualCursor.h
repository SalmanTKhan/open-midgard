#pragma once

namespace gamepad {

// Stick-driven on-screen cursor used while the gamepad is steering the legacy
// UI. The world surface gets the same cursor (synthesizing mouse clicks for
// click-to-move).
class VirtualCursor {
public:
    void Reset(int clientWidth, int clientHeight);
    void SetClientSize(int clientWidth, int clientHeight);

    // axisX/axisY in [-1, 1]. dtSeconds is wall-clock since the previous tick.
    // Returns true when the cursor moved enough to warrant a synthesized
    // OnMouseMove.
    bool Tick(float axisX, float axisY, float dtSeconds);
    void SetPosition(int x, int y);

    int X() const { return m_x; }
    int Y() const { return m_y; }
    void SetSpeedPxPerSec(float speed) { m_speedPxPerSec = speed; }

private:
    float m_fx = 0.0f;
    float m_fy = 0.0f;
    int   m_x = 0;
    int   m_y = 0;
    int   m_clientW = 1920;
    int   m_clientH = 1080;
    float m_speedPxPerSec = 1400.0f;
};

}  // namespace gamepad
