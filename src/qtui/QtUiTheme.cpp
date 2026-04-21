#include "QtUiTheme.h"

#include "DebugLog.h"

namespace {

const char* ModeName(QtUiTheme::Mode mode)
{
    switch (mode) {
    case QtUiTheme::Mode::Dark:       return "dark";
    case QtUiTheme::Mode::Midnight:   return "midnight";
    case QtUiTheme::Mode::Slate:      return "slate";
    case QtUiTheme::Mode::Forest:     return "forest";
    case QtUiTheme::Mode::Parchment:  return "parchment";
    case QtUiTheme::Mode::Light:
    default:                           return "light";
    }
}

QtUiTheme::Mode ParseModeName(const QString& lowered)
{
    if (lowered == QStringLiteral("dark"))      return QtUiTheme::Mode::Dark;
    if (lowered == QStringLiteral("midnight"))  return QtUiTheme::Mode::Midnight;
    if (lowered == QStringLiteral("slate"))     return QtUiTheme::Mode::Slate;
    if (lowered == QStringLiteral("forest"))    return QtUiTheme::Mode::Forest;
    if (lowered == QStringLiteral("parchment")) return QtUiTheme::Mode::Parchment;
    return QtUiTheme::Mode::Light;
}

} // namespace

QtUiTheme::QtUiTheme(QObject* parent)
    : QObject(parent)
{
    applyPalette();
}

QString QtUiTheme::mode() const
{
    return QString::fromLatin1(ModeName(m_mode));
}

void QtUiTheme::setMode(Mode mode)
{
    DbgLog("[Theme] setMode from=%s to=%s\n", ModeName(m_mode), ModeName(mode));
    if (mode == m_mode) {
        return;
    }
    m_mode = mode;
    applyPalette();
    emit changed();
    DbgLog("[Theme] emitted changed signal, bg=rgba(%d,%d,%d,%d)\n",
        m_p.background.red(), m_p.background.green(), m_p.background.blue(), m_p.background.alpha());
}

void QtUiTheme::setModeByName(const QString& name)
{
    setMode(ParseModeName(name.trimmed().toLower()));
}

bool QtUiTheme::IsKnownModeName(const QString& name)
{
    const QString lowered = name.trimmed().toLower();
    return lowered == QStringLiteral("light")
        || lowered == QStringLiteral("dark")
        || lowered == QStringLiteral("midnight")
        || lowered == QStringLiteral("slate")
        || lowered == QStringLiteral("forest")
        || lowered == QStringLiteral("parchment");
}

void QtUiTheme::applyPalette()
{
    switch (m_mode) {
    case Mode::Dark:       m_p = DarkPalette(); break;
    case Mode::Midnight:   m_p = MidnightPalette(); break;
    case Mode::Slate:      m_p = SlatePalette(); break;
    case Mode::Forest:     m_p = ForestPalette(); break;
    case Mode::Parchment:  m_p = ParchmentPalette(); break;
    case Mode::Light:
    default:                m_p = LightPalette(); break;
    }
}

QtUiTheme::Palette QtUiTheme::LightPalette()
{
    Palette p;
    p.background     = QColor(243, 240, 231, 230);
    p.surface        = QColor(255, 255, 255, 230);
    p.surfaceAlt     = QColor(234, 228, 213, 230);
    p.overlayDim     = QColor(0, 0, 0, 120);
    p.border         = QColor(175, 160, 128);
    p.borderStrong   = QColor(120, 112, 96);

    p.text           = QColor(30, 24, 16);
    p.textMuted      = QColor(96, 88, 72);
    p.textInverted   = QColor(245, 243, 238);
    p.accent         = QColor(56, 110, 196);
    p.accentText     = QColor(255, 255, 255);

    p.buttonBg       = QColor(232, 225, 205, 230);
    p.buttonBgHover  = QColor(245, 238, 216, 230);
    p.buttonBgPressed= QColor(210, 200, 176, 230);
    p.buttonBorder   = QColor(140, 128, 96);
    p.buttonText     = QColor(30, 24, 16);

    p.inputBg        = QColor(255, 253, 236);
    p.inputBorder    = QColor(140, 128, 96);
    p.inputText      = QColor(30, 24, 16);

    p.scrollTrack    = QColor(220, 212, 190, 200);
    p.scrollThumb    = QColor(140, 128, 96, 230);

    p.hpFill         = QColor(200, 48, 48);
    p.spFill         = QColor(48, 120, 200);
    p.expFill        = QColor(80, 160, 80);

    p.chatSystem     = QColor(120, 80, 24);
    p.chatWhisper    = QColor(168, 72, 168);
    p.chatParty      = QColor(48, 128, 168);
    p.chatGuild      = QColor(64, 128, 64);
    return p;
}

