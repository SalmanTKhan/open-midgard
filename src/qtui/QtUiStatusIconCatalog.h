#pragma once

namespace qtui {

struct StatusIconCatalogEntry {
    int statusType;
    int skillId;
    const char* fallbackLabel;
    const char* legacyIconFileName;
};

constexpr char kStatusIconAngelus[] = "\xBE\xC8\xC1\xA9\xB7\xE7\xBD\xBA\x2E\x74\x67\x61";
constexpr char kStatusIconBlessing[] = "\xBA\xED\xB7\xB9\xBD\xCC\x2E\x74\x67\x61";
constexpr char kStatusIconIncreaseAgi[] = "\xB9\xCE\xC3\xB8\xBC\xBA\xC1\xF5\xB0\xA1\x2E\x74\x67\x61";
constexpr char kStatusIconImproveConcentration[] = "\xC1\xFD\xC1\xDF\xB7\xC2\xC7\xE2\xBB\xF3\x2E\x74\x67\x61";
constexpr char kStatusIconImpositioManus[] = "\xC0\xD3\xC6\xF7\xBD\xC3\xC6\xBC\xBF\xC0\xB8\xB6\xB4\xA9\xBD\xBA\x2E\x74\x67\x61";
constexpr char kStatusIconSuffragium[] = "\xBC\xF6\xC7\xC1\xB6\xF3\xB1\xE2\xBF\xF2\x2E\x74\x67\x61";
constexpr char kStatusIconAspersio[] = "\xBE\xC6\xBD\xBA\xC6\xE4\xB8\xA3\xBD\xC3\xBF\xC0\x2E\x74\x67\x61";
constexpr char kStatusIconKyrieEleison[] = "\xB1\xE2\xB8\xAE\xBF\xA1\xBF\xA4\xB7\xB9\xC0\xCC\xBC\xD5\x2E\x74\x67\x61";
constexpr char kStatusIconMagnificat[] = "\xB8\xB6\xB4\xCF\xC7\xC7\xC4\xB1\x2E\x74\x67\x61";
constexpr char kStatusIconGloria[] = "\xB1\xDB\xB7\xCE\xB8\xAE\xBE\xC6\x2E\x74\x67\x61";
constexpr char kStatusIconAdrenalineRush[] = "\xBE\xC6\xB5\xE5\xB7\xB9\xB3\xAF\xB8\xB0\xB7\xAF\xBD\xAC\x2E\x74\x67\x61";
constexpr char kStatusIconWeaponPerfection[] = "\xBF\xFE\xC6\xF9\xC6\xDB\xC6\xE5\xBC\xC7\x2E\x74\x67\x61";
constexpr char kStatusIconOverThrust[] = "\xBF\xC0\xB9\xF6\xC6\xAE\xB7\xAF\xBD\xBA\xC6\xAE\x2E\x74\x67\x61";
constexpr char kStatusIconEnergyCoat[] = "\xBF\xA1\xB3\xCA\xC1\xF6\xC4\xDA\xC6\xAE\x2E\x74\x67\x61";
constexpr char kStatusIconChemicalProtectionWeapon[] = "\xC4\xC9\xB9\xCC\xC4\xC3\xC7\xC1\xB7\xCE\xC5\xD8\xBC\xC7\x5B\xBF\xFE\xC6\xF9\x5D\x2E\x74\x67\x61";
constexpr char kStatusIconChemicalProtectionShield[] = "\xC4\xC9\xB9\xCC\xC4\xC3\xC7\xC1\xB7\xCE\xC5\xD8\xBC\xC7\x5B\xBD\xAF\xB5\xE5\x5D\x2E\x74\x67\x61";
constexpr char kStatusIconChemicalProtectionArmor[] = "\xC4\xC9\xB9\xCC\xC4\xC3\xC7\xC1\xB7\xCE\xC5\xD8\xBC\xC7\x5B\xBE\xC6\xB8\xD3\x5D\x2E\x74\x67\x61";
constexpr char kStatusIconChemicalProtectionHelm[] = "\xC4\xC9\xB9\xCC\xC4\xC3\xC7\xC1\xB7\xCE\xC5\xD8\xBC\xC7\x5B\xC7\xEF\xB8\xA7\x5D\x2E\x74\x67\x61";
constexpr char kStatusIconAutoGuard[] = "\xBF\xC0\xC5\xE4\xB0\xA1\xB5\xE5\x2E\x74\x67\x61";
constexpr char kStatusIconReflectShield[] = "\xB8\xAE\xC7\xC3\xB7\xBA\xC6\xAE\xBD\xAF\xB5\xE5\x2E\x74\x67\x61";
constexpr char kStatusIconSpearQuicken[] = "\xBD\xBA\xC7\xC7\xBE\xEE\xC4\xFB\xC5\xAB\x2E\x74\x67\x61";
constexpr char kStatusIconTwoHandQuicken[] = "\xC5\xF5\xC7\xDA\xB5\xE5\xC4\xFB\xC5\xAB\x2E\x74\x67\x61";

// statusType matches rathena's EFST_ enum (EFST_PROVOKE=0). Entries with
// legacyIconFileName==nullptr fall back to text-label rendering on the Qt
// overlay (QtUiSpikeBridge.cpp:639). Kept to pre-Renewal-era statuses; later
// EFST values are intentionally omitted to stay aligned with the 2008 client.
constexpr StatusIconCatalogEntry kStatusIconCatalog[] = {
    {0, 6, "Provoke", nullptr},
    {1, 8, "Endure", "endure.tga"},
    {2, 60, "Two-Hand Quicken", kStatusIconTwoHandQuicken},
    {3, 45, "Improve Concentration", kStatusIconImproveConcentration},
    {4, 135, "Hiding", nullptr},
    {5, 136, "Cloaking", nullptr},
    {6, 84, "Enchant Poison", nullptr},
    {7, 85, "Poison React", nullptr},
    {8, 79, "Quagmire", nullptr},
    {9, 33, "Angelus", kStatusIconAngelus},
    {10, 34, "Blessing", kStatusIconBlessing},
    {11, 35, "Signum Crucis", nullptr},
    {12, 29, "Increase Agi", kStatusIconIncreaseAgi},
    {13, 30, "Decrease Agi", nullptr},
    {14, 32, "Slow Poison", nullptr},
    {15, 66, "Impositio Manus", kStatusIconImpositioManus},
    {16, 67, "Suffragium", kStatusIconSuffragium},
    {17, 68, "Aspersio", kStatusIconAspersio},
    {18, 71, "B.S Sacramenti", nullptr},
    {19, 73, "Kyrie Eleison", kStatusIconKyrieEleison},
    {20, 74, "Magnificat", kStatusIconMagnificat},
    {21, 75, "Gloria", kStatusIconGloria},
    {22, 78, "Lex Aeterna", nullptr},
    {23, 111, "Adrenaline Rush", kStatusIconAdrenalineRush},
    {24, 112, "Weapon Perfection", kStatusIconWeaponPerfection},
    {25, 113, "Over Thrust", kStatusIconOverThrust},
    {26, 114, "Maximize Power", nullptr},
    {27, 63, "Peco Riding", nullptr},
    {28, 122, "Falcon", nullptr},
    {29, 130, "Trick Dead", nullptr},
    {30, 39, "Loud Voice", nullptr},
    {31, 157, "Energy Coat", kStatusIconEnergyCoat},
    {32, 0, "Broken Armor", nullptr},
    {33, 0, "Broken Weapon", nullptr},
    {34, 0, "Illusion", nullptr},
    {35, 0, "Weight 50%", nullptr},
    {36, 0, "Weight 90%", nullptr},
    {37, 0, "Speed Potion 1", nullptr},
    {38, 0, "Speed Potion 2", nullptr},
    {39, 0, "Speed Potion 3", nullptr},
    {41, 0, "Movement Speed Up", nullptr},
    {43, 50, "Auto Counter", nullptr},
    {44, 116, "Splasher", nullptr},
    {45, 121, "Ankle Snare", nullptr},
    {46, 0, "Post Delay", nullptr},
    {49, 132, "Magnum Break", nullptr},
    {54, 234, "Chemical Protection Weapon", kStatusIconChemicalProtectionWeapon},
    {55, 235, "Chemical Protection Shield", kStatusIconChemicalProtectionShield},
    {56, 236, "Chemical Protection Armor", kStatusIconChemicalProtectionArmor},
    {57, 237, "Chemical Protection Helm", kStatusIconChemicalProtectionHelm},
    {58, 249, "Auto Guard", kStatusIconAutoGuard},
    {59, 252, "Reflect Shield", kStatusIconReflectShield},
    {60, 250, "Devotion", nullptr},
    {61, 253, "Providence", nullptr},
    {62, 251, "Defender", nullptr},
    {63, 81, "Magic Rod", nullptr},
    {64, 0, "Endow", nullptr},
    {65, 254, "Auto Spell", nullptr},
    {68, 258, "Spear Quicken", kStatusIconSpearQuicken},
    {69, 305, "Bard / Dancer", nullptr},
    {70, 312, "Whistle", nullptr},
    {71, 313, "Assassin Cross of Sunset", nullptr},
    {72, 318, "Poem of Bragi", nullptr},
    {73, 319, "Apple of Idun", nullptr},
    {74, 314, "Humming", nullptr},
    {75, 315, "Don't Forget Me", nullptr},
    {76, 316, "Fortune's Kiss", nullptr},
    {77, 317, "Service for You", nullptr},
    {80, 304, "Eternal Chaos", nullptr},
    {81, 308, "Drum Battlefield", nullptr},
    {82, 309, "Ring of Nibelungen", nullptr},
    {83, 310, "Roki's Veil", nullptr},
    {84, 311, "Into the Abyss", nullptr},
    {85, 307, "Invulnerable Siegfried", nullptr},
    {87, 359, "Explosion Spirits", nullptr},
    {88, 360, "Steel Body", nullptr},
    {89, 271, "Asura Strike", nullptr},
    {90, 266, "Combo Finish", nullptr},
    {96, 470, "Stop", nullptr},
    {99, 366, "Power-Up", nullptr},
    {100, 367, "Agi-Up", nullptr},
    {103, 376, "Aura Blade", nullptr},
    {104, 377, "Parrying", nullptr},
    {105, 378, "Lord's Concentration", nullptr},
    {106, 379, "Tension Relax", nullptr},
    {107, 386, "Berserk", nullptr},
    {110, 361, "Assumptio", nullptr},
    {111, 404, "Basilica", nullptr},
    {112, 412, "Magic Power", nullptr},
    {113, 379, "EDP", nullptr},
    {114, 422, "True Sight", nullptr},
    {115, 423, "Wind Walk", nullptr},
    {116, 424, "Meltdown", nullptr},
    {117, 153, "Cart Boost", nullptr},
    {118, 442, "Chase Walk", nullptr},
    {119, 444, "Sword Reject", nullptr},
    {122, 363, "Bleeding", nullptr},
    {124, 405, "Memorize", nullptr},
    {125, 80, "Fog Wall", nullptr},
    {126, 79, "Spider Web", nullptr},
    {128, 376, "Auto Berserk", nullptr},
    {129, 0, "Run", nullptr},
    {138, 0, "Soul Link", nullptr},
};

inline const StatusIconCatalogEntry* FindStatusIconCatalogEntry(int statusType)
{
    for (const StatusIconCatalogEntry& entry : kStatusIconCatalog) {
        if (entry.statusType == statusType) {
            return &entry;
        }
    }
    return nullptr;
}

} // namespace qtui