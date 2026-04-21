#include "UiSkin.h"

#include "core/SettingsIni.h"
#include "core/File.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>

namespace ui_skin {
namespace {

constexpr char kUiSettingsSection[] = "UI";
constexpr char kUiSkinKey[] = "Skin";
constexpr char kDefaultUiSkinName[] = "default";
// CP949 bytes for "유저인터페이스" (Korean "User Interface").
constexpr const char* kUiKorSegment = "\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA";
constexpr const char* kUiKorPrefix =
    "texture\\"
    "\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA"
    "\\";

std::string ToLowerAscii(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string NormalizeSlash(std::string value)
{
    std::replace(value.begin(), value.end(), '/', '\\');
    return value;
}

std::string TrimAscii(std::string value)
{
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

void AddUniqueCandidate(std::vector<std::string>& out, const std::string& raw)
{
    if (raw.empty()) {
        return;
    }

    const std::string normalized = NormalizeSlash(raw);
    const std::string lowered = ToLowerAscii(normalized);
    for (const std::string& existing : out) {
        if (ToLowerAscii(existing) == lowered) {
            return;
        }
    }
    out.push_back(normalized);
}

void AddUniqueValue(std::vector<std::string>& out, std::string value)
{
    const std::string lowered = ToLowerAscii(value);
    for (const std::string& existing : out) {
        if (ToLowerAscii(existing) == lowered) {
            return;
        }
    }
    out.push_back(std::move(value));
}

std::vector<std::filesystem::path> GetSkinRoots()
{
    std::vector<std::filesystem::path> out;
    const std::filesystem::path baseDir = GetOpenMidgardIniPath().parent_path();
    if (baseDir.empty()) {
        return out;
    }

    out.push_back(baseDir / "skin");
    out.push_back(baseDir / "skins");
    return out;
}

void AppendSkinPrefixesForRoot(std::vector<std::string>& out, const std::string& rootName, const std::string& skinName, SkinEra era)
{
    if (skinName.empty()) {
        return;
    }

    const std::string base = rootName + "\\" + skinName + "\\";
    AddUniqueCandidate(out, base);
    AddUniqueCandidate(out, base + "basic_interface\\");
    AddUniqueCandidate(out, base + "login_interface\\");
    AddUniqueCandidate(out, base + "interface\\");
    AddUniqueCandidate(out, base + "illust\\");

    if (era == SkinEra::Renewal) {
        const std::string korBase = base + "texture\\" + kUiKorSegment + "\\";
        AddUniqueCandidate(out, korBase);
        AddUniqueCandidate(out, korBase + "basic_interface\\");
        AddUniqueCandidate(out, korBase + "login_interface\\");
        AddUniqueCandidate(out, korBase + "renewal_interface\\");
        AddUniqueCandidate(out, base + "renewal_interface\\");
        AddUniqueCandidate(out, base + "texture\\interface\\");
        AddUniqueCandidate(out, base + "texture\\interface\\basic_interface\\");
        AddUniqueCandidate(out, base + "texture\\login_interface\\");
    }
}

void AppendSkinBasePrefixesForRoot(std::vector<std::string>& out, const std::string& rootName, const std::string& skinName)
{
    if (skinName.empty()) {
        return;
    }

    AddUniqueCandidate(out, rootName + "\\" + skinName + "\\");
}

} // namespace

std::string CanonicalizeSkinName(std::string value)
{
    value = NormalizeSlash(TrimAscii(std::move(value)));
    while (!value.empty() && value.front() == '\\') {
        value.erase(value.begin());
    }
    while (!value.empty() && value.back() == '\\') {
        value.pop_back();
    }

    const std::string lowered = ToLowerAscii(value);
    if (lowered.rfind("skin\\", 0) == 0 && value.size() > 5) {
        value = value.substr(5);
    } else if (lowered.rfind("skins\\", 0) == 0 && value.size() > 6) {
        value = value.substr(6);
    }

    return value.empty() ? std::string(kDefaultUiSkinName) : value;
}

std::string GetConfiguredUiSkinName()
{
    return CanonicalizeSkinName(LoadSettingsIniString(kUiSettingsSection, kUiSkinKey, kDefaultUiSkinName));
}

std::vector<std::string> EnumerateAvailableSkins()
{
    std::vector<std::string> out;
    AddUniqueValue(out, std::string(kDefaultUiSkinName));

    for (const std::filesystem::path& root : GetSkinRoots()) {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) {
            continue;
        }

        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(root, ec)) {
            if (ec || !entry.is_directory()) {
                continue;
            }

            const std::string skinName = CanonicalizeSkinName(entry.path().filename().string());
            if (!skinName.empty()) {
                AddUniqueValue(out, skinName);
            }
        }
    }

