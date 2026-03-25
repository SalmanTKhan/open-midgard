#include "RenderDevice.h"

#include "Device.h"
#include "D3dutil.h"
#include "DebugLog.h"
#include "res/Texture.h"

#include <d3d11.h>
#include <algorithm>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace {

template <typename T>
void SafeRelease(T*& value)
{
    if (value) {
        value->Release();
        value = nullptr;
    }
}

unsigned int CountTrailingZeros(unsigned int mask)
{
    if (mask == 0u) {
        return 0u;
    }

    unsigned int shift = 0u;
    while ((mask & 1u) == 0u) {
        mask >>= 1u;
        ++shift;
    }
    return shift;
}

unsigned int CountBits(unsigned int mask)
{
    unsigned int bits = 0u;
    while (mask != 0u) {
        bits += mask & 1u;
        mask >>= 1u;
    }
    return bits;
}

unsigned int PackChannel(unsigned int value, unsigned int mask)
{
    if (mask == 0u) {
        return 0u;
    }

    const unsigned int shift = CountTrailingZeros(mask);
    const unsigned int bits = CountBits(mask);
    if (bits == 0u) {
        return 0u;
    }

    const unsigned int maxValue = (1u << bits) - 1u;
    const unsigned int scaled = (value * maxValue + 127u) / 255u;
    return (scaled << shift) & mask;
}

unsigned int ConvertArgbToSurfacePixel(unsigned int argb, const DDPIXELFORMAT& pf)
{
    const unsigned int alpha = (argb >> 24) & 0xFFu;
    const unsigned int red = (argb >> 16) & 0xFFu;
    const unsigned int green = (argb >> 8) & 0xFFu;
    const unsigned int blue = argb & 0xFFu;

    if (pf.dwRGBBitCount == 32
        && pf.dwRBitMask == 0x00FF0000u
        && pf.dwGBitMask == 0x0000FF00u
        && pf.dwBBitMask == 0x000000FFu
        && pf.dwRGBAlphaBitMask == 0xFF000000u) {
        return argb;
    }

    return PackChannel(alpha, pf.dwRGBAlphaBitMask)
        | PackChannel(red, pf.dwRBitMask)
        | PackChannel(green, pf.dwGBitMask)
        | PackChannel(blue, pf.dwBBitMask);
}

unsigned int GetSurfaceColorKey(const DDPIXELFORMAT& pf)
{
    return pf.dwRBitMask | pf.dwBBitMask;
}

void ReleaseTextureMembers(CTexture* texture)
{
    if (!texture) {
        return;
    }

    if (texture->m_pddsSurface) {
        texture->m_pddsSurface->Release();
        texture->m_pddsSurface = nullptr;
    }

    if (texture->m_backendTextureView) {
        texture->m_backendTextureView->Release();
        texture->m_backendTextureView = nullptr;
    }

    if (texture->m_backendTextureObject) {
        texture->m_backendTextureObject->Release();
        texture->m_backendTextureObject = nullptr;
    }
}

void WritePackedPixel(unsigned char* dst, unsigned int bytesPerPixel, unsigned int value)
{
    switch (bytesPerPixel) {
    case 4:
        *reinterpret_cast<unsigned int*>(dst) = value;
        break;
    case 3:
        dst[0] = static_cast<unsigned char>(value & 0xFFu);
        dst[1] = static_cast<unsigned char>((value >> 8) & 0xFFu);
        dst[2] = static_cast<unsigned char>((value >> 16) & 0xFFu);
        break;
    case 2:
        *reinterpret_cast<unsigned short*>(dst) = static_cast<unsigned short>(value & 0xFFFFu);
        break;
    case 1:
        *dst = static_cast<unsigned char>(value & 0xFFu);
        break;
    default:
        break;
    }
}

class LegacyRenderDevice final : public IRenderDevice {
public:
    LegacyRenderDevice()
        : m_hwnd(nullptr), m_renderWidth(0), m_renderHeight(0)
    {
        m_bootstrap.backend = RenderBackendType::LegacyDirect3D7;
        m_bootstrap.initHr = -1;
    }

