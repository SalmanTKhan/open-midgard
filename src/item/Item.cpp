#include "Item.h"

#include "DebugLog.h"
#include "core/File.h"
#include "lua/LuaBridge.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <vector>

namespace {

constexpr const char* kAccessorySpriteRoot = "data\\sprite\\\xBE\xC7\xBC\xBC\xBB\xE7\xB8\xAE\\";
constexpr const char* kFemaleSex = "\xBF\xA9";
constexpr const char* kMaleSex = "\xB3\xB2";
constexpr const char* kAccessoryNameTableScript = "lua files\\datainfo\\accname.lub";
constexpr const char* kAccessoryNameTableName = "AccNameTable";
constexpr int kMaxAccessoryViewId = 8192;

struct AggregatedCardLabel {
	std::string label;
	int count = 0;
};

std::string AppendSlotSuffix(std::string baseName, int slotCount)
{
	if (baseName.empty() || slotCount <= 0) {
		return baseName;
	}

	baseName += " [";
	baseName += std::to_string(slotCount);
	baseName += "]";
	return baseName;
}

const char* GetRepeatedCardPrefixWord(int count)
{
	switch (count) {
	case 2:
		return "Double";
	case 3:
		return "Triple";
	case 4:
		return "Quadruple";
	default:
		return nullptr;
	}
}

void AppendCardPrefixLabel(std::vector<AggregatedCardLabel>& labels, std::string&& label)
{
	if (label.empty()) {
		return;
	}

	for (AggregatedCardLabel& existing : labels) {
		if (existing.label == label) {
			++existing.count;
			return;
		}
	}

	AggregatedCardLabel entry;
	entry.label = std::move(label);
	entry.count = 1;
	labels.push_back(std::move(entry));
}

std::string BuildAggregatedCardPrefixText(const std::vector<AggregatedCardLabel>& labels)
{
	std::string text;
	for (const AggregatedCardLabel& entry : labels) {
		if (entry.label.empty() || entry.count <= 0) {
			continue;
		}

		if (!text.empty()) {
			text += " ";
		}

		if (const char* repeatWord = GetRepeatedCardPrefixWord(entry.count)) {
			text += repeatWord;
			text += " ";
		}

		text += entry.label;
	}
	return text;
}

std::string TrimLine(std::string value)
{
	while (!value.empty() && (value.back() == '\r' || value.back() == '\n')) {
		value.pop_back();
	}
	return value;
}

std::string NormalizeDisplayToken(std::string value)
{
	std::replace(value.begin(), value.end(), '_', ' ');
	return value;
}

std::string ToLowerAscii(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

std::string TrimAsciiWhitespace(std::string value)
{
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
		value.erase(value.begin());
	}
	while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
		value.pop_back();
	}
	return value;
}

std::string MakeItemLookupKey(std::string value)
{
	return ToLowerAscii(TrimAsciiWhitespace(std::move(value)));
}

void RegisterItemLookupAlias(std::unordered_map<std::string, unsigned int>& lookup,
	unsigned int itemId,
	const std::string& value,
	bool normalizeDisplay)
{
	if (itemId == 0 || value.empty()) {
		return;
	}

	const std::string rawKey = MakeItemLookupKey(value);
	if (!rawKey.empty()) {
		lookup.emplace(rawKey, itemId);
	}

	if (normalizeDisplay) {
		const std::string normalizedKey = MakeItemLookupKey(NormalizeDisplayToken(value));
		if (!normalizedKey.empty()) {
			lookup.emplace(normalizedKey, itemId);
		}
	}
}

void RegisterItemResourceAlias(std::unordered_map<std::string, std::string>& lookup,
	const std::string& keySource,
	const std::string& resourceName)
{
	if (keySource.empty() || resourceName.empty()) {
		return;
	}

	const std::string rawKey = MakeItemLookupKey(keySource);
	if (!rawKey.empty()) {
		lookup.emplace(rawKey, resourceName);
	}

	const std::string normalizedKey = MakeItemLookupKey(NormalizeDisplayToken(keySource));
	if (!normalizedKey.empty()) {
		lookup.emplace(normalizedKey, resourceName);
	}
}

void RegisterStringAlias(std::unordered_map<std::string, std::string>& lookup,
	const std::string& keySource,
	const std::string& value)
{
	if (keySource.empty() || value.empty()) {
		return;
	}

	const std::string rawKey = MakeItemLookupKey(keySource);
	if (!rawKey.empty()) {
		lookup.emplace(rawKey, value);
	}

	const std::string normalizedKey = MakeItemLookupKey(NormalizeDisplayToken(keySource));
	if (!normalizedKey.empty()) {
		lookup.emplace(normalizedKey, value);
	}
}

