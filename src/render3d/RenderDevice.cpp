#include "RenderDevice.h"

#include "Device.h"

#include <algorithm>

namespace {

class LegacyRenderDevice final : public IRenderDevice {
public:
    LegacyRenderDevice()
        : m_hwnd(nullptr), m_renderWidth(0), m_renderHeight(0)
    {
        m_bootstrap.backend = RenderBackendType::LegacyDirect3D7;
        m_bootstrap.initHr = -1;
    }

    bool Initialize(HWND hwnd, RenderBackendBootstrapResult* outResult) override
    {
        m_hwnd = hwnd;
        const bool ok = InitializeRenderBackend(hwnd, &m_bootstrap);
        RefreshRenderSize();
        if (outResult) {
            *outResult = m_bootstrap;
        }
        return ok;
    }

    void Shutdown() override
    {
        g_3dDevice.DestroyObjects();
        m_renderWidth = 0;
        m_renderHeight = 0;
    }

    void RefreshRenderSize() override
    {
        if (!m_hwnd) {
            m_renderWidth = 0;
            m_renderHeight = 0;
            return;
        }

        RECT clientRect{};
        GetClientRect(m_hwnd, &clientRect);
        m_renderWidth = (std::max)(1L, clientRect.right - clientRect.left);
        m_renderHeight = (std::max)(1L, clientRect.bottom - clientRect.top);
    }

    int GetRenderWidth() const override
    {
        return m_renderWidth;
    }

    int GetRenderHeight() const override
    {
        return m_renderHeight;
    }

    HWND GetWindowHandle() const override
    {
        return m_hwnd;
    }

    IDirect3DDevice7* GetLegacyDevice() const override
    {
        return g_3dDevice.m_pd3dDevice;
    }

    int ClearColor(unsigned int color) override
    {
        return g_3dDevice.Clear(color);
    }

    int ClearDepth() override
    {
        return g_3dDevice.ClearZBuffer();
    }

    int Present(bool vertSync) override
    {
        return g_3dDevice.ShowFrame(vertSync);
    }

private:
    HWND m_hwnd;
    int m_renderWidth;
    int m_renderHeight;
    RenderBackendBootstrapResult m_bootstrap;
};

} // namespace

IRenderDevice& GetRenderDevice()
{
    static LegacyRenderDevice s_renderDevice;
    return s_renderDevice;
}