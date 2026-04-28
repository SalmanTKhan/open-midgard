#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct SkillMetadata {
    int skillId = 0;
    std::string skillIdName;
    std::string displayName;
    std::vector<std::string> descriptionLines;
    std::vector<int> levelSpCosts;
};

// Per-job skill tree entry sourced from Ref/eAthena/db/skill_tree.txt.
// Format per line: jobId, skillId, maxLevel, jobLevelRequired, then up to 5
// (prereqSkillId, prereqSkillLv) pairs.
struct JobSkillEntry {
    int maxLevel = 0;
    int jobLevelRequired = 0;
    std::vector<std::pair<int /*prereqSkillId*/, int /*prereqLv*/>> prerequisites;
};

class CSkillMgr {
public:
    CSkillMgr();
    ~CSkillMgr();

    bool EnsureLoaded();
    const SkillMetadata* GetSkillMetadata(int skillId) const;
    const SkillMetadata* GetSkillMetadataByName(const std::string& skillIdName) const;
    std::string GetSkillIconPath(int skillId) const;
    const JobSkillEntry* GetJobSkillEntry(int jobId, int skillId) const;

private:
    bool LoadSkillMetadata();
    bool BuildSkillIdMapFromNameTable();
    void LoadSkillDisplayNames();
    void LoadSkillDescriptions();
    void LoadSkillLevelSpCosts();
    void LoadSkillTree();

    std::unordered_map<int, SkillMetadata> m_byId;
    std::unordered_map<std::string, int> m_idBySkillName;
    std::unordered_map<int, std::unordered_map<int, JobSkillEntry>> m_skillTreeByJob;
    bool m_loaded;
};

extern CSkillMgr g_skillMgr;
