# Packet Version Alignment

## Decision

Chosen client-originating map-server profile: `packet_ver 23` from `Ref/RunningServer/packet_db.txt` (`2008-09-10aSakexe`).

This is the intended target for packets the client sends to the zone/map server.

## Scope Boundary

In scope for this migration:
- map-server client-originating packets whose meaning depends on `packet_ver`
- packet creation and serialization in `LoginMode.cpp`, `GameMode.cpp`, and related helpers
- `src/network/GronPacket.cpp` only where receive-side compatibility must be kept aligned with what the server actually sends

Out of scope for the first migration pass:
- legacy plain account-server login packet `0x0064` as the only account-auth mechanism
- char-server enter packet `0x0065`
- char select packet `0x0066`
- unrelated packet families not currently emitted by the rebuilt client

Receive-side rule:
- do not force the receive table to a pure packet_ver 23 fantasy
- keep receive parsing aligned with the actual server-outgoing family observed from `Ref/eAthena_src_2011`
- current evidence still matches the older actor stream used with `PACKETVER <= 20081217`, including `0x0078=55`, `0x007C=42`, `0x022C=65`, `0x02EC=67`, `0x02ED=59`, `0x02EE=60`

Runtime override rule:
- `Packets.MapSendProfile` is now the runtime packet-profile selector for the active login/char/zone/map stack abstraction
- current implementation switches zone-entry handoff as well, using a semantic builder that emits `0x0436 / 19`, `0x009B / 26`, `0x0072 / 19`, or `0x0072 / 22` depending on the resolved zone-connect profile
- `Packets.MapGameplaySendProfile` can now override only the in-world client-send family; when left at `inherit`, a `legacy0072` compact zone handshake automatically selects the legacy packet_ver 5 gameplay send profile for time sync, walk, direction, and name-request packets
- receive parsing is still only partially profile-aware; keep receive-table decisions grounded in observed server output rather than assuming the client-send profile fully determines them

## Why This Target

`Ref/RunningServer/packet_db.txt` shows that the effective `packet_ver 23` map-server client-send profile switches these core requests:

| Function | packet_ver 22 | packet_ver 23 |
|---|---:|---:|
| wanttoconnection | `0x009B / 26` | `0x0436 / 19` |
| actionrequest | `0x0190 / 19` | `0x0437 / 7` |
| useskilltoid | `0x0072 / 25` | `0x0438 / 10` |
| useitem | `0x009F / 14` | `0x0439 / 8` |
| walktoxy | `0x00A7 / 8` | `0x00A7 / 8` |
| changedir | `0x0085 / 11` | `0x0085 / 11` |
| ticksend | `0x0089 / 8` | `0x0089 / 8` |
| getcharnamerequest | `0x008C / 11` | `0x008C / 11` |
| globalmessage | `0x00F3 / -1` | `0x00F3 / -1` |

For `changedir`, `packet_ver 23` inherits the pre-Renewal Sakexe layout from `packet_ver 22` rather than defining a new override, so the effective byte positions remain `7:10` within the `11` byte packet.

`Ref/eAthena_src_2011/map/clif.c` also shows that packet version is inferred from the WantToConnection opcode and layout, including `0x0072 (CZ_ENTER)` versus `0x0436 (CZ_ENTER2)`.

## Current Emitted Packet Matrix

These are the client-originating packets currently emitted by the rebuilt client.

### Account / Char Server

| Purpose | Current | Status |
|---|---:|---|
| account login negotiation | `0x01DB / 2` | aligned to RunningServer encrypted auth challenge request |
| account login response | `0x01DD / 47` | used when the server answers with `0x01DC`; fallback to `0x0064 / 55` retained for legacy servers |
| char server enter | `0x0065 / 17` | keep as-is for this migration pass |
| char select | `0x0066 / 3` | keep as-is for this migration pass |

### Map Server

| Purpose | Current client | packet_ver 23 target | Status |
|---|---:|---:|---|
| wanttoconnection | `0x0436 / 19` | `0x0436 / 19` | aligned |
| load end ack | `0x007D / 2` | legacy packet, not remapped here | intentional legacy |
| ticksend | `0x0089 / 8` | `0x0089 / 8` | aligned |
| walktoxy | `0x00A7 / 8` | `0x00A7 / 8` | aligned |
| getcharnamerequest | `0x008C / 11` | `0x008C / 11` | aligned |
| global chat | `0x00F3 / -1` | `0x00F3 / -1` | aligned |