QtUiTheme::Palette QtUiTheme::DarkPalette()
{
    Palette p;
    p.background     = QColor(24, 26, 32, 235);
    p.surface        = QColor(36, 40, 48, 235);
    p.surfaceAlt     = QColor(48, 52, 62, 235);
    p.overlayDim     = QColor(0, 0, 0, 160);
    p.border         = QColor(72, 80, 96);
    p.borderStrong   = QColor(140, 152, 176);

    p.text           = QColor(232, 232, 236);
    p.textMuted      = QColor(186, 192, 206);
    p.textInverted   = QColor(20, 22, 28);
    p.accent         = QColor(56, 114, 200);
    p.accentText     = QColor(244, 248, 252);

    p.buttonBg       = QColor(52, 60, 76, 235);
    p.buttonBgHover  = QColor(72, 84, 108, 235);
    p.buttonBgPressed= QColor(36, 44, 58, 235);
    p.buttonBorder   = QColor(110, 124, 150);
    p.buttonText     = QColor(232, 232, 236);

    p.inputBg        = QColor(20, 22, 28);
    p.inputBorder    = QColor(110, 124, 150);
    p.inputText      = QColor(232, 232, 236);

    p.scrollTrack    = QColor(40, 44, 54, 200);
    p.scrollThumb    = QColor(110, 124, 150, 230);

    p.hpFill         = QColor(220, 80, 80);
    p.spFill         = QColor(88, 160, 240);
    p.expFill        = QColor(120, 200, 120);

    p.chatSystem     = QColor(224, 176, 96);
    p.chatWhisper    = QColor(224, 144, 224);
    p.chatParty      = QColor(110, 190, 230);
    p.chatGuild      = QColor(130, 210, 130);
    return p;
}

QtUiTheme::Palette QtUiTheme::MidnightPalette()
{
    Palette p;
    p.background     = QColor(10, 12, 20, 240);
    p.surface        = QColor(18, 22, 34, 240);
    p.surfaceAlt     = QColor(28, 34, 50, 240);
    p.overlayDim     = QColor(0, 0, 0, 180);
    p.border         = QColor(46, 58, 82);
    p.borderStrong   = QColor(96, 176, 220);

    p.text           = QColor(220, 228, 240);
    p.textMuted      = QColor(154, 170, 198);
    p.textInverted   = QColor(10, 12, 20);
    p.accent         = QColor(42, 132, 170);
    p.accentText     = QColor(240, 248, 252);

    p.buttonBg       = QColor(26, 34, 52, 240);
    p.buttonBgHover  = QColor(44, 60, 88, 240);
    p.buttonBgPressed= QColor(16, 22, 36, 240);
    p.buttonBorder   = QColor(72, 128, 176);
    p.buttonText     = QColor(220, 236, 248);

    p.inputBg        = QColor(8, 12, 22);
    p.inputBorder    = QColor(72, 128, 176);
    p.inputText      = QColor(220, 236, 248);

    p.scrollTrack    = QColor(20, 26, 40, 210);
    p.scrollThumb    = QColor(88, 144, 192, 230);

    p.hpFill         = QColor(232, 96, 108);
    p.spFill         = QColor(96, 184, 248);
    p.expFill        = QColor(128, 216, 160);

    p.chatSystem     = QColor(248, 200, 96);
    p.chatWhisper    = QColor(232, 152, 232);
    p.chatParty      = QColor(120, 204, 240);
    p.chatGuild      = QColor(144, 228, 152);
    return p;
}