    return out;
}

namespace {

std::string GetConfiguredUiSkinName()
{
    return ui_skin::GetConfiguredUiSkinName();
}

void AppendSkinAssetPrefixes(std::vector<std::string>& out, const std::string& skinName)
{
    if (skinName.empty()) {
        return;
    }

    const SkinEra era = ui_skin::DetectSkinEra(skinName);
    AppendSkinPrefixesForRoot(out, "skin", skinName, era);
    AppendSkinPrefixesForRoot(out, "skins", skinName, era);
}

void AppendSkinBasePrefixes(std::vector<std::string>& out, const std::string& skinName)
{
    if (skinName.empty()) {
        return;
    }

    AppendSkinBasePrefixesForRoot(out, "skin", skinName);
    AppendSkinBasePrefixesForRoot(out, "skins", skinName);
}

void AppendActiveSkinAssetPrefixes(std::vector<std::string>& out)
{
    const std::string configuredSkin = GetConfiguredUiSkinName();
    AppendSkinAssetPrefixes(out, configuredSkin);
    if (ToLowerAscii(configuredSkin) != kDefaultUiSkinName) {
        AppendSkinAssetPrefixes(out, kDefaultUiSkinName);
    }
}

void AppendActiveSkinBasePrefixes(std::vector<std::string>& out)
{
    const std::string configuredSkin = GetConfiguredUiSkinName();
    AppendSkinBasePrefixes(out, configuredSkin);
    if (ToLowerAscii(configuredSkin) != kDefaultUiSkinName) {
        AppendSkinBasePrefixes(out, kDefaultUiSkinName);
    }
}

} // namespace

std::vector<std::string> BuildUiAssetCandidates(const char* fileName)
{
    std::vector<std::string> out;
    if (!fileName || !*fileName) {
        return out;
    }

    static const char* pathPrefixes[] = {
        "",
        "texture\\",
        "texture\\interface\\",
        "texture\\interface\\basic_interface\\",
        "texture\\login_interface\\",
        "data\\",
        "data\\texture\\",
        "data\\texture\\interface\\",
        "data\\texture\\interface\\basic_interface\\",
        "data\\texture\\login_interface\\",
        kUiKorPrefix,
        "texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\basic_interface\\",
        "texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\login_interface\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\basic_interface\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\login_interface\\",
        nullptr
    };

    std::string base = NormalizeSlash(fileName);
    AddUniqueCandidate(out, base);

    std::string filenameOnly = base;
    const size_t slashPos = filenameOnly.find_last_of('\\');
    if (slashPos != std::string::npos && slashPos + 1 < filenameOnly.size()) {
        filenameOnly = filenameOnly.substr(slashPos + 1);
    }

    std::vector<std::string> skinPrefixes;
    AppendActiveSkinAssetPrefixes(skinPrefixes);
    for (const std::string& prefix : skinPrefixes) {
        AddUniqueCandidate(out, prefix + filenameOnly);
    }

    for (int index = 0; pathPrefixes[index]; ++index) {
        AddUniqueCandidate(out, std::string(pathPrefixes[index]) + filenameOnly);
    }

    return out;
}

std::string TryResolveUiAssetPath(const char* fileName)
{
    if (!fileName || !*fileName) {
        return {};
    }

    for (const std::string& candidate : BuildUiAssetCandidates(fileName)) {
        if (g_fileMgr.IsDataExist(candidate.c_str())) {
            return candidate;
        }
    }

    return {};
}

std::string ResolveUiAssetPath(const char* fileName)
{
    const std::string resolved = TryResolveUiAssetPath(fileName);
    if (!resolved.empty()) {
        return resolved;
    }
    return NormalizeSlash(fileName ? fileName : "");
}

std::vector<std::string> BuildWallpaperCandidates(const std::string& requestedWallpaper)
{
    std::vector<std::string> out;

    static const char* directDefaults[] = {
        "ad_title.png",
        "ad_title.jpg",
        "rag_title.jpg",
        "title.bmp",
        "title.jpg",
        "login_background.jpg",
        "login_background.bmp",
        nullptr
    };

    static const char* pathPrefixes[] = {
        "",
        "data\\",
        "data\\texture\\",
        "data\\texture\\interface\\",
        "data\\texture\\interface\\basic_interface\\",
        "data\\texture\\login_interface\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\basic_interface\\",
        "data\\texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\login_interface\\",
        "texture\\",
        "texture\\interface\\",
        "texture\\interface\\basic_interface\\",
        "texture\\login_interface\\",
        "ui\\",
        kUiKorPrefix,
        "texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\basic_interface\\",
        "texture\\\xC0\xAF\xC0\xFA\xC0\xCE\xC5\xCD\xC6\xE4\xC0\xCC\xBD\xBA\\login_interface\\",
        nullptr
    };

    std::vector<std::string> baseNames;
    if (!requestedWallpaper.empty()) {
        baseNames.push_back(requestedWallpaper);
    }
    for (int index = 0; directDefaults[index]; ++index) {
        baseNames.push_back(directDefaults[index]);
    }

    std::vector<std::string> skinPrefixes;
    AppendActiveSkinAssetPrefixes(skinPrefixes);

    for (const std::string& baseRaw : baseNames) {
        std::string base = NormalizeSlash(baseRaw);
        if (base.empty()) {
            continue;
        }

        std::string filenameOnly = base;
        const size_t slashPos = filenameOnly.find_last_of('\\');
        if (slashPos != std::string::npos && slashPos + 1 < filenameOnly.size()) {
            filenameOnly = filenameOnly.substr(slashPos + 1);
        }

        const bool hasExtension = filenameOnly.find('.') != std::string::npos;
        std::vector<std::string> nameForms;
        nameForms.push_back(base);
        if (filenameOnly != base) {
            nameForms.push_back(filenameOnly);
        }
        if (!hasExtension) {
            nameForms.push_back(filenameOnly + ".bmp");
            nameForms.push_back(filenameOnly + ".jpg");
            nameForms.push_back(filenameOnly + ".png");
            nameForms.push_back(filenameOnly + ".tga");
        }

        for (const std::string& nameForm : nameForms) {
            for (const std::string& prefix : skinPrefixes) {
                AddUniqueCandidate(out, prefix + nameForm);
            }
            for (int index = 0; pathPrefixes[index]; ++index) {
                AddUniqueCandidate(out, std::string(pathPrefixes[index]) + nameForm);
            }
        }
    }

    return out;
}

std::vector<std::string> BuildUiButtonSoundCandidates()
{
    std::vector<std::string> out;

    static const std::array<const char*, 7> kSoundNames = {
        "\xB9\xF6\xC6\xB0\xBC\xD2\xB8\xAE.wav",
        "click.wav",
        "button.wav",
        "btnok.wav",
        "btn_ok.wav",
        "ok.wav",
        "enter.wav",
    };

    std::vector<std::string> skinBasePrefixes;
    AppendActiveSkinBasePrefixes(skinBasePrefixes);
    for (const std::string& prefix : skinBasePrefixes) {
        for (const char* soundName : kSoundNames) {
            AddUniqueCandidate(out, prefix + "wav\\" + soundName);
            AddUniqueCandidate(out, prefix + soundName);
        }
    }

    for (const char* soundName : kSoundNames) {
        AddUniqueCandidate(out, std::string("wav\\") + soundName);
        AddUniqueCandidate(out, std::string("data\\wav\\") + soundName);
    }

    return out;
}

namespace {

std::mutex g_eraCacheMutex;
std::map<std::string, SkinEra> g_eraCache;

bool TryParseSkinCfgEra(const std::filesystem::path& cfgPath, SkinEra* outEra)
{
    std::ifstream stream(cfgPath, std::ios::in | std::ios::binary);
    if (!stream.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const auto hashPos = line.find('#');
        if (hashPos != std::string::npos) {
            line.erase(hashPos);
        }
        const std::string trimmed = TrimAscii(line);
        if (trimmed.empty()) {
            continue;
        }

        const auto eqPos = trimmed.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }
        std::string key = ToLowerAscii(TrimAscii(trimmed.substr(0, eqPos)));
        std::string value = ToLowerAscii(TrimAscii(trimmed.substr(eqPos + 1)));
        if (key != "type") {
            continue;
        }

        if (value == "renewal" || value == "re") {
            *outEra = SkinEra::Renewal;
            return true;
        }
        if (value == "pre-renewal" || value == "prerenewal" || value == "pre_renewal" || value == "classic") {
            *outEra = SkinEra::PreRenewal;
            return true;
        }
    }
    return false;
}

bool AnySubpathExists(const std::filesystem::path& dir, std::initializer_list<const char*> relatives)
{
    std::error_code ec;
    for (const char* rel : relatives) {
        std::filesystem::path candidate = dir;
        for (const char* segment = rel; segment && *segment; ) {
            const char* sep = segment;
            while (*sep && *sep != '/' && *sep != '\\') {
                ++sep;
            }
            candidate /= std::string(segment, sep);
            if (*sep == '\0') {
                break;
            }
            segment = sep + 1;
        }
        if (std::filesystem::exists(candidate, ec)) {
            return true;
        }
    }
    return false;
}

SkinEra DetectSkinEraUncached(const std::string& canonicalName)
{
    if (canonicalName.empty()) {
        return SkinEra::Unknown;
    }
    if (_stricmp(canonicalName.c_str(), kDefaultUiSkinName) == 0) {
        return SkinEra::Unknown;
    }

    for (const std::filesystem::path& root : GetSkinRoots()) {
        std::error_code ec;
        if (!std::filesystem::exists(root, ec) || !std::filesystem::is_directory(root, ec)) {
            continue;
        }

        const std::filesystem::path skinDir = root / canonicalName;
        if (!std::filesystem::exists(skinDir, ec) || !std::filesystem::is_directory(skinDir, ec)) {
            continue;
        }

        SkinEra cfgEra = SkinEra::Unknown;
        if (TryParseSkinCfgEra(skinDir / "skin.cfg", &cfgEra) && cfgEra != SkinEra::Unknown) {
            return cfgEra;
        }

        // ASCII-only subdir heuristics; CP949 "texture\유저인터페이스\..." paths are not
        // safe to construct via std::filesystem::path on MinGW (requires UTF-8 narrow).
        if (AnySubpathExists(skinDir, { "renewal_interface" })) {
            return SkinEra::Renewal;
        }

        if (AnySubpathExists(skinDir, {
                "basic_interface",
                "interface",
                "login_interface",
                "illust",
            })) {
            return SkinEra::PreRenewal;
        }
    }

    const std::string lowered = ToLowerAscii(canonicalName);
    if (lowered.find("renewal") != std::string::npos && lowered.find("pre") == std::string::npos) {
        return SkinEra::Renewal;
    }
    if (lowered.find("classic") != std::string::npos
        || lowered.find("pre-renewal") != std::string::npos
        || lowered.find("prerenewal") != std::string::npos) {
        return SkinEra::PreRenewal;
    }
    return SkinEra::Unknown;
}

} // namespace