    RenderBackendType GetBackendType() const override
    {
        return RenderBackendType::LegacyDirect3D7;
    }

    bool Initialize(HWND hwnd, RenderBackendBootstrapResult* outResult) override
    {
        Shutdown();
        m_hwnd = hwnd;
        GUID deviceCandidates[] = {
            IID_IDirect3DTnLHalDevice,
            IID_IDirect3DHALDevice,
            IID_IDirect3DRGBDevice
        };

        m_bootstrap.backend = RenderBackendType::LegacyDirect3D7;
        m_bootstrap.initHr = -1;
        for (GUID& deviceGuid : deviceCandidates) {
            m_bootstrap.initHr = g_3dDevice.Init(hwnd, nullptr, &deviceGuid, nullptr, 0);
            if (m_bootstrap.initHr >= 0) {
                break;
            }
        }

        RefreshRenderSize();
        if (outResult) {
            *outResult = m_bootstrap;
        }
        if (m_bootstrap.initHr >= 0) {
            DbgLog("[Render] Initialized backend '%s'.\n", GetRenderBackendName(m_bootstrap.backend));
        }
        return m_bootstrap.initHr >= 0;
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

    bool AcquireBackBufferDC(HDC* outDc) override
    {
        if (!outDc) {
            return false;
        }

        *outDc = nullptr;
        IDirectDrawSurface7* backBuffer = g_3dDevice.m_pddsBackBuffer;
        if (!backBuffer) {
            return false;
        }

        HDC dc = nullptr;
        if (FAILED(backBuffer->GetDC(&dc)) || !dc) {
            return false;
        }

        *outDc = dc;
        return true;
    }

    void ReleaseBackBufferDC(HDC dc) override
    {
        if (!dc) {
            return;
        }

        IDirectDrawSurface7* backBuffer = g_3dDevice.m_pddsBackBuffer;
        if (backBuffer) {
            backBuffer->ReleaseDC(dc);
        }
    }

    bool BeginScene() override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        return device && SUCCEEDED(device->BeginScene());
    }

