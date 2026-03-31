#pragma once

#include "QtUiRuntime.h"

inline bool IsQtUiSpikeCompiled() { return IsQtUiRuntimeCompiled(); }
inline bool IsQtUiSpikeEnabled() { return IsQtUiRuntimeEnabled(); }
inline void InitializeQtUiSpike(HWND mainWindow) { InitializeQtUiRuntime(mainWindow); }
inline void ShutdownQtUiSpike() { ShutdownQtUiRuntime(); }
inline void ProcessQtUiSpikeEvents() { ProcessQtUiRuntimeEvents(); }
inline void NotifyQtUiSpikeWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) { NotifyQtUiRuntimeWindowMessage(msg, wParam, lParam); }
inline bool CompositeQtUiSpikeGameplayOverlay(CGameMode& mode, void* bgraPixels, int width, int height, int pitch)
{
    return CompositeQtUiGameplayOverlay(mode, bgraPixels, width, height, pitch);
}
inline bool RenderQtUiSpikeGameplayOverlayTexture(CGameMode& mode, CTexture* texture, int width, int height)
{
    return RenderQtUiGameplayOverlayTexture(mode, texture, width, height);
}