SkinEra DetectSkinEra(const std::string& skinName)
{
    const std::string canonical = CanonicalizeSkinName(skinName);
    const std::string key = ToLowerAscii(canonical);

    {
        std::lock_guard<std::mutex> lock(g_eraCacheMutex);
        auto it = g_eraCache.find(key);
        if (it != g_eraCache.end()) {
            return it->second;
        }
    }

    const SkinEra era = DetectSkinEraUncached(canonical);

    std::lock_guard<std::mutex> lock(g_eraCacheMutex);
    g_eraCache[key] = era;
    return era;
}

const char* SkinEraLabel(SkinEra era)
{
    switch (era) {
    case SkinEra::PreRenewal: return "Pre-Renewal";
    case SkinEra::Renewal:    return "Renewal";
    case SkinEra::Unknown:
    default:                  return "";
    }
}

std::string FormatSkinDisplayName(const std::string& skinName)
{
    if (_stricmp(skinName.c_str(), kDefaultUiSkinName) == 0) {
        return "Default";
    }
    const char* tag = SkinEraLabel(DetectSkinEra(skinName));
    if (tag && *tag) {
        return skinName + " [" + tag + "]";
    }
    return skinName;
}

void InvalidateSkinCache()
{
    std::lock_guard<std::mutex> lock(g_eraCacheMutex);
    g_eraCache.clear();
}

} // namespace ui_skin
