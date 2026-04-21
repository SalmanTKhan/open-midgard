#pragma once

#include <QColor>
#include <QObject>
#include <QString>

class QtUiTheme : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString mode READ mode NOTIFY changed)

    Q_PROPERTY(QColor background READ background NOTIFY changed)
    Q_PROPERTY(QColor surface READ surface NOTIFY changed)
    Q_PROPERTY(QColor surfaceAlt READ surfaceAlt NOTIFY changed)
    Q_PROPERTY(QColor overlayDim READ overlayDim NOTIFY changed)
    Q_PROPERTY(QColor border READ border NOTIFY changed)
    Q_PROPERTY(QColor borderStrong READ borderStrong NOTIFY changed)

    Q_PROPERTY(QColor text READ text NOTIFY changed)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY changed)
    Q_PROPERTY(QColor textInverted READ textInverted NOTIFY changed)
    Q_PROPERTY(QColor accent READ accent NOTIFY changed)
    Q_PROPERTY(QColor accentText READ accentText NOTIFY changed)

    Q_PROPERTY(QColor buttonBg READ buttonBg NOTIFY changed)
    Q_PROPERTY(QColor buttonBgHover READ buttonBgHover NOTIFY changed)
    Q_PROPERTY(QColor buttonBgPressed READ buttonBgPressed NOTIFY changed)
    Q_PROPERTY(QColor buttonBorder READ buttonBorder NOTIFY changed)
    Q_PROPERTY(QColor buttonText READ buttonText NOTIFY changed)

    Q_PROPERTY(QColor inputBg READ inputBg NOTIFY changed)
    Q_PROPERTY(QColor inputBorder READ inputBorder NOTIFY changed)
    Q_PROPERTY(QColor inputText READ inputText NOTIFY changed)

    Q_PROPERTY(QColor scrollTrack READ scrollTrack NOTIFY changed)
    Q_PROPERTY(QColor scrollThumb READ scrollThumb NOTIFY changed)

    Q_PROPERTY(QColor hpFill READ hpFill NOTIFY changed)
    Q_PROPERTY(QColor spFill READ spFill NOTIFY changed)
    Q_PROPERTY(QColor expFill READ expFill NOTIFY changed)

    Q_PROPERTY(QColor chatSystem READ chatSystem NOTIFY changed)
    Q_PROPERTY(QColor chatWhisper READ chatWhisper NOTIFY changed)
    Q_PROPERTY(QColor chatParty READ chatParty NOTIFY changed)
    Q_PROPERTY(QColor chatGuild READ chatGuild NOTIFY changed)

public:
    enum class Mode { Light, Dark, Midnight, Slate, Forest, Parchment };

    explicit QtUiTheme(QObject* parent = nullptr);

    QString mode() const;
    Mode modeEnum() const { return m_mode; }
    void setMode(Mode mode);
    void setModeByName(const QString& name);
    static bool IsKnownModeName(const QString& name);

    QColor background() const    { return m_p.background; }
    QColor surface() const       { return m_p.surface; }
    QColor surfaceAlt() const    { return m_p.surfaceAlt; }
    QColor overlayDim() const    { return m_p.overlayDim; }
    QColor border() const        { return m_p.border; }
    QColor borderStrong() const  { return m_p.borderStrong; }

    QColor text() const          { return m_p.text; }
    QColor textMuted() const     { return m_p.textMuted; }
    QColor textInverted() const  { return m_p.textInverted; }
    QColor accent() const        { return m_p.accent; }
    QColor accentText() const    { return m_p.accentText; }

    QColor buttonBg() const      { return m_p.buttonBg; }
    QColor buttonBgHover() const { return m_p.buttonBgHover; }
    QColor buttonBgPressed() const { return m_p.buttonBgPressed; }
    QColor buttonBorder() const  { return m_p.buttonBorder; }
    QColor buttonText() const    { return m_p.buttonText; }

    QColor inputBg() const       { return m_p.inputBg; }
    QColor inputBorder() const   { return m_p.inputBorder; }
    QColor inputText() const     { return m_p.inputText; }

    QColor scrollTrack() const   { return m_p.scrollTrack; }
    QColor scrollThumb() const   { return m_p.scrollThumb; }

    QColor hpFill() const        { return m_p.hpFill; }
    QColor spFill() const        { return m_p.spFill; }
    QColor expFill() const       { return m_p.expFill; }

    QColor chatSystem() const    { return m_p.chatSystem; }
    QColor chatWhisper() const   { return m_p.chatWhisper; }
    QColor chatParty() const     { return m_p.chatParty; }
    QColor chatGuild() const     { return m_p.chatGuild; }

signals:
    void changed();

private:
    struct Palette {
        QColor background;
        QColor surface;
        QColor surfaceAlt;
        QColor overlayDim;
        QColor border;
        QColor borderStrong;
        QColor text;
        QColor textMuted;
        QColor textInverted;
        QColor accent;
        QColor accentText;
        QColor buttonBg;
        QColor buttonBgHover;
        QColor buttonBgPressed;
        QColor buttonBorder;
        QColor buttonText;
        QColor inputBg;
        QColor inputBorder;
        QColor inputText;
        QColor scrollTrack;
        QColor scrollThumb;
        QColor hpFill;
        QColor spFill;
        QColor expFill;
        QColor chatSystem;
        QColor chatWhisper;
        QColor chatParty;
        QColor chatGuild;
    };

    static Palette LightPalette();
    static Palette DarkPalette();
    static Palette MidnightPalette();
    static Palette SlatePalette();
    static Palette ForestPalette();
    static Palette ParchmentPalette();
    void applyPalette();

    Mode m_mode = Mode::Dark;
    Palette m_p;
};
