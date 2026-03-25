#pragma once

#include <windows.h>
#include <ddraw.h>
#include <d3d.h>

#include "RenderBackend.h"

class IRenderDevice {
public:
    virtual ~IRenderDevice() = default;

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

    virtual void AdjustTextureSize(unsigned int* width, unsigned int* height) = 0;
    virtual bool CreateTextureSurface(unsigned int requestedWidth, unsigned int requestedHeight,
        unsigned int* outSurfaceWidth, unsigned int* outSurfaceHeight, IDirectDrawSurface7** outSurface) = 0;
    virtual bool UploadTextureSurface(IDirectDrawSurface7* surface, int x, int y, int w, int h,
        const unsigned int* data, bool skipColorKey, int pitch) = 0;
};

IRenderDevice& GetRenderDevice();