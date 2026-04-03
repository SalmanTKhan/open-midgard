#include "RenderDevice.h"

#include <algorithm>

namespace {

QtUiRenderTargetInfo MakeUnavailableQtUiRenderTargetInfo(RenderBackendType backend, int width, int height)
{
    QtUiRenderTargetInfo info{};
    info.backend = backend;
    info.width = width > 0 ? static_cast<unsigned int>(width) : 0u;
    info.height = height > 0 ? static_cast<unsigned int>(height) : 0u;
    return info;
}

class UnsupportedRenderDevice final : public IRenderDevice {
public:
    UnsupportedRenderDevice()
        : m_backend(RO_HAS_VULKAN ? RenderBackendType::Vulkan : RenderBackendType::LegacyDirect3D7)
    {
    }

    RenderBackendType GetBackendType() const override { return m_backend; }

    bool Initialize(RoNativeWindowHandle hwnd, RenderBackendBootstrapResult* outResult) override
    {
        m_window = hwnd;
        if (outResult) {
            outResult->backend = m_backend;
            outResult->initHr = -1;
        }
        return false;
    }

    void Shutdown() override
    {
        m_window = nullptr;
    }

    void RefreshRenderSize() override {}
    int GetRenderWidth() const override { return 0; }
    int GetRenderHeight() const override { return 0; }
    RoNativeWindowHandle GetWindowHandle() const override { return m_window; }
    IDirect3DDevice7* GetLegacyDevice() const override { return nullptr; }
    int ClearColor(unsigned int color) override { (void)color; return -1; }
    int ClearDepth() override { return -1; }
    int Present(bool vertSync) override { (void)vertSync; return -1; }
    bool UpdateBackBufferFromMemory(const void* bgraPixels, int width, int height, int pitch) override
    {
        (void)bgraPixels;
        (void)width;
        (void)height;
        (void)pitch;
        return false;
    }

    bool BeginScene() override { return false; }
    bool PrepareOverlayPass() override { return false; }
    void EndScene() override {}
    void SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) override { (void)state; (void)matrix; }
    void SetRenderState(D3DRENDERSTATETYPE state, RoDword value) override { (void)state; (void)value; }
    void SetTextureStageState(RoDword stage, D3DTEXTURESTAGESTATETYPE type, RoDword value) override
    {
        (void)stage;
        (void)type;
        (void)value;
    }
    void BindTexture(RoDword stage, CTexture* texture) override { (void)stage; (void)texture; }
    void DrawPrimitive(D3DPRIMITIVETYPE primitiveType, RoDword vertexFormat,
        const void* vertices, RoDword vertexCount, RoDword flags) override
    {
        (void)primitiveType;
        (void)vertexFormat;
        (void)vertices;
        (void)vertexCount;
        (void)flags;
    }
    void DrawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, RoDword vertexFormat,
        const void* vertices, RoDword vertexCount, const unsigned short* indices,
        RoDword indexCount, RoDword flags) override
    {
        (void)primitiveType;
        (void)vertexFormat;
        (void)vertices;
        (void)vertexCount;
        (void)indices;
        (void)indexCount;
        (void)flags;
    }

    void AdjustTextureSize(unsigned int* width, unsigned int* height) override
    {
        (void)width;
        (void)height;
    }

    void ReleaseTextureResource(CTexture* texture) override
    {
        (void)texture;
    }

    bool CreateTextureResource(CTexture* texture, unsigned int requestedWidth, unsigned int requestedHeight,
        int pixelFormat, unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight) override
    {
        (void)texture;
        (void)requestedWidth;
        (void)requestedHeight;
        (void)pixelFormat;
        if (outSurfaceWidth) {
            *outSurfaceWidth = 0u;
        }
        if (outSurfaceHeight) {
            *outSurfaceHeight = 0u;
        }
        return false;
    }

    bool UpdateTextureResource(CTexture* texture, int x, int y, int w, int h,
        const unsigned int* data, bool skipColorKey, int pitch) override
    {
        (void)texture;
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)data;
        (void)skipColorKey;
        (void)pitch;
        return false;
    }

    bool GetQtUiRenderTargetInfo(QtUiRenderTargetInfo* outInfo) const override
    {
        if (outInfo) {
            *outInfo = MakeUnavailableQtUiRenderTargetInfo(m_backend, 0, 0);
        }
        return false;
    }

    bool GetQtUiTextureTargetInfo(const CTexture* texture, QtUiRenderTargetInfo* outInfo) const override
    {
        (void)texture;
        if (outInfo) {
            *outInfo = MakeUnavailableQtUiRenderTargetInfo(m_backend, 0, 0);
        }
        return false;
    }

private:
    RenderBackendType m_backend;
    RoNativeWindowHandle m_window = nullptr;
};

} // namespace

IRenderDevice& GetRenderDevice()
{
    static UnsupportedRenderDevice device;
    return device;
}