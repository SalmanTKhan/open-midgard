#pragma once
//===========================================================================
// GronPacket.h  –  Ragnarok packet-size table and helpers
//===========================================================================
#include "Types.h"

namespace ro::net {

constexpr s16 kVariablePacketSize = -1;

// Initialize the known packet-size tables for the supported receive profiles.
void InitializePacketSize();
void ReloadPacketSizeForVersion(int clientVersion);
void ReloadPacketSizeForVersion(int clientVersion);

// Returns:
//  > 0  fixed packet byte length
// == -1 variable length (size is encoded in bytes 2..3)
// == 0  unknown / unregistered packet id
s16 GetPacketSize(u16 packetId);

bool IsVariableLengthPacket(u16 packetId);

} // namespace ro::net
