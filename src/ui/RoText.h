#pragma once

// Parser + renderers for Ragnarok Online inline text codes.
//
//   ^RRGGBB <text>   switches foreground color; ^000000 restores default.
//
// Two outputs are supported:
//   Strip()  — color codes removed, returns plain text (old SanitizeRoText).
//   ToHtml() — color codes converted to <span style="color:#RRGGBB"> runs with
//              HTML-escaped content, so the same string drives both QML
//              `Text.RichText` and `QTextDocument::setHtml()` in the legacy GDI
//              fallback via QPainter.

#include <cctype>
#include <cstddef>
#include <string>

namespace ro_text {

inline bool IsHexDigit(char ch)
{
    return std::isxdigit(static_cast<unsigned char>(ch)) != 0;
}

inline bool PeekColorCode(const std::string& text, size_t i)
{
    if (i >= text.size() || text[i] != '^' || i + 6 >= text.size()) {
        return false;
    }
    for (size_t j = 1; j <= 6; ++j) {
        if (!IsHexDigit(text[i + j])) {
            return false;
        }
    }
    return true;
}

inline std::string Strip(const std::string& text)
{
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (PeekColorCode(text, i)) {
            i += 6;
            continue;
        }
        const char ch = text[i];
        if (ch == '\r') {
            continue;
        }
        out.push_back(ch == '_' ? ' ' : ch);
    }
    return out;
}

inline void AppendHtmlEscaped(std::string& out, char ch)
{
    switch (ch) {
        case '<':  out.append("&lt;"); break;
        case '>':  out.append("&gt;"); break;
        case '&':  out.append("&amp;"); break;
        case '"':  out.append("&quot;"); break;
        case '\'': out.append("&#39;"); break;
        case '\n': out.append("<br/>"); break;
        case '\r': break;
        case ' ':  out.append("&nbsp;"); break;
        case '_':  out.append("&nbsp;"); break;
        default:   out.push_back(ch); break;
    }
}

// Convert RO-coded text to HTML. `defaultColorHex` (e.g. "#111111") is used for
// runs with no active color or when ^000000 resets the color. Newlines become
// <br/> and consecutive spaces are preserved via non-breaking spaces so the
// layout of pre-formatted RO descriptions survives the round trip.
inline std::string ToHtml(const std::string& text, const char* defaultColorHex = "#111111")
{
    std::string out;
    out.reserve(text.size() + 32);
    bool spanOpen = false;
    const auto closeSpan = [&]() {
        if (spanOpen) {
            out.append("</span>");
            spanOpen = false;
        }
    };

    for (size_t i = 0; i < text.size(); ++i) {
        if (PeekColorCode(text, i)) {
            const std::string code = text.substr(i + 1, 6);
            closeSpan();
            if (code != "000000") {
                out.append("<span style=\"color:#");
                out.append(code);
                out.append("\">");
                spanOpen = true;
            }
            i += 6;
            continue;
        }
        AppendHtmlEscaped(out, text[i]);
    }
    closeSpan();

    if (!out.empty()) {
        std::string wrapped = "<span style=\"color:";
        wrapped.append(defaultColorHex);
        wrapped.append("\">");
        wrapped.append(out);
        wrapped.append("</span>");
        return wrapped;
    }
    return out;
}

} // namespace ro_text