void RegisterIntegerAlias(std::unordered_map<std::string, int>& lookup,
	const std::string& keySource,
	int value)
{
	if (keySource.empty() || value < 0) {
		return;
	}

	const std::string rawKey = MakeItemLookupKey(keySource);
	if (!rawKey.empty()) {
		lookup.emplace(rawKey, value);
	}

	const std::string normalizedKey = MakeItemLookupKey(NormalizeDisplayToken(keySource));
	if (!normalizedKey.empty()) {
		lookup.emplace(normalizedKey, value);
	}
}

void RegisterLookupKey(std::unordered_set<std::string>& lookup, const std::string& keySource)
{
	if (keySource.empty()) {
		return;
	}

	const std::string rawKey = MakeItemLookupKey(keySource);
	if (!rawKey.empty()) {
		lookup.insert(rawKey);
	}

	const std::string normalizedKey = MakeItemLookupKey(NormalizeDisplayToken(keySource));
	if (!normalizedKey.empty()) {
		lookup.insert(normalizedKey);
	}
}

bool ParseHashPairLine(const std::string& line, unsigned int& outId, std::string& outValue)
{
	if (line.empty() || line[0] == '/' || line[0] == '#') {
		return false;
	}

	const size_t firstHash = line.find('#');
	if (firstHash == std::string::npos || firstHash == 0) {
		return false;
	}
	const size_t secondHash = line.find('#', firstHash + 1);
	if (secondHash == std::string::npos) {
		return false;
	}

	outId = static_cast<unsigned int>(std::strtoul(line.substr(0, firstHash).c_str(), nullptr, 10));
	if (outId == 0) {
		return false;
	}

	outValue = line.substr(firstHash + 1, secondHash - firstHash - 1);
	return true;
}

bool ParseLegacyItemNameLine(const std::string& line, std::string& outDisplayName, std::string& outResourceName)
{
	outDisplayName.clear();
	outResourceName.clear();
	if (line.empty() || line[0] == '/' || line[0] == '#') {
		return false;
	}

	const size_t firstHash = line.find('#');
	if (firstHash == std::string::npos || firstHash == 0) {
		return false;
	}
	const size_t secondHash = line.find('#', firstHash + 1);
	if (secondHash == std::string::npos || secondHash == firstHash + 1) {
		return false;
	}

	outDisplayName = TrimAsciiWhitespace(line.substr(0, firstHash));
	outResourceName = TrimAsciiWhitespace(line.substr(firstHash + 1, secondHash - firstHash - 1));
	return !outDisplayName.empty() && !outResourceName.empty();
}

bool ParseLegacySingleValueLine(const std::string& line, std::string& outValue)
{
	outValue.clear();
	if (line.empty() || line[0] == '/' || line[0] == '#') {
		return false;
	}

	const size_t firstHash = line.find('#');
	if (firstHash == std::string::npos || firstHash == 0) {
		return false;
	}

	outValue = TrimAsciiWhitespace(line.substr(0, firstHash));
	return !outValue.empty();
}

bool ParseHashIdLine(const std::string& line, unsigned int& outId)
{
	if (line.empty() || line[0] == '/' || line[0] == '#') {
		return false;
	}

	const size_t firstHash = line.find('#');
	if (firstHash == std::string::npos || firstHash == 0) {
		return false;
	}

	outId = static_cast<unsigned int>(std::strtoul(line.substr(0, firstHash).c_str(), nullptr, 10));
	return outId != 0;
}

bool ParseDescriptionBlockHeaderLine(const std::string& line, unsigned int& outId)
{
	outId = 0;
	if (line.empty() || line[0] == '/' || line[0] == '#') {
		return false;
	}

	const size_t firstHash = line.find('#');
	if (firstHash == std::string::npos || firstHash == 0) {
		return false;
	}

	for (size_t index = firstHash + 1; index < line.size(); ++index) {
		if (!std::isspace(static_cast<unsigned char>(line[index]))) {
			return false;
		}
	}

	outId = static_cast<unsigned int>(std::strtoul(line.substr(0, firstHash).c_str(), nullptr, 10));
	return outId != 0;
}

