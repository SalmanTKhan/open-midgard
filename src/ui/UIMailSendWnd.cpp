#include "UIMailSendWnd.h"

#include "UIWindowMgr.h"
#include "UINpcInputWnd.h"
#include "core/ClientFeature.h"
#include "gamemode/GameMode.h"
#include "main/WinMain.h"
#include "qtui/QtUiRuntime.h"

#include <cstdio>
#include <windows.h>

bool RequestMailSend(const char* recipient, const char* subject, const char* body);
bool RequestMailResetAttachment();

namespace {

constexpr int kPad = 8;
constexpr int kRowHeight = 22;
constexpr int kLabelWidth = 80;
constexpr int kButtonHeight = 22;
constexpr int kButtonWidth = 70;

void FillSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void FrameSolid(HDC hdc, const RECT& rc, COLORREF color)
{
    HBRUSH brush = CreateSolidBrush(color);
    FrameRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void DrawLabel(HDC hdc, int x, int y, COLORREF color, const std::string& text,
               UINT format = DT_LEFT | DT_TOP | DT_SINGLELINE)
{
    if (text.empty()) return;
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);
    RECT rc{ x, y, x + 600, y + 32 };
    ::DrawTextA(hdc, text.c_str(), static_cast<int>(text.size()), &rc, format);
}

}  // namespace

UIMailSendWnd::UIMailSendWnd()
{
    Create(kFullWidth, kFullHeight);
    Move(kDefaultX, kDefaultY);
    int sx = m_x;
    int sy = m_y;
    if (LoadUiWindowPlacement("MailSendWnd", &sx, &sy)) {
        g_windowMgr.ClampWindowToClient(&sx, &sy, m_w, m_h);
        Move(sx, sy);
    }
}

UIMailSendWnd::~UIMailSendWnd() = default;

void UIMailSendWnd::SetShow(int show)
{
    if (show && !IsFeatureEnabled(ClientFeature::MailLegacy)) {
        UIWindow::SetShow(0);
        return;
    }
    UIWindow::SetShow(show);
}

UIMailSendWnd::InteractRect UIMailSendWnd::GetFieldRect(int fieldId) const
{
    int rowIndex = 0;
    switch (fieldId) {
    case FieldRecipient: rowIndex = 0; break;
    case FieldSubject:   rowIndex = 1; break;
    case FieldBody:      rowIndex = 2; break;
    case FieldZeny:      rowIndex = 3; break;
    default: rowIndex = 0; break;
    }
    const int x = m_x + kPad + kLabelWidth;
    const int y = m_y + kTitleHeight + kPad + rowIndex * kRowHeight;
    const int w = m_w - (kPad * 2) - kLabelWidth;
    const int h = (fieldId == FieldBody) ? (kRowHeight * 4) : kRowHeight - 2;
    return InteractRect{ x, y, w, h };
}

UIMailSendWnd::InteractRect UIMailSendWnd::GetActionButtonRect(int buttonId) const
{
    const int y = m_y + m_h - kButtonHeight - kPad;
    int slotFromRight = 0;
    switch (buttonId) {
    case ButtonCancel:      slotFromRight = 0; break;
    case ButtonSend:        slotFromRight = 1; break;
    case ButtonResetAttach: slotFromRight = 2; break;
    default: break;
    }
    const int gap = 6;
    const int x = m_x + m_w - kPad - (slotFromRight + 1) * kButtonWidth - slotFromRight * gap;
    return InteractRect{ x, y, kButtonWidth, kButtonHeight };
}

int UIMailSendWnd::HitTestField(int x, int y) const
{
    auto inRect = [&](const InteractRect& r) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    };
    if (inRect(GetFieldRect(FieldRecipient))) return FieldRecipient;
    if (inRect(GetFieldRect(FieldSubject)))   return FieldSubject;
    if (inRect(GetFieldRect(FieldBody)))      return FieldBody;
    if (inRect(GetFieldRect(FieldZeny)))      return FieldZeny;
    return 0;
}

