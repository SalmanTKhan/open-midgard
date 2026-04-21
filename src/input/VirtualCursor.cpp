#include "VirtualCursor.h"

#include <algorithm>
#include <cmath>

namespace gamepad {

void VirtualCursor::Reset(int clientWidth, int clientHeight)
{
    SetClientSize(clientWidth, clientHeight);
    m_fx = static_cast<float>(clientWidth) * 0.5f;
    m_fy = static_cast<float>(clientHeight) * 0.5f;
    m_x = static_cast<int>(m_fx);
    m_y = static_cast<int>(m_fy);
}

void VirtualCursor::SetClientSize(int clientWidth, int clientHeight)
{
    if (clientWidth > 0)  m_clientW = clientWidth;
    if (clientHeight > 0) m_clientH = clientHeight;
}

void VirtualCursor::SetPosition(int x, int y)
{
    m_fx = std::clamp(static_cast<float>(x), 0.0f, static_cast<float>(m_clientW - 1));
    m_fy = std::clamp(static_cast<float>(y), 0.0f, static_cast<float>(m_clientH - 1));
    m_x = static_cast<int>(m_fx);
    m_y = static_cast<int>(m_fy);
}

bool VirtualCursor::Tick(float axisX, float axisY, float dtSeconds)
{
    const float magSq = axisX * axisX + axisY * axisY;
    if (magSq <= 1e-6f || dtSeconds <= 0.0f) {
        return false;
    }

    // Quadratic ramp so small stick deflections give precise control.
    const float mag = std::sqrt(magSq);
    const float scaled = mag * mag;
    const float nx = axisX / mag * scaled;
    const float ny = axisY / mag * scaled;

    m_fx += nx * m_speedPxPerSec * dtSeconds;
    m_fy += ny * m_speedPxPerSec * dtSeconds;

    m_fx = std::clamp(m_fx, 0.0f, static_cast<float>(m_clientW - 1));
    m_fy = std::clamp(m_fy, 0.0f, static_cast<float>(m_clientH - 1));

    const int nxI = static_cast<int>(m_fx);
    const int nyI = static_cast<int>(m_fy);
    if (nxI == m_x && nyI == m_y) {
        return false;
    }
    m_x = nxI;
    m_y = nyI;
    return true;
}

}  // namespace gamepad
