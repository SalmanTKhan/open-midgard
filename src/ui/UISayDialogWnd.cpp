#include "UISayDialogWnd.h"

#include "NpcDialogColoredText.h"
#include "input/Gamepad.h"
#include "gamemode/CursorRenderer.h"
#include "render/DC.h"
#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"
#include "gamemode/Mode.h"
#include "main/WinMain.h"
#include "qtui/QtUiRuntime.h"
#include "TextScale.h"
#include "ui/UINewChatWnd.h"

#include <windows.h>

#include <algorithm>
#include <cmath>

#if RO_ENABLE_QT6_UI
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include <QString>
#endif

namespace {

constexpr int kDialogWidth = 360;
constexpr int kDialogHeight = 160;
constexpr int kBorder = 1;
constexpr int kPadding = 10;
constexpr int kHeaderHeight = 26;
constexpr int kHeaderInnerPadding = 7;
constexpr int kButtonWidth = 68;
constexpr int kButtonHeight = 22;
constexpr int kCornerRadius = 10;
constexpr int kBodyFontMin = 11;
constexpr int kBodyFontMax = 32;
constexpr int kScrollWheelStep = 24;

int GetNpcDialogBodyFontPixelSize()
{
    const UINewChatWnd* const chatWnd = g_windowMgr.m_chatWnd;
    const int fontSize = chatWnd ? chatWnd->GetFontPixelSize() : 13;
    const int scaledFontSize = static_cast<int>(std::lround(static_cast<double>(fontSize) * GetConfiguredTextScaleFactor()));
    return std::clamp(scaledFontSize, kBodyFontMin, kBodyFontMax);
}

HFONT GetNpcDialogBodyFont()
{
    static HFONT s_font = nullptr;
    static int s_fontPixelSize = 0;
    const int desiredPixelSize = GetNpcDialogBodyFontPixelSize();
    if (!s_font || s_fontPixelSize != desiredPixelSize) {
        if (s_font) {
            DeleteObject(s_font);
            s_font = nullptr;
        }
        s_font = CreateFontA(-desiredPixelSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
        s_fontPixelSize = desiredPixelSize;
    }
    return s_font;
}

HFONT GetNpcDialogHeaderFont()
{
    static HFONT s_font = nullptr;
    static int s_fontPixelSize = 0;
    const int desiredPixelSize = std::clamp(GetNpcDialogBodyFontPixelSize() + 2, 13, 40);
    if (!s_font || s_fontPixelSize != desiredPixelSize) {
        if (s_font) {
            DeleteObject(s_font);
            s_font = nullptr;
        }
        s_font = CreateFontA(-desiredPixelSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "MS Sans Serif");
        s_fontPixelSize = desiredPixelSize;
    }
    return s_font;
}

void FillSolidRect(HDC hdc, const RECT& rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void DrawRectOutline(HDC hdc, const RECT& rect, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void FillRoundedRect(HDC hdc, const RECT& rect, COLORREF fillColor, COLORREF borderColor, int radius)
{
    HBRUSH brush = CreateSolidBrush(fillColor);
    HPEN pen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

RECT MakeRect(int left, int top, int width, int height)
{
    RECT rect{ left, top, left + width, top + height };
    return rect;
}

void SnapCursorToRect(const RECT& rect)
{
#if RO_HAS_GAMEPAD
    if (!gamepad::g_gamepad.IsConnected()) {
        return;
    }

    POINT cursorPos{};
    if (GetModeCursorClientPos(&cursorPos)) {
        if (cursorPos.x >= rect.left && cursorPos.x < rect.right
            && cursorPos.y >= rect.top && cursorPos.y < rect.bottom) {
            return;
        }
    }

    const int centerX = (rect.left + rect.right) / 2;
    const int centerY = (rect.top + rect.bottom) / 2;
    SetModeCursorClientPos(centerX, centerY);
#else
    (void)rect;
#endif
}

std::string NormalizeDialogNewlines(const std::string& text)
{
    std::string normalized;
    normalized.reserve(text.size() + 8);
    for (size_t i = 0; i < text.size(); ++i) {
        const char ch = text[i];
        if (ch == '\r') {
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                ++i;
            }
            normalized += "\r\n";
        } else if (ch == '\n') {
            normalized += "\r\n";
        } else {
            normalized += ch;
        }
    }
    return normalized;
}

#if RO_ENABLE_QT6_UI
QFont BuildDialogUiFontFromHdc(HDC hdc)
{
    LOGFONTA logFont{};
    if (hdc) {
        if (HGDIOBJ fontObject = GetCurrentObject(hdc, OBJ_FONT)) {
            GetObjectA(fontObject, sizeof(logFont), &logFont);
        }
    }

    const QString family = logFont.lfFaceName[0] != '\0'
        ? QString::fromLocal8Bit(logFont.lfFaceName)
        : QStringLiteral("MS Sans Serif");
    QFont font(family);
    font.setPixelSize(logFont.lfHeight != 0 ? (std::max)(1, static_cast<int>(std::abs(logFont.lfHeight))) : 13);
    font.setBold(logFont.lfWeight >= FW_BOLD);
    font.setStyleStrategy(QFont::NoAntialias);
    return font;
}

void DrawDialogUiText(HDC hdc, const RECT& rect, const char* text, COLORREF color, Qt::Alignment alignment)
{
    if (!hdc || !text || rect.right <= rect.left || rect.bottom <= rect.top) {
        return;
    }

    const QString label = QString::fromLocal8Bit(text);
    if (label.isEmpty()) {
        return;
    }

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    std::vector<unsigned int> pixels(static_cast<size_t>(width) * static_cast<size_t>(height), 0u);
    QImage image(reinterpret_cast<uchar*>(pixels.data()), width, height, width * static_cast<int>(sizeof(unsigned int)), QImage::Format_ARGB32);
    if (image.isNull()) {
        return;
    }

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::TextAntialiasing, false);
    painter.setFont(BuildDialogUiFontFromHdc(hdc));
    painter.setPen(QColor(GetRValue(color), GetGValue(color), GetBValue(color)));
    painter.drawText(QRect(0, 0, width, height), alignment | Qt::TextSingleLine, label);
    AlphaBlendArgbToHdc(hdc, rect.left, rect.top, width, height, pixels.data(), width, height);
}
#endif

} // namespace

UISayDialogWnd::UISayDialogWnd()
    : m_npcId(0),
      m_actionButton(ActionButton::None),
      m_clearOnNextText(false),
      m_hoverAction(false),
      m_pressAction(false)
{
    Create(kDialogWidth, kDialogHeight);
    int defaultX = 140;
    int defaultY = 280;
    LoadUiWindowPlacement("NpcSayDialog", &defaultX, &defaultY);
    if (g_hMainWnd) {
        RECT clientRect{};
        GetClientRect(g_hMainWnd, &clientRect);
        if (defaultX == 140 && defaultY == 280) {
            defaultX = (clientRect.right - clientRect.left - m_w) / 2;
            defaultY = (clientRect.bottom - clientRect.top) - m_h - 26;
        }
    }
    g_windowMgr.ClampWindowToClient(&defaultX, &defaultY, m_w, m_h);
    UIWindow::Move(defaultX, defaultY);
    UIWindow::SetShow(0);
}

UISayDialogWnd::~UISayDialogWnd() = default;

void UISayDialogWnd::SetShow(int show)
{
    UIWindow::SetShow(show);
    if (show == 0) {
        m_isDragging = 0;
        m_hoverAction = false;
        m_pressAction = false;
    }
}

void UISayDialogWnd::Move(int x, int y)
{
    g_windowMgr.ClampWindowToClient(&x, &y, m_w, m_h);
    UIWindow::Move(x, y);
}

void UISayDialogWnd::StoreInfo()
{
    SaveUiWindowPlacement("NpcSayDialog", m_x, m_y);
}

u32 UISayDialogWnd::GetNpcId() const
{
    return m_npcId;
}

std::string UISayDialogWnd::GetDisplayText() const
{
    return BuildDisplayText();
}

bool UISayDialogWnd::HasActionButton() const
{
    return m_actionButton != ActionButton::None;
}

bool UISayDialogWnd::IsNextAction() const
{
    return m_actionButton == ActionButton::Next;
}

bool UISayDialogWnd::IsHoveringAction() const
{
    return m_hoverAction;
}

bool UISayDialogWnd::IsPressingAction() const
{
    return m_pressAction;
}

bool UISayDialogWnd::GetActionRectForQt(RECT* outRect) const
{
    if (!outRect || m_actionButton == ActionButton::None) {
        return false;
    }
    *outRect = GetActionRect();
    return true;
}

int UISayDialogWnd::GetScrollOffset() const
{
    return m_scrollOffset;
}

int UISayDialogWnd::GetScrollContentHeight() const
{
    return m_scrollContentHeight;
}

RECT UISayDialogWnd::GetActionRect() const
{
    return MakeRect(m_x + m_w - kPadding - kButtonWidth,
        m_y + m_h - kPadding - kButtonHeight,
        kButtonWidth,
        kButtonHeight);
}

RECT UISayDialogWnd::GetTextRect() const
{
    const int bottomInset = (m_actionButton == ActionButton::None) ? kPadding : (kPadding + kButtonHeight + 8);
    return MakeRect(m_x + kPadding,
        m_y + kHeaderHeight + kPadding,
        m_w - kPadding * 2,
        m_h - kHeaderHeight - kPadding - bottomInset);
}

bool UISayDialogWnd::IsPointInRect(const RECT& rect, int x, int y) const
{
    return x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom;
}

void UISayDialogWnd::StartDragging(int x, int y)
{
    m_isDragging = 1;
    m_startGlobalX = x;
    m_startGlobalY = y;
    m_orgX = m_x;
    m_orgY = m_y;
}

void UISayDialogWnd::StopDragging()
{
    if (m_isDragging == 0) {
        return;
    }
    m_isDragging = 0;
    StoreInfo();
}

void UISayDialogWnd::ClampScrollOffset()
{
    const int textRectHeight = static_cast<int>(GetTextRect().bottom - GetTextRect().top);
    const int viewportHeight = m_scrollViewportHeight > 0
        ? m_scrollViewportHeight
        : (std::max)(0, textRectHeight);
    const int maxScrollOffset = (std::max)(0, m_scrollContentHeight - viewportHeight);
    m_scrollOffset = std::clamp(m_scrollOffset, 0, maxScrollOffset);
}

void UISayDialogWnd::AdjustScrollOffset(int delta)
{
    const int oldOffset = m_scrollOffset;
    m_scrollOffset += delta;
    ClampScrollOffset();
    if (m_scrollOffset != oldOffset) {
        Invalidate();
    }
}

void UISayDialogWnd::DrawActionButton(HDC hdc, const RECT& rect) const
{
    const bool active = m_hoverAction || m_pressAction;
    FillSolidRect(hdc, rect, m_pressAction ? RGB(196, 196, 196) : (active ? RGB(228, 228, 228) : RGB(240, 240, 240)));
    DrawRectOutline(hdc, rect, RGB(110, 110, 110));

    RECT textRect = rect;
    const char* label = (m_actionButton == ActionButton::Next) ? "Next" : "Close";
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
#if RO_ENABLE_QT6_UI
    DrawDialogUiText(hdc, textRect, label, RGB(0, 0, 0), Qt::AlignCenter | Qt::AlignVCenter);
#else
    DrawTextA(hdc, label, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
#endif
}

void UISayDialogWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) {
        return;
    }

    if (!g_hMainWnd || m_show == 0) {
        return;
    }

    HDC hdc = AcquireDrawTarget();
    if (!hdc) {
        return;
    }

    RECT outer = MakeRect(m_x, m_y, m_w, m_h);
    RECT inner = MakeRect(m_x + kBorder, m_y + kBorder, m_w - 2 * kBorder, m_h - 2 * kBorder);
    RECT header = MakeRect(m_x + kBorder, m_y + kBorder, m_w - 2 * kBorder, kHeaderHeight);
    RECT body = MakeRect(m_x + kBorder, m_y + kBorder + kHeaderHeight, m_w - 2 * kBorder, m_h - (kBorder * 2) - kHeaderHeight);
    FillRoundedRect(hdc, outer, RGB(42, 39, 36), RGB(20, 18, 16), kCornerRadius);
    FillRoundedRect(hdc, inner, RGB(58, 55, 50), RGB(34, 31, 28), kCornerRadius - 2);
    FillRoundedRect(hdc, body, RGB(46, 43, 39), RGB(38, 35, 31), kCornerRadius - 3);
    FillRoundedRect(hdc, header, RGB(94, 87, 74), RGB(70, 63, 52), kCornerRadius - 3);

    HFONT bodyFont = GetNpcDialogBodyFont();
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, bodyFont));
    RECT textRect = GetTextRect();
    SetBkMode(hdc, TRANSPARENT);
    DrawNpcSayDialogColoredText(hdc, textRect, BuildDisplayText());

    RECT titleRect = MakeRect(m_x + kHeaderInnerPadding, m_y + kBorder + 2, m_w - (kHeaderInnerPadding * 2), kHeaderHeight - 4);
    const char* titleText = "NPC";
    HFONT oldBodyFont = static_cast<HFONT>(SelectObject(hdc, GetNpcDialogHeaderFont()));
#if RO_ENABLE_QT6_UI
    DrawDialogUiText(hdc, titleRect, titleText, RGB(255, 255, 255), Qt::AlignLeft | Qt::AlignVCenter);
#else
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextA(hdc, titleText, -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
#endif
    SelectObject(hdc, oldBodyFont);

    if (m_actionButton != ActionButton::None) {
        DrawActionButton(hdc, GetActionRect());
    }

    SelectObject(hdc, oldFont);
    ReleaseDrawTarget(hdc);
}

void UISayDialogWnd::AppendText(u32 npcId, const std::string& text)
{
    m_npcId = npcId;
    if (m_clearOnNextText) {
        m_textLines.clear();
        m_clearOnNextText = false;
    }
    m_textLines.push_back(NormalizeDialogNewlines(text));
    m_scrollOffset = 0;
    m_scrollContentHeight = 0;
    m_scrollViewportHeight = 0;
    SetShow(1);
    Invalidate();
}

void UISayDialogWnd::ShowNext(u32 npcId)
{
    m_npcId = npcId;
    m_actionButton = ActionButton::Next;
    m_hoverAction = false;
    m_pressAction = false;
    SetShow(1);
    SnapCursorToRect(GetActionRect());
    Invalidate();
}

void UISayDialogWnd::ShowClose(u32 npcId)
{
    m_npcId = npcId;
    m_actionButton = ActionButton::Close;
    m_hoverAction = false;
    m_pressAction = false;
    SetShow(1);
    SnapCursorToRect(GetActionRect());
    Invalidate();
}

void UISayDialogWnd::ClearAction()
{
    if (m_actionButton == ActionButton::None && !m_hoverAction && !m_pressAction) {
        return;
    }
    m_actionButton = ActionButton::None;
    m_hoverAction = false;
    m_pressAction = false;
    Invalidate();
}

void UISayDialogWnd::HideConversation()
{
    m_npcId = 0;
    m_textLines.clear();
    m_actionButton = ActionButton::None;
    m_clearOnNextText = false;
    m_hoverAction = false;
    m_pressAction = false;
    m_scrollOffset = 0;
    m_scrollContentHeight = 0;
    m_scrollViewportHeight = 0;
    SetShow(0);
}

void UISayDialogWnd::SetScrollMetrics(int contentHeight, int viewportHeight)
{
    const int clampedHeight = (std::max)(0, contentHeight);
    const int clampedViewportHeight = (std::max)(0, viewportHeight);
    if (m_scrollContentHeight == clampedHeight && m_scrollViewportHeight == clampedViewportHeight) {
        return;
    }
    m_scrollContentHeight = clampedHeight;
    m_scrollViewportHeight = clampedViewportHeight;
    ClampScrollOffset();
    Invalidate();
}

void UISayDialogWnd::SetScrollContentHeight(int contentHeight)
{
    SetScrollMetrics(contentHeight, m_scrollViewportHeight);
}

std::string UISayDialogWnd::BuildDisplayText() const
{
    std::string combined;
    for (size_t i = 0; i < m_textLines.size(); ++i) {
        if (i != 0) {
            combined += "\r\n";
        }
        combined += m_textLines[i];
    }
    return combined;
}

bool UISayDialogWnd::HandleKeyDown(int virtualKey)
{
    if (m_show == 0) {
        return false;
    }

    if (virtualKey == VK_RETURN && m_actionButton == ActionButton::Next && m_npcId != 0) {
        m_clearOnNextText = true;
        ClearAction();
        return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestNpcNext, m_npcId, 0, 0) != 0;
    }

    if ((virtualKey == VK_RETURN || virtualKey == VK_ESCAPE)
        && m_actionButton == ActionButton::Close
        && m_npcId != 0) {
        const u32 npcId = m_npcId;
        g_windowMgr.CloseNpcDialogWindows();
        return g_modeMgr.SendMsg(CGameMode::GameMsg_RequestNpcCloseDialog, npcId, 0, 0) != 0;
    }

    return true;
}

void UISayDialogWnd::OnWheel(int delta)
{
    if (m_show == 0 || m_scrollContentHeight <= 0) {
        return;
    }

    const int textRectHeight = static_cast<int>(GetTextRect().bottom - GetTextRect().top);
    const int viewportHeight = m_scrollViewportHeight > 0
        ? m_scrollViewportHeight
        : (std::max)(0, textRectHeight);
    if (m_scrollContentHeight <= viewportHeight) {
        return;
    }

    if (delta > 0) {
        AdjustScrollOffset(-kScrollWheelStep);
    } else if (delta < 0) {
        AdjustScrollOffset(kScrollWheelStep);
    }
}

void UISayDialogWnd::OnLBtnDown(int x, int y)
{
    if (m_show == 0) {
        return;
    }

    if (m_actionButton != ActionButton::None && IsPointInRect(GetActionRect(), x, y)) {
        m_pressAction = true;
        Invalidate();
        return;
    }

    if (IsPointInRect(MakeRect(m_x, m_y, m_w, m_h), x, y)) {
        StartDragging(x, y);
    }
}

void UISayDialogWnd::OnMouseMove(int x, int y)
{
    if (m_isDragging != 0) {
        int snappedX = m_orgX + (x - m_startGlobalX);
        int snappedY = m_orgY + (y - m_startGlobalY);
        g_windowMgr.SnapWindowToNearby(this, &snappedX, &snappedY);
        Move(snappedX, snappedY);
        return;
    }

    const bool hover = (m_actionButton != ActionButton::None) && IsPointInRect(GetActionRect(), x, y);
    if (m_hoverAction != hover) {
        m_hoverAction = hover;
        Invalidate();
    }
}

void UISayDialogWnd::OnLBtnUp(int x, int y)
{
    if (m_show == 0) {
        return;
    }

    if (m_isDragging != 0) {
        StopDragging();
        return;
    }

    if (!m_pressAction) {
        return;
    }

    const bool clicked = (m_actionButton != ActionButton::None) && IsPointInRect(GetActionRect(), x, y);
    m_pressAction = false;
    m_hoverAction = clicked;
    Invalidate();
    if (!clicked || m_npcId == 0) {
        return;
    }

    PlayUiButtonSound();
    if (m_actionButton == ActionButton::Next) {
        m_clearOnNextText = true;
        ClearAction();
        g_modeMgr.SendMsg(CGameMode::GameMsg_RequestNpcNext, m_npcId, 0, 0);
        return;
    }

    const u32 npcId = m_npcId;
    g_windowMgr.CloseNpcDialogWindows();
    g_modeMgr.SendMsg(CGameMode::GameMsg_RequestNpcCloseDialog, npcId, 0, 0);
}