int UIMailSendWnd::HitTestActionButton(int x, int y) const
{
    auto inRect = [&](const InteractRect& r) {
        return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
    };
    if (inRect(GetActionButtonRect(ButtonSend)))        return ButtonSend;
    if (inRect(GetActionButtonRect(ButtonCancel)))      return ButtonCancel;
    if (inRect(GetActionButtonRect(ButtonResetAttach))) return ButtonResetAttach;
    return 0;
}

void UIMailSendWnd::OpenFieldEditor(int fieldId)
{
    auto* input = static_cast<UINpcInputWnd*>(
        g_windowMgr.MakeWindow(UIWindowMgr::WID_NPCINPUTWND));
    if (!input) return;

    switch (fieldId) {
    case FieldRecipient:
        input->OpenGameStringPrompt("Recipient name",
                                    CGameMode::GameMsg_RequestMailRecipient, 0);
        break;
    case FieldSubject:
        input->OpenGameStringPrompt("Mail subject",
                                    CGameMode::GameMsg_RequestMailSubject, 0);
        break;
    case FieldBody:
        input->OpenGameStringPrompt("Mail body (max 200 chars)",
                                    CGameMode::GameMsg_RequestMailBody, 0);
        break;
    case FieldZeny:
        input->OpenGameNumberPrompt("Attach zeny",
                                    CGameMode::GameMsg_RequestMailZeny,
                                    0, m_zeny, 1000000000u);
        break;
    default: break;
    }
}

bool UIMailSendWnd::SubmitSend()
{
    if (m_recipient.empty() || m_subject.empty()) {
        return false;
    }
    const bool ok = RequestMailSend(m_recipient.c_str(),
                                    m_subject.c_str(),
                                    m_body.c_str());
    if (ok) {
        // Clear draft and hide; server emits 0x0249 with the actual result.
        m_recipient.clear();
        m_subject.clear();
        m_body.clear();
        m_zeny = 0;
        m_attachInventoryIndex = 0;
        m_attachAmount = 0;
        SetShow(0);
    }
    return ok;
}

void UIMailSendWnd::OnDraw()
{
    if (IsQtUiRuntimeEnabled()) return;
    if (m_show == 0) return;
    HDC hdc = AcquireDrawTarget();
    if (!hdc) return;

    const RECT bg{ m_x, m_y, m_x + m_w, m_y + m_h };
    FillSolid(hdc, bg, RGB(232, 220, 192));
    FrameSolid(hdc, bg, RGB(80, 60, 30));

    const RECT title{ m_x, m_y, m_x + m_w, m_y + kTitleHeight };
    FillSolid(hdc, title, RGB(126, 96, 62));
    DrawLabel(hdc, m_x + 8, m_y + 2, RGB(255, 248, 220), "Send Mail");

    if (m_minimized) {
        ReleaseDrawTarget(hdc);
        m_isDirty = 0;
        return;
    }

    auto drawField = [&](int fieldId, const char* label, const std::string& value) {
        const InteractRect r = GetFieldRect(fieldId);
        const int rowY = r.y;
        DrawLabel(hdc, m_x + kPad, rowY + 3, RGB(60, 40, 10), label);
        const RECT box{ r.x, r.y, r.x + r.w, r.y + r.h };
        FillSolid(hdc, box, RGB(252, 248, 232));
        FrameSolid(hdc, box, RGB(160, 140, 90));
        DrawLabel(hdc, r.x + 4, r.y + 3, RGB(20, 20, 20),
                  value.empty() ? std::string("(click to edit)") : value);
    };

    drawField(FieldRecipient, "To:", m_recipient);
    drawField(FieldSubject,   "Subject:", m_subject);
    drawField(FieldBody,      "Body:", m_body);

    char zenyBuf[24] = {};
    std::snprintf(zenyBuf, sizeof(zenyBuf), "%u z", m_zeny);
    drawField(FieldZeny, "Zeny:", zenyBuf);

    // Attachment summary line just below the zeny field.
    const InteractRect zenyR = GetFieldRect(FieldZeny);
    const int attachY = zenyR.y + zenyR.h + 4;
    char attachBuf[64] = {};
    if (m_attachInventoryIndex == 0 || m_attachAmount == 0) {
        std::snprintf(attachBuf, sizeof(attachBuf), "Attachment: (none)");
    } else {
        std::snprintf(attachBuf, sizeof(attachBuf), "Attachment: inv #%u  x %d",
                      m_attachInventoryIndex, m_attachAmount);
    }
    DrawLabel(hdc, m_x + kPad, attachY, RGB(60, 40, 10), attachBuf);

    auto drawButton = [&](int id, const char* label) {
        const InteractRect r = GetActionButtonRect(id);
        const RECT rc{ r.x, r.y, r.x + r.w, r.y + r.h };
        FillSolid(hdc, rc, RGB(210, 195, 145));
        FrameSolid(hdc, rc, RGB(80, 60, 30));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20, 20, 20));
        RECT textRc = rc;
        ::DrawTextA(hdc, label, -1, &textRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    };
    drawButton(ButtonSend, "Send");
    drawButton(ButtonResetAttach, "Reset Atch");
    drawButton(ButtonCancel, "Cancel");

    ReleaseDrawTarget(hdc);
    m_isDirty = 0;
}

void UIMailSendWnd::ToggleMinimized()
{
    m_minimized = !m_minimized;
    Resize(kFullWidth, m_minimized ? kTitleHeight : kFullHeight);
    Invalidate();
}

void UIMailSendWnd::BuildSystemButtons(std::vector<SystemButton>* out) const
{
    if (!out) return;
    out->clear();
    SystemButton minimize{}; minimize.id = 0; minimize.x = m_x + m_w - 34; minimize.y = m_y + 3; out->push_back(minimize);
    SystemButton close{}; close.id = 1; close.x = m_x + m_w - 17; close.y = m_y + 3; out->push_back(close);
}

void UIMailSendWnd::OnLBtnDown(int x, int y)
{
    if (m_show == 0) return;
    std::vector<SystemButton> buttons;
    BuildSystemButtons(&buttons);
    for (const SystemButton& b : buttons) {
        if (x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h) {
            if (b.id == 1) SetShow(0);
            else if (b.id == 0) ToggleMinimized();
            return;
        }
    }

    if (!m_minimized) {
        const int actionId = HitTestActionButton(x, y);
        if (actionId == ButtonSend) {
            SubmitSend();
            return;
        }
        if (actionId == ButtonCancel) {
            SetShow(0);
            return;
        }
        if (actionId == ButtonResetAttach) {
            RequestMailResetAttachment();
            m_attachInventoryIndex = 0;
            m_attachAmount = 0;
            Invalidate();
            return;
        }

        const int fieldId = HitTestField(x, y);
        if (fieldId != 0) {
            OpenFieldEditor(fieldId);
            return;
        }
    }

    UIFrameWnd::OnLBtnDown(x, y);
}

void UIMailSendWnd::StoreInfo()
{
    SaveUiWindowPlacement("MailSendWnd", m_x, m_y);
}

msgresult_t UIMailSendWnd::SendMsg(UIWindow*, int, msgparam_t, msgparam_t, msgparam_t)
{
    return 0;
}

bool UIMailSendWnd::GetDisplayDataForQt(DisplayData* outData) const
{
    if (!outData) return false;
    outData->title = "Send Mail";
    outData->recipient = m_recipient;
    outData->subject = m_subject;
    outData->body = m_body;
    outData->zeny = m_zeny;
    outData->attachInventoryIndex = m_attachInventoryIndex;
    outData->attachAmount = m_attachAmount;
    outData->minimized = m_minimized;
    BuildSystemButtons(&outData->systemButtons);
    return true;
}
