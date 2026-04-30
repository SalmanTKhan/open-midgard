#pragma once
//===========================================================================
// PacketRegistry.h - per-version packet table.
//===========================================================================
// Stage 0 wired the size tables. Stage 1 (this file) adds a semantic-name
// layer so consumers can ask "what opcode is `WalkToXY` in the active
// version?" instead of branching on `MapGameplaySendProfileId`. Existing
// consumers still use the explicit profile structs in MapSendProfile.h; the
// semantic layer is populated but not yet consulted.
//===========================================================================
#include "MapSendProfile.h"
#include "Types.h"

#include <array>
#include <unordered_map>

namespace ro::net {

//---------------------------------------------------------------------------
// SemanticPacket - stable, version-agnostic packet identity. Names mirror
// fields on AccountLoginPacketProfile / CharacterPacketProfile /
// ZonePacketProfile / MapGameplaySendProfile / MapReceiveProfile so the
// migration is mechanical when consumers move over.
//---------------------------------------------------------------------------
enum class SemanticPacket : u16 {
    kUnknown = 0,

    // Account login chain
    AccountLogin,
    AccountLoginChannel,
    RequestPasswordHash,
    PasswordHashChallenge,
    PasswordHashLogin,
    ConnectInfoChanged,
    ExeHashCheck,
    NotifyError,

    // Character server
    CharServerEnter,
    SelectCharacter,
    MakeCharacter,
    DeleteCharacter,

    // Zone handshake
    WantToConnection,
    QuitGame,

    // Map gameplay send
    ActionRequest,
    UseSkillToId,
    CartOff,
    ChangeCart,
    UseSkillToPos,
    UseSkillMap,
    UseItem,
    TakeItem,
    DropItem,
    ItemCompositionList,
    ItemComposition,
    ItemIdentify,
    SkillUp,
    EquipItem,
    UnequipItem,
    WalkToXY,
    ChangeDir,
    TickSend,
    NotifyActorInit,
    GetCharNameRequest,
    Whisper,
    GlobalMessage,
    ReqEmotion,
    Emotion,

    // Map receive - lifecycle / world state
    AcceptEnterLegacy,
    AcceptEnterModern,
    NotifyTime,
    MapChangeBasic,
    MapChangeServerMove,
    SelfMoveAck,
    BroadcastBasic,
    BroadcastColored,

    // Map receive - actor action / position
    ActorActionNotifyBasic,
    ActorActionNotifyExtended,
    ActorSetPositionBasic,
    ActorSetPositionHighJump,

    // Map receive - ground items
    GroundItemEntryExisting,
    GroundItemEntryDropped,
    ItemPickupAckBasic,
    ItemPickupAckExtended,

    // Map receive - inventory / storage lists
    NormalInventoryListBasic,
    NormalInventoryListCardSlots,
    NormalInventoryListTimed,
    EquipInventoryListBasic,
    EquipInventoryListTimed,
    EquipInventoryListTimedOwned,
    NormalStorageListBasic,
    NormalStorageListCardSlots,
    NormalStorageListTimed,
    EquipStorageListBasic,
    EquipStorageListTimedOwned,
    StorageItemAddedBasic,
    StorageItemAddedTyped,

    // Map receive - item interactions
    UseItemAckBasic,
    UseItemAckExtended,
    ItemRemoveBasic,
    ItemRemoveExtended,

    // Map receive - party
    PartyInviteAckBasic,
    PartyInviteAckExtended,
    PartyInviteRequestBasic,
    PartyInviteRequestExtended,
    PartyMemberAddedBasic,
    PartyMemberAddedExtended,
    PartyHpUpdateBasic,
    PartyHpUpdateExtended,

    // Map receive - skills
    SkillDamagePositionNotify,
    GroundSkillNotify,
    SkillNoDamageNotify,
    SkillUnitSetBasic,
    SkillUnitSetExtended,
    SkillDamageNotifyBasic,
    SkillDamageNotifyExtended,
    NotifyEffectBasic,
    NotifyEffectDirect,

    // Map receive - actor spawn / move (legacy / variable / modern)
    ActorSpawnLegacyIdle,
    ActorSpawnLegacySpawn,
    ActorSpawnLegacyAlt,
    ActorSpawnLegacyNpc,
    ActorSpawnLegacyIdleShifted,
    ActorSpawnLegacySpawnShifted,
    ActorMoveLegacy,
    ActorMoveLegacyShifted,
    ActorSpawnVariableIdle,
    ActorSpawnVariableSpawn,
    ActorSpawnVariableIdleRobe,
    ActorSpawnVariableSpawnRobe,
    ActorMoveVariable,
    ActorMoveVariableRobe,
    ActorSpawnModernIdle,
    ActorSpawnModernSpawn,
    ActorSpawnModernIdleFont,
    ActorSpawnModernSpawnFont,
    ActorMoveModern,
    ActorMoveModernFont,

    // Map receive - actor name / state
    ActorNameAckBasic,
    ActorNameAckParty,
    ActorNameAckFull,
    ActorStateChangeBasic,
    ActorStateChangeExtended,

    kCount,
};

struct PacketDef {
    u16 opcode = 0;
    s16 size = 0;
    SemanticPacket semantic = SemanticPacket::kUnknown;
};

struct VersionTable {
    PacketVersionId id;
    std::array<PacketDef, 0x10000> byOpcode{};
    std::unordered_map<SemanticPacket, u16> byName;
};

const VersionTable& GetVersionTable(PacketVersionId id);
const VersionTable& GetActiveVersionTable();

// 0 means "this version does not define an opcode for this semantic".
u16 GetOpcode(const VersionTable& table, SemanticPacket sem);
u16 GetActiveOpcode(SemanticPacket sem);

// SemanticPacket::kUnknown means "this opcode has no registered semantic".
SemanticPacket GetSemantic(const VersionTable& table, u16 opcode);
SemanticPacket GetActiveSemantic(u16 opcode);

} // namespace ro::net