Not yet emitted by the rebuilt client, but must follow `packet_ver 23` once implemented or migrated:
- `actionrequest -> 0x0437 / 7`
- `useskilltoid -> 0x0438 / 10`
- `useitem -> 0x0439 / 8`

## Matrix Buckets

Already correct or now aligned:
- `wanttoconnection -> 0x0436 / 19`
- `ticksend -> 0x0089 / 8`
- `walktoxy -> 0x00A7 / 8`
- `getcharnamerequest -> 0x008C / 11`
- `globalmessage -> 0x00F3 / -1`
- `notifyactorinit -> 0x007D / 2` kept intentionally until runtime validation proves otherwise

Still missing or not yet implemented in the rebuilt client:
- `actionrequest -> 0x0437 / 7`
- `useskilltoid -> 0x0438 / 10`
- `useitem -> 0x0439 / 8`

Runtime send-profile support:
- `Packets.MapSendProfile=packetver23` keeps the intended packet_ver 23 gameplay send path
- `Packets.MapSendProfile=packetver22` now also switches the zone-entry handoff packet layout through the active packet-profile abstraction
- version-sensitive gameplay packets are migrating behind semantic builders in `src/network/MapSendProfile.*` so `GameMode.cpp` no longer needs to branch on raw packet layouts inline

Current validation aid:
- `src/network/Connection.cpp` now logs the first 24 receive packets on each fresh connection so map-server handshake acceptance and returned packet family can be confirmed from `debug_hp_<pid>.log`

## First Implementation Order

1. Introduce a central packet-profile definition for map-server client-send packets.
2. Migrate WantToConnection first so the server can classify the client as packet_ver 23.
3. Migrate `ticksend`, `walktoxy`, `getcharnamerequest`, and `globalmessage` immediately after handshake.
4. Re-test what the server sends back before changing receive parsing.
5. Only then migrate additional gameplay packets such as action, skill, and item requests.

## Validation Result

Validated with live logs from `D:\Spel\OldRO\debug_hp_55264.log` and `D:\Spel\OldRO\debug_hp_3524.log`.

What is now confirmed:
- RunningServer accepts `CZ_ENTER2 0x0436` and still returns `ZC_ACCEPT_ENTER 0x0073` successfully.
- The migrated client can login, reach char select, enter map, move, send tick sync, send name requests, and send global chat under the packet_ver 23 send profile.
- Remote players are now visible again; the second client log shows remote player `2000002` arriving through `0x02EE len=60` and being created as a runtime `CPc`.
- The server-outgoing actor family is still mixed older eAthena-style actor traffic, including `0x0078 len=55` and `0x02EE len=60`, which matches the earlier receive-side assumption.

Still open after this validation:
- Receive-side support is still incomplete for at least `0x02D0` and packet handling around `0x0104`; those packets are still causing unknown-packet drops and stream damage during map load.
- Because the stream recovers and players are visible again, these missing packets are now a targeted receive-table/handler cleanup task rather than a packet-version bootstrap blocker.

Follow-up applied after validation:
- `0x02D0`, `0x02D1`, and `0x02D2` were added to `src/network/GronPacket.cpp` as variable-length packets so the receive queue no longer desyncs on that family.
- `0x0104` and the `0x02D0`-`0x02D2` family were registered as safe ignore packets in `src/gamemode/GameModePacket.cpp` until a real gameplay/UI consumer is needed.

## Receive Revalidation

Validated with fresh logs from `D:\Spel\OldRO\debug_hp_68972.log` and `D:\Spel\OldRO\debug_hp_55684.log`.

What is now confirmed:
- `0x02D0` is no longer reported as unknown and no longer causes the receive stream to desync during map load.
- Remote-player bootstrap still works after the receive-table fix; one client sees the other arrive through `0x02ED len=59` and the other through `0x02EE len=60`.
- Out-of-sight preserve/restore still works after the packet-table change, with the remote player hidden on `0x0080` and restored locally until a later hard vanish.
- A later server move to `prontera` also completed cleanly in the new logs, so the current receive-side changes did not regress map transitions.

