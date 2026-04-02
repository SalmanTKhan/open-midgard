#include "WinMain.h"

#include "render3d/RenderDevice.h"

#include <cstdio>

const char* const WINDOW_NAME = "open-midgard";

int WINDOW_WIDTH = 1920;
int WINDOW_HEIGHT = 1080;

HWND g_hMainWnd = nullptr;
HINSTANCE g_hInstance = nullptr;
bool g_isAppActive = false;
bool g_multiSTOP = false;
int g_soundMode = 1;
int g_isSoundOn = 1;
int g_frameskip = 0;
char g_baseDir[MAX_PATH] = {};

bool InitApp(HINSTANCE, int)
{
    return false;
}

int ReadRegistry()
{
    return 0;
}

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM)
{
    return 0;
}

void ExitApp() {}
void CheckSystemMessage() {}

void SetWindowActiveMode(int active)
{
    g_isAppActive = active != 0;
}

bool GetWindowActiveMode()
{
    return g_isAppActive;
}

void RefreshMainWindowTitle(const char*) {}
void RecordMainWindowFrame() {}

RenderBackendType GetActiveRenderBackend()
{
    return GetRenderDevice().GetBackendType();
}

bool RelaunchCurrentApplication()
{
    return false;
}

bool UpdatePatch(const char*, const char*)
{
    return false;
}

void* ExcuteProgram(const char*)
{
    return nullptr;
}

bool SearchProcessIn9X()
{
    return false;
}

bool SearchProcessInNT()
{
    return false;
}

int main(int, char**)
{
    std::fputs("open-midgard non-Windows entry point is not implemented yet.\n", stderr);
    return 1;
}