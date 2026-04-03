#pragma once
//===========================================================================
// WinMain.h  –  Application entry point and Win32 window management
// Clean C++17 rewrite.
//===========================================================================
#include "Types.h"
#include "render3d/RenderBackend.h"

// Win32 window / app lifecycle
#if RO_PLATFORM_WINDOWS
int  __stdcall  WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                         char* lpCmdLine, int nCmdShow);
#else
int             main(int argc, char** argv);
#endif
bool            InitApp(HINSTANCE hInstance, int nCmdShow);
int             ReadRegistry();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void            ExitApp();
void            CheckSystemMessage();
void            SetWindowActiveMode(int active);
bool            GetWindowActiveMode();
void            RefreshMainWindowTitle(const char* status = nullptr);
void            RecordMainWindowFrame();
RenderBackendType GetActiveRenderBackend();
bool            RelaunchCurrentApplication();

// Patch-upgrade helpers
bool            UpdatePatch(const char* patchExe, const char* patchUp);
void*           ExcuteProgram(const char* exePath);

// Process validation (checks if another client instance is running)
bool            SearchProcessIn9X();
bool            SearchProcessInNT();

// DLL management handled by CDllMgr

// Global window / instance handles
extern HWND     g_hMainWnd;
extern HINSTANCE g_hInstance;
extern bool     g_multiSTOP;    // True if another instance detected

// Window class and size constants
extern const char* const WINDOW_NAME;
extern int  WINDOW_WIDTH;
extern int  WINDOW_HEIGHT;