    void EndScene() override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device) {
            device->EndScene();
        }
    }

    void SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device && matrix) {
            device->SetTransform(state, const_cast<D3DMATRIX*>(matrix));
        }
    }

    void SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device) {
            device->SetRenderState(state, value);
        }
    }

    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device) {
            device->SetTextureStageState(stage, type, value);
        }
    }

    void BindTexture(DWORD stage, CTexture* texture) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (!device) {
            return;
        }

        IDirectDrawSurface7* surface = texture ? texture->m_pddsSurface : nullptr;
        device->SetTexture(stage, surface);
    }

    void DrawPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, DWORD flags) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device && vertices && vertexCount > 0) {
            device->DrawPrimitive(primitiveType, vertexFormat, const_cast<void*>(vertices), vertexCount, flags);
        }
    }

    void DrawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, const unsigned short* indices,
        DWORD indexCount, DWORD flags) override
    {
        IDirect3DDevice7* device = g_3dDevice.m_pd3dDevice;
        if (device && vertices && vertexCount > 0 && indices && indexCount > 0) {
            device->DrawIndexedPrimitive(primitiveType, vertexFormat, const_cast<void*>(vertices), vertexCount,
                const_cast<unsigned short*>(indices), indexCount, flags);
        }
    }

    void AdjustTextureSize(unsigned int* width, unsigned int* height) override
    {
        if (!width || !height) {
            return;
        }
        g_3dDevice.AdjustTextureSize(width, height);
    }

    void ReleaseTextureResource(CTexture* texture) override
    {
        ReleaseTextureMembers(texture);
    }

    bool CreateTextureResource(CTexture* texture, unsigned int requestedWidth, unsigned int requestedHeight,
        int pixelFormat, unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight) override
    {
        (void)pixelFormat;
        if (!texture || !g_3dDevice.m_pDD) {
            return false;
        }

        ReleaseTextureMembers(texture);

        unsigned int surfaceWidth = requestedWidth;
        unsigned int surfaceHeight = requestedHeight;
        AdjustTextureSize(&surfaceWidth, &surfaceHeight);

        DDSURFACEDESC2 ddsd{};
        auto initDesc = [&](DWORD caps) {
            std::memset(&ddsd, 0, sizeof(ddsd));
            D3DUtil_InitSurfaceDesc(&ddsd, DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT, caps);
            ddsd.dwWidth = surfaceWidth;
            ddsd.dwHeight = surfaceHeight;
            ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
            ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
            ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
            ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FF;
            ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;
        };

        IDirectDrawSurface7* surface = nullptr;
        const DWORD preferredCaps = DDSCAPS_TEXTURE | (g_3dDevice.m_dwDeviceMemType ? g_3dDevice.m_dwDeviceMemType : DDSCAPS_SYSTEMMEMORY);
        initDesc(preferredCaps);
        HRESULT createHr = g_3dDevice.m_pDD->CreateSurface(&ddsd, &surface, nullptr);
        if (createHr != DD_OK && (preferredCaps & DDSCAPS_VIDEOMEMORY)) {
            initDesc(DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY);
            createHr = g_3dDevice.m_pDD->CreateSurface(&ddsd, &surface, nullptr);
        }

        if (createHr != DD_OK || !surface) {
            return false;
        }

        DDCOLORKEY colorKey{};
        colorKey.dwColorSpaceLowValue = GetSurfaceColorKey(ddsd.ddpfPixelFormat);
        colorKey.dwColorSpaceHighValue = colorKey.dwColorSpaceLowValue;
        surface->SetColorKey(DDCKEY_SRCBLT, &colorKey);

        if (outSurfaceWidth) {
            *outSurfaceWidth = surfaceWidth;
        }
        if (outSurfaceHeight) {
            *outSurfaceHeight = surfaceHeight;
        }
        texture->m_pddsSurface = surface;
        return true;
    }

    bool UpdateTextureResource(CTexture* texture, int x, int y, int w, int h,
        const unsigned int* data, bool skipColorKey, int pitch) override
    {
        IDirectDrawSurface7* surface = texture ? texture->m_pddsSurface : nullptr;
        if (!surface || !data || w <= 0 || h <= 0) {
            return false;
        }

        DDSURFACEDESC2 ddsd{};
        ddsd.dwSize = sizeof(ddsd);
        if (surface->Lock(nullptr, &ddsd, DDLOCK_WAIT, nullptr) != DD_OK) {
            return false;
        }

        unsigned char* dstBase = static_cast<unsigned char*>(ddsd.lpSurface);
        const unsigned int bytesPerPixel = (ddsd.ddpfPixelFormat.dwRGBBitCount + 7u) / 8u;
        const int srcPitch = pitch > 0 ? pitch : w * static_cast<int>(sizeof(unsigned int));
        const unsigned int colorKey = GetSurfaceColorKey(ddsd.ddpfPixelFormat);
        for (int row = 0; row < h; ++row) {
            unsigned char* dstRow = dstBase + (y + row) * static_cast<int>(ddsd.lPitch) + x * static_cast<int>(bytesPerPixel);
            const unsigned int* srcRow = reinterpret_cast<const unsigned int*>(reinterpret_cast<const unsigned char*>(data) + static_cast<size_t>(row) * static_cast<size_t>(srcPitch));
            for (int col = 0; col < w; ++col) {
                const unsigned int srcPixel = srcRow[col];
                unsigned int packedPixel = ConvertArgbToSurfacePixel(srcPixel, ddsd.ddpfPixelFormat);
                if (!skipColorKey && (srcPixel & 0x00FFFFFFu) == 0x00FF00FFu) {
                    packedPixel = colorKey;
                }
                WritePackedPixel(dstRow + static_cast<size_t>(col) * bytesPerPixel, bytesPerPixel, packedPixel);
            }
        }

        surface->Unlock(nullptr);
        return true;
    }

