#pragma once

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

#include "RenderBackend.h"

class CTexture;

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

    virtual RenderBackendType GetBackendType() const = 0;
    virtual bool Initialize(HWND hwnd, RenderBackendBootstrapResult* outResult) = 0;
    virtual void Shutdown() = 0;
    virtual void RefreshRenderSize() = 0;

    virtual int GetRenderWidth() const = 0;
    virtual int GetRenderHeight() const = 0;
    virtual HWND GetWindowHandle() const = 0;
    virtual IDirect3DDevice7* GetLegacyDevice() const = 0;

    virtual int ClearColor(unsigned int color) = 0;
    virtual int ClearDepth() = 0;
    virtual int Present(bool vertSync) = 0;
    virtual bool AcquireBackBufferDC(HDC* outDc) = 0;
    virtual void ReleaseBackBufferDC(HDC dc) = 0;
    virtual bool UpdateBackBufferFromMemory(const void* bgraPixels, int width, int height, int pitch) = 0;

    virtual bool BeginScene() = 0;
    virtual bool PrepareOverlayPass() = 0;
    virtual void EndScene() = 0;
    virtual void SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix) = 0;
    virtual void SetRenderState(D3DRENDERSTATETYPE state, DWORD value) = 0;
    virtual void SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) = 0;
    virtual void BindTexture(DWORD stage, CTexture* texture) = 0;
    virtual void DrawPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, DWORD flags) = 0;
    virtual void DrawIndexedPrimitive(D3DPRIMITIVETYPE primitiveType, DWORD vertexFormat,
        const void* vertices, DWORD vertexCount, const unsigned short* indices,
        DWORD indexCount, DWORD flags) = 0;

    virtual void AdjustTextureSize(unsigned int* width, unsigned int* height) = 0;
    virtual void ReleaseTextureResource(CTexture* texture) = 0;
    virtual bool CreateTextureResource(CTexture* texture, unsigned int requestedWidth, unsigned int requestedHeight,
        int pixelFormat, unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight) = 0;
    virtual bool UpdateTextureResource(CTexture* texture, int x, int y, int w, int h,
        const unsigned int* data, bool skipColorKey, int pitch) = 0;
};

IRenderDevice& GetRenderDevice();