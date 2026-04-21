#pragma once

enum class ClientFeature {
    Party = 0,
    Cart,
    Guild,
    GuildStorage,
    MailLegacy,
    Pet,
    Homunculus,
    Mercenary,
    Friends,
    ItemScript,
    SkillScript,
    SkillEffects,
    Emotion,
    WorldMap,
    Book,
    RespawnMenu,
    JobsFirstClass,
    JobsSecondClass,
    JobsTranscendent,
    JobsExpanded,
    JobsThirdClass,
    JobsDoram,
    JobsBaby,
    SkillsPreRenewal,
    SkillsRenewal,
    Count
};

bool IsFeatureEnabled(ClientFeature feature);
void SetFeatureEnabled(ClientFeature feature, bool enabled);
void LoadFeatureOverridesFromIni();
const char* GetFeatureName(ClientFeature feature);