void AssignUnidentifiedDisplayName(ItemMetadata& metadata, std::string&& value)
{
	metadata.unidentifiedDisplayName = NormalizeDisplayToken(std::move(value));
}

void AssignIdentifiedDisplayName(ItemMetadata& metadata, std::string&& value)
{
	metadata.identifiedDisplayName = NormalizeDisplayToken(std::move(value));
}

void AssignResourceName(ItemMetadata& metadata, std::string&& value)
{
	metadata.unidentifiedResourceName = std::move(value);
}

void AssignIdentifiedResourceName(ItemMetadata& metadata, std::string&& value)
{
	metadata.identifiedResourceName = std::move(value);
}

void AssignCardIllustName(ItemMetadata& metadata, std::string&& value)
{
	metadata.cardIllustName = std::move(value);
}

void AssignSlotCount(ItemMetadata& metadata, int value)
{
	metadata.slotCount = (std::max)(0, value);
}

bool LoadTextFileFromGameData(const char* fileName, std::string& outText)
{
	outText.clear();
	if (!fileName || !*fileName) {
		return false;
	}

	std::vector<std::string> candidates;
	candidates.emplace_back(fileName);
	if (std::strchr(fileName, '\\') == nullptr && std::strchr(fileName, '/') == nullptr) {
		candidates.emplace_back(std::string("data\\") + fileName);
		candidates.emplace_back(std::string("data/") + fileName);
	}

	for (const std::string& candidate : candidates) {
		int size = 0;
		unsigned char* bytes = g_fileMgr.GetData(candidate.c_str(), &size);
		if (!bytes || size <= 0) {
			delete[] bytes;
			continue;
		}

		outText.assign(reinterpret_cast<const char*>(bytes), static_cast<size_t>(size));
		delete[] bytes;
		return true;
	}

	return false;
}

bool HasAccessorySpriteResourceForSex(const std::string& resourceName, const char* sexToken)
{
	if (resourceName.empty() || !sexToken || !*sexToken) {
		return false;
	}

	char actPath[512] = {};
	char sprPath[512] = {};
	const char* separator = resourceName.front() == '_' ? "" : "_";
	std::snprintf(actPath, sizeof(actPath), "%s%s\\%s%s%s.act", kAccessorySpriteRoot, sexToken, sexToken, separator, resourceName.c_str());
	std::snprintf(sprPath, sizeof(sprPath), "%s%s\\%s%s%s.spr", kAccessorySpriteRoot, sexToken, sexToken, separator, resourceName.c_str());
	return g_fileMgr.IsDataExist(actPath) && g_fileMgr.IsDataExist(sprPath);
}

bool HasAccessorySpriteResource(const std::string& resourceName)
{
	return HasAccessorySpriteResourceForSex(resourceName, kMaleSex)
		|| HasAccessorySpriteResourceForSex(resourceName, kFemaleSex);
}

} // namespace

CItemMgr g_ttemmgr;

void ITEM_INFO::SetItemId(unsigned int itemId)
{
	m_itemName = std::to_string(itemId);
}

unsigned int ITEM_INFO::GetItemId() const
{
	if (m_itemName.empty()) {
		return 0;
	}
	return static_cast<unsigned int>(std::strtoul(m_itemName.c_str(), nullptr, 10));
}

std::string ITEM_INFO::GetDisplayName() const
{
	const unsigned int itemId = GetItemId();
	if (itemId != 0) {
		return g_ttemmgr.GetDisplayName(itemId, m_isIdentified != 0);
	}
	if (const std::string displayName = g_ttemmgr.FindDisplayNameByName(m_itemName); !displayName.empty()) {
		return displayName;
	}
	return m_itemName;
}

std::string ITEM_INFO::GetEquipDisplayName() const
{
	const unsigned int itemId = GetItemId();
	if (itemId != 0) {
		return g_ttemmgr.GetEquipDisplayName(*this);
	}

	std::string text;
	if (m_refiningLevel > 0) {
		text += "+";
		text += std::to_string(m_refiningLevel);
		text += " ";
	}
	const std::string displayName = GetDisplayName();
	text += displayName.empty() ? m_itemName : displayName;
	return text;
}

std::string ITEM_INFO::GetDescription() const
{
	const unsigned int itemId = GetItemId();
	if (itemId != 0) {
		return g_ttemmgr.GetDescription(itemId);
	}
	return g_ttemmgr.FindDescriptionByName(m_itemName);
}

