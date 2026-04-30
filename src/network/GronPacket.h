#pragma once
//===========================================================================
// GronPacket.h  -  thin wrapper over the per-version PacketRegistry.
// The actual size tables live in PacketRegistry.{h,cpp}.
//===========================================================================
#include "Types.h"

namespace ro::net {

constexpr s16 kVariablePacketSize = -1;

// Forces eager construction of the active version's packet table. Optional —
// GetPacketSize() will lazily build on first access.
void InitializePacketSize();

// Returns:
//  > 0  fixed packet byte length
// == -1 variable length (size encoded in bytes 2..3 of the packet)
// == 0  unknown / unregistered packet id for the active version
s16 GetPacketSize(u16 packetId);

bool IsVariableLengthPacket(u16 packetId);

} // namespace ro::net
