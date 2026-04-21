#pragma once

#include <string>
#include <vector>

namespace ui_skin {

enum class SkinEra {
    Unknown = 0,
    PreRenewal,
    Renewal,
};

std::string CanonicalizeSkinName(std::string value);
std::string GetConfiguredUiSkinName();
std::vector<std::string> EnumerateAvailableSkins();
std::vector<std::string> BuildUiAssetCandidates(const char* fileName);
std::string TryResolveUiAssetPath(const char* fileName);
std::string ResolveUiAssetPath(const char* fileName);
std::vector<std::string> BuildWallpaperCandidates(const std::string& requestedWallpaper);
std::vector<std::string> BuildUiButtonSoundCandidates();

SkinEra DetectSkinEra(const std::string& skinName);
const char* SkinEraLabel(SkinEra era);
std::string FormatSkinDisplayName(const std::string& skinName);
void InvalidateSkinCache();

} // namespace ui_skin