private:
    HWND m_hwnd;
    int m_renderWidth;
    int m_renderHeight;
    RenderBackendBootstrapResult m_bootstrap;
};

class D3D11RenderDevice final : public IRenderDevice {
public:
    D3D11RenderDevice()
        : m_hwnd(nullptr), m_renderWidth(0), m_renderHeight(0),
          m_swapChain(nullptr), m_device(nullptr), m_context(nullptr), m_renderTargetView(nullptr),
            m_warnedLegacyOps(false)
    {
    }

    RenderBackendType GetBackendType() const override
    {
        return RenderBackendType::Direct3D11;
    }

    bool Initialize(HWND hwnd, RenderBackendBootstrapResult* outResult) override
    {
        Shutdown();
        m_hwnd = hwnd;

        DXGI_SWAP_CHAIN_DESC swapChainDesc{};
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        const D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

        const UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            createFlags,
            featureLevels,
            static_cast<UINT>(std::size(featureLevels)),
            D3D11_SDK_VERSION,
            &swapChainDesc,
            &m_swapChain,
            &m_device,
            &featureLevel,
            &m_context);
        if (FAILED(hr)) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr,
                D3D_DRIVER_TYPE_WARP,
                nullptr,
                createFlags,
                featureLevels,
                static_cast<UINT>(std::size(featureLevels)),
                D3D11_SDK_VERSION,
                &swapChainDesc,
                &m_swapChain,
                &m_device,
                &featureLevel,
                &m_context);
        }

        RenderBackendBootstrapResult result{};
        result.backend = RenderBackendType::Direct3D11;
        result.initHr = static_cast<int>(hr);
        if (FAILED(hr)) {
            Shutdown();
            if (outResult) {
                *outResult = result;
            }
            return false;
        }

        if (!RefreshRenderTarget()) {
            result.initHr = -1;
            Shutdown();
            if (outResult) {
                *outResult = result;
            }
            return false;
        }

        RefreshRenderSize();
        DbgLog("[Render] Initialized backend '%s' with feature level 0x%04X.\n",
            GetRenderBackendName(result.backend),
            static_cast<unsigned int>(featureLevel));

        if (outResult) {
            *outResult = result;
        }
        return true;
    }

    void Shutdown() override
    {
        SafeRelease(m_renderTargetView);
        SafeRelease(m_context);
        SafeRelease(m_device);
        SafeRelease(m_swapChain);
        m_renderWidth = 0;
        m_renderHeight = 0;
        m_hwnd = nullptr;
        m_warnedLegacyOps = false;
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
        const int newWidth = (std::max)(1L, clientRect.right - clientRect.left);
        const int newHeight = (std::max)(1L, clientRect.bottom - clientRect.top);
        if (newWidth != m_renderWidth || newHeight != m_renderHeight) {
            m_renderWidth = newWidth;
            m_renderHeight = newHeight;
            ResizeSwapChainBuffers();
        }
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
        return nullptr;
    }

    int ClearColor(unsigned int color) override
    {
        if (!m_context || !m_renderTargetView) {
            return -1;
        }

        const float clearColor[4] = {
            static_cast<float>((color >> 16) & 0xFFu) / 255.0f,
            static_cast<float>((color >> 8) & 0xFFu) / 255.0f,
            static_cast<float>(color & 0xFFu) / 255.0f,
            static_cast<float>((color >> 24) & 0xFFu) / 255.0f,
        };

        m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
        m_context->ClearRenderTargetView(m_renderTargetView, clearColor);
        return 0;
    }

    int ClearDepth() override
    {
        return 0;
    }

    int Present(bool vertSync) override
    {
        if (!m_swapChain) {
            return -1;
        }

        return static_cast<int>(m_swapChain->Present(vertSync ? 1 : 0, 0));
    }

    bool AcquireBackBufferDC(HDC* outDc) override
    {
        if (!outDc || !m_hwnd) {
            return false;
        }

        *outDc = GetDC(m_hwnd);
        return *outDc != nullptr;
    }

    void ReleaseBackBufferDC(HDC dc) override
    {
        if (m_hwnd && dc) {
            ReleaseDC(m_hwnd, dc);
        }
    }

    bool BeginScene() override
    {
        if (m_context && m_renderTargetView) {
            m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
        }
        return m_context != nullptr;
    }

    void EndScene() override
    {
    }

    void SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) override
    {
        (void)state;
        (void)matrix;
        WarnLegacyFixedFunctionUnavailable("SetTransform");
    }

    void SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override
    {
        (void)state;
        (void)value;
        WarnLegacyFixedFunctionUnavailable("SetRenderState");
    }

    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override
    {
        (void)stage;
        (void)type;
        (void)value;
        WarnLegacyFixedFunctionUnavailable("SetTextureStageState");
    }

    void BindTexture(DWORD stage, CTexture* texture) override
    {
        (void)stage;
        (void)texture;
        WarnLegacyFixedFunctionUnavailable("BindTexture");
    }

    void DrawPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, DWORD flags) override
    {
        (void)primitiveType;
        (void)vertexFormat;
        (void)vertices;
        (void)vertexCount;
        (void)flags;
        WarnLegacyFixedFunctionUnavailable("DrawPrimitive");
    }

    void DrawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, const unsigned short* indices,
        DWORD indexCount, DWORD flags) override
    {
        (void)primitiveType;
        (void)vertexFormat;
        (void)vertices;
        (void)vertexCount;
        (void)indices;
        (void)indexCount;
        (void)flags;
        WarnLegacyFixedFunctionUnavailable("DrawIndexedPrimitive");
    }

    void AdjustTextureSize(unsigned int* width, unsigned int* height) override
    {
        if (!width || !height) {
            return;
        }
        *width = (std::max)(1u, *width);
        *height = (std::max)(1u, *height);
    }

    void ReleaseTextureResource(CTexture* texture) override
    {
        ReleaseTextureMembers(texture);
    }

    bool CreateTextureResource(CTexture* texture, unsigned int requestedWidth, unsigned int requestedHeight,
        int pixelFormat, unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight) override
    {
        (void)pixelFormat;
        if (!texture || !m_device) {
            return false;
        }

        ReleaseTextureMembers(texture);

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = (std::max)(1u, requestedWidth);
        desc.Height = (std::max)(1u, requestedHeight);
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        ID3D11Texture2D* textureObject = nullptr;
        HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &textureObject);
        if (FAILED(hr) || !textureObject) {
            SafeRelease(textureObject);
            return false;
        }

        ID3D11ShaderResourceView* textureView = nullptr;
        hr = m_device->CreateShaderResourceView(textureObject, nullptr, &textureView);
        if (FAILED(hr) || !textureView) {
            SafeRelease(textureView);
            SafeRelease(textureObject);
            return false;
        }

        texture->m_backendTextureObject = textureObject;
        texture->m_backendTextureView = textureView;
        if (outSurfaceWidth) {
            *outSurfaceWidth = desc.Width;
        }
        if (outSurfaceHeight) {
            *outSurfaceHeight = desc.Height;
        }
        return true;
    }

    bool UpdateTextureResource(CTexture* texture, int x, int y, int w, int h,
        const unsigned int* data, bool skipColorKey, int pitch) override
    {
        if (!texture || !texture->m_backendTextureObject || !m_context || !data || w <= 0 || h <= 0) {
            return false;
        }

        ID3D11Texture2D* textureObject = static_cast<ID3D11Texture2D*>(texture->m_backendTextureObject);
        const int srcPitch = pitch > 0 ? pitch : w * static_cast<int>(sizeof(unsigned int));
        std::vector<unsigned int> uploadBuffer(static_cast<size_t>(w) * static_cast<size_t>(h));

        for (int row = 0; row < h; ++row) {
            const unsigned int* srcRow = reinterpret_cast<const unsigned int*>(reinterpret_cast<const unsigned char*>(data) + static_cast<size_t>(row) * static_cast<size_t>(srcPitch));
            unsigned int* dstRow = uploadBuffer.data() + static_cast<size_t>(row) * static_cast<size_t>(w);
            for (int col = 0; col < w; ++col) {
                unsigned int pixel = srcRow[col];
                if (!skipColorKey && (pixel & 0x00FFFFFFu) == 0x00FF00FFu) {
                    pixel = 0x00000000u;
                }
                dstRow[col] = pixel;
            }
        }

        D3D11_BOX updateBox{};
        updateBox.left = static_cast<UINT>(x);
        updateBox.top = static_cast<UINT>(y);
        updateBox.front = 0;
        updateBox.right = static_cast<UINT>(x + w);
        updateBox.bottom = static_cast<UINT>(y + h);
        updateBox.back = 1;
        m_context->UpdateSubresource(textureObject, 0, &updateBox, uploadBuffer.data(), static_cast<UINT>(w * sizeof(unsigned int)), 0);
        return true;
    }

