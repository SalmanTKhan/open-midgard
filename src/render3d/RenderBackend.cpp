#include "RenderBackend.h"

#include "Device.h"
#include "DebugLog.h"

#include <algorithm>
#include <cctype>
#include <cstring>

namespace {

RenderBackendType ParseRenderBackendName(const char* value)
{
    if (!value || !*value) {
        return RenderBackendType::LegacyDirect3D7;
    }

    char normalized[32] = {};
    size_t writeIndex = 0;
    for (size_t readIndex = 0; value[readIndex] != '\0' && writeIndex + 1 < sizeof(normalized); ++readIndex) {
        const unsigned char ch = static_cast<unsigned char>(value[readIndex]);
        if (ch == ' ' || ch == '_' || ch == '-') {
            continue;
        }
        normalized[writeIndex++] = static_cast<char>(std::tolower(ch));
    }
    normalized[writeIndex] = '\0';

    if (std::strcmp(normalized, "dx11") == 0 || std::strcmp(normalized, "d3d11") == 0 || std::strcmp(normalized, "direct3d11") == 0) {
        return RenderBackendType::Direct3D11;
    }
    if (std::strcmp(normalized, "dx12") == 0 || std::strcmp(normalized, "d3d12") == 0 || std::strcmp(normalized, "direct3d12") == 0) {
        return RenderBackendType::Direct3D12;
    }
    if (std::strcmp(normalized, "vk") == 0 || std::strcmp(normalized, "vulkan") == 0) {
        return RenderBackendType::Vulkan;
    }
    return RenderBackendType::LegacyDirect3D7;
}

int InitializeLegacyDirect3D7(HWND hwnd)
{
    GUID deviceCandidates[] = {
        IID_IDirect3DTnLHalDevice,
        IID_IDirect3DHALDevice,
        IID_IDirect3DRGBDevice
    };

    int renderInitHr = -1;
    for (GUID& deviceGuid : deviceCandidates) {
        renderInitHr = g_3dDevice.Init(hwnd, nullptr, &deviceGuid, nullptr, 0);
        if (renderInitHr >= 0) {
            break;
        }
    }
    return renderInitHr;
}

} // namespace

const char* GetRenderBackendName(RenderBackendType backend)
{
    switch (backend) {
    case RenderBackendType::LegacyDirect3D7:
        return "Direct3D7";
    case RenderBackendType::Direct3D11:
        return "Direct3D11";
    case RenderBackendType::Direct3D12:
        return "Direct3D12";
    case RenderBackendType::Vulkan:
        return "Vulkan";
    default:
        return "Unknown";
    }
}

RenderBackendType GetRequestedRenderBackend()
{
    char buffer[64] = {};
    const DWORD length = GetEnvironmentVariableA("OPEN_MIDGARD_RENDER_BACKEND", buffer, static_cast<DWORD>(sizeof(buffer)));
    if (length == 0 || length >= sizeof(buffer)) {
        return RenderBackendType::LegacyDirect3D7;
    }
    return ParseRenderBackendName(buffer);
}

bool InitializeRenderBackend(HWND hwnd, RenderBackendBootstrapResult* outResult)
{
    const RenderBackendType requestedBackend = GetRequestedRenderBackend();
    RenderBackendBootstrapResult result{};
    result.backend = requestedBackend;
    result.initHr = -1;

    switch (requestedBackend) {
    case RenderBackendType::Direct3D11:
    case RenderBackendType::Direct3D12:
    case RenderBackendType::Vulkan:
        DbgLog("[Render] Requested backend '%s' is not implemented yet. Falling back to Direct3D7.\n",
            GetRenderBackendName(requestedBackend));
        result.backend = RenderBackendType::LegacyDirect3D7;
        break;

    case RenderBackendType::LegacyDirect3D7:
    default:
        result.backend = RenderBackendType::LegacyDirect3D7;
        break;
    }

    result.initHr = InitializeLegacyDirect3D7(hwnd);
    if (result.initHr >= 0) {
        DbgLog("[Render] Initialized backend '%s'.\n", GetRenderBackendName(result.backend));
    }

    if (outResult) {
        *outResult = result;
    }
    return result.initHr >= 0;
}