#include "UIItemShopWnd.h"

#include "UIShopCommon.h"
#include "UIWindowMgr.h"
#include "gamemode/GameMode.h"
#include "gamemode/Mode.h"
#include "main/WinMain.h"
#include "qtui/QtUiRuntime.h"
#include "session/Session.h"

#include <windows.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace {

constexpr int kWindowWidth = 320;
constexpr int kWindowHeight = 274;
constexpr int kListTop = 22;
constexpr int kListBottomMargin = 12;
constexpr int kListSideMargin = 8;
constexpr int kRowHeight = 18;

std::string FormatPrice(int value)
{
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    return std::string(buffer);
}

} // namespace

UIItemShopWnd::UIItemShopWnd()
    : m_viewOffset(0),
      m_hoverRow(-1),
      m_lastDrawStateToken(0ull),
      m_hasDrawStateToken(false)
{
    Create(kWindowWidth, kWindowHeight);
    Move(84, 98);
}

UIItemShopWnd::~UIItemShopWnd()
{
    m_iconCache.clear();
}

void UIItemShopWnd::SetShow(int show)
{
    UIWindow::SetShow(show);
    if (show == 0) {
        m_hoverRow = -1;
        return;
    }

    int savedX = m_x;
    int savedY = m_y;
    if (LoadUiWindowPlacement("ItemShopWnd", &savedX, &savedY)) {
        g_windowMgr.ClampWindowToClient(&savedX, &savedY, m_w, m_h);
        Move(savedX, savedY);
    }
}

bool UIItemShopWnd::IsUpdateNeed()
{
    if (m_show == 0) {
        return false;
    }
    if (m_isDirty != 0 || !m_hasDrawStateToken) {
        return true;
    }
    return BuildDisplayStateToken() != m_lastDrawStateToken;
}

void UIItemShopWnd::StoreInfo()
{
    SaveUiWindowPlacement("ItemShopWnd", m_x, m_y);
}

int UIItemShopWnd::GetVisibleRowCount() const
{
    return (std::max)(1, ((m_h - kListTop - kListBottomMargin) / kRowHeight) - 1);
}

int UIItemShopWnd::GetMaxViewOffset() const
{
    const int visibleRows = GetVisibleRowCount();
    if (static_cast<int>(g_session.m_shopRows.size()) <= visibleRows) {
        return 0;
    }
    return static_cast<int>(g_session.m_shopRows.size()) - visibleRows;
}

RECT UIItemShopWnd::GetListRect() const
{
    return shopui::MakeRect(m_x + kListSideMargin, m_y + kListTop, m_w - kListSideMargin * 2, m_h - kListTop - kListBottomMargin);
}

int UIItemShopWnd::HitTestSourceRow(int x, int y) const
{
    const RECT listRect = GetListRect();
    if (x < listRect.left || x >= listRect.right || y < listRect.top || y >= listRect.bottom) {
        return -1;
    }

    const int localRow = (y - listRect.top) / kRowHeight;
    if (localRow <= 0) {
        return -1;
    }
    const int rowIndex = m_viewOffset + localRow - 1;
    if (rowIndex < 0 || rowIndex >= static_cast<int>(g_session.m_shopRows.size())) {
        return -1;
    }
    return rowIndex;
}

const shopui::BitmapPixels* UIItemShopWnd::GetItemIcon(const ITEM_INFO& item)
{
    const unsigned int itemId = item.GetItemId();
    const auto found = m_iconCache.find(itemId);
    if (found != m_iconCache.end()) {
        return found->second.IsValid() ? &found->second : nullptr;
    }

    shopui::BitmapPixels bitmap;
    for (const std::string& candidate : shopui::BuildItemIconCandidates(item)) {
        if (!g_fileMgr.IsDataExist(candidate.c_str())) {
            continue;
        }
        bitmap = shopui::LoadBitmapPixelsFromGameData(candidate, true);
        if (bitmap.IsValid()) {
            break;
        }
    }

    auto inserted = m_iconCache.emplace(itemId, std::move(bitmap));
    return inserted.first->second.IsValid() ? &inserted.first->second : nullptr;
}

void UIItemShopWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) {
        m_lastDrawStateToken = BuildDisplayStateToken();
        m_hasDrawStateToken = true;
        return;
    }

    HDC hdc = AcquireDrawTarget();
    if (!hdc || m_show == 0) {
        return;
    }

    const char* title = g_session.m_shopMode == NpcShopMode::Sell ? "Sellable Items" : "Shop Items";
    const RECT bounds = shopui::MakeRect(m_x, m_y, m_w, m_h);
    const RECT listRect = GetListRect();
    shopui::DrawFrameWindow(hdc, bounds, title);
    shopui::FillRectColor(hdc, listRect, RGB(248, 248, 248));
    shopui::FrameRectColor(hdc, listRect, RGB(120, 120, 120));

    const RECT headerRect = shopui::MakeRect(listRect.left + 1, listRect.top + 1, listRect.right - listRect.left - 2, kRowHeight - 1);
    shopui::FillRectColor(hdc, headerRect, RGB(222, 229, 237));
    shopui::DrawWindowTextRect(hdc, shopui::MakeRect(headerRect.left + 26, headerRect.top + 1, 170, headerRect.bottom - headerRect.top - 2), "Item", RGB(30, 30, 30), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    if (g_session.m_shopMode == NpcShopMode::Sell) {
        shopui::DrawWindowTextRect(hdc, shopui::MakeRect(headerRect.left + 190, headerRect.top + 1, 38, headerRect.bottom - headerRect.top - 2), "Qty", RGB(30, 30, 30), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    shopui::DrawWindowTextRect(hdc, shopui::MakeRect(headerRect.right - 78, headerRect.top + 1, 70, headerRect.bottom - headerRect.top - 2), "Price", RGB(30, 30, 30), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

    const int visibleRows = GetVisibleRowCount();
    const int startRow = m_viewOffset;
    const int endRow = (std::min)(static_cast<int>(g_session.m_shopRows.size()), startRow + visibleRows);
    for (int rowIndex = startRow; rowIndex < endRow; ++rowIndex) {
        const NPC_SHOP_ROW& row = g_session.m_shopRows[static_cast<size_t>(rowIndex)];
        RECT rowRect = shopui::MakeRect(listRect.left + 1,
            listRect.top + 1 + (rowIndex - startRow + 1) * kRowHeight,
            listRect.right - listRect.left - 2,
            kRowHeight);
        const bool selected = g_session.m_shopSelectedSourceRow == rowIndex;
        const bool hot = m_hoverRow == rowIndex;
        if (selected) {
            shopui::FillRectColor(hdc, rowRect, RGB(188, 204, 226));
        } else if (hot) {
            shopui::FillRectColor(hdc, rowRect, RGB(226, 234, 244));
        }

        RECT iconRect = shopui::MakeRect(rowRect.left + 3, rowRect.top + 1, 16, 16);
        if (const shopui::BitmapPixels* icon = GetItemIcon(row.itemInfo)) {
            shopui::DrawBitmapPixelsTransparent(hdc, *icon, iconRect);
        }

        const std::string itemName = shopui::GetItemDisplayName(row.itemInfo);
        shopui::DrawWindowTextRect(hdc,
            shopui::MakeRect(rowRect.left + 24, rowRect.top + 1, 160, rowRect.bottom - rowRect.top - 2),
            itemName,
            RGB(26, 26, 26),
            DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        if (g_session.m_shopMode == NpcShopMode::Sell) {
            shopui::DrawWindowTextRect(hdc,
                shopui::MakeRect(rowRect.left + 188, rowRect.top + 1, 38, rowRect.bottom - rowRect.top - 2),
                FormatPrice(row.availableCount),
                RGB(48, 48, 48),
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        shopui::DrawWindowTextRect(hdc,
            shopui::MakeRect(rowRect.right - 80, rowRect.top + 1, 74, rowRect.bottom - rowRect.top - 2),
            FormatPrice(g_session.GetNpcShopUnitPrice(row)),
            RGB(28, 60, 98),
            DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }

    m_lastDrawStateToken = BuildDisplayStateToken();
    m_hasDrawStateToken = true;
    ReleaseDrawTarget(hdc);
}

void UIItemShopWnd::OnLBtnDown(int x, int y)
{
    const int rowIndex = HitTestSourceRow(x, y);
    if (rowIndex >= 0) {
        g_session.m_shopSelectedSourceRow = rowIndex;
        return;
    }
    UIFrameWnd::OnLBtnDown(x, y);
}

void UIItemShopWnd::OnLBtnDblClk(int x, int y)
{
    const int rowIndex = HitTestSourceRow(x, y);
    if (rowIndex >= 0) {
        g_session.AdjustNpcShopDealBySourceRow(static_cast<size_t>(rowIndex), 1);
    }
}

void UIItemShopWnd::OnMouseMove(int x, int y)
{
    m_hoverRow = HitTestSourceRow(x, y);
    UIFrameWnd::OnMouseMove(x, y);
}

void UIItemShopWnd::OnWheel(int delta)
{
    const int step = delta > 0 ? -1 : 1;
    m_viewOffset = (std::max)(0, (std::min)(GetMaxViewOffset(), m_viewOffset + step));
}

void UIItemShopWnd::HandleKeyDown(int virtualKey)
{
    if (g_session.m_shopRows.empty()) {
        return;
    }

    int selectedRow = g_session.m_shopSelectedSourceRow;
    if (selectedRow < 0) {
        selectedRow = 0;
    }

    switch (virtualKey) {
    case VK_UP:
        selectedRow = (std::max)(0, selectedRow - 1);
        break;
    case VK_DOWN:
        selectedRow = (std::min)(static_cast<int>(g_session.m_shopRows.size()) - 1, selectedRow + 1);
        break;
    case VK_RETURN:
        g_session.AdjustNpcShopDealBySourceRow(static_cast<size_t>(selectedRow), 1);
        return;
    default:
        return;
    }

    g_session.m_shopSelectedSourceRow = selectedRow;
    if (selectedRow < m_viewOffset) {
        m_viewOffset = selectedRow;
    } else if (selectedRow >= m_viewOffset + GetVisibleRowCount()) {
        m_viewOffset = selectedRow - GetVisibleRowCount() + 1;
    }
    m_viewOffset = (std::max)(0, (std::min)(GetMaxViewOffset(), m_viewOffset));
}

int UIItemShopWnd::GetViewOffset() const
{
    return m_viewOffset;
}

int UIItemShopWnd::GetHoverRow() const
{
    return m_hoverRow;
}

int UIItemShopWnd::GetVisibleRowCountForQt() const
{
    return GetVisibleRowCount();
}

unsigned long long UIItemShopWnd::BuildDisplayStateToken() const
{
    unsigned long long hash = 1469598103934665603ull;
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_show));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_x));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_y));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_w));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_h));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(m_viewOffset));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(static_cast<unsigned int>(m_hoverRow + 1)));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(static_cast<int>(g_session.m_shopMode)));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(g_session.m_shopSelectedSourceRow + 1));
    shopui::HashTokenValue(&hash, static_cast<unsigned long long>(g_session.m_shopRows.size()));
    for (const NPC_SHOP_ROW& row : g_session.m_shopRows) {
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.itemInfo.GetItemId()));
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.itemInfo.m_itemType));
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.sourceItemIndex));
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.price));
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.secondaryPrice));
        shopui::HashTokenValue(&hash, static_cast<unsigned long long>(row.availableCount));
    }
    return hash;
}