QtUiTheme::Palette QtUiTheme::SlatePalette()
{
    Palette p;
    p.background     = QColor(46, 52, 60, 235);
    p.surface        = QColor(58, 66, 76, 235);
    p.surfaceAlt     = QColor(72, 82, 94, 235);
    p.overlayDim     = QColor(0, 0, 0, 140);
    p.border         = QColor(98, 110, 126);
    p.borderStrong   = QColor(158, 178, 200);

    p.text           = QColor(232, 236, 240);
    p.textMuted      = QColor(190, 202, 216);
    p.textInverted   = QColor(32, 38, 46);
    p.accent         = QColor(68, 120, 170);
    p.accentText     = QColor(240, 246, 252);

    p.buttonBg       = QColor(76, 86, 100, 235);
    p.buttonBgHover  = QColor(100, 116, 136, 235);
    p.buttonBgPressed= QColor(54, 62, 74, 235);
    p.buttonBorder   = QColor(132, 150, 172);
    p.buttonText     = QColor(232, 236, 240);

    p.inputBg        = QColor(32, 38, 46);
    p.inputBorder    = QColor(132, 150, 172);
    p.inputText      = QColor(232, 236, 240);

    p.scrollTrack    = QColor(60, 68, 80, 210);
    p.scrollThumb    = QColor(132, 150, 172, 230);

    p.hpFill         = QColor(216, 96, 96);
    p.spFill         = QColor(96, 160, 224);
    p.expFill        = QColor(128, 200, 128);

    p.chatSystem     = QColor(232, 184, 104);
    p.chatWhisper    = QColor(224, 152, 224);
    p.chatParty      = QColor(120, 196, 232);
    p.chatGuild      = QColor(140, 216, 140);
    return p;
}

QtUiTheme::Palette QtUiTheme::ForestPalette()
{
    Palette p;
    p.background     = QColor(26, 38, 30, 236);
    p.surface        = QColor(38, 54, 42, 236);
    p.surfaceAlt     = QColor(52, 72, 56, 236);
    p.overlayDim     = QColor(0, 0, 0, 150);
    p.border         = QColor(80, 104, 80);
    p.borderStrong   = QColor(192, 160, 88);

    p.text           = QColor(232, 228, 208);
    p.textMuted      = QColor(194, 196, 170);
    p.textInverted   = QColor(20, 30, 22);
    p.accent         = QColor(208, 172, 88);
    p.accentText     = QColor(28, 22, 8);

    p.buttonBg       = QColor(52, 72, 56, 236);
    p.buttonBgHover  = QColor(76, 100, 76, 236);
    p.buttonBgPressed= QColor(36, 52, 40, 236);
    p.buttonBorder   = QColor(136, 112, 64);
    p.buttonText     = QColor(240, 228, 196);

    p.inputBg        = QColor(20, 30, 22);
    p.inputBorder    = QColor(136, 112, 64);
    p.inputText      = QColor(240, 228, 196);

    p.scrollTrack    = QColor(40, 56, 44, 210);
    p.scrollThumb    = QColor(136, 112, 64, 230);

    p.hpFill         = QColor(208, 96, 80);
    p.spFill         = QColor(120, 168, 200);
    p.expFill        = QColor(168, 200, 96);

    p.chatSystem     = QColor(232, 184, 96);
    p.chatWhisper    = QColor(216, 160, 216);
    p.chatParty      = QColor(128, 192, 208);
    p.chatGuild      = QColor(176, 216, 120);
    return p;
}

QtUiTheme::Palette QtUiTheme::ParchmentPalette()
{
    Palette p;
    p.background     = QColor(238, 226, 196, 232);
    p.surface        = QColor(248, 240, 218, 232);
    p.surfaceAlt     = QColor(224, 208, 172, 232);
    p.overlayDim     = QColor(40, 24, 10, 130);
    p.border         = QColor(156, 120, 72);
    p.borderStrong   = QColor(108, 64, 40);

    p.text           = QColor(56, 30, 14);
    p.textMuted      = QColor(116, 80, 48);
    p.textInverted   = QColor(248, 240, 218);
    p.accent         = QColor(140, 40, 40);
    p.accentText     = QColor(248, 240, 218);

    p.buttonBg       = QColor(226, 208, 168, 232);
    p.buttonBgHover  = QColor(240, 224, 184, 232);
    p.buttonBgPressed= QColor(200, 176, 128, 232);
    p.buttonBorder   = QColor(128, 88, 48);
    p.buttonText     = QColor(56, 30, 14);

    p.inputBg        = QColor(252, 244, 222);
    p.inputBorder    = QColor(128, 88, 48);
    p.inputText      = QColor(56, 30, 14);

    p.scrollTrack    = QColor(212, 192, 148, 210);
    p.scrollThumb    = QColor(140, 96, 56, 230);

    p.hpFill         = QColor(176, 40, 40);
    p.spFill         = QColor(56, 96, 160);
    p.expFill        = QColor(88, 136, 64);

    p.chatSystem     = QColor(120, 72, 16);
    p.chatWhisper    = QColor(144, 56, 144);
    p.chatParty      = QColor(40, 104, 144);
    p.chatGuild      = QColor(64, 112, 48);
    return p;
}
