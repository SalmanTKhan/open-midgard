#include "UINpcCutinWnd.h"

#include "UIWindowMgr.h"
#include "main/WinMain.h"

#include <algorithm>
#include <cctype>
#include <windows.h>

namespace {

constexpr const char* kIllustDirPrefix = "illust\\";

bool HasFileExtension(const std::string& name)
{
    const size_t dot = name.find_last_of('.');
    if (dot == std::string::npos) return false;
    const size_t slash = name.find_last_of("/\\");
    return slash == std::string::npos || dot > slash;
}

std::string BuildIllustPath(const std::string& imageName)
{
    if (imageName.empty()) return {};
    std::string normalized = imageName;
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
    if (!HasFileExtension(normalized)) {
        normalized += ".bmp";
    }
    return kIllustDirPrefix + normalized;
}

void GetClientSize(int* outW, int* outH)
{
    *outW = 800;
    *outH = 600;
    if (!g_hMainWnd) return;
    RECT rc{};
    if (GetClientRect(g_hMainWnd, &rc)) {
        *outW = rc.right - rc.left;
        *outH = rc.bottom - rc.top;
    }
}

}  // namespace

UINpcCutinWnd::UINpcCutinWnd() = default;
UINpcCutinWnd::~UINpcCutinWnd() = default;

void UINpcCutinWnd::SetCutin(const std::string& imageName, unsigned char position)
{
    if (imageName.empty() || position == Pos_Hide) {
        ClearCutin();
        return;
    }

    if (imageName != m_imageName) {
        m_imageName = imageName;
        LoadBitmapForCurrentName();
    }
    m_position = position;

    if (!m_bitmap.IsValid()) {
        // Image failed to load — keep state recorded but don't show a
        // colored placeholder. Hide the window.
        SetShow(0);
        Invalidate();
        return;
    }

    Create(m_bitmap.width, m_bitmap.height);
    Reposition();
    SetShow(1);
    Invalidate();
}

void UINpcCutinWnd::ClearCutin()
{
    m_imageName.clear();
    m_position = Pos_Hide;
    m_bitmap.Clear();
    SetShow(0);
    Invalidate();
}

bool UINpcCutinWnd::HasCutin() const
{
    return m_position != Pos_Hide && !m_imageName.empty();
}

const std::string& UINpcCutinWnd::GetImageName() const
{
    return m_imageName;
}

unsigned char UINpcCutinWnd::GetPosition() const
{
    return m_position;
}

bool UINpcCutinWnd::IsUpdateNeed()
{
    if (UIFrameWnd::IsUpdateNeed()) return true;
    return false;
}

void UINpcCutinWnd::OnDraw()
{
    if (m_show == 0 || !m_bitmap.IsValid()) {
        return;
    }
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT dst{ m_x, m_y, m_x + m_w, m_y + m_h };
    shopui::DrawBitmapPixelsTransparent(hdc, m_bitmap, dst);

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UINpcCutinWnd::Reposition()
{
    int clientW = 0;
    int clientH = 0;
    GetClientSize(&clientW, &clientH);

    int x = 0;
    int y = clientH - m_h;
    switch (m_position) {
    case Pos_BottomLeft:
        x = 0;
        y = clientH - m_h;
        break;
    case Pos_BottomCenter:
        x = (clientW - m_w) / 2;
        y = clientH - m_h;
        break;
    case Pos_BottomRight:
        x = clientW - m_w;
        y = clientH - m_h;
        break;
    case Pos_MiddleExt:
    case Pos_MiddleCenter:
        x = (clientW - m_w) / 2;
        y = (clientH - m_h) / 2;
        break;
    default:
        x = 0;
        y = clientH - m_h;
        break;
    }
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    Move(x, y);
}

bool UINpcCutinWnd::LoadBitmapForCurrentName()
{
    m_bitmap.Clear();
    if (m_imageName.empty()) return false;
    const std::string path = BuildIllustPath(m_imageName);
    m_bitmap = shopui::LoadBitmapPixelsFromGameData(path, true);
    return m_bitmap.IsValid();
}
