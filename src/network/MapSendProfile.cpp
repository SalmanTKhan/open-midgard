#include "MapSendProfile.h"

#include "Packet.h"
#include "core/ClientInfoLocale.h"
#include "core/SettingsIni.h"
#include "DebugLog.h"
#include "gamemode/PacketPadding.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>

namespace ro::net {
namespace {

enum class ClientProfilePreset {
    Auto,
    PacketVer23,
    PacketVer22,
    Legacy0072,
    SabineAlpha, // 100
    SabineBeta1, // 200
    SabineBeta2, // 300
    SabineEP3,   // 400
    SabineEP3_2, // 500
    SabineEP4,   // 600
    SabineEP8,   // 700
    SabineBEP5,  // 800
};

bool MatchesPacketId(u16 packetId, std::initializer_list<u16> ids)
{
    for (const u16 id : ids) {
        if (id != 0 && packetId == id) {
            return true;
        }
    }
    return false;
}

AccountLoginPacketProfile MakeLoginProfile(PacketVersionId id, const char* name)
{
    return {
        id,
        name,
        PacketProfile::PacketVer23LoginChain::kClientDate,
        PacketProfile::PacketVer23LoginChain::kAccountLogin,
        PacketProfile::PacketVer23LoginChain::kAccountLoginChannel,
        PacketProfile::PacketVer23LoginChain::kRequestPasswordHash,
        PacketProfile::PacketVer23LoginChain::kPasswordHashChallenge,
        PacketProfile::PacketVer23LoginChain::kPasswordHashLogin,
        PACKETID_CA_CONNECT_INFO_CHANGED,
        PacketProfile::PacketVer23LoginChain::kExeHashCheck,
        PacketProfile::PacketVer23LoginChain::kNotifyError,
    };
}

CharacterPacketProfile MakeCharacterProfile(PacketVersionId id, const char* name)
{
    return {
        id,
        name,
        PacketProfile::PacketVer23LoginChain::kCharServerEnter,
        PacketProfile::PacketVer23LoginChain::kSelectCharacter,
        PacketProfile::PacketVer23LoginChain::kMakeCharacter,
        PacketProfile::PacketVer23LoginChain::kDeleteCharacter,
    };
}

ZonePacketProfile MakePacketVer23ZoneProfile()
{
    return {
        PacketVersionId::PacketVer23,
        "packetver23",
        PacketProfile::PacketVer23MapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Compact19,
    };
}

ZonePacketProfile MakePacketVer100ZoneProfile()
{
    return {
        PacketVersionId::PacketVer22,
        "packetver100",
        PacketProfile::EarlyMapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Compact19,
    };
}

ZonePacketProfile MakePacketVer22ZoneProfile()
{
    return {
        PacketVersionId::PacketVer22,
        "packetver22",
        PacketProfile::PacketVer22MapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Legacy26,
    };
}

ZonePacketProfile MakePacketVer200ZoneProfile()
{
    return {
        PacketVersionId::PacketVer22,
        "packetver200",
        // Beta1 keeps the early 200-era 0x000E zone-entry opcode with the compact 19-byte layout.
        PacketProfile::EarlyMapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Compact19,
    };
}

ZonePacketProfile MakePacketVer300ZoneProfile()
{
    return {
        PacketVersionId::PacketVer300,
        "packetver300",
        0x0072, // CZ_ENTER shifted by 100
        ZoneConnectPacketLayout::Compact19,
    };
}

ZonePacketProfile MakePacketVer400ZoneProfile()
{
    return {
        PacketVersionId::PacketVer400,
        "packetver400",
        0x0072,
        ZoneConnectPacketLayout::Compact19,
    };
}


ZonePacketProfile MakeLegacy72ZoneProfile()
{
    return {
        PacketVersionId::PacketVer22,
        "legacy0072",
        PacketProfile::LegacyMapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Compact19,
    };
}

ZonePacketProfile MakeLegacy72Padded22ZoneProfile()
{
    return {
        PacketVersionId::PacketVer22,
        "legacy0072padded22",
        PacketProfile::LegacyMapServerSend::kWantToConnection,
        ZoneConnectPacketLayout::Padded22,
    };
}

MapGameplaySendProfile MakePacketVer23Profile()
{
    return {
        MapGameplaySendProfileId::PacketVer23,
        "packetver23",
        PacketProfile::PacketVer23MapServerSend::kActionRequest,
        PacketProfile::PacketVer23MapServerSend::kUseSkillToId,
        PacketProfile::PacketVer23MapServerSend::kCartOff,
        PacketProfile::PacketVer23MapServerSend::kChangeCart,
        PacketProfile::PacketVer23MapServerSend::kUseSkillToPos,
        PacketProfile::PacketVer23MapServerSend::kUseSkillMap,
        PacketProfile::PacketVer23MapServerSend::kUseItem,
        PacketProfile::PacketVer23MapServerSend::kTakeItem,
        PacketProfile::PacketVer23MapServerSend::kDropItem,
        PacketProfile::PacketVer23MapServerSend::kItemCompositionList,
        PacketProfile::PacketVer23MapServerSend::kItemComposition,
        PacketProfile::PacketVer23MapServerSend::kItemIdentify,
        PacketProfile::PacketVer23MapServerSend::kSkillUp,
        PacketProfile::PacketVer23MapServerSend::kEquipItem,
        PacketProfile::PacketVer23MapServerSend::kUnequipItem,
        PacketProfile::PacketVer23MapServerSend::kWalkToXY,
        PacketProfile::PacketVer23MapServerSend::kChangeDir,
        PacketProfile::PacketVer23MapServerSend::kTickSend,
        PacketProfile::PacketVer23MapServerSend::kNotifyActorInit,
        PacketProfile::PacketVer23MapServerSend::kGetCharNameRequest,
        PacketProfile::PacketVer23MapServerSend::kWhisper,
        PacketProfile::PacketVer23MapServerSend::kGlobalMessage,
    };
}

MapGameplaySendProfile MakePacketVer22Profile()
{
    return {
        MapGameplaySendProfileId::PacketVer22,
        "packetver22",
        PacketProfile::PacketVer22MapServerSend::kActionRequest,
        PacketProfile::PacketVer22MapServerSend::kUseSkillToId,
        PacketProfile::PacketVer22MapServerSend::kCartOff,
        PacketProfile::PacketVer22MapServerSend::kChangeCart,
        PacketProfile::PacketVer22MapServerSend::kUseSkillToPos,
        PacketProfile::PacketVer22MapServerSend::kUseSkillMap,
        PacketProfile::PacketVer22MapServerSend::kUseItem,
        PacketProfile::PacketVer22MapServerSend::kTakeItem,
        PacketProfile::PacketVer22MapServerSend::kDropItem,
        PacketProfile::PacketVer22MapServerSend::kItemCompositionList,
        PacketProfile::PacketVer22MapServerSend::kItemComposition,
        PacketProfile::PacketVer22MapServerSend::kItemIdentify,
        PacketProfile::PacketVer22MapServerSend::kSkillUp,
        PacketProfile::PacketVer22MapServerSend::kEquipItem,
        PacketProfile::PacketVer22MapServerSend::kUnequipItem,
        PacketProfile::PacketVer22MapServerSend::kWalkToXY,
        PacketProfile::PacketVer22MapServerSend::kChangeDir,
        PacketProfile::PacketVer22MapServerSend::kTickSend,
        PacketProfile::PacketVer22MapServerSend::kNotifyActorInit,
        PacketProfile::PacketVer22MapServerSend::kGetCharNameRequest,
        PacketProfile::PacketVer22MapServerSend::kWhisper,
        PacketProfile::PacketVer22MapServerSend::kGlobalMessage,
    };
}

MapGameplaySendProfile MakePacketVer200Profile()
{
    // Beta1 shifts all Alpha opcodes >= 0x0027 by +1 (ZC_NOTIFY_ACT_POSITION insertion).
    // EarlyMapServerSend constants use Alpha opcodes; fields below 0x0027 are reused
    // directly; fields >= 0x0027 use the corrected Beta1 value (+1).
    return {
        MapGameplaySendProfileId::PacketVer200,
        "packetver200",
        PacketProfile::EarlyMapServerSend::kActionRequest,     // 0x0025 — unchanged
        PacketProfile::EarlyMapServerSend::kUseSkillToId,      // 0x00AF — beta1 new (appended)
        PacketProfile::EarlyCartSend::kCartOff,
        PacketProfile::PacketVer23MapServerSend::kChangeCart,
        PacketProfile::EarlyMapServerSend::kUseSkillToPos,     // 0x00B2 — beta1 new (appended)
        PacketProfile::PacketVer23MapServerSend::kUseSkillMap,
        0x0043,                                                 // CZ_USE_ITEM      (alpha 0x0042 +1)
        0x003B,                                                 // CZ_ITEM_PICKUP   (alpha 0x003A +1)
        0x003E,                                                 // CZ_ITEM_THROW    (alpha 0x003D +1)
        PacketProfile::PacketVer23MapServerSend::kItemCompositionList,
        PacketProfile::PacketVer23MapServerSend::kItemComposition,
        PacketProfile::PacketVer23MapServerSend::kItemIdentify,
        PacketProfile::EarlyMapServerSend::kSkillUp,           // 0x00AE — beta1 new (appended)
        0x0045,                                                 // CZ_REQ_WEAR_EQUIP    (alpha 0x0044 +1)
        0x0047,                                                 // CZ_REQ_TAKEOFF_EQUIP (alpha 0x0046 +1)
        PacketProfile::EarlyMapServerSend::kWalkToXY,          // 0x0021 — unchanged
        0x0037,                                                 // CZ_CHANGE_DIRECTION (alpha 0x0036 +1)
        PacketProfile::EarlyMapServerSend::kTickSend,          // 0x001A — unchanged
        PacketProfile::EarlyMapServerSend::kNotifyActorInit,   // 0x0019 — unchanged
        0x0030,                                                 // CZ_REQNAME       (alpha 0x002F +1)
        0x0032,                                                 // CZ_WHISPER       (alpha 0x0031 +1)
        0x0028,                                                 // CZ_REQUEST_CHAT  (alpha 0x0027 +1)
    };
}

MapGameplaySendProfile MakeLegacyGameplayProfile()
{
    return {
        MapGameplaySendProfileId::Legacy,
        "legacy0072",
        0x0089,
        0x0113,
        PacketProfile::PacketVer23MapServerSend::kCartOff,
        PacketProfile::PacketVer23MapServerSend::kChangeCart,
        0x0116,
        0x011B,
        0x00A7,
        0x009F,
        0x00A2,
        PacketProfile::PacketVer22MapServerSend::kItemCompositionList,
        PacketProfile::PacketVer22MapServerSend::kItemComposition,
        PacketProfile::PacketVer22MapServerSend::kItemIdentify,
        PacketProfile::LegacyMapServerSend::kSkillUp,
        0x00A9,
        0x00AB,
        PacketProfile::LegacyMapServerSend::kWalkToXY,
        0x009B,
        PacketProfile::LegacyMapServerSend::kTickSend,
        PacketProfile::LegacyMapServerSend::kNotifyActorInit,
        PacketProfile::LegacyMapServerSend::kGetCharNameRequest,
        PacketProfile::LegacyMapServerSend::kWhisper,
        PacketProfile::LegacyMapServerSend::kGlobalMessage,
    };
}

MapReceiveProfile MakePacketVer23ReceiveProfile()
{
    MapReceiveProfile profile{};
    profile.id = PacketVersionId::PacketVer23;
    profile.name = "packetver23";
    profile.deferProactiveNameRequests = false;
    profile.usesLegacyActorStream = true;
    profile.acceptEnterLegacy = PACKETID_ZC_ACCEPT_ENTER;
    profile.acceptEnterModern = 0x02EB;
    profile.notifyTime = 0x007F;
    profile.mapChangeBasic = 0x0091;
    profile.mapChangeServerMove = 0x0092;
    profile.actorActionNotifyBasic = 0x008A;
    profile.actorActionNotifyExtended = 0x02E1;
    profile.actorSetPositionBasic = 0x0088;
    profile.actorSetPositionHighJump = 0x01FF;
    profile.selfMoveAck = 0x0087;              // ZC_NOTIFY_PLAYERMOVE (PV23)
    profile.broadcastBasic = 0x009A;
    profile.broadcastColored = 0x01C3;
    profile.groundItemEntryExisting = 0x009D;
    profile.groundItemEntryDropped = 0x009E;
    profile.itemPickupAckBasic = 0x00A0;
    profile.itemPickupAckExtended = 0x02D4;
    profile.normalInventoryListBasic = 0x00A3;
    profile.normalInventoryListCardSlots = 0x01EE;
    profile.normalInventoryListTimed = 0x02E8;
    profile.equipInventoryListBasic = 0x00A4;
    profile.equipInventoryListTimed = 0x01EF;
    profile.equipInventoryListTimedOwned = 0x02D0;
    profile.normalStorageListBasic = 0x00A5;
    profile.normalStorageListCardSlots = 0x01F0;
    profile.normalStorageListTimed = 0x02EA;
    profile.equipStorageListBasic = 0x00A6;
    profile.equipStorageListTimedOwned = 0x02D1;
    profile.storageItemAddedBasic = 0x00F4;
    profile.storageItemAddedTyped = 0x01C4;
    profile.useItemAckBasic = 0x00A8;
    profile.useItemAckExtended = 0x01C8;
    profile.itemRemoveBasic = 0x00AF;
    profile.itemRemoveExtended = 0x07FA;
    profile.partyInviteAckBasic = 0x00FD;
    profile.partyInviteAckExtended = 0x02C5;
    profile.partyInviteRequestBasic = 0x00FE;
    profile.partyInviteRequestExtended = 0x02C6;
    profile.skillDamagePositionNotify = 0x0115;
    profile.groundSkillNotify = 0x0117;
    profile.skillNoDamageNotify = 0x011A;
    profile.skillUnitSetBasic = 0x011F;
    profile.skillUnitSetExtended = 0x01C9;
    profile.notifyEffectBasic = 0x019B;
    profile.notifyEffectDirect = 0x01F3;
    profile.actorSpawnLegacyIdle = 0x0078;
    profile.actorSpawnLegacySpawn = 0x0079;
    profile.actorSpawnLegacyAlt = 0x007A;
    profile.actorSpawnLegacyNpc = 0x007C;
    profile.actorSpawnLegacyIdleShifted = 0x01D8;
    profile.actorSpawnLegacySpawnShifted = 0x01D9;
    profile.actorMoveLegacy = 0x007B;
    profile.actorMoveLegacyShifted = 0x01DA;
    profile.actorSpawnVariableIdle = 0x07F9;
    profile.actorSpawnVariableSpawn = 0x07F8;
    profile.actorSpawnVariableIdleRobe = 0x0857;
    profile.actorSpawnVariableSpawnRobe = 0x0858;
    profile.actorMoveVariable = 0x07F7;
    profile.actorMoveVariableRobe = 0x0856;
    profile.actorSpawnModernIdle = 0x022A;
    profile.actorSpawnModernSpawn = 0x022B;
    profile.actorSpawnModernIdleFont = 0x02EE;
    profile.actorSpawnModernSpawnFont = 0x02ED;
    profile.actorMoveModern = 0x022C;
    profile.actorMoveModernFont = 0x02EC;
    profile.actorNameAckBasic = 0x0095;
    profile.actorNameAckParty = 0x0194;
    profile.actorNameAckFull = 0x0195;
    profile.actorStateChangeBasic = 0x0119;
    profile.actorStateChangeExtended = 0x0229;
    profile.partyMemberAddedBasic = 0x0104;
    profile.partyMemberAddedExtended = 0x01E9;
    profile.partyHpUpdateBasic = 0x0106;
    profile.partyHpUpdateExtended = 0x080E;
    profile.skillDamageNotifyBasic = 0x0114;
    profile.skillDamageNotifyExtended = 0x01DE;
    return profile;
}

MapReceiveProfile MakePacketVer22ReceiveProfile()
{
    MapReceiveProfile profile = MakePacketVer23ReceiveProfile();
    profile.id = PacketVersionId::PacketVer22;
    profile.name = "packetver22";
    profile.deferProactiveNameRequests = true;
    return profile;
}

MapReceiveProfile MakePacketVer200ReceiveProfile()
{
    // Sabine Beta1 (iRO Ragexe 2002-02-20). Opcode numbers inherited from the
    // Alpha packet table; many payload sizes are enlarged in Beta1. Reference:
    // E:/Projects/GitHub/Sabine/src/Shared/Network/PacketTable.{100_Alpha,200_Beta1}.cs
    // and Sabine/src/ZoneServer/Network/Helpers/Character.cs for actor layouts.
    //
    // Beta1 inserts ZC_NOTIFY_ACT_POSITION at 0x0027, shifting all Alpha opcodes
    // >= 0x0027 up by +1 (plus further shifts at 0x009D/0x009E for higher opcodes).
    // All opcodes below list both their Alpha value (comment) and the Beta1 final value.
    MapReceiveProfile profile{};
    profile.id = PacketVersionId::PacketVer200;
    profile.name = "packetver200";
    profile.deferProactiveNameRequests = false;
    profile.usesLegacyActorStream = true;

    profile.acceptEnterLegacy = 0x000F;        // ZC_ACCEPT_ENTER / 11 (alpha 0x000F, unchanged)
    profile.acceptEnterModern = 0;
    profile.notifyTime = 0x001B;               // ZC_NOTIFY_TIME / 6 (alpha 0x001B, unchanged)
    profile.mapChangeBasic = 0x002D;           // ZC_NPCACK_MAPMOVE / 22 (alpha 0x002C +1)
    profile.mapChangeServerMove = 0x002E;      // ZC_NPCACK_SERVERMOVE / 28 (alpha 0x002D +1)
    profile.actorActionNotifyBasic = 0x0026;   // ZC_NOTIFY_ACT / 27 (alpha 0x0026, unchanged)
    profile.actorActionNotifyExtended = 0x0027; // ZC_NOTIFY_ACT_POSITION / 23 (new in Beta1 at 0x0027)
    profile.actorSetPositionBasic = 0;          // ZC_NOTIFY_SETPOS doesn't exist in Alpha/Beta1
    profile.actorSetPositionHighJump = 0;
    profile.selfMoveAck = 0x0023;              // ZC_NOTIFY_PLAYERMOVE / 12 (alpha 0x0023, unchanged)
    profile.broadcastBasic = 0x0036;           // ZC_BROADCAST / var (alpha 0x0035 +1)
    profile.broadcastColored = 0;
    profile.groundItemEntryExisting = 0x0039;  // ZC_ITEM_ENTRY / 38 (alpha 0x0038 +1)
    profile.groundItemEntryDropped = 0x003A;   // ZC_ITEM_FALL_ENTRY / 38 (alpha 0x0039 +1)
    profile.itemPickupAckBasic = 0x003C;       // ZC_ITEM_PICKUP_ACK / 33 (alpha 0x003B +1)
    profile.itemPickupAckExtended = 0;
    profile.useItemAckBasic = 0x0044;          // ZC_USE_ITEM_ACK / 7 (alpha 0x0043 +1)
    profile.useItemAckExtended = 0;
    profile.itemRemoveBasic = 0x004B;          // ZC_ITEM_THROW_ACK / 6 (alpha 0x004A +1)
    profile.itemRemoveExtended = 0;

    // Alpha/Beta1 actor stream: the four entry packets + the monster-move packet (0x0022).
    // These are all < 0x0027 so not shifted.
    profile.actorSpawnLegacyIdle = 0x0014;     // ZC_NOTIFY_STANDENTRY / 26
    profile.actorSpawnLegacySpawn = 0x0015;    // ZC_NOTIFY_NEWENTRY / 25
    profile.actorSpawnLegacyAlt = 0;
    profile.actorSpawnLegacyNpc = 0x0018;      // ZC_NOTIFY_STANDENTRY_NPC / 25
    profile.actorSpawnLegacyIdleShifted = 0;
    profile.actorSpawnLegacySpawnShifted = 0;
    profile.actorMoveLegacy = 0x0017;          // ZC_NOTIFY_MOVEENTRY / 32
    profile.actorMoveLegacyShifted = 0x0022;   // ZC_NOTIFY_MOVE / 16 (monster/NPC move on other clients)

    profile.actorSpawnVariableIdle = 0;
    profile.actorSpawnVariableSpawn = 0;
    profile.actorSpawnVariableIdleRobe = 0;
    profile.actorSpawnVariableSpawnRobe = 0;
    profile.actorMoveVariable = 0;
    profile.actorMoveVariableRobe = 0;
    profile.actorSpawnModernIdle = 0;
    profile.actorSpawnModernSpawn = 0;
    profile.actorSpawnModernIdleFont = 0;
    profile.actorSpawnModernSpawnFont = 0;
    profile.actorMoveModern = 0;
    profile.actorMoveModernFont = 0;

    profile.actorNameAckBasic = 0x0031;        // ZC_ACK_REQNAME / 54 (alpha 0x0030 +1, beta1 size 54)
    profile.actorNameAckParty = 0;
    profile.actorNameAckFull = 0;
    profile.actorStateChangeBasic = 0x005A;    // ZC_STATUS_CHANGE / 5 (alpha 0x0059 +1)
    profile.actorStateChangeExtended = 0x00B5; // ZC_STATE_CHANGE / 13 (new in Beta1)

    profile.partyMemberAddedBasic = 0;
    profile.partyMemberAddedExtended = 0;
    profile.partyHpUpdateBasic = 0;
    profile.partyHpUpdateExtended = 0;
    profile.partyInviteAckBasic = 0;
    profile.partyInviteAckExtended = 0;
    profile.partyInviteRequestBasic = 0;
    profile.partyInviteRequestExtended = 0;

    profile.skillDamagePositionNotify = 0x00B1; // ZC_NOTIFY_SKILL_POSITION / 35 (beta1 new)
    profile.groundSkillNotify = 0x00B3;         // ZC_NOTIFY_GROUNDSKILL / 18 (beta1 new)
    profile.skillNoDamageNotify = 0x00B6;       // ZC_USE_SKILL / 15 (beta1 new)
    profile.skillUnitSetBasic = 0x00BB;         // ZC_SKILL_ENTRY / 11 (beta1 new)
    profile.skillUnitSetExtended = 0;
    profile.skillDamageNotifyBasic = 0x00B0;    // ZC_NOTIFY_SKILL / 31 (beta1 new)
    profile.skillDamageNotifyExtended = 0;
    profile.notifyEffectBasic = 0;
    profile.notifyEffectDirect = 0;

    profile.normalInventoryListBasic = 0x003F;  // ZC_NORMAL_ITEMLIST / var (alpha 0x003E +1)
    profile.normalInventoryListCardSlots = 0;
    profile.normalInventoryListTimed = 0;
    profile.equipInventoryListBasic = 0x0040;   // ZC_EQUIPMENT_ITEMLIST / var (alpha 0x003F +1)
    profile.equipInventoryListTimed = 0;
    profile.equipInventoryListTimedOwned = 0;
    profile.normalStorageListBasic = 0x0041;    // ZC_STORE_NORMAL_ITEMLIST / var (alpha 0x0040 +1)
    profile.normalStorageListCardSlots = 0;
    profile.normalStorageListTimed = 0;
    profile.equipStorageListBasic = 0x0042;     // ZC_STORE_EQUIPMENT_ITEMLIST / var (alpha 0x0041 +1)
    profile.equipStorageListTimedOwned = 0;
    profile.storageItemAddedBasic = 0x0090;     // ZC_ADD_ITEM_TO_STORE / 32 (alpha 0x008F +1, beta1 size 32)
    profile.storageItemAddedTyped = 0;

    return profile;
}

PacketProfileSet MakePacketVer23PacketProfileSet()
{
    return {
        PacketVersionId::PacketVer23,
        "packetver23",
        MakeLoginProfile(PacketVersionId::PacketVer23, "packetver23"),
        MakeCharacterProfile(PacketVersionId::PacketVer23, "packetver23"),
        MakePacketVer23ZoneProfile(),
        MakePacketVer23Profile(),
        MakePacketVer23ReceiveProfile(),
    };
}

PacketProfileSet MakePacketVer200PacketProfileSet()
{
    return {
        PacketVersionId::PacketVer200,
        "packetver200",
        MakeLoginProfile(PacketVersionId::PacketVer200, "packetver200"),
        MakeCharacterProfile(PacketVersionId::PacketVer200, "packetver200"),
        MakePacketVer200ZoneProfile(),
        MakePacketVer200Profile(),
        MakePacketVer200ReceiveProfile(),
    };
}

PacketProfileSet MakePacketVer22PacketProfileSet()
{
    return {
        PacketVersionId::PacketVer22,
        "packetver22",
        MakeLoginProfile(PacketVersionId::PacketVer22, "packetver22"),
        MakeCharacterProfile(PacketVersionId::PacketVer22, "packetver22"),
        MakePacketVer22ZoneProfile(),
        MakePacketVer22Profile(),
        MakePacketVer22ReceiveProfile(),
    };
}

std::string NormalizeProfileToken(std::string value)
{
    value.erase(std::remove_if(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0 || ch == '_' || ch == '-';
    }), value.end());
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

ClientProfilePreset ParseClientProfilePreset(const std::string& rawValue)
{
    const std::string value = NormalizeProfileToken(rawValue);
    if (value.empty() || value == "auto") {
        return ClientProfilePreset::Auto;
    }
    if (value == "packetver23" || value == "23" || value == "modern" || value == "20080910") {
        return ClientProfilePreset::PacketVer23;
    }
    if (value == "rathena") {
        return ClientProfilePreset::PacketVer23;
    }
    if (value == "packetver22" || value == "22" || value == "pre20080910") {
        return ClientProfilePreset::PacketVer22;
    }
    if (value == "eathena") {
        return ClientProfilePreset::PacketVer22;
    }
    if (value == "legacy0072" || value == "classic" || value == "legacy") {
        return ClientProfilePreset::Legacy0072;
    }
    if (value == "sabine" || value == "sabinebeta1" || value == "beta1") {
        return ClientProfilePreset::SabineBeta1;
    }
    if (value == "alpha" || value == "sabinealpha") {
        return ClientProfilePreset::SabineAlpha;
    }
    if (value == "beta2" || value == "sabinebeta2") {
        return ClientProfilePreset::SabineBeta2;
    }
    return ClientProfilePreset::Auto;
}

ClientProfilePreset ResolveClientProfilePreset()
{
    std::string configuredProfile;
    if (TryLoadSelectedClientInfoSettingString("clientprofile", &configuredProfile)) {
        const ClientProfilePreset preset = ParseClientProfilePreset(configuredProfile);
        if (preset != ClientProfilePreset::Auto) {
            return preset;
        }
    }
    if (TryLoadSettingsIniString("Client", "Profile", &configuredProfile)) {
        const ClientProfilePreset preset = ParseClientProfilePreset(configuredProfile);
        if (preset != ClientProfilePreset::Auto) {
            return preset;
        }
    }

    int legacyPacketVersion = 0;
    if (!TryLoadSettingsIniInt("Network", "PacketVersion", &legacyPacketVersion) || legacyPacketVersion <= 0) {
        return ClientProfilePreset::Auto;
    }

    if (legacyPacketVersion == 23) {
        return ClientProfilePreset::PacketVer23;
    }
    if (legacyPacketVersion == 22) {
        return ClientProfilePreset::PacketVer22;
    }
    if (legacyPacketVersion < PacketProfile::kSabineVersionBeta1) {
        return ClientProfilePreset::SabineAlpha;
    }
    if (legacyPacketVersion < PacketProfile::kSabineVersionBeta2) {
        return ClientProfilePreset::SabineBeta1;
    }
    if (legacyPacketVersion < static_cast<int>(PacketProfile::PacketVer23LoginChain::kClientDate)) {
        return ClientProfilePreset::SabineBeta2;
    }

    return ClientProfilePreset::Auto;
}

std::string ResolveMapSendProfileToken()
{
    const ClientProfilePreset preset = ResolveClientProfilePreset();
    std::string configured;
    if (!TryLoadSelectedClientInfoSettingString("mapsendprofile", &configured)) {
        configured = LoadSettingsIniString("Packets", "MapSendProfile", "packetver23");
    }
    configured = NormalizeProfileToken(std::move(configured));
    if (configured == "inherit" || configured == "active" || configured == "mapsendprofile") {
        switch (preset) {
        case ClientProfilePreset::SabineBeta1:
            return "packetver200";
        default:
            return configured;
        }
    }
    if (configured != "packetver23" || preset == ClientProfilePreset::Auto) {
        return configured;
    }

    switch (preset) {
    case ClientProfilePreset::PacketVer22:
    case ClientProfilePreset::Legacy0072:
    case ClientProfilePreset::SabineAlpha:
        return "packetver22";
    case ClientProfilePreset::SabineBeta1:
        return "packetver200";
    case ClientProfilePreset::SabineBeta2:
        return "packetver23";
    case ClientProfilePreset::Auto:
    case ClientProfilePreset::PacketVer23:
    default:
        return configured;
    }
}

std::string ResolveZoneConnectProfileToken()
{
    const ClientProfilePreset preset = ResolveClientProfilePreset();
    std::string configured;
    if (!TryLoadSelectedClientInfoSettingString("zoneconnectprofile", &configured)) {
        configured = LoadSettingsIniString("Packets", "ZoneConnectProfile", "inherit");
    }
    configured = NormalizeProfileToken(std::move(configured));
    if (configured != "inherit" || preset == ClientProfilePreset::Auto) {
        return configured;
    }

    switch (preset) {
    case ClientProfilePreset::Legacy0072:
    case ClientProfilePreset::SabineAlpha:
        return "packetver100";
    case ClientProfilePreset::SabineBeta1:
        return "packetver200";
    case ClientProfilePreset::SabineBeta2:
        return "packetver300";
    case ClientProfilePreset::Auto:
    case ClientProfilePreset::PacketVer23:
    case ClientProfilePreset::PacketVer22:
    default:
        return configured;
    }
}

std::string ResolveGameplaySendProfileToken()
{
    const ClientProfilePreset preset = ResolveClientProfilePreset();
    std::string configured;
    if (!TryLoadSelectedClientInfoSettingString("mapgameplaysendprofile", &configured)) {
        configured = LoadSettingsIniString("Packets", "MapGameplaySendProfile", "inherit");
    }
    configured = NormalizeProfileToken(std::move(configured));
    if (configured != "inherit" || preset == ClientProfilePreset::Auto) {
        return configured;
    }

    switch (preset) {
    case ClientProfilePreset::Legacy0072:
    case ClientProfilePreset::SabineAlpha:
        return "legacy0072";
    case ClientProfilePreset::SabineBeta1:
        return "packetver200";
    case ClientProfilePreset::Auto:
    case ClientProfilePreset::PacketVer23:
    case ClientProfilePreset::PacketVer22:
    case ClientProfilePreset::SabineBeta2:
    default:
        return configured;
    }
}

const PacketProfileSet& ResolveConfiguredProfile()
{
    static const PacketProfileSet kPacketVer23 = MakePacketVer23PacketProfileSet();
    static const PacketProfileSet kPacketVer200 = MakePacketVer200PacketProfileSet();
    static const PacketProfileSet kPacketVer22 = MakePacketVer22PacketProfileSet();

    const std::string configured = ResolveMapSendProfileToken();
    if (configured == "packetver200" || configured == "200" || configured == "beta1" || configured == "sabinebeta1") {
        return kPacketVer200;
    }
    if (configured == "packetver22" || configured == "22" || configured == "legacy" || configured == "pre20080910") {
        return kPacketVer22;
    }
    return kPacketVer23;
}

const ZonePacketProfile& ResolveConfiguredZoneProfile(const PacketProfileSet& baseProfile)
{
    static const ZonePacketProfile kPacketVer23Zone = MakePacketVer23ZoneProfile();
    static const ZonePacketProfile kPacketVer100Zone = MakePacketVer100ZoneProfile();
    static const ZonePacketProfile kPacketVer22Zone = MakePacketVer22ZoneProfile();
    static const ZonePacketProfile kPacketVer200Zone = MakePacketVer200ZoneProfile();
    static const ZonePacketProfile kPacketVer300Zone = MakePacketVer300ZoneProfile();
    static const ZonePacketProfile kLegacy72Zone = MakeLegacy72ZoneProfile();
    static const ZonePacketProfile kLegacy72Padded22Zone = MakeLegacy72Padded22ZoneProfile();

    const std::string configured = ResolveZoneConnectProfileToken();
    if (configured.empty() || configured == "inherit" || configured == "active" || configured == "mapsendprofile") {
        return baseProfile.zone;
    }

    if (configured == "legacy0072padded22" || configured == "0072padded22" || configured == "packetver6"
        || configured == "22byte0072" || configured == "padded22") {
        return kLegacy72Padded22Zone;
    }

    if (configured == "packetver100" || configured == "100" || configured == "alpha" || configured == "sabinealpha") {
        return kPacketVer100Zone;
    }

    if (configured == "packetver200" || configured == "200" || configured == "beta1" || configured == "sabinebeta1") {
        return kPacketVer200Zone;
    }

    if (configured == "packetver300" || configured == "300" || configured == "beta2" || configured == "sabinebeta2") {
        return kPacketVer300Zone;
    }

    if (configured == "legacy0072" || configured == "0072" || configured == "classic" || configured == "legacy19") {
        return kLegacy72Zone;
    }

    if (configured == "packetver22" || configured == "22" || configured == "009b" || configured == "legacy26") {
        return kPacketVer22Zone;
    }

    if (configured == "packetver23" || configured == "23" || configured == "0436" || configured == "compact19") {
        return kPacketVer23Zone;
    }

    return baseProfile.zone;
}

const MapGameplaySendProfile& ResolveConfiguredGameplaySendProfile(const PacketProfileSet& baseProfile, const ZonePacketProfile& zoneProfile)
{
    static const MapGameplaySendProfile kPacketVer23Profile = MakePacketVer23Profile();
    static const MapGameplaySendProfile kPacketVer22Profile = MakePacketVer22Profile();
    static const MapGameplaySendProfile kPacketVer200Profile = MakePacketVer200Profile();
    static const MapGameplaySendProfile kLegacyProfile = MakeLegacyGameplayProfile();

    const std::string configured = ResolveGameplaySendProfileToken();
    if (configured == "legacy0072" || configured == "legacy005" || configured == "packetver5"
        || configured == "classic" || configured == "legacyworld") {
        return kLegacyProfile;
    }

    if (configured == "packetver22" || configured == "22") {
        return kPacketVer22Profile;
    }

    if (configured == "packetver200" || configured == "200" || configured == "beta1" || configured == "sabinebeta1") {
        return kPacketVer200Profile;
    }

    if (configured == "packetver23" || configured == "23") {
        return kPacketVer23Profile;
    }

    if (configured.empty() || configured == "inherit" || configured == "active" || configured == "mapsendprofile") {
        if (zoneProfile.wantToConnection == PacketProfile::LegacyMapServerSend::kWantToConnection
            && zoneProfile.wantToConnectionLayout == ZoneConnectPacketLayout::Compact19) {
            return kLegacyProfile;
        }
        return baseProfile.mapSend;
    }

    return baseProfile.mapSend;
}

struct ResolvedPacketProfiles {
    int generation = -1;
    const PacketProfileSet* packetProfile = nullptr;
    const ZonePacketProfile* zoneProfile = nullptr;
    const MapGameplaySendProfile* gameplayProfile = nullptr;
};

ResolvedPacketProfiles& GetResolvedPacketProfiles()
{
    static ResolvedPacketProfiles cache;
    const int generation = GetClientInfoStateGeneration();
    if (cache.generation == generation && cache.packetProfile && cache.zoneProfile && cache.gameplayProfile) {
        return cache;
    }

    cache.packetProfile = &ResolveConfiguredProfile();
    cache.zoneProfile = &ResolveConfiguredZoneProfile(*cache.packetProfile);
    cache.gameplayProfile = &ResolveConfiguredGameplaySendProfile(*cache.packetProfile, *cache.zoneProfile);
    cache.generation = generation;

    const MapReceiveProfile& receiveProfile = cache.packetProfile->mapReceive;
    DbgLog("[PacketProfile] active packet profile=%s\n", cache.packetProfile->name);
    DbgLog("[PacketProfile] active zone connect profile=%s opcode=0x%04X layout=%u\n",
        cache.zoneProfile->name,
        static_cast<unsigned int>(cache.zoneProfile->wantToConnection),
        static_cast<unsigned int>(cache.zoneProfile->wantToConnectionLayout));
    DbgLog("[PacketProfile] active map receive profile=%s id=%u acceptEnter=0x%04X standEntry=0x%04X\n",
        receiveProfile.name,
        static_cast<unsigned int>(receiveProfile.id),
        static_cast<unsigned int>(receiveProfile.acceptEnterLegacy),
        static_cast<unsigned int>(receiveProfile.actorSpawnLegacyIdle));
    DbgLog("[PacketProfile] active map gameplay send profile=%s\n", cache.gameplayProfile->name);
    return cache;
}

} // namespace

const PacketProfileSet& GetActivePacketProfile()
{
    return *GetResolvedPacketProfiles().packetProfile;
}

const AccountLoginPacketProfile& GetActiveAccountLoginPacketProfile()
{
    return GetActivePacketProfile().login;
}

const CharacterPacketProfile& GetActiveCharacterPacketProfile()
{
    return GetActivePacketProfile().character;
}

const ZonePacketProfile& GetActiveZonePacketProfile()
{
    return *GetResolvedPacketProfiles().zoneProfile;
}

const MapReceiveProfile& GetActiveMapReceiveProfile()
{
    return GetResolvedPacketProfiles().packetProfile->mapReceive;
}

const MapGameplaySendProfile& GetActiveMapGameplaySendProfile()
{
    return *GetResolvedPacketProfiles().gameplayProfile;
}

bool IsPacketVer22MapGameplaySendProfile()
{
    return GetActiveMapGameplaySendProfile().id == MapGameplaySendProfileId::PacketVer22;
}

bool IsPacketVer200MapGameplaySendProfile()
{
    return GetActiveMapGameplaySendProfile().id == MapGameplaySendProfileId::PacketVer200;
}

bool IsLegacyMapGameplaySendProfile()
{
    return GetActiveMapGameplaySendProfile().id == MapGameplaySendProfileId::Legacy;
}

bool IsPacketVer22PacketProfile()
{
    return GetActivePacketProfile().id == PacketVersionId::PacketVer22;
}

bool IsActiveAcceptEnterPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.acceptEnterLegacy, profile.acceptEnterModern});
}

bool IsActiveMapChangePacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.mapChangeBasic, profile.mapChangeServerMove});
}

bool IsActiveServerMoveMapChangePacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().mapChangeServerMove;
}

bool IsActiveActorActionNotifyExtendedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().actorActionNotifyExtended;
}

bool IsActiveActorSetPositionPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.actorSetPositionBasic, profile.actorSetPositionHighJump});
}

bool IsActiveBroadcastPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.broadcastBasic, profile.broadcastColored});
}

bool IsActiveColoredBroadcastPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().broadcastColored;
}

bool IsActiveGroundItemEntryPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.groundItemEntryExisting, profile.groundItemEntryDropped});
}

bool IsActiveGroundItemDroppedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().groundItemEntryDropped;
}

bool IsActiveItemPickupAckPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.itemPickupAckBasic, profile.itemPickupAckExtended});
}

bool IsActiveNormalInventoryListPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {profile.normalInventoryListBasic, profile.normalInventoryListCardSlots, profile.normalInventoryListTimed});
}

bool IsActiveEquipInventoryListPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {profile.equipInventoryListBasic, profile.equipInventoryListTimed, profile.equipInventoryListTimedOwned});
}

bool IsActiveNormalStorageListPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {profile.normalStorageListBasic, profile.normalStorageListCardSlots, profile.normalStorageListTimed});
}

bool IsActiveEquipStorageListPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.equipStorageListBasic, profile.equipStorageListTimedOwned});
}

bool IsActiveStorageItemAddedPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.storageItemAddedBasic, profile.storageItemAddedTyped});
}

bool IsActiveStorageItemAddedTypedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().storageItemAddedTyped;
}

bool IsActiveUseItemAckPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.useItemAckBasic, profile.useItemAckExtended});
}

bool IsActiveExtendedUseItemAckPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().useItemAckExtended;
}

bool IsActiveItemRemovePacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.itemRemoveBasic, profile.itemRemoveExtended});
}

bool IsActiveExtendedItemRemovePacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().itemRemoveExtended;
}

bool IsActivePartyInviteAckPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.partyInviteAckBasic, profile.partyInviteAckExtended});
}

bool IsActiveExtendedPartyInviteAckPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().partyInviteAckExtended;
}

bool IsActivePartyInviteRequestPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.partyInviteRequestBasic, profile.partyInviteRequestExtended});
}

bool IsActiveSkillUnitSetPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.skillUnitSetBasic, profile.skillUnitSetExtended});
}

int GetActiveStackableItemListEntrySize(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    if (MatchesPacketId(packetId, {profile.normalInventoryListBasic, profile.normalStorageListBasic})) {
        return 10;
    }
    if (MatchesPacketId(packetId, {profile.normalInventoryListCardSlots, profile.normalStorageListCardSlots})) {
        return 18;
    }
    if (MatchesPacketId(packetId, {profile.normalInventoryListTimed, profile.normalStorageListTimed})) {
        return 22;
    }
    return 0;
}

int GetActiveEquipmentItemListEntrySize(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    if (MatchesPacketId(packetId, {profile.equipInventoryListBasic, profile.equipStorageListBasic})) {
        return 20;
    }
    if (packetId == profile.equipInventoryListTimed) {
        return 24;
    }
    if (MatchesPacketId(packetId, {profile.equipInventoryListTimedOwned, profile.equipStorageListTimedOwned})) {
        return 26;
    }
    return 0;
}

bool IsActiveActorSpawnPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {
            profile.actorSpawnLegacyIdle,
            profile.actorSpawnLegacySpawn,
            profile.actorSpawnLegacyAlt,
            profile.actorSpawnLegacyNpc,
            profile.actorSpawnLegacyIdleShifted,
            profile.actorSpawnLegacySpawnShifted,
            profile.actorSpawnVariableIdle,
            profile.actorSpawnVariableSpawn,
            profile.actorSpawnVariableIdleRobe,
            profile.actorSpawnVariableSpawnRobe,
            profile.actorSpawnModernIdle,
            profile.actorSpawnModernSpawn,
            profile.actorSpawnModernIdleFont,
            profile.actorSpawnModernSpawnFont,
        });
}

bool IsActiveLegacyActorSpawnPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {
            profile.actorSpawnLegacyIdle,
            profile.actorSpawnLegacySpawn,
            profile.actorSpawnLegacyAlt,
            profile.actorSpawnLegacyNpc,
            profile.actorSpawnLegacyIdleShifted,
            profile.actorSpawnLegacySpawnShifted,
        });
}

bool IsActiveActorMovePacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId,
        {
            profile.actorMoveLegacy,
            profile.actorMoveLegacyShifted,
            profile.actorMoveVariable,
            profile.actorMoveVariableRobe,
            profile.actorMoveModern,
            profile.actorMoveModernFont,
        });
}

bool IsActiveLegacyActorMovePacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.actorMoveLegacy, profile.actorMoveLegacyShifted});
}

bool IsActiveActorNameAckPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.actorNameAckBasic, profile.actorNameAckParty, profile.actorNameAckFull});
}

bool IsActiveActorNameAckFullPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().actorNameAckFull;
}

bool IsActiveActorStateChangeExtendedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().actorStateChangeExtended;
}

bool IsActivePartyMemberAddedPacket(u16 packetId)
{
    const MapReceiveProfile& profile = GetActiveMapReceiveProfile();
    return MatchesPacketId(packetId, {profile.partyMemberAddedBasic, profile.partyMemberAddedExtended});
}

bool IsActivePartyMemberAddedExtendedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().partyMemberAddedExtended;
}

bool IsActivePartyHpUpdateExtendedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().partyHpUpdateExtended;
}

bool IsActiveSkillDamageNotifyExtendedPacket(u16 packetId)
{
    return packetId == GetActiveMapReceiveProfile().skillDamageNotifyExtended;
}

bool BuildActiveWantToConnectionPacket(u32 aid,
    u32 gid,
    u32 authCode,
    u32 clientTick,
    u8 sex,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize <= 0) {
        return false;
    }

    const ZonePacketProfile& profile = GetActiveZonePacketProfile();
    if (profile.wantToConnectionLayout == ZoneConnectPacketLayout::Legacy26) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_ENTER_PACKETVER22))) {
            return false;
        }

        PACKET_CZ_ENTER_PACKETVER22 packet{};
        packet.PacketType = profile.wantToConnection;
        FillPacketPadding(packet.padding0, static_cast<int>(sizeof(packet.padding0)));
        packet.AID = aid;
        FillPacketPadding(&packet.padding1, 1);
        packet.GID = gid;
        FillPacketPadding(packet.padding2, static_cast<int>(sizeof(packet.padding2)));
        packet.AuthCode = authCode;
        packet.ClientTick = clientTick;
        packet.Sex = sex;
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (profile.wantToConnectionLayout == ZoneConnectPacketLayout::Padded22) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_ENTER_PADDED22))) {
            return false;
        }

        PACKET_CZ_ENTER_PADDED22 packet{};
        packet.PacketType = profile.wantToConnection;
        FillPacketPadding(packet.padding, static_cast<int>(sizeof(packet.padding)));
        packet.AID = aid;
        packet.GID = gid;
        packet.AuthCode = authCode;
        packet.ClientTick = clientTick;
        packet.Sex = sex;
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_ENTER2))) {
        return false;
    }

    PACKET_CZ_ENTER2 packet{};
    packet.PacketType = profile.wantToConnection;
    packet.AID = aid;
    packet.GID = gid;
    packet.AuthCode = authCode;
    packet.ClientTick = clientTick;
    packet.Sex = sex;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveActionRequestPacket(u32 targetGid,
    u8 action,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize <= 0) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    if (profile.id == MapGameplaySendProfileId::PacketVer22) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_ACTION_REQUEST_PACKETVER22))) {
            return false;
        }

        PACKET_CZ_ACTION_REQUEST_PACKETVER22 packet{};
        packet.PacketType = profile.actionRequest;
        FillPacketPadding(packet.padding0, static_cast<int>(sizeof(packet.padding0)));
        packet.TargetGID = targetGid;
        FillPacketPadding(packet.padding1, static_cast<int>(sizeof(packet.padding1)));
        packet.Action = action;
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_ACTION_REQUEST2))) {
        return false;
    }

    PACKET_CZ_ACTION_REQUEST2 packet{};
    packet.PacketType = profile.actionRequest;
    packet.TargetGID = targetGid;
    packet.Action = action;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveUseSkillToIdPacket(u16 skillId,
    u16 skillLevel,
    u32 targetGid,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize <= 0) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    if (profile.id == MapGameplaySendProfileId::PacketVer22) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_USESKILLTOID_PACKETVER22))) {
            return false;
        }

        PACKET_CZ_USESKILLTOID_PACKETVER22 packet{};
        packet.PacketType = profile.useSkillToId;
        FillPacketPadding(packet.padding0, static_cast<int>(sizeof(packet.padding0)));
        packet.SkillId = skillId;
        packet.SkillLevel = skillLevel;
        FillPacketPadding(packet.padding1, static_cast<int>(sizeof(packet.padding1)));
        FillPacketPadding(packet.padding2, static_cast<int>(sizeof(packet.padding2)));
        packet.TargetGID = targetGid;
        FillPacketPadding(&packet.padding3, 1);
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_USESKILLTOID2))) {
        return false;
    }

    PACKET_CZ_USESKILLTOID2 packet{};
    packet.PacketType = profile.useSkillToId;
    packet.SkillId = skillId;
    packet.SkillLevel = skillLevel;
    packet.TargetGID = targetGid;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveCartOffPacket(void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize < static_cast<int>(sizeof(PACKET_CZ_REQ_CARTOFF))) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    PACKET_CZ_REQ_CARTOFF packet{};
    packet.PacketType = profile.cartOff;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveChangeCartPacket(u16 type,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize < static_cast<int>(sizeof(PACKET_CZ_REQ_CHANGECART))) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    PACKET_CZ_REQ_CHANGECART packet{};
    packet.PacketType = profile.changeCart;
    packet.Type = type;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveUseSkillMapPacket(u16 skillId,
    const char* mapName,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize < static_cast<int>(sizeof(PACKET_CZ_USESKILLMAP)) || !mapName) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    PACKET_CZ_USESKILLMAP packet{};
    packet.PacketType = profile.useSkillMap;
    packet.SkillId = skillId;
    std::strncpy(packet.MapName, mapName, sizeof(packet.MapName) - 1);
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveUseItemPacket(u16 itemIndex,
    u32 targetAid,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize <= 0) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    if (profile.id == MapGameplaySendProfileId::PacketVer22) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_USEITEM_PACKETVER22))) {
            return false;
        }

        PACKET_CZ_USEITEM_PACKETVER22 packet{};
        packet.PacketType = profile.useItem;
        FillPacketPadding(packet.padding0, static_cast<int>(sizeof(packet.padding0)));
        packet.ItemIndex = itemIndex;
        FillPacketPadding(packet.padding1, static_cast<int>(sizeof(packet.padding1)));
        packet.TargetAID = targetAid;
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_USEITEM2))) {
        return false;
    }

    PACKET_CZ_USEITEM2 packet{};
    packet.PacketType = profile.useItem;
    packet.ItemIndex = itemIndex;
    packet.TargetAID = targetAid;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

bool BuildActiveTakeItemPacket(u32 objectAid,
    void* outBuffer,
    int outBufferSize,
    int* outPacketLength)
{
    if (!outBuffer || !outPacketLength || outBufferSize <= 0 || objectAid == 0) {
        return false;
    }

    const MapGameplaySendProfile& profile = GetActiveMapGameplaySendProfile();
    if (profile.id == MapGameplaySendProfileId::Legacy || profile.id == MapGameplaySendProfileId::PacketVer200) {
        if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_TAKE_ITEM_LEGACY))) {
            return false;
        }

        PACKET_CZ_TAKE_ITEM_LEGACY packet{};
        packet.PacketType = profile.takeItem;
        packet.ObjectAID = objectAid;
        std::memcpy(outBuffer, &packet, sizeof(packet));
        *outPacketLength = static_cast<int>(sizeof(packet));
        return true;
    }

    if (outBufferSize < static_cast<int>(sizeof(PACKET_CZ_TAKE_ITEM2))) {
        return false;
    }

    PACKET_CZ_TAKE_ITEM2 packet{};
    packet.PacketType = profile.takeItem;
    packet.padding = 0;
    packet.ObjectAID = objectAid;
    std::memcpy(outBuffer, &packet, sizeof(packet));
    *outPacketLength = static_cast<int>(sizeof(packet));
    return true;
}

} // namespace ro::net