std::string ITEM_INFO::GetResourceName() const
{
	const unsigned int itemId = GetItemId();
	if (itemId != 0) {
		return g_ttemmgr.GetResourceName(itemId, m_isIdentified != 0);
	}
	if (!m_resourceName.empty()) {
		return m_resourceName;
	}
	return m_itemName;
}

int ITEM_INFO::GetSlotCount() const
{
	const unsigned int itemId = GetItemId();
	if (itemId != 0) {
		return g_ttemmgr.GetSlotCount(itemId);
	}
	return g_ttemmgr.FindSlotCountByName(m_itemName);
}

std::string ITEM_INFO::GetCardIllustName() const
{
	return g_ttemmgr.GetCardIllustName(GetItemId());
}

CItemMgr::CItemMgr()
	: m_loaded(false)
{
}

CItemMgr::~CItemMgr() = default;

void CItemMgr::EnsureLoaded()
{
	if (m_loaded) {
		return;
	}

	m_itemIdsByLookupKey.clear();
	m_displayNamesByLookupKey.clear();
	m_descriptionsByLookupKey.clear();
	m_resourceNamesByLookupKey.clear();
	m_slotCountsByLookupKey.clear();
	m_visibleHeadgearResourceNamesByViewId.clear();
	m_visibleHeadgearViewIdsByResourceName.clear();
	m_cardPrefixNames.clear();
	m_cardPostfixIds.clear();
	m_cardItemIds.clear();
	m_cardItemLookupKeys.clear();

	LoadDisplayTable();
	LoadResourceTable();
	LoadDescriptionTable();
	LoadSlotCountTable();
	LoadCardIllustTable();
	LoadCardPrefixTable();
	LoadCardPostfixTable();
	LoadCardItemTable();
	for (const auto& entry : m_metadata) {
		const unsigned int itemId = entry.first;
		const ItemMetadata& metadata = entry.second;
		RegisterItemLookupAlias(m_itemIdsByLookupKey, itemId, metadata.unidentifiedDisplayName, true);
		RegisterItemLookupAlias(m_itemIdsByLookupKey, itemId, metadata.identifiedDisplayName, true);
		RegisterItemLookupAlias(m_itemIdsByLookupKey, itemId, metadata.unidentifiedResourceName, false);
		RegisterItemLookupAlias(m_itemIdsByLookupKey, itemId, metadata.identifiedResourceName, false);
		RegisterStringAlias(m_displayNamesByLookupKey, metadata.unidentifiedDisplayName, metadata.unidentifiedDisplayName);
		RegisterStringAlias(m_displayNamesByLookupKey, metadata.identifiedDisplayName, metadata.identifiedDisplayName);
		RegisterStringAlias(m_descriptionsByLookupKey, metadata.unidentifiedDisplayName, metadata.description);
		RegisterStringAlias(m_descriptionsByLookupKey, metadata.identifiedDisplayName, metadata.description);
		RegisterItemResourceAlias(m_resourceNamesByLookupKey, metadata.unidentifiedDisplayName, metadata.unidentifiedResourceName);
		RegisterItemResourceAlias(m_resourceNamesByLookupKey, metadata.identifiedDisplayName, metadata.identifiedResourceName);
		RegisterIntegerAlias(m_slotCountsByLookupKey, metadata.unidentifiedDisplayName, metadata.slotCount);
		RegisterIntegerAlias(m_slotCountsByLookupKey, metadata.identifiedDisplayName, metadata.slotCount);
	}
	LoadLegacyItemNameTable();
	m_loaded = true;
}

const ItemMetadata* CItemMgr::GetMetadata(unsigned int itemId)
{
	EnsureLoaded();
	const auto it = m_metadata.find(itemId);
	return it != m_metadata.end() ? &it->second : nullptr;
}

std::string CItemMgr::GetDisplayName(unsigned int itemId, bool identified)
{
	if (const ItemMetadata* metadata = GetMetadata(itemId)) {
		const std::string& preferredName = identified
			? metadata->identifiedDisplayName
			: metadata->unidentifiedDisplayName;
		if (!preferredName.empty()) {
			return AppendSlotSuffix(preferredName, metadata->slotCount);
		}

		const std::string& fallbackName = identified
			? metadata->unidentifiedDisplayName
			: metadata->identifiedDisplayName;
		if (!fallbackName.empty()) {
			return AppendSlotSuffix(fallbackName, metadata->slotCount);
		}
	}
	return itemId != 0 ? std::to_string(itemId) : std::string();
}

std::string CItemMgr::GetEquipDisplayName(const ITEM_INFO& item)
{
	const unsigned int itemId = item.GetItemId();
	if (itemId == 0) {
		return std::string();
	}

	EnsureLoaded();

	std::string text;
	if (item.m_refiningLevel > 0) {
		text += "+";
		text += std::to_string(item.m_refiningLevel);
		text += " ";
	}

	std::vector<AggregatedCardLabel> prefixLabels;
	for (int slotIndex = 0; slotIndex < 4; ++slotIndex) {
		const unsigned int cardId = static_cast<unsigned int>(item.m_slot[slotIndex]);
		if (cardId == 0 || !IsCardItem(cardId) || IsPostfixCard(cardId)) {
			continue;
		}

		std::string prefix = GetCardPrefixName(cardId);
		if (prefix.empty()) {
			prefix = GetDisplayName(cardId, true);
		}
		if (prefix.empty()) {
			continue;
		}

		AppendCardPrefixLabel(prefixLabels, std::move(prefix));
	}

	const std::string prefixText = BuildAggregatedCardPrefixText(prefixLabels);
	if (!prefixText.empty()) {
		text += prefixText;
		text += " ";
	}

	text += GetDisplayName(itemId, item.m_isIdentified != 0);

	for (int slotIndex = 0; slotIndex < 4; ++slotIndex) {
		const unsigned int cardId = static_cast<unsigned int>(item.m_slot[slotIndex]);
		if (cardId == 0 || !IsCardItem(cardId) || !IsPostfixCard(cardId)) {
			continue;
		}

		std::string postfix = GetCardPrefixName(cardId);
		if (postfix.empty()) {
			postfix = GetDisplayName(cardId, true);
		}
		if (postfix.empty()) {
			continue;
		}

		text += " ";
		text += postfix;
	}

	return text;
}

std::string CItemMgr::GetDescription(unsigned int itemId)
{
	if (const ItemMetadata* metadata = GetMetadata(itemId)) {
		return metadata->description;
	}
	return std::string();
}

std::string CItemMgr::GetResourceName(unsigned int itemId, bool identified)
{
	if (const ItemMetadata* metadata = GetMetadata(itemId)) {
		const std::string& preferredName = identified
			? metadata->identifiedResourceName
			: metadata->unidentifiedResourceName;
		if (!preferredName.empty()) {
			return preferredName;
		}

		const std::string& fallbackName = identified
			? metadata->unidentifiedResourceName
			: metadata->identifiedResourceName;
		if (!fallbackName.empty()) {
			return fallbackName;
		}
	}
	return std::string();
}

int CItemMgr::GetSlotCount(unsigned int itemId)
{
	if (const ItemMetadata* metadata = GetMetadata(itemId)) {
		return metadata->slotCount;
	}
	return 0;
}

std::string CItemMgr::GetCardIllustName(unsigned int itemId)
{
	if (const ItemMetadata* metadata = GetMetadata(itemId)) {
		return metadata->cardIllustName;
	}
	return std::string();
}

unsigned int CItemMgr::FindItemIdByName(const std::string& name)
{
	EnsureLoaded();
	if (name.empty()) {
		return 0;
	}

	const auto exact = m_itemIdsByLookupKey.find(MakeItemLookupKey(name));
	if (exact != m_itemIdsByLookupKey.end()) {
		return exact->second;
	}

	const auto normalized = m_itemIdsByLookupKey.find(MakeItemLookupKey(NormalizeDisplayToken(name)));
	return normalized != m_itemIdsByLookupKey.end() ? normalized->second : 0;
}

std::string CItemMgr::FindDisplayNameByName(const std::string& name)
{
	EnsureLoaded();
	if (name.empty()) {
		return std::string();
	}

	const auto exact = m_displayNamesByLookupKey.find(MakeItemLookupKey(name));
	if (exact != m_displayNamesByLookupKey.end()) {
		return exact->second;
	}

	const auto normalized = m_displayNamesByLookupKey.find(MakeItemLookupKey(NormalizeDisplayToken(name)));
	return normalized != m_displayNamesByLookupKey.end() ? normalized->second : std::string();
}

std::string CItemMgr::FindDescriptionByName(const std::string& name)
{
	EnsureLoaded();
	if (name.empty()) {
		return std::string();
	}

	const auto exact = m_descriptionsByLookupKey.find(MakeItemLookupKey(name));
	if (exact != m_descriptionsByLookupKey.end()) {
		return exact->second;
	}

	const auto normalized = m_descriptionsByLookupKey.find(MakeItemLookupKey(NormalizeDisplayToken(name)));
	return normalized != m_descriptionsByLookupKey.end() ? normalized->second : std::string();
}

std::string CItemMgr::FindResourceNameByName(const std::string& name)
{
	EnsureLoaded();
	if (name.empty()) {
		return std::string();
	}

	const auto exact = m_resourceNamesByLookupKey.find(MakeItemLookupKey(name));
	if (exact != m_resourceNamesByLookupKey.end()) {
		return exact->second;
	}

	const auto normalized = m_resourceNamesByLookupKey.find(MakeItemLookupKey(NormalizeDisplayToken(name)));
	return normalized != m_resourceNamesByLookupKey.end() ? normalized->second : std::string();
}

int CItemMgr::FindSlotCountByName(const std::string& name)
{
	EnsureLoaded();
	if (name.empty()) {
		return 0;
	}

	const auto exact = m_slotCountsByLookupKey.find(MakeItemLookupKey(name));
	if (exact != m_slotCountsByLookupKey.end()) {
		return exact->second;
	}

	const auto normalized = m_slotCountsByLookupKey.find(MakeItemLookupKey(NormalizeDisplayToken(name)));
	return normalized != m_slotCountsByLookupKey.end() ? normalized->second : 0;
}

int CItemMgr::GetVisibleHeadgearViewId(unsigned int itemId)
{
	EnsureLoaded();
	const ItemMetadata* metadata = GetMetadata(itemId);
	if (!metadata) {
		return 0;
	}

	const std::string& resourceName = !metadata->identifiedResourceName.empty()
		? metadata->identifiedResourceName
		: metadata->unidentifiedResourceName;
	if (resourceName.empty()) {
		return 0;
	}

	const auto cached = m_visibleHeadgearViewIdsByResourceName.find(resourceName);
	if (cached != m_visibleHeadgearViewIdsByResourceName.end()) {
		return cached->second;
	}

	int resolvedViewId = 0;
	std::string mappedResourceName;
	if (g_buabridge.LoadRagnarokScriptOnce(kAccessoryNameTableScript)) {
		for (int viewId = 1; viewId <= kMaxAccessoryViewId; ++viewId) {
			mappedResourceName.clear();
			if (!g_buabridge.GetGlobalTableStringByIntegerKey(kAccessoryNameTableName, viewId, &mappedResourceName)
				|| mappedResourceName != resourceName) {
				continue;
			}

			if (resolvedViewId != 0 && resolvedViewId != viewId) {
				DbgLog("[Item] ambiguous headgear view mapping item=%u resource='%s' views=%d,%d\n",
					itemId,
					resourceName.c_str(),
					resolvedViewId,
					viewId);
				resolvedViewId = 0;
				break;
			}

			resolvedViewId = viewId;
		}
	}

	m_visibleHeadgearViewIdsByResourceName[resourceName] = resolvedViewId;
	return resolvedViewId;
}

std::string CItemMgr::GetVisibleHeadgearResourceNameByViewId(int viewId)
{
	EnsureLoaded();
	if (viewId <= 0) {
		return std::string();
	}

	const auto cached = m_visibleHeadgearResourceNamesByViewId.find(viewId);
	if (cached != m_visibleHeadgearResourceNamesByViewId.end()) {
		return cached->second;
	}

	std::string resolved;
	if (g_buabridge.LoadRagnarokScriptOnce(kAccessoryNameTableScript)
		&& g_buabridge.GetGlobalTableStringByIntegerKey(kAccessoryNameTableName, viewId, &resolved)) {
		if (!HasAccessorySpriteResource(resolved)) {
			DbgLog("[Item] accessory sprite missing view=%d resource='%s'\n", viewId, resolved.c_str());
		}
	} else {
		resolved.clear();
	}

	m_visibleHeadgearResourceNamesByViewId[viewId] = resolved;
	return resolved;
}

std::string CItemMgr::GetCardPrefixName(unsigned int itemId)
{
	EnsureLoaded();
	const auto it = m_cardPrefixNames.find(itemId);
	return it != m_cardPrefixNames.end() ? it->second : std::string();
}

bool CItemMgr::IsCardItem(unsigned int itemId)
{
	EnsureLoaded();
	return m_cardItemIds.find(itemId) != m_cardItemIds.end();
}

bool CItemMgr::IsCardItemName(const std::string& name)
{
	EnsureLoaded();
	return m_cardItemLookupKeys.find(MakeItemLookupKey(name)) != m_cardItemLookupKeys.end();
}

bool CItemMgr::IsPostfixCard(unsigned int itemId)
{
	EnsureLoaded();
	return m_cardPostfixIds.find(itemId) != m_cardPostfixIds.end();
}

bool CItemMgr::LoadDisplayTable()
{
	const bool loadedUnidentified = ParsePairTable("num2itemdisplaynametable.txt", AssignUnidentifiedDisplayName);
	const bool loadedIdentified = ParsePairTable("idnum2itemdisplaynametable.txt", AssignIdentifiedDisplayName);
	return loadedUnidentified || loadedIdentified || LoadLegacyDisplayTable();
}

bool CItemMgr::LoadResourceTable()
{
	const bool loadedUnidentified = ParsePairTable("num2itemresnametable.txt", AssignResourceName);
	const bool loadedIdentified = ParsePairTable("idnum2itemresnametable.txt", AssignIdentifiedResourceName);
	return loadedUnidentified || loadedIdentified;
}

bool CItemMgr::LoadDescriptionTable()
{
	const bool loadedUnidentified = ParseDescriptionBlocks("num2itemdesctable.txt");
	const bool loadedIdentified = ParseDescriptionBlocks("idnum2itemdesctable.txt");
	return loadedUnidentified || loadedIdentified || LoadLegacyDescriptionTable();
}

bool CItemMgr::LoadSlotCountTable()
{
	return ParseIntegerPairTable("itemSlotCountTable.txt", AssignSlotCount) || LoadLegacySlotCountTable();
}

bool CItemMgr::LoadCardIllustTable()
{
	return ParsePairTable("Num2CardIllustNameTable.txt", AssignCardIllustName);
}

bool CItemMgr::LoadCardPrefixTable()
{
	std::string text;
	if (!LoadTextFileFromGameData("cardprefixnametable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	while (std::getline(input, line)) {
		unsigned int itemId = 0;
		std::string value;
		if (!ParseHashPairLine(TrimLine(line), itemId, value)) {
			continue;
		}
		m_cardPrefixNames[itemId] = NormalizeDisplayToken(std::move(value));
	}

	return true;
}

bool CItemMgr::LoadCardPostfixTable()
{
	return ParseIdSetTable("cardpostfixnametable.txt", m_cardPostfixIds);
}

bool CItemMgr::LoadCardItemTable()
{
	if (ParseIdSetTable("carditemnametable.txt", m_cardItemIds)) {
		return true;
	}

	std::string text;
	if (!LoadTextFileFromGameData("carditemnametable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	bool loadedAny = false;
	while (std::getline(input, line)) {
		std::string itemName;
		if (!ParseLegacySingleValueLine(TrimLine(line), itemName)) {
			continue;
		}
		RegisterLookupKey(m_cardItemLookupKeys, itemName);
		loadedAny = true;
	}
	return loadedAny;
}

bool CItemMgr::LoadLegacyDisplayTable()
{
	std::string text;
	if (!LoadTextFileFromGameData("itemdisplaynametable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	bool loadedAny = false;
	while (std::getline(input, line)) {
		std::string keyName;
		std::string displayName;
		unsigned int unusedId = 0;
		if (!ParseHashPairLine(TrimLine(line), unusedId, displayName)) {
			const std::string trimmed = TrimLine(line);
			const size_t firstHash = trimmed.find('#');
			const size_t secondHash = firstHash == std::string::npos ? std::string::npos : trimmed.find('#', firstHash + 1);
			if (firstHash == std::string::npos || secondHash == std::string::npos || firstHash == 0) {
				continue;
			}
			keyName = TrimAsciiWhitespace(trimmed.substr(0, firstHash));
			displayName = NormalizeDisplayToken(trimmed.substr(firstHash + 1, secondHash - firstHash - 1));
		} else {
			continue;
		}
		if (keyName.empty() || displayName.empty()) {
			continue;
		}
		RegisterStringAlias(m_displayNamesByLookupKey, keyName, displayName);
		loadedAny = true;
	}
	return loadedAny;
}

bool CItemMgr::LoadLegacyDescriptionTable()
{
	std::string text;
	if (!LoadTextFileFromGameData("itemdesctable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	std::string currentItemName;
	std::ostringstream description;
	bool loadedAny = false;
	while (std::getline(input, line)) {
		line = TrimLine(line);
		const std::string trimmed = TrimAsciiWhitespace(line);
		if (trimmed.empty()) {
			continue;
		}

		if (trimmed == "#") {
			if (!currentItemName.empty()) {
				RegisterStringAlias(m_descriptionsByLookupKey, currentItemName, description.str());
				loadedAny = true;
			}
			currentItemName.clear();
			description.str(std::string());
			description.clear();
			continue;
		}

		if (currentItemName.empty() && trimmed.back() == '#') {
			currentItemName = TrimAsciiWhitespace(trimmed.substr(0, trimmed.size() - 1));
			continue;
		}

		if (!description.str().empty()) {
			description << "\n";
		}
		description << trimmed;
	}

	if (!currentItemName.empty() && !description.str().empty()) {
		RegisterStringAlias(m_descriptionsByLookupKey, currentItemName, description.str());
		loadedAny = true;
	}

	return loadedAny;
}

bool CItemMgr::LoadLegacySlotCountTable()
{
	std::string text;
	if (!LoadTextFileFromGameData("itemslottable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	std::string currentItemName;
	bool loadedAny = false;
	while (std::getline(input, line)) {
		std::string value;
		if (!ParseLegacySingleValueLine(TrimLine(line), value)) {
			continue;
		}

		if (currentItemName.empty()) {
			currentItemName = value;
			continue;
		}

		RegisterIntegerAlias(m_slotCountsByLookupKey, currentItemName, std::max(0, std::atoi(value.c_str())));
		currentItemName.clear();
		loadedAny = true;
	}
	return loadedAny;
}

bool CItemMgr::LoadLegacyItemNameTable()
{
	std::string text;
	if (!LoadTextFileFromGameData("itemnametable.txt", text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	bool loadedAny = false;
	while (std::getline(input, line)) {
		std::string displayName;
		std::string resourceName;
		if (!ParseLegacyItemNameLine(TrimLine(line), displayName, resourceName)) {
			continue;
		}
		RegisterItemResourceAlias(m_resourceNamesByLookupKey, displayName, resourceName);
		loadedAny = true;
	}

	return loadedAny;
}

bool CItemMgr::ParsePairTable(const char* fileName, void (*assignValue)(ItemMetadata&, std::string&&))
{
	std::string text;
	if (!LoadTextFileFromGameData(fileName, text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	while (std::getline(input, line)) {
		unsigned int itemId = 0;
		std::string value;
		if (!ParseHashPairLine(TrimLine(line), itemId, value)) {
			continue;
		}
		assignValue(m_metadata[itemId], std::move(value));
	}

	return true;
}

bool CItemMgr::ParseIntegerPairTable(const char* fileName, void (*assignValue)(ItemMetadata&, int))
{
	std::string text;
	if (!LoadTextFileFromGameData(fileName, text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	while (std::getline(input, line)) {
		unsigned int itemId = 0;
		std::string value;
		if (!ParseHashPairLine(TrimLine(line), itemId, value)) {
			continue;
		}
		assignValue(m_metadata[itemId], std::atoi(value.c_str()));
	}

	return true;
}

bool CItemMgr::ParseDescriptionBlocks(const char* fileName)
{
	std::string text;
	if (!LoadTextFileFromGameData(fileName, text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	unsigned int currentItemId = 0;
	std::ostringstream description;
	bool collecting = false;

	while (std::getline(input, line)) {
		line = TrimLine(line);
		if (!collecting) {
			unsigned int itemId = 0;
			if (ParseDescriptionBlockHeaderLine(line, itemId)) {
				currentItemId = itemId;
				description.str(std::string());
				description.clear();
				collecting = true;
			}
			continue;
		}

		if (line == "#") {
			m_metadata[currentItemId].description = description.str();
			currentItemId = 0;
			collecting = false;
			continue;
		}

		if (description.tellp() > 0) {
			description << '\n';
		}
		description << line;
	}

	return true;
}

bool CItemMgr::ParseIdSetTable(const char* fileName, std::unordered_set<unsigned int>& outSet)
{
	std::string text;
	if (!LoadTextFileFromGameData(fileName, text)) {
		return false;
	}

	std::istringstream input(text);
	std::string line;
	while (std::getline(input, line)) {
		unsigned int itemId = 0;
		if (!ParseHashIdLine(TrimLine(line), itemId)) {
			continue;
		}
		outSet.insert(itemId);
	}

	return true;
}
