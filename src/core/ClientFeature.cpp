#include "ClientFeature.h"

#include "SettingsIni.h"

#include <array>
#include <atomic>
#include <cstddef>

namespace {

constexpr std::size_t kFeatureCount = static_cast<std::size_t>(ClientFeature::Count);

struct FeatureDescriptor {
    const char* name;
    bool defaultEnabled;
};

constexpr std::array<FeatureDescriptor, kFeatureCount> kFeatureDescriptors = { {
    { "Party",            true  },
    { "Cart",             true  },
    { "Guild",            true  },
    { "GuildStorage",     true  },
    { "MailLegacy",       true  },
    { "Pet",              true  },
    { "Homunculus",       true  },
    { "Mercenary",        true  },
    { "Friends",          true  },
    { "ItemScript",       true  },
    { "SkillScript",      true  },
    { "SkillEffects",     true  },
    { "Emotion",          true  },
    { "WorldMap",         true  },
    { "Book",             true  },
    { "RespawnMenu",      true  },
    { "JobsFirstClass",   true  },
    { "JobsSecondClass",  true  },
    { "JobsTranscendent", true  },
    { "JobsExpanded",     true  },
    { "JobsThirdClass",   false },
    { "JobsDoram",        false },
    { "JobsBaby",         true  },
    { "SkillsPreRenewal", true  },
    { "SkillsRenewal",    false },
} };

std::array<std::atomic<int>, kFeatureCount> g_overrides = {}; // 0 = unset, 1 = force on, -1 = force off

int GetFeatureIndex(ClientFeature feature)
{
    const auto index = static_cast<std::size_t>(feature);
    return index < kFeatureCount ? static_cast<int>(index) : -1;
}

} // namespace

const char* GetFeatureName(ClientFeature feature)
{
    const int index = GetFeatureIndex(feature);
    if (index < 0) {
        return "";
    }
    return kFeatureDescriptors[index].name;
}

bool IsFeatureEnabled(ClientFeature feature)
{
    const int index = GetFeatureIndex(feature);
    if (index < 0) {
        return false;
    }
    const int override = g_overrides[index].load(std::memory_order_relaxed);
    if (override > 0) {
        return true;
    }
    if (override < 0) {
        return false;
    }
    return kFeatureDescriptors[index].defaultEnabled;
}

void SetFeatureEnabled(ClientFeature feature, bool enabled)
{
    const int index = GetFeatureIndex(feature);
    if (index < 0) {
        return;
    }
    g_overrides[index].store(enabled ? 1 : -1, std::memory_order_relaxed);
}

void LoadFeatureOverridesFromIni()
{
    for (std::size_t index = 0; index < kFeatureCount; ++index) {
        int value = 0;
        if (TryLoadSettingsIniInt("Features", kFeatureDescriptors[index].name, &value)) {
            g_overrides[index].store(value != 0 ? 1 : -1, std::memory_order_relaxed);
        }
    }
}
