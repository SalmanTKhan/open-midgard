# rAthena <-> OpenMidgard Compatibility Patch Notes

This document lists the source-level edits that were required to make rAthena interoperate with the old `2008-09-10aSakexe` client behavior we were matching against OpenMidgard.


## Packet Version Setup

Set these in `src/custom/defines_pre.hpp`:

```cpp
#define PACKETVER 20080910
#define PACKETVER_SAK_NUM 20080910
```

### What these do

- `PACKETVER 20080910` selects the general packet family for `2008-09-10aSakexe`.
- `PACKETVER_SAK_NUM 20080910` is also required so the code can distinguish this old Sakexe client from newer packet layouts that still fall under older `PACKETVER` ranges.

Without `PACKETVER_SAK_NUM`, the server still compiles parts of the newer character-info layout and sends fields the client does not decode correctly.

## Source Patches

### 1. `src/common/packets.hpp`

Add a legacy Sakexe guard before `CHARACTER_INFO`:

```cpp
#if defined(PACKETVER_SAK_NUM) && PACKETVER_SAK_NUM > 0 && PACKETVER_SAK_NUM < 20090225
    #define PACKETVER_SAK_LEGACY_CHARINFO
#endif
```

Then make `CHARACTER_INFO` use the old 108-byte layout for this client:

```cpp
struct CHARACTER_INFO{
    uint32 GID;
#if PACKETVER >= 20170830
    int64 exp;
#else
    int32 exp;
#endif
    int32 money;
#if PACKETVER >= 20170830
    int64 jobexp;
#else
    int32 jobexp;
#endif
    int32 joblevel;
    int32 bodystate;
    int32 healthstate;
    int32 effectstate;
    int32 virtue;
    int32 honor;
    int16 jobpoint;
#ifdef PACKETVER_SAK_LEGACY_CHARINFO
    int16 hp;
    int16 maxhp;
    int16 sp;
    int16 maxsp;
#else
#if PACKETVER_RE_NUM >= 20211103 || PACKETVER_MAIN_NUM >= 20220330
    int64 hp;
    int64 maxhp;
    int64 sp;
    int64 maxsp;
#else
    int32 hp;
    int32 maxhp;
    int16 sp;
    int16 maxsp;
#endif
#endif
    int16 speed;
    int16 job;
    int16 head;
#if PACKETVER >= 20141022
    int16 body;
#endif
    int16 weapon;
    int16 level;
    int16 sppoint;
    int16 accessory;
    int16 shield;
    int16 accessory2;
    int16 accessory3;
    int16 headpalette;
    int16 bodypalette;
    char name[24];
    uint8 Str;
    uint8 Agi;
    uint8 Vit;
    uint8 Int;
    uint8 Dex;
    uint8 Luk;
#ifdef PACKETVER_SAK_LEGACY_CHARINFO
    int16 CharNum;
    int16 hairColor;
#else
    uint8 CharNum;
    uint8 hairColor;
    int16 bIsChangedCharName;
#endif
#if (PACKETVER >= 20100720 && PACKETVER <= 20100727) || PACKETVER >= 20100803
    char mapName[16];
#endif
#if PACKETVER >= 20100803
    int32 DelRevDate;
#endif
#if PACKETVER >= 20110111
    int32 robePalette;
#endif
#if PACKETVER >= 20110928
    int32 chr_slot_changeCnt;
#endif
#if PACKETVER >= 20111025
    int32 chr_name_changeCnt;
#endif
#if PACKETVER >= 20141016
    uint8 sex;
#endif
} __attribute__((packed));

#ifdef PACKETVER_SAK_LEGACY_CHARINFO
static_assert( sizeof( CHARACTER_INFO ) == 108, "Legacy Sakexe CHARACTER_INFO size mismatch" );
#endif
```

### Why this patch was needed

The old Sakexe client expects the classic 108-byte character block in the character list and character-create success packets. rAthena was otherwise sending a newer layout, which caused the client to decode the slot, class visuals, stats, and name fields incorrectly.

### 2. `src/char/char.cpp`

Inside `char_mmo_char_tobuf`, serialize the legacy fields with the old widths when `PACKETVER_SAK_LEGACY_CHARINFO` is active:

```cpp
#ifdef PACKETVER_SAK_LEGACY_CHARINFO
    info.hp = u32min( p.hp, INT16_MAX );
    info.maxhp = u32min( p.max_hp, INT16_MAX );
    info.sp = u32min( p.sp, INT16_MAX );
    info.maxsp = u32min( p.max_sp, INT16_MAX );
#else
    info.hp = p.hp;
    info.maxhp = p.max_hp;
    info.sp = min( p.sp, INT16_MAX );
    info.maxsp = min( p.max_sp, INT16_MAX );
#endif
```

Also serialize slot and hair color using the legacy field sizes and omit the rename flag in the old path:

```cpp
#ifdef PACKETVER_SAK_LEGACY_CHARINFO
    info.CharNum = p.slot;
    info.hairColor = u16min( p.hair_color, INT16_MAX );
#else
    info.CharNum = p.slot;
    info.hairColor = (uint8)u16min( p.hair_color, UINT8_MAX );
    info.bIsChangedCharName = ( p.rename > 0 ) ? 0 : 1;
#endif
```

### Why this patch was needed

Changing the struct definition alone is not enough. The serializer must write values using the old field sizes, or the packet layout still becomes invalid for the client.

### 3. `src/char/char_clif.cpp`

In `chclif_block_character`, do not send `HC_BLOCK_CHARACTER (0x020D)` to the old Sakexe client:

```cpp
void chclif_block_character( int32 fd, char_session_data& sd){
#ifdef PACKETVER_SAK_LEGACY_CHARINFO
    return;
#endif

    time_t now = time( nullptr );
    ...
}
```

### Why this patch was needed

This client does not handle that packet correctly. Suppressing it avoids sending a character-server packet that belongs to later client behavior.

### 4. `src/map/clif.cpp`

Adjust `clif_refresh_clothcolor` to use const-correct parameters:

```cpp
void clif_refresh_clothcolor( const block_list& bl, enum send_target target, const block_list* tbl = nullptr ){
#if PACKETVER < 20091103
    const view_data* vd = status_get_viewdata( &bl );

    if( vd == nullptr ){
        return;
    }

    if( vd->look[LOOK_CLOTHES_COLOR] == 0 ){
        return;
    }

    if( tbl == nullptr ){
        tbl = &bl;
    }

    clif_sprite_change( tbl, bl.id, LOOK_CLOTHES_COLOR, vd->look[LOOK_CLOTHES_COLOR], 0, target );
#endif
}
```

### Why this patch was needed

The older packet path exposed a compile-time mismatch in this helper. This change does not alter protocol behavior; it fixes the source so the old-client code path builds cleanly.

## Resulting Behavior

With the packet version defines set to:

```cpp
#define PACKETVER 20080910
#define PACKETVER_SAK_NUM 20080910
```

and the four source edits above applied:

- character creation uses the correct old Sakexe packet family
- character list packets use the expected 108-byte `CHARACTER_INFO` layout
- character slot and appearance data decode correctly on the client
- unsupported newer char-server packets are not sent to this client

This is the source patch set required on the rAthena side for the old client compatibility work that was completed.