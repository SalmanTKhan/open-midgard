#include "NpcDialogColoredText.h"

namespace {

bool IsHexDigit(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int HexValue(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return 0;
}

bool IsColorCodeAt(const std::string& t, size_t i)
{
    if (i + 7 > t.size() || t[i] != '^') {
        return false;
    }
    for (size_t k = 1; k <= 6; ++k) {
        if (!IsHexDigit(t[i + k])) {
            return false;
        }
    }
    return true;
}

bool TryConsumeColorCode(const std::string& t, size_t& i, COLORREF& out)
{
    if (!IsColorCodeAt(t, i)) {
        return false;
    }
    const int r = (HexValue(t[i + 1]) << 4) | HexValue(t[i + 2]);
    const int g = (HexValue(t[i + 3]) << 4) | HexValue(t[i + 4]);
    const int b = (HexValue(t[i + 5]) << 4) | HexValue(t[i + 6]);
    out = RGB(r, g, b);
    i += 7;
    return true;
}

void DrawFragmentWrapped(
    HDC hdc, const RECT& area, int& x, int& y, int lineHeight, COLORREF color, const std::string& frag)
{
    if (frag.empty()) {
        return;
    }

    SetTextColor(hdc, color);
    const int left = area.left;
    const int right = area.right;
    size_t p = 0;

    while (p < frag.size()) {
        if (frag[p] == ' ' || frag[p] == '\t') {
            size_t q = p;
            while (q < frag.size() && (frag[q] == ' ' || frag[q] == '\t')) {
                ++q;
            }
            const std::string ws = frag.substr(p, q - p);
            SIZE sz{};
            GetTextExtentPoint32A(hdc, ws.c_str(), static_cast<int>(ws.size()), &sz);
            if (x + sz.cx > right && x > left) {
                x = left;
                y += lineHeight;
            }
            TextOutA(hdc, x, y, ws.c_str(), static_cast<int>(ws.size()));
            x += sz.cx;
            p = q;
            continue;
        }

        size_t q = p;
        while (q < frag.size() && frag[q] != ' ' && frag[q] != '\t') {
            ++q;
        }
        const std::string word = frag.substr(p, q - p);
        SIZE sz{};
        GetTextExtentPoint32A(hdc, word.c_str(), static_cast<int>(word.size()), &sz);

        if (x + sz.cx > right && x > left) {
            x = left;
            y += lineHeight;
        }

        if (x + sz.cx > right && x == left) {
            size_t wi = 0;
            while (wi < word.size()) {
                if (y + lineHeight > area.bottom) {
                    return;
                }
                int lo = 1;
                int hi = static_cast<int>(word.size() - wi);
                int best = 1;
                while (lo <= hi) {
                    const int mid = (lo + hi) / 2;
                    GetTextExtentPoint32A(hdc, word.c_str() + wi, mid, &sz);
                    if (x + sz.cx <= right) {
                        best = mid;
                        lo = mid + 1;
                    } else {
                        hi = mid - 1;
                    }
                }
                if (best < 1) {
                    best = 1;
                }
                GetTextExtentPoint32A(hdc, word.c_str() + wi, best, &sz);
                TextOutA(hdc, x, y, word.c_str() + wi, best);
                x += sz.cx;
                wi += static_cast<size_t>(best);
                if (wi < word.size()) {
                    x = left;
                    y += lineHeight;
                }
            }
            p = q;
            continue;
        }

        TextOutA(hdc, x, y, word.c_str(), static_cast<int>(word.size()));
        x += sz.cx;
        p = q;
    }
}

} // namespace

void DrawNpcSayDialogColoredText(HDC hdc, const RECT& textRect, const std::string& text)
{
    TEXTMETRICA tm{};
    GetTextMetricsA(hdc, &tm);
    const int lineHeight = tm.tmHeight;
    int x = textRect.left;
    int y = textRect.top;
    COLORREF color = RGB(0, 0, 0);
    size_t i = 0;

    while (i < text.size()) {
        if (y + lineHeight > textRect.bottom) {
            break;
        }

        if (text[i] == '\r' && i + 1 < text.size() && text[i + 1] == '\n') {
            i += 2;
            x = textRect.left;
            y += lineHeight;
            continue;
        }
        if (text[i] == '\n') {
            ++i;
            x = textRect.left;
            y += lineHeight;
            continue;
        }

        COLORREF newColor{};
        if (TryConsumeColorCode(text, i, newColor)) {
            color = newColor;
            continue;
        }

        const size_t start = i;
        while (i < text.size()) {
            if (text[i] == '\r' || text[i] == '\n') {
                break;
            }
            if (text[i] == '^' && IsColorCodeAt(text, i)) {
                break;
            }
            ++i;
        }

        if (i > start) {
            DrawFragmentWrapped(
                hdc, textRect, x, y, lineHeight, color, text.substr(start, i - start));
        }
    }
}

void DrawNpcMenuOptionColoredText(HDC hdc, const RECT& textRect, const std::string& text)
{
    TEXTMETRICA tm{};
    GetTextMetricsA(hdc, &tm);
    const int lineHeight = tm.tmHeight;
    const int yBase = textRect.top + ((textRect.bottom - textRect.top) - lineHeight) / 2;

    COLORREF color = RGB(0, 0, 0);
    int x = textRect.left;
    size_t i = 0;

    while (i < text.size() && x < textRect.right) {
        COLORREF newColor{};
        if (TryConsumeColorCode(text, i, newColor)) {
            color = newColor;
            continue;
        }

        const size_t start = i;
        while (i < text.size()) {
            if (text[i] == '^' && IsColorCodeAt(text, i)) {
                break;
            }
            ++i;
        }

        if (i <= start) {
            break;
        }

        const std::string seg = text.substr(start, i - start);
        SetTextColor(hdc, color);
        SIZE sz{};
        GetTextExtentPoint32A(hdc, seg.c_str(), static_cast<int>(seg.size()), &sz);
        const int avail = textRect.right - x;
        if (sz.cx > avail) {
            static const char kEll[] = "...";
            SIZE esz{};
            GetTextExtentPoint32A(hdc, kEll, 3, &esz);
            if (avail <= esz.cx) {
                break;
            }
            int lo = 0;
            int hi = static_cast<int>(seg.size());
            int best = 0;
            while (lo <= hi) {
                const int mid = (lo + hi) / 2;
                SIZE msz{};
                GetTextExtentPoint32A(hdc, seg.c_str(), mid, &msz);
                if (msz.cx + esz.cx <= avail) {
                    best = mid;
                    lo = mid + 1;
                } else {
                    hi = mid - 1;
                }
            }
            SIZE prefixSz{};
            GetTextExtentPoint32A(hdc, seg.c_str(), best, &prefixSz);
            TextOutA(hdc, x, yBase, seg.c_str(), best);
            TextOutA(hdc, x + prefixSz.cx, yBase, kEll, 3);
            break;
        }

        TextOutA(hdc, x, yBase, seg.c_str(), static_cast<int>(seg.size()));
        x += sz.cx;
    }
}
