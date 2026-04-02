#pragma once

#include "QtUiRuntime.h"

inline bool IsQtUiSpikeCompiled() { return IsQtUiRuntimeCompiled(); }
inline bool IsQtUiSpikeEnabled() { return IsQtUiRuntimeEnabled(); }
inline void InitializeQtUiSpike(RoNativeWindowHandle mainWindow) { InitializeQtUiRuntime(mainWindow); }
inline void ShutdownQtUiSpike() { ShutdownQtUiRuntime(); }
inline void ProcessQtUiSpikeEvents() { ProcessQtUiRuntimeEvents(); }
inline void NotifyQtUiSpikeWindowMessage(RoWindowMessage msg, RoWindowWParam wParam, RoWindowLParam lParam) { NotifyQtUiRuntimeWindowMessage(msg, wParam, lParam); }
inline bool CompositeQtUiSpikeGameplayOverlay(CGameMode& mode, void* bgraPixels, int width, int height, int pitch)
{
    return CompositeQtUiGameplayOverlay(mode, bgraPixels, width, height, pitch);
}
inline bool RenderQtUiSpikeGameplayOverlayTexture(CGameMode& mode, CTexture* texture, int width, int height)
{
    return RenderQtUiGameplayOverlayTexture(mode, texture, width, height);
}