private:
    bool RefreshRenderTarget()
    {
        if (!m_swapChain || !m_device) {
            return false;
        }

        SafeRelease(m_renderTargetView);
        ID3D11Texture2D* backBuffer = nullptr;
        HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
        if (FAILED(hr) || !backBuffer) {
            SafeRelease(backBuffer);
            return false;
        }

        hr = m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView);
        SafeRelease(backBuffer);
        if (FAILED(hr) || !m_renderTargetView) {
            return false;
        }

        m_context->OMSetRenderTargets(1, &m_renderTargetView, nullptr);
        return true;
    }

    void ResizeSwapChainBuffers()
    {
        if (!m_swapChain || m_renderWidth <= 0 || m_renderHeight <= 0) {
            return;
        }

        SafeRelease(m_renderTargetView);
        HRESULT hr = m_swapChain->ResizeBuffers(0,
            static_cast<UINT>(m_renderWidth),
            static_cast<UINT>(m_renderHeight),
            DXGI_FORMAT_UNKNOWN,
            0);
        if (FAILED(hr)) {
            DbgLog("[Render] D3D11 swap-chain resize failed hr=0x%08X.\n", static_cast<unsigned int>(hr));
            return;
        }

        RefreshRenderTarget();
    }

    void WarnLegacyFixedFunctionUnavailable(const char* operation)
    {
        if (!m_warnedLegacyOps) {
            m_warnedLegacyOps = true;
            DbgLog("[Render] D3D11 backend does not yet implement the legacy fixed-function renderer path. Operation '%s' is ignored.\n",
                operation ? operation : "unknown");
        }
    }

    HWND m_hwnd;
    int m_renderWidth;
    int m_renderHeight;
    IDXGISwapChain* m_swapChain;
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11RenderTargetView* m_renderTargetView;
    bool m_warnedLegacyOps;
};