Follow-up applied after revalidation:
- map-change reconnect was aligned to the active map-server send profile as well, so server moves no longer fall back to legacy `CZ_ENTER 0x0072` while initial zone entry uses `CZ_ENTER2 0x0436`.
- explicit packet_ver 23 packet structs were added for the remaining send-side gameplay families: `actionrequest`, `useskilltoid`, and `useitem`.

## Files To Touch First

- `src/network/Packet.h`
- `src/gamemode/LoginMode.cpp`
- `src/gamemode/GameMode.cpp`
- `src/network/GronPacket.cpp` only if receive-side validation proves a missing size or wrong assumption
## Sabine Beta1 profile (packet_ver 200)

OpenMidgard optionally speaks the **iRO Beta1 (2002-02-20) Ragexe** protocol that
the Sabine server (<https://github.com/exectails/Sabine>) emulates. Activate it via
either `data/clientinfo.xml` attribute `clientprofile="beta1"` or
`settings.ini` `[Client] Profile=beta1`. Accepted aliases: `sabine`,
`sabinebeta1`, `200`, `packetver200`.

Resolution flow:

- `Client.Profile=beta1` → `ClientProfilePreset::SabineBeta1`
- `Packets.MapSendProfile=inherit` → `packetver200` (see `ResolveMapSendProfileToken`)
- `Packets.ZoneConnectProfile=inherit` → `packetver200` → `CZ_ENTER = 0x000E/19`
- `Packets.MapGameplaySendProfile=inherit` → `packetver200` (`EarlyMapServerSend::*`)
- Receive table: `MakePacketVer200ReceiveProfile` in `src/network/MapSendProfile.cpp`
  (standalone, does **not** inherit PV23)
- Framer size table: `g_packetSizePacketVer200` in `src/network/GronPacket.cpp`,
  built from the shared fill + `ApplyPacketVer200Overrides` for the
  `0x0058`/`0x00B0..0x00BB` range that collides with PV23 semantics

Key inbound opcode/size map (Alpha numbers, Beta1 resized payloads):

| Opcode | Name | Size |
|---|---|---|
| `0x000F` | ZC_ACCEPT_ENTER | 11 |
| `0x0014` | ZC_NOTIFY_STANDENTRY | 26 |
| `0x0015` | ZC_NOTIFY_NEWENTRY | 25 |
| `0x0017` | ZC_NOTIFY_MOVEENTRY | 32 |
| `0x0018` | ZC_NOTIFY_STANDENTRY_NPC | 25 |
| `0x001B` | ZC_NOTIFY_TIME | 6 |
| `0x001C` | ZC_NOTIFY_VANISH | 7 |
| `0x0022` | ZC_NOTIFY_MOVE | 16 |
| `0x0023` | ZC_NOTIFY_PLAYERMOVE | 12 |
| `0x0026` | ZC_NOTIFY_ACT | 27 |
| `0x0027` | ZC_NOTIFY_ACT_POSITION | 23 |
| `0x002C` | ZC_NPCACK_MAPMOVE | 22 |
| `0x0030` | ZC_ACK_REQNAME | 54 |
| `0x0038` | ZC_ITEM_ENTRY | 38 |
| `0x003B` | ZC_ITEM_PICKUP_ACK | 33 |
| `0x0058` | ZC_STATUS | 44 |

Disconnect: client sends `CZ_REQUEST_QUIT = 0x001E` (2 bytes, opcode only) via
`PacketProfile::GetQuitGameOpcode()` whenever `UsesEarlyMapServerSendProfile()`
is true, because the Sabine packet table has no entry for the modern
`0x018A CZ_QUITGAME` and throws `ArgumentException` on receipt.

Known gaps (follow-up):
- The legacy `HandleActorSpawn/MoveSkeleton` decoders still read PV23-ish offsets,
  so Beta1 remote actors currently land at approximate — not exact — positions
  until the handlers grow a `PacketVersionId::PacketVer200` branch that decodes
  the 26/25/32-byte Beta1 layouts from `Sabine/src/ZoneServer/Network/Helpers/Character.cs`.
- `MakePacketVer200ReceiveProfile` leaves party/chatroom/trade/storage opcodes
  at `0`; those subsystems are silent-consumed rather than rendered.

