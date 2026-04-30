#include "GronPacket.h"

#include "PacketRegistry.h"

namespace ro::net {

void InitializePacketSize()
{
    // Tables are constructed lazily on first access via PacketRegistry. This
    // entry point is retained because callers (e.g. Connection.cpp) still
    // invoke it; touching the active table here forces eager construction.
    (void)GetActiveVersionTable();
}

s16 GetPacketSize(u16 packetId)
{
    return GetActiveVersionTable().byOpcode[packetId].size;
}

bool IsVariableLengthPacket(u16 packetId)
{
    return GetPacketSize(packetId) == kVariablePacketSize;
}

} // namespace ro::net