class RoutedRenderDevice final : public IRenderDevice {
public:
    RoutedRenderDevice()
        : m_active(&m_legacy)
    {
    }

    RenderBackendType GetBackendType() const override
    {
        return m_active->GetBackendType();
    }

    bool Initialize(HWND hwnd, RenderBackendBootstrapResult* outResult) override
    {
        Shutdown();

        const RenderBackendType requestedBackend = GetRequestedRenderBackend();
        RenderBackendBootstrapResult result{};
        result.backend = requestedBackend;
        result.initHr = -1;

        switch (requestedBackend) {
        case RenderBackendType::Direct3D11:
            if (m_d3d11.Initialize(hwnd, &result)) {
                m_active = &m_d3d11;
            } else {
                DbgLog("[Render] Failed to initialize backend '%s' (hr=0x%08X). Falling back to Direct3D7.\n",
                    GetRenderBackendName(requestedBackend),
                    static_cast<unsigned int>(result.initHr));
                m_active = &m_legacy;
                if (!m_legacy.Initialize(hwnd, &result)) {
                    if (outResult) {
                        *outResult = result;
                    }
                    return false;
                }
            }
            break;

        case RenderBackendType::Direct3D12:
        case RenderBackendType::Vulkan:
            DbgLog("[Render] Requested backend '%s' is not implemented yet. Falling back to Direct3D7.\n",
                GetRenderBackendName(requestedBackend));
            m_active = &m_legacy;
            if (!m_legacy.Initialize(hwnd, &result)) {
                if (outResult) {
                    *outResult = result;
                }
                return false;
            }
            break;

        case RenderBackendType::LegacyDirect3D7:
        default:
            m_active = &m_legacy;
            if (!m_legacy.Initialize(hwnd, &result)) {
                if (outResult) {
                    *outResult = result;
                }
                return false;
            }
            break;
        }

        if (outResult) {
            *outResult = result;
        }
        return true;
    }

    void Shutdown() override
    {
        m_d3d11.Shutdown();
        m_legacy.Shutdown();
        m_active = &m_legacy;
    }

    void RefreshRenderSize() override { m_active->RefreshRenderSize(); }
    int GetRenderWidth() const override { return m_active->GetRenderWidth(); }
    int GetRenderHeight() const override { return m_active->GetRenderHeight(); }
    HWND GetWindowHandle() const override { return m_active->GetWindowHandle(); }
    IDirect3DDevice7* GetLegacyDevice() const override { return m_active->GetLegacyDevice(); }
    int ClearColor(unsigned int color) override { return m_active->ClearColor(color); }
    int ClearDepth() override { return m_active->ClearDepth(); }
    int Present(bool vertSync) override { return m_active->Present(vertSync); }
    bool AcquireBackBufferDC(HDC* outDc) override { return m_active->AcquireBackBufferDC(outDc); }
    void ReleaseBackBufferDC(HDC dc) override { m_active->ReleaseBackBufferDC(dc); }
    bool BeginScene() override { return m_active->BeginScene(); }
    void EndScene() override { m_active->EndScene(); }
    void SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) override { m_active->SetTransform(state, matrix); }
    void SetRenderState(D3DRENDERSTATETYPE state, DWORD value) override { m_active->SetRenderState(state, value); }
    void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) override { m_active->SetTextureStageState(stage, type, value); }
    void BindTexture(DWORD stage, CTexture* texture) override { m_active->BindTexture(stage, texture); }
    void DrawPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat, const void* vertices, DWORD vertexCount, DWORD flags) override { m_active->DrawPrimitive(primitiveType, vertexFormat, vertices, vertexCount, flags); }
    void DrawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat, const void* vertices, DWORD vertexCount, const unsigned short* indices, DWORD indexCount, DWORD flags) override { m_active->DrawIndexedPrimitive(primitiveType, vertexFormat, vertices, vertexCount, indices, indexCount, flags); }
    void AdjustTextureSize(unsigned int* width, unsigned int* height) override { m_active->AdjustTextureSize(width, height); }
    void ReleaseTextureResource(CTexture* texture) override { m_active->ReleaseTextureResource(texture); }
    bool CreateTextureResource(CTexture* texture, unsigned int requestedWidth, unsigned int requestedHeight, int pixelFormat, unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight) override { return m_active->CreateTextureResource(texture, requestedWidth, requestedHeight, pixelFormat, outSurfaceWidth, outSurfaceHeight); }
    bool UpdateTextureResource(CTexture* texture, int x, int y, int w, int h, const unsigned int* data, bool skipColorKey, int pitch) override { return m_active->UpdateTextureResource(texture, x, y, w, h, data, skipColorKey, pitch); }

private:
    LegacyRenderDevice m_legacy;
    D3D11RenderDevice m_d3d11;
    IRenderDevice* m_active;
};

} // namespace

IRenderDevice& GetRenderDevice()
{
    static RoutedRenderDevice s_renderDevice;
    return s_renderDevice;
}