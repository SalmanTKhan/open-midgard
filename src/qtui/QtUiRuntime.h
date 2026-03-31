#pragma once

#include <windows.h>

class CGameMode;
class CTexture;

#if RO_ENABLE_QT6_UI
bool IsQtUiRuntimeCompiled();
bool IsQtUiRuntimeEnabled();
void InitializeQtUiRuntime(HWND mainWindow);
void ShutdownQtUiRuntime();
void ProcessQtUiRuntimeEvents();
void NotifyQtUiRuntimeWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
bool HandleQtUiRuntimeWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam);
bool CompositeQtUiMenuOverlay(void* bgraPixels, int width, int height, int pitch);
bool RenderQtUiMenuOverlayTexture(CTexture* texture, int width, int height);
bool CompositeQtUiGameplayOverlay(CGameMode& mode, void* bgraPixels, int width, int height, int pitch);
bool RenderQtUiGameplayOverlayTexture(CGameMode& mode, CTexture* texture, int width, int height);
#else
inline bool IsQtUiRuntimeCompiled() { return false; }
inline bool IsQtUiRuntimeEnabled() { return false; }
inline void InitializeQtUiRuntime(HWND) {}
inline void ShutdownQtUiRuntime() {}
inline void ProcessQtUiRuntimeEvents() {}
inline void NotifyQtUiRuntimeWindowMessage(UINT, WPARAM, LPARAM) {}
inline bool HandleQtUiRuntimeWindowMessage(UINT, WPARAM, LPARAM) { return false; }
inline bool CompositeQtUiMenuOverlay(void*, int, int, int) { return false; }
inline bool RenderQtUiMenuOverlayTexture(CTexture*, int, int) { return false; }
inline bool CompositeQtUiGameplayOverlay(CGameMode&, void*, int, int, int) { return false; }
inline bool RenderQtUiGameplayOverlayTexture(CGameMode&, CTexture*, int, int) { return false; }
#endif
