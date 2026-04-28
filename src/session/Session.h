#pragma once
#include <array>
#include <mutex>
#include <unordered_map>
#include <string>
#include <list>
#include <vector>
#include "Types.h"
#include "item/Item.h"
#include "skill/Skill.h"

struct accountInfo {
    std::string display;
    std::string desc;
    std::string balloon;
    std::string address;
    std::string port;
};

constexpr int JT_G_MASTER = 20002;

struct PLAYER_SKILL_INFO {
    int m_isValid = 0;
    int SKID = 0;
    int type = 0;
    int level = 0;
    int spcost = 0;
    int upgradable = 0;
    int attackRange = 0;
    int skillPos = 0;
    int skillMaxLv = 0;
    std::string skillIdName;
    std::string skillName;
    std::vector<std::string> descriptionLines;
    std::vector<int> needSkillList;
};

struct FRIEND_INFO {
    int isValid = 0;
    u32 AID = 0;
    u32 GID = 0;
    std::string characterName;
    std::string mapName;
    int role = 0;
    int state = 0;
    u32 color = 0;
    int partyHp = 0;
    int partyMaxHp = 0;
};

enum class NpcShopMode : int {
    None = 0,
    Buy = 1,
    Sell = 2,
};

struct NPC_SHOP_ROW {
    ITEM_INFO itemInfo;
    unsigned int sourceItemIndex = 0;
    int price = 0;
    int secondaryPrice = 0;
    int availableCount = 0;
};

struct NPC_SHOP_DEAL_ROW {
    ITEM_INFO itemInfo;
    unsigned int sourceItemIndex = 0;
    int unitPrice = 0;
    int quantity = 0;
};

struct SHORTCUT_SLOT {
    unsigned char isSkill = 0;
    unsigned int id = 0;
    unsigned short count = 0;
};

struct ACTIVE_STATUS_ICON {
    int statusType = 0;
    bool hasTimer = false;
    u32 expireServerTime = 0;
};

struct QUEST_HUNT_INFO {
    u32 monsterId = 0;
    u32 count = 0;
    u32 maxCount = 0;
    std::string monsterName;
};

struct QUEST_INFO {
    u32 questId = 0;
    bool active = false;
    u32 startServerTime = 0;
    u32 endServerTime = 0;
    std::vector<QUEST_HUNT_INFO> hunts;
};

struct TRADE_PANE {
    std::list<ITEM_INFO> items;
    int zeny = 0;
    bool ready = false;
};

struct TRADE_STATE {
    bool active = false;
    bool finalConfirmAvailable = false;  // both sides hit Ready -> Commit allowed
    std::string partnerName;
    TRADE_PANE mine;
    TRADE_PANE theirs;
};

struct VENDING_ITEM {
    u32 amount = 0;
    u32 price = 0;
    u16 inventoryIndex = 0;
    u16 itemId = 0;
    u8 identified = 0;
    u8 damaged = 0;
    u8 refine = 0;
    int slot[4] = { 0, 0, 0, 0 };
    int itemType = 0;
};

struct VENDING_STATE {                  // I am the seller
    bool active = false;
    std::string shopTitle;
    std::vector<VENDING_ITEM> items;
};

struct VENDING_SHOP_BROWSE_STATE {      // I am the buyer
    bool active = false;
    u32 partnerAid = 0;
    std::string partnerShopTitle;
    std::vector<VENDING_ITEM> items;
};

struct GUILD_MEMBER {
    u32 aid = 0;
    u32 gid = 0;
    int headType = 0;
    int headPalette = 0;
    int sex = 0;
    int job = 0;
    int level = 0;
    int memberExp = 0;
    int currentState = 0;
    int positionId = 0;
    char memo[50] = {};
    char name[25] = {};
};

struct MAIL_HEADER {
    u32 mailId = 0;
    char title[40] = {};
    char sender[25] = {};
    u32 expireTime = 0;
    u8 isRead = 0;
    u8 hasAttachment = 0;
};

struct MAIL_BODY {
    u32 mailId = 0;
    char title[40] = {};
    char sender[25] = {};
    char body[200] = {};
    u32 zeny = 0;
    u16 attachItemId = 0;
    u16 attachAmount = 0;
    u8 attachIdentified = 0;
    u8 attachDamaged = 0;
    u8 attachRefine = 0;
    u16 attachCards[4] = {};
};

struct SessionFogParameter {
    float start = 0.0f;
    float end = 1.0f;
    u32 color = 0;
    float density = 0.0f;
};

constexpr int kShortcutSlotsPerPage = 9;
constexpr int kShortcutPageCount = 3;
constexpr int kShortcutSlotCount = kShortcutSlotsPerPage * kShortcutPageCount;

class CSession
{
public:
    CSession();
    ~CSession();

    bool InitAccountInfo();
    
    std::vector<accountInfo> m_accountInfo;
    char m_userId[24];
    char m_userPassword[24];
    u32  m_aid;
    u32  m_authCode;
    u32  m_userLevel;
    u32  m_gid;
    u8   m_sex;
    bool m_isEffectOn;
    bool m_isMinEffect;
    bool m_fogOn;
    char m_charServerAddr[64];
    int  m_charServerPort;
    char m_zoneServerAddr[64] = {};
    int  m_zoneServerPort = 0;
    int  m_pendingReturnToCharSelect;
    
    char m_curMap[24];
    int m_playerPosX;
    int m_playerPosY;
    int m_playerDir;
    int m_playerJob;
    int m_playerHead;
    int m_playerBodyPalette;
    int m_playerHeadPalette;
    int m_playerWeapon;
    int m_playerShield;
    int m_playerAccessory;
    int m_playerAccessory2;
    int m_playerAccessory3;
    char m_playerName[25];
    int m_plusStr = 0;
    int m_plusAgi = 0;
    int m_plusVit = 0;
    int m_plusInt = 0;
    int m_plusDex = 0;
    int m_plusLuk = 0;
    int m_standardStr = 2;
    int m_standardAgi = 2;
    int m_standardVit = 2;
    int m_standardInt = 2;
    int m_standardDex = 2;
    int m_standardLuk = 2;
    int m_attPower = 0;
    int m_refiningPower = 0;
    int m_maxMatkPower = 0;
    int m_minMatkPower = 0;
    int m_itemDefPower = 0;
    int m_plusDefPower = 0;
    int m_mdefPower = 0;
    int m_plusMdefPower = 0;
    int m_hitSuccessValue = 0;
    int m_avoidSuccessValue = 0;
    int m_plusAvoidSuccessValue = 0;
    int m_criticalSuccessValue = 0;
    int m_aspd = 0;
    int m_plusAspd = 0;
    u32 m_shopNpcId = 0;
    NpcShopMode m_shopMode = NpcShopMode::None;
    int m_shopSelectedSourceRow = -1;
    int m_shopSelectedDealRow = -1;
    int m_shopDealTotal = 0;
    std::vector<NPC_SHOP_ROW> m_shopRows;
    std::vector<NPC_SHOP_DEAL_ROW> m_shopDealRows;
    std::list<FRIEND_INFO> m_partyList;
    std::list<FRIEND_INFO> m_friendList;
    std::string m_partyName;
    bool m_amIPartyMaster = false;
    bool m_partyExpShare = false;
    bool m_itemDivType = false;
    bool m_itemCollectType = false;
    bool m_storageOpen = false;
    int m_storageCurrentCount = 0;
    int m_storageMaxCount = 0;
    bool m_cartActive = false;
    int m_cartCurrentCount = 0;
    int m_cartMaxCount = 0;
    int m_cartCurrentWeight = 0;
    int m_cartMaxWeight = 0;
    bool m_petActive = false;
    char m_petName[25] = {};
    int m_petLevel = 0;
    int m_petFullness = 0;
    int m_petIntimacy = 0;
    int m_petItemId = 0;
    int m_petJob = 0;
    std::vector<int> m_petEggList;
    bool m_homunActive = false;
    char m_homunName[25] = {};
    int m_homunLevel = 0;
    int m_homunHunger = 0;
    int m_homunIntimacy = 0;
    int m_homunHp = 0;
    int m_homunMaxHp = 0;
    int m_homunSp = 0;
    int m_homunMaxSp = 0;
    u32 m_homunGid = 0;
    bool m_mercActive = false;
    char m_mercName[25] = {};
    int m_mercLevel = 0;
    int m_mercHp = 0;
    int m_mercMaxHp = 0;
    int m_mercSp = 0;
    int m_mercMaxSp = 0;
    int m_mercFaith = 0;
    int m_mercCalls = 0;
    u32 m_mercGid = 0;
    u32 m_mercExpireTime = 0;
    int m_guildId = 0;
    int m_guildEmblemId = 0;
    char m_guildName[25] = {};
    char m_guildMasterName[25] = {};
    char m_guildNoticeSubject[60] = {};
    char m_guildNoticeBody[120] = {};
    std::vector<GUILD_MEMBER> m_guildMembers;
    bool m_mailBoxOpen = false;
    std::vector<MAIL_HEADER> m_mailHeaders;
    MAIL_BODY m_mailReadBody{};
    int m_shortcutPage = 0;
    std::array<SHORTCUT_SLOT, kShortcutSlotCount> m_shortcutSlots{};
    int m_GaugePacket = 0;
    
    void SetServerTime(u32 time);
    void UpdateServerTime(u32 time);
    u32 GetServerTime() const;
    bool GetFogParameter(const char* rswName, SessionFogParameter* outParameter);
    void SetPlayerPosDir(int x, int y, int dir);
    void SetSelectedCharacterAppearance(const CHARACTER_INFO& info);
    const CHARACTER_INFO* GetSelectedCharacterInfo() const;
    CHARACTER_INFO* GetMutableSelectedCharacterInfo();
    void SetBaseExpValue(int value);
    void SetNextBaseExpValue(int value);
    void SetJobExpValue(int value);
    void SetNextJobExpValue(int value);
    bool TryGetBaseExpPercent(int* outPercent) const;
    bool TryGetJobExpPercent(int* outPercent) const;
    void ClearInventoryItems();
    void ClearEquipmentInventoryItems();
    void ClearStorageItems();
    void ClearCartItems();
    void ClearSkillItems();
    void ClearHomunSkillItems();
    void ClearMercSkillItems();
    void SetInventoryItem(const ITEM_INFO& itemInfo);
    void AddInventoryItem(const ITEM_INFO& itemInfo);
    void OpenStorage(int currentCount, int maxCount);
    void CloseStorage();
    void SetStorageItem(const ITEM_INFO& itemInfo);
    void AddStorageItem(const ITEM_INFO& itemInfo);
    void OpenCart(int currentCount, int maxCount, int currentWeight, int maxWeight);
    void CloseCart();
    void SetCartItem(const ITEM_INFO& itemInfo);
    void AddCartItem(const ITEM_INFO& itemInfo);
    void RemoveCartItem(unsigned int itemIndex, int amount);
    bool IsCartActive() const;
    int GetCartCurrentCount() const;
    int GetCartMaxCount() const;
    int GetCartCurrentWeight() const;
    int GetCartMaxWeight() const;
    const std::list<ITEM_INFO>& GetCartItems() const;
    const ITEM_INFO* GetCartItemByIndex(unsigned int itemIndex) const;
    void SetPetProperty(const char* name, int level, int fullness, int intimacy, int itemId, int job);
    void SetPetEggList(const std::vector<int>& eggs);
    void ClearPet();
    bool IsPetActive() const;
    void SetHomunProperty(u32 gid, const char* name, int level, int hp, int maxHp, int sp, int maxSp, int hunger, int intimacy);
    void ClearHomun();
    bool IsHomunActive() const;
    void SetMercProperty(u32 gid, const char* name, int level, int hp, int maxHp, int sp, int maxSp, int faith, int calls, u32 expireTime);
    void ClearMerc();
    bool IsMercActive() const;
    void SetGuildBasic(int guildId, int emblemId, const char* name, const char* masterName);
    void SetGuildNotice(const char* subject, const char* body);
    void ClearGuild();
    bool IsInGuild() const;
    void SetGuildMembers(std::vector<GUILD_MEMBER> members);
    const std::vector<GUILD_MEMBER>& GetGuildMembers() const;
    void OpenMailBox();
    void CloseMailBox();
    bool IsMailBoxOpen() const;
    void SetMailHeaders(std::vector<MAIL_HEADER> headers);
    const std::vector<MAIL_HEADER>& GetMailHeaders() const;
    void SetMailReadBody(const MAIL_BODY& body);
    const MAIL_BODY& GetMailReadBody() const;
    bool RemoveMailHeader(u32 mailId);
    void SetSkillItem(const PLAYER_SKILL_INFO& skillInfo);
    void SetHomunSkillItem(const PLAYER_SKILL_INFO& skillInfo);
    void SetMercSkillItem(const PLAYER_SKILL_INFO& skillInfo);
    void RemoveInventoryItem(unsigned int itemIndex, int amount);
    void RemoveStorageItem(unsigned int itemIndex, int amount);
    bool SetInventoryItemWearLocation(unsigned int itemIndex, int wearLocation);
    void ClearInventoryWearLocationMask(int wearMask, unsigned int exceptItemIndex = 0);
    void RebuildPlayerEquipmentAppearanceFromInventory();
    const std::list<ITEM_INFO>& GetInventoryItems() const;
    const ITEM_INFO* GetInventoryItemByIndex(unsigned int itemIndex) const;
    const ITEM_INFO* GetInventoryItemByItemId(unsigned int itemId) const;
    const std::list<ITEM_INFO>& GetStorageItems() const;
    const ITEM_INFO* GetStorageItemByIndex(unsigned int itemIndex) const;
    bool IsStorageOpen() const;
    int GetStorageCurrentCount() const;
    int GetStorageMaxCount() const;
    const std::list<PLAYER_SKILL_INFO>& GetSkillItems() const;
    const std::list<PLAYER_SKILL_INFO>& GetHomunSkillItems() const;
    const std::list<PLAYER_SKILL_INFO>& GetMercSkillItems() const;
    const PLAYER_SKILL_INFO* GetSkillItemBySkillId(int skillId) const;
    const PLAYER_SKILL_INFO* GetHomunSkillItemBySkillId(int skillId) const;
    const PLAYER_SKILL_INFO* GetMercSkillItemBySkillId(int skillId) const;
    void ClearNpcShopState();
    void SetNpcShopChoice(u32 npcId);
    void SetNpcShopRows(u32 npcId, NpcShopMode mode, const std::vector<NPC_SHOP_ROW>& rows);
    bool AdjustNpcShopDealBySourceRow(size_t sourceRowIndex, int deltaQuantity);
    bool AdjustNpcShopDealByDealRow(size_t dealRowIndex, int deltaQuantity);
    int GetNpcShopUnitPrice(const NPC_SHOP_ROW& row) const;
    void ClearParty();
    unsigned int GetNumParty() const;
    void AddMemberToParty(const FRIEND_INFO& info);
    unsigned int GetMemberAidFromParty(const char* characterName) const;
    void DeleteMemberFromParty(const char* characterName);
    void ChangeRoleFromParty(unsigned int aid, int role);
    bool SetPartyMemberHp(unsigned int aid, int hp, int maxHp);
    const FRIEND_INFO* FindPartyMemberByAid(unsigned int aid) const;
    void RefreshPartyUI();
    const std::list<FRIEND_INFO>& GetPartyList() const;
    void ClearFriend();
    unsigned int GetNumFriend() const;
    bool IsFriendName(const char* characterName) const;
    bool DeleteFriendFromList(unsigned int gid);
    void AddFriendToList(const FRIEND_INFO& info);
    bool SetFriendState(unsigned int aid, unsigned int gid, unsigned char state);
    void RefreshFriendUI();
    const std::list<FRIEND_INFO>& GetFriendList() const;
    void ClearShortcutSlots();
    void ClearActiveStatusIcons();
    void SetActiveStatusIcon(int statusType, bool active, u32 remainingMs);
    void PruneExpiredStatusIcons(u32 serverTime);
    void SetSkillCooldown(int skillId, u32 durationMs);
    void ClearAllSkillCooldowns();
    u32 GetSkillCooldownRemainingMs(int skillId) const;
    void ClearQuests();
    void AddQuest(const QUEST_INFO& info);
    void RemoveQuest(u32 questId);
    void UpdateQuestHunt(u32 questId, u32 monsterId, u32 newCount);
    const std::vector<QUEST_INFO>& GetQuests() const;
    void BeginTrade(const std::string& partnerName);
    void EndTrade();
    bool IsTradeActive() const;
    TRADE_STATE& GetTradeState();
    const TRADE_STATE& GetTradeState() const;
    void BeginVending(const std::string& shopTitle);
    void EndVending();
    bool IsVendingActive() const;
    VENDING_STATE& GetVendingState();
    const VENDING_STATE& GetVendingState() const;
    void BeginVendingBrowse(u32 partnerAid, const std::string& shopTitle);
    void EndVendingBrowse();
    bool IsVendingBrowseActive() const;
    VENDING_SHOP_BROWSE_STATE& GetVendingBrowseState();
    const VENDING_SHOP_BROWSE_STATE& GetVendingBrowseState() const;
    void SetActorShopTitle(u32 aid, const std::string& title);
    void ClearActorShopTitle(u32 aid);
    std::string GetActorShopTitle(u32 aid) const;
    int GetShortcutPage() const;
    void SetShortcutPage(int page);
    int GetShortcutSlotAbsoluteIndex(int visibleSlot) const;
    const SHORTCUT_SLOT* GetShortcutSlotByAbsoluteIndex(int absoluteIndex) const;
    const SHORTCUT_SLOT* GetShortcutSlotByVisibleIndex(int visibleSlot) const;
    bool SetShortcutSlotByAbsoluteIndex(int absoluteIndex, unsigned char isSkill, unsigned int id, unsigned short count);
    bool SetShortcutSlotByVisibleIndex(int visibleSlot, unsigned char isSkill, unsigned int id, unsigned short count);
    bool ClearShortcutSlotByAbsoluteIndex(int absoluteIndex);
    bool ClearShortcutSlotByVisibleIndex(int visibleSlot);
    int FindShortcutSlotByItemId(unsigned int itemId) const;
    int FindShortcutSlotBySkillId(int skillId) const;
    const std::vector<ACTIVE_STATUS_ICON>& GetActiveStatusIcons() const;
    int GetPlayerSkillPointCount() const;
    int GetWeaponTypeByItemId(int itemId) const;
    int MakeWeaponTypeByItemId(int primaryWeaponItemId, int secondaryWeaponItemId) const;
    int ResolvePackedWeaponType(int job, int weaponValue) const;
    int GetCurrentPlayerWeaponValue() const;
    std::string GetPlayerWeaponToken(int weaponType) const;
    bool IsSecondAttack(int job, int sex, int weaponItemId) const;
    float GetPCAttackMotion(int job, int sex, int weaponItemId, int isSecondAttack) const;
    unsigned int GetEquippedLeftHandWeaponItemId() const;
    unsigned int GetEquippedRightHandWeaponItemId() const;
    const char* GetPlayerName() const;
    const char* GetJobDisplayName(int job) const;
    const char* GetJobName(int job) const;
    const char* GetAttrWaveName(int attr) const;
    const char* GetJobHitWaveName(int job) const;
    const char* GetWeaponHitWaveName(int weapon) const;
    int GetSex() const;
    char* GetJobActName(int job, int sex, char* buf);
    char* GetJobSprName(int job, int sex, char* buf);
    char* GetPlayerBodyActName(int job, int sex, int weaponItemId, char* buf);
    char* GetPlayerBodySprName(int job, int sex, int weaponItemId, char* buf);
    char* GetHeadActName(int job, int* head, int sex, char* buf);
    char* GetHeadSprName(int job, int* head, int sex, char* buf);
    char* GetAccessoryActName(int job, int* head, int sex, int accessory, char* buf);
    char* GetAccessorySprName(int job, int* head, int sex, int accessory, char* buf);
    char* GetImfName(int job, int head, int sex, char* buf);
    char* GetBodyPaletteName(int job, int sex, int palNum, char* buf);
    char* GetHeadPaletteName(int head, int job, int sex, int palNum, char* buf);
    
private:
    void EnsureFogParameterTableLoaded();
    void EnsureAccessoryNameTableLoaded();
    void InitJobHitWaveName();
    void InitWeaponHitWaveName();
    int NormalizeJob(int job) const;
    u32 m_serverTime;
    int m_numLatePacket;
    CHARACTER_INFO m_selectedCharacterInfo;
    bool m_hasSelectedCharacterInfo;
    int m_baseExpValue;
    int m_nextBaseExpValue;
    int m_jobExpValue;
    int m_nextJobExpValue;
    bool m_hasBaseExpValue;
    bool m_hasNextBaseExpValue;
    bool m_hasJobExpValue;
    bool m_hasNextJobExpValue;
    std::list<ITEM_INFO> m_inventoryItems;
    std::list<ITEM_INFO> m_storageItems;
    std::list<ITEM_INFO> m_cartItems;
    std::list<PLAYER_SKILL_INFO> m_skillItems;
    std::list<PLAYER_SKILL_INFO> m_homunSkillItems;
    std::list<PLAYER_SKILL_INFO> m_mercSkillItems;
    mutable std::mutex m_jobDisplayNameMutex;
    mutable std::unordered_map<int, std::string> m_jobDisplayNameCache;
    std::vector<ACTIVE_STATUS_ICON> m_activeStatusIcons;
    std::unordered_map<int, u32> m_skillCooldownEndServerTime;
    std::vector<QUEST_INFO> m_quests;
    TRADE_STATE m_tradeState;
    VENDING_STATE m_vendingState;
    VENDING_SHOP_BROWSE_STATE m_vendingBrowseState;
    std::unordered_map<u32, std::string> m_actorShopTitles;
    bool m_fogParameterTableLoaded;
    std::unordered_map<std::string, SessionFogParameter> m_fogParameterTable;
    bool m_accessoryNameTableLoaded;
    std::vector<std::string> m_accessoryNameTable;
    std::vector<std::string> m_jobHitWaveNameTable;
    std::vector<std::string> m_weaponHitWaveNameTable;
};

extern CSession g_session;
