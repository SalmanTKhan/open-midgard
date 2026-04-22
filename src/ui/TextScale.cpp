#include "TextScale.h"

#include "core/SettingsIni.h"

#include <algorithm>

namespace {

constexpr char kSection[] = "OptionWnd";
constexpr char kValue[] = "TextScalePercent";

int g_textScalePercent = -1;

int LoadFromSettings()
{
    return ClampTextScalePercent(LoadSettingsIniInt(kSection, kValue, kTextScaleDefaultPercent));
}

}

int ClampTextScalePercent(int percent)
{
    return (std::max)(kTextScaleMinPercent, (std::min)(kTextScaleMaxPercent, percent));
}

int GetConfiguredTextScalePercent()
{
    if (g_textScalePercent < 0) {
        g_textScalePercent = LoadFromSettings();
    }
    return g_textScalePercent;
}

float GetConfiguredTextScaleFactor()
{
    return static_cast<float>(GetConfiguredTextScalePercent()) / 100.0f;
}

void SetRuntimeTextScalePercent(int percent)
{
    g_textScalePercent = ClampTextScalePercent(percent);
}

void SaveConfiguredTextScalePercent(int percent)
{
    const int clamped = ClampTextScalePercent(percent);
    g_textScalePercent = clamped;
    SaveSettingsIniInt(kSection, kValue, clamped);
}
