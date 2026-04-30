#include "PacketRegistry.h"

#include "GronPacket.h"
#include "MapSendProfile.h"

namespace ro::net {
namespace {

void Set(VersionTable& t, u16 op, s16 size)
{
    t.byOpcode[op].opcode = op;
    t.byOpcode[op].size = size;
}

// Bind a semantic name to an opcode in this version's table. Both directions
// (byOpcode[op].semantic and byName[sem]) are kept in sync so consumers can
// query either way.
void Bind(VersionTable& t, u16 op, SemanticPacket sem)
{
    t.byOpcode[op].opcode = op;
    t.byOpcode[op].semantic = sem;
    t.byName[sem] = op;
}

//===========================================================================
// PV23 — packet_ver 23 (2008-09-10aSakexe).
// Mirrors the previous FillPacketSizeTable contents verbatim. Alpha/Beta1
// low-numbered opcodes are preserved here because they don't collide with
// any PV23 packet and the legacy login/char chain still leans on them.
//===========================================================================
void PopulatePv23(VersionTable& t)
{
    Set(t, 0x0000, 54);
    Set(t, 0x0001, 17);
    Set(t, 0x0002, 3);
    Set(t, 0x0004, 46);
    Set(t, 0x0005, kVariablePacketSize);
    Set(t, 0x0006, 3);
    Set(t, 0x0007, kVariablePacketSize);
    Set(t, 0x0008, 3);
    Set(t, 0x0009, 92);
    Set(t, 0x000A, 3);
    Set(t, 0x000B, 2);
    Set(t, 0x000C, 3);
    Set(t, 0x000D, 28);
    Set(t, 0x000E, 19);
    Set(t, 0x000F, 11);
    Set(t, 0x0010, 3);
    Set(t, 0x001D, 3);
    Set(t, 0x0011, kVariablePacketSize);
    Set(t, 0x0012, 9);
    Set(t, 0x0013, 5);
    Set(t, 0x0014, 26);
    Set(t, 0x0015, 25);
    Set(t, 0x0016, 30);
    Set(t, 0x0017, 32);
    Set(t, 0x0018, 25);
    Set(t, 0x001B, 6);
    Set(t, 0x001C, 7);
    Set(t, 0x001F, 2);
    Set(t, 0x0020, 2);
    Set(t, 0x0022, 16);
    Set(t, 0x0023, 12);
    Set(t, 0x0024, 10);
    Set(t, 0x0026, 27);
    Set(t, 0x0027, 23);
    Set(t, 0x0028, kVariablePacketSize);
    Set(t, 0x0029, kVariablePacketSize);
    Set(t, 0x002C, 22);
    Set(t, 0x002D, 28);
    Set(t, 0x002E, 2);
    Set(t, 0x0030, 54);
    Set(t, 0x0032, kVariablePacketSize);
    Set(t, 0x0033, 3);
    Set(t, 0x0035, kVariablePacketSize);
    Set(t, 0x0037, 7);
    Set(t, 0x0038, 38);
    Set(t, 0x0039, 38);
    Set(t, 0x003B, 33);
    Set(t, 0x003C, 6);
    Set(t, 0x003E, kVariablePacketSize);
    Set(t, 0x003F, kVariablePacketSize);
    Set(t, 0x0040, kVariablePacketSize);
    Set(t, 0x0041, kVariablePacketSize);
    Set(t, 0x0043, 7);
    Set(t, 0x0045, 6);
    Set(t, 0x0047, 6);
    Set(t, 0x0049, kVariablePacketSize);
    Set(t, 0x004A, 6);
    Set(t, 0x004B, 6);
    Set(t, 0x004C, 8);
    Set(t, 0x004E, 3);
    Set(t, 0x004F, kVariablePacketSize);
    Set(t, 0x0050, 6);
    Set(t, 0x0051, 6);
    Set(t, 0x0052, kVariablePacketSize);
    Set(t, 0x0057, 6);
    Set(t, 0x0058, 20);
    Set(t, 0x0059, 5);
    Set(t, 0x005B, 7);
    Set(t, 0x005D, 6);
    Set(t, 0x005E, 8);
    Set(t, 0x005F, 6);
    Set(t, 0x0064, 55);
    Set(t, 0x0065, 17);
    Set(t, 0x0066, 3);
    Set(t, 0x0067, 37);
    Set(t, 0x0069, kVariablePacketSize);
    Set(t, 0x006A, 23);
    Set(t, 0x006B, kVariablePacketSize);
    Set(t, 0x006C, 3);
    Set(t, 0x006D, 110);
    Set(t, 0x006E, 3);
    Set(t, 0x006F, 2);
    Set(t, 0x0070, 3);
    Set(t, 0x0071, 28);
    Set(t, 0x0072, 25);
    Set(t, 0x0073, 11);
    Set(t, 0x0078, 55);
    Set(t, 0x0079, 53);
    Set(t, 0x007A, 58);
    Set(t, 0x007B, 60);
    Set(t, 0x007C, 42);
    Set(t, 0x007E, 6);
    Set(t, 0x007F, 6);
    Set(t, 0x0080, 7);
    Set(t, 0x0081, 3);
    Set(t, 0x0085, 11);
    Set(t, 0x0086, 16);
    Set(t, 0x0087, 12);
    Set(t, 0x0088, 10);
    Set(t, 0x0089, 8);
    Set(t, 0x008A, 29);
    Set(t, 0x008D, kVariablePacketSize);
    Set(t, 0x008E, kVariablePacketSize);
    Set(t, 0x0091, 22);
    Set(t, 0x0092, 28);
    Set(t, 0x0095, 30);
    Set(t, 0x0097, kVariablePacketSize);
    Set(t, 0x0098, 3);
    Set(t, 0x0099, kVariablePacketSize);
    Set(t, 0x009A, kVariablePacketSize);
    Set(t, 0x009B, 26);
    Set(t, 0x009C, 9);
    Set(t, 0x009D, 17);
    Set(t, 0x009E, 17);
    Set(t, 0x00A0, 23);
    Set(t, 0x00A1, 6);
    Set(t, 0x00A3, kVariablePacketSize);
    Set(t, 0x00A4, kVariablePacketSize);
    Set(t, 0x00A5, kVariablePacketSize);
    Set(t, 0x00A6, kVariablePacketSize);
    Set(t, 0x00A7, 8);
    Set(t, 0x00A8, 7);
    Set(t, 0x00A9, 6);
    Set(t, 0x00AA, 7);
    Set(t, 0x00AC, 7);
    Set(t, 0x00AF, 6);
    Set(t, 0x00B0, 8);
    Set(t, 0x00B1, 8);
    Set(t, 0x00B2, 3);
    Set(t, 0x00B3, 3);
    Set(t, 0x00B4, kVariablePacketSize);
    Set(t, 0x00B5, 6);
    Set(t, 0x00B6, 6);
    Set(t, 0x00B7, kVariablePacketSize);
    Set(t, 0x00BA, 2);
    Set(t, 0x00BB, 5);
    Set(t, 0x00BC, 6);
    Set(t, 0x00BD, 44);
    Set(t, 0x00BE, 5);
    Set(t, 0x00BF, 3);
    Set(t, 0x00C0, 7);
    Set(t, 0x00C1, 2);
    Set(t, 0x00C2, 6);
    Set(t, 0x00C3, 8);
    Set(t, 0x00C4, 6);
    Set(t, 0x00C5, 7);
    Set(t, 0x00C6, kVariablePacketSize);
    Set(t, 0x00C7, kVariablePacketSize);
    Set(t, 0x00C8, kVariablePacketSize);
    Set(t, 0x00C9, kVariablePacketSize);
    Set(t, 0x00CA, 3);
    Set(t, 0x00CB, 3);
    Set(t, 0x00D1, 4);
    Set(t, 0x00D2, 4);
    Set(t, 0x00D3, 2);
    Set(t, 0x00D4, kVariablePacketSize);
    Set(t, 0x00D6, 3);
    Set(t, 0x00D7, kVariablePacketSize);
    Set(t, 0x00D8, 6);
    Set(t, 0x00D9, 14);
    Set(t, 0x00DA, 3);
    Set(t, 0x00DC, 28);
    Set(t, 0x00DD, 29);
    Set(t, 0x00DE, kVariablePacketSize);
    Set(t, 0x00DF, kVariablePacketSize);
    Set(t, 0x00E0, 30);
    Set(t, 0x00E1, 30);
    Set(t, 0x00E2, 26);
    Set(t, 0x00E3, 2);
    Set(t, 0x00E4, 6);
    Set(t, 0x00E5, 26);
    Set(t, 0x00E6, 3);
    Set(t, 0x00E7, 3);
    Set(t, 0x00E8, 8);
    Set(t, 0x00E9, 19);
    Set(t, 0x00EA, 5);
    Set(t, 0x00EB, 2);
    Set(t, 0x00EC, 3);
    Set(t, 0x00ED, 2);
    Set(t, 0x00EE, 2);
    Set(t, 0x00EF, 2);
    Set(t, 0x00F0, 3);
    Set(t, 0x00F2, 6);
    Set(t, 0x00F3, kVariablePacketSize);
    Set(t, 0x00F4, 21);
    Set(t, 0x00F5, 8);
    Set(t, 0x00F6, 8);
    Set(t, 0x00F7, 22);
    Set(t, 0x00F8, 2);
    Set(t, 0x00F9, 26);
    Set(t, 0x00FA, 3);
    Set(t, 0x00FB, kVariablePacketSize);
    Set(t, 0x00FC, 6);
    Set(t, 0x00FD, 27);
    Set(t, 0x00FE, 30);
    Set(t, 0x00FF, 10);
    Set(t, 0x0100, 2);
    Set(t, 0x0101, 6);
    Set(t, 0x0102, 6);
    Set(t, 0x0103, 30);
    Set(t, 0x0104, 79);
    Set(t, 0x0105, 31);
    Set(t, 0x0106, 10);
    Set(t, 0x0107, 10);
    Set(t, 0x0109, kVariablePacketSize);
    Set(t, 0x010E, 11);
    Set(t, 0x010F, kVariablePacketSize);
    Set(t, 0x0110, 10);
    Set(t, 0x0111, 39);
    Set(t, 0x0113, 22);
    Set(t, 0x0114, 31);
    Set(t, 0x0115, 35);
    Set(t, 0x0116, 10);
    Set(t, 0x0117, 18);
    Set(t, 0x0119, 13);
    Set(t, 0x011A, 15);
    Set(t, 0x011F, 16);
    Set(t, 0x0120, 6);
    Set(t, 0x0121, 14);
    Set(t, 0x012B, 2);
    Set(t, 0x012D, 4);
    Set(t, 0x012E, 2);
    Set(t, 0x012F, kVariablePacketSize);
    Set(t, 0x0130, 6);
    Set(t, 0x0131, 86);
    Set(t, 0x0132, 6);
    Set(t, 0x0133, kVariablePacketSize);
    Set(t, 0x0134, kVariablePacketSize);
    Set(t, 0x0135, 7);
    Set(t, 0x0136, kVariablePacketSize);
    Set(t, 0x0137, 6);
    Set(t, 0x0138, 3);
    Set(t, 0x0139, 16);
    Set(t, 0x013A, 4);
    Set(t, 0x013B, 4);
    Set(t, 0x013C, 4);
    Set(t, 0x013D, 6);
    Set(t, 0x013E, 24);
    Set(t, 0x013F, 26);
    Set(t, 0x0140, 22);
    Set(t, 0x0141, 14);
    Set(t, 0x0142, 6);
    Set(t, 0x0145, 19);
    Set(t, 0x0147, 39);
    Set(t, 0x0148, 8);
    Set(t, 0x014C, kVariablePacketSize);
    Set(t, 0x014E, 6);
    Set(t, 0x0152, kVariablePacketSize);
    Set(t, 0x0154, kVariablePacketSize);
    Set(t, 0x0156, kVariablePacketSize);
    Set(t, 0x015A, 66);
    Set(t, 0x015C, 66);
    Set(t, 0x015E, 6);
    Set(t, 0x0160, kVariablePacketSize);
    Set(t, 0x0162, kVariablePacketSize);
    Set(t, 0x0163, kVariablePacketSize);
    Set(t, 0x0164, kVariablePacketSize);
    Set(t, 0x0165, 30);
    Set(t, 0x0166, kVariablePacketSize);
    Set(t, 0x0167, 3);
    Set(t, 0x0168, 14);
    Set(t, 0x016A, 14);
    Set(t, 0x016C, 114);
    Set(t, 0x016D, 14);
    Set(t, 0x016E, 186);
    Set(t, 0x016F, 182);
    Set(t, 0x0170, 14);
    Set(t, 0x0171, 30);
    Set(t, 0x0172, 10);
    Set(t, 0x0173, 3);
    Set(t, 0x0174, kVariablePacketSize);
    Set(t, 0x0175, 6);
    Set(t, 0x0177, kVariablePacketSize);
    Set(t, 0x0178, 4);
    Set(t, 0x0179, 5);
    Set(t, 0x017B, kVariablePacketSize);
    Set(t, 0x017C, 6);
    Set(t, 0x017D, 7);
    Set(t, 0x0192, 24);
    Set(t, 0x0193, 2);
    Set(t, 0x0194, 30);
    Set(t, 0x0195, 102);
    Set(t, 0x0196, 9);
    Set(t, 0x0199, 4);
    Set(t, 0x019B, 10);
    Set(t, 0x01A2, 37);
    Set(t, 0x01A3, 5);
    Set(t, 0x01A4, 11);
    Set(t, 0x01A5, 26);
    Set(t, 0x01A6, kVariablePacketSize);
    Set(t, 0x01A7, 4);
    Set(t, 0x01A8, 4);
    Set(t, 0x01A9, 6);
    Set(t, 0x01AA, 10);
    Set(t, 0x01AB, 12);
    Set(t, 0x01AC, 6);
    Set(t, 0x01AD, kVariablePacketSize);
    Set(t, 0x01AE, 4);
    Set(t, 0x01AF, 4);
    Set(t, 0x01B0, 11);
    Set(t, 0x01B1, 7);
    Set(t, 0x01B2, kVariablePacketSize);
    Set(t, 0x01B3, 67);
    Set(t, 0x01B4, 12);
    Set(t, 0x01B5, 18);
    Set(t, 0x01B6, 114);
    Set(t, 0x01B7, 6);
    Set(t, 0x01B8, 3);
    Set(t, 0x01B9, 6);
    Set(t, 0x01BA, 26);
    Set(t, 0x01C3, kVariablePacketSize);
    Set(t, 0x01C4, 22);
    Set(t, 0x01C8, 13);
    Set(t, 0x01C9, 97);
    Set(t, 0x01CF, 28);
    Set(t, 0x01D0, 8);
    Set(t, 0x01D4, 6);
    Set(t, 0x01D6, 4);
    Set(t, 0x01D7, 11);
    Set(t, 0x01D8, 54);
    Set(t, 0x01D9, 53);
    Set(t, 0x01DA, 60);
    Set(t, 0x01DB, 2);
    Set(t, 0x01DC, kVariablePacketSize);
    Set(t, 0x01DD, 47);
    Set(t, 0x01DE, 33);
    Set(t, 0x01E1, 8);
    Set(t, 0x01E9, 81);
    Set(t, 0x01EE, kVariablePacketSize);
    Set(t, 0x01EF, kVariablePacketSize);
    Set(t, 0x01F0, kVariablePacketSize);
    Set(t, 0x01F1, kVariablePacketSize);
    Set(t, 0x01F3, 10);
    Set(t, 0x01F8, 2);
    Set(t, 0x01FF, 10);
    Set(t, 0x0201, kVariablePacketSize);
    Set(t, 0x0204, 18);
    Set(t, 0x0206, 11);
    Set(t, 0x0207, 34);
    Set(t, 0x0209, 36);
    Set(t, 0x020A, 10);
    Set(t, 0x0214, 42);
    Set(t, 0x0220, 10);
    Set(t, 0x0221, kVariablePacketSize);
    Set(t, 0x0222, 6);
    Set(t, 0x0223, 8);
    Set(t, 0x0229, 15);
    Set(t, 0x022A, 58);
    Set(t, 0x022B, 57);
    Set(t, 0x022C, 65);
    Set(t, 0x0235, kVariablePacketSize);
    Set(t, 0x0239, 11);
    Set(t, 0x025A, kVariablePacketSize);
    Set(t, 0x0283, 6);
    Set(t, 0x029D, kVariablePacketSize);
    Set(t, 0x029E, 11);
    Set(t, 0x02B0, 85);
    Set(t, 0x02B1, kVariablePacketSize);
    Set(t, 0x02B2, kVariablePacketSize);
    Set(t, 0x02B3, 107);
    Set(t, 0x02B4, 6);
    Set(t, 0x02B5, kVariablePacketSize);
    Set(t, 0x02B9, 191);
    Set(t, 0x02BA, 11);
    Set(t, 0x02C5, 30);
    Set(t, 0x02C6, 30);
    Set(t, 0x02C9, 3);
    Set(t, 0x02D0, kVariablePacketSize);
    Set(t, 0x02D1, kVariablePacketSize);
    Set(t, 0x02D2, kVariablePacketSize);
    Set(t, 0x02D3, 4);
    Set(t, 0x02D4, 29);
    Set(t, 0x02D5, 2);
    Set(t, 0x02D7, kVariablePacketSize);
    Set(t, 0x02D9, 10);
    Set(t, 0x02DA, 3);
    Set(t, 0x02DC, kVariablePacketSize);
    Set(t, 0x02DD, 32);
    Set(t, 0x02E1, 33);
    Set(t, 0x02E7, kVariablePacketSize);
    Set(t, 0x02E8, kVariablePacketSize);
    Set(t, 0x02E9, kVariablePacketSize);
    Set(t, 0x02EA, kVariablePacketSize);
    Set(t, 0x02EB, 13);
    Set(t, 0x02EC, 67);
    Set(t, 0x02ED, 59);
    Set(t, 0x02EE, 60);
    Set(t, 0x02EF, 8);
    Set(t, 0x043F, 25);
    Set(t, 0x0459, 8);
    Set(t, 0x045A, 10);
    Set(t, 0x045B, kVariablePacketSize);
    Set(t, 0x045C, kVariablePacketSize);
    Set(t, 0x0461, kVariablePacketSize);
    Set(t, 0x0463, kVariablePacketSize);
    Set(t, 0x0465, 10);
    Set(t, 0x0466, 10);
    Set(t, 0x0467, 12);
    Set(t, 0x0468, 12);
    Set(t, 0x0469, 10);
    Set(t, 0x046F, 12);
    Set(t, 0x0470, 12);
    Set(t, 0x0471, 10);
    Set(t, 0x0477, 8);
    Set(t, 0x0502, 11);
    Set(t, 0x0517, 4);
    Set(t, 0x0518, 4);
    Set(t, 0x0519, 6);
    Set(t, 0x0569, 8);
    Set(t, 0x05EA, 10);
    Set(t, 0x06B3, 14);
    Set(t, 0x06B4, 14);
    Set(t, 0x06C8, 6);
    Set(t, 0x06C9, 7);
    Set(t, 0x06CA, 8);
    Set(t, 0x06CE, 10);
    Set(t, 0x07F7, kVariablePacketSize);
    Set(t, 0x07F8, kVariablePacketSize);
    Set(t, 0x07F9, kVariablePacketSize);
    Set(t, 0x07FA, 8);
    Set(t, 0x080E, 14);
    Set(t, 0x0814, 86);
    Set(t, 0x0816, 6);
    Set(t, 0x0856, kVariablePacketSize);
    Set(t, 0x0857, kVariablePacketSize);
    Set(t, 0x0858, kVariablePacketSize);
    Set(t, 0x8482, 4);
    Set(t, 0x8483, 4);
}

//===========================================================================
// PV200 — Sabine Beta1 (2002-02-20 Ragexe).
// Authoritative source: PacketTable.0100_Alpha.cs + PacketTable.0200_Beta1.cs
// in E:\Projects\GitHub\Sabine\src\Shared\Network. Built from scratch — no
// inheritance from PV23. Beta1 inserts ZC_NOTIFY_ACT_POSITION at 0x0027 and
// shifts Alpha 0x0027-0x009B by +1; ZC_GROUPINFO_CHANGE / CZ_CHANGE_GROUPEXPOPTION
// shift Alpha 0x009C-0x00AA by +3.
//===========================================================================
void PopulatePv200(VersionTable& t)
{
    // Alpha 0x0000-0x0026 (no shift), with Beta1 size overrides applied.
    Set(t, 0x0000, 54);  // CA_LOGIN
    Set(t, 0x0001, 17);  // CH_ENTER
    Set(t, 0x0002, 3);   // CH_SELECT_CHAR
    Set(t, 0x0003, 34);  // CH_MAKE_CHAR
    Set(t, 0x0004, 46);  // CH_DELETE_CHAR
    Set(t, 0x0005, kVariablePacketSize); // AC_ACCEPT_LOGIN
    Set(t, 0x0006, 3);   // AC_REFUSE_LOGIN
    Set(t, 0x0007, kVariablePacketSize); // HC_ACCEPT_ENTER
    Set(t, 0x0008, 3);   // HC_REFUSE_ENTER
    Set(t, 0x0009, 92);  // HC_ACCEPT_MAKECHAR
    Set(t, 0x000A, 3);   // HC_REFUSE_MAKECHAR
    Set(t, 0x000B, 2);   // HC_ACCEPT_DELETECHAR
    Set(t, 0x000C, 3);   // HC_REFUSE_DELETECHAR
    Set(t, 0x000D, 28);  // HC_NOTIFY_ZONESVR
    Set(t, 0x000E, 19);  // CZ_ENTER
    Set(t, 0x000F, 11);  // ZC_ACCEPT_ENTER
    Set(t, 0x0010, 3);   // ZC_REFUSE_ENTER
    Set(t, 0x0011, kVariablePacketSize); // ZC_NOTIFY_INITCHAR
    Set(t, 0x0012, 9);   // ZC_NOTIFY_UPDATECHAR
    Set(t, 0x0013, 5);   // ZC_NOTIFY_UPDATEPLAYER
    Set(t, 0x0014, 26);  // ZC_NOTIFY_STANDENTRY
    Set(t, 0x0015, 25);  // ZC_NOTIFY_NEWENTRY
    Set(t, 0x0016, 30);  // ZC_NOTIFY_ACTENTRY
    Set(t, 0x0017, 32);  // ZC_NOTIFY_MOVEENTRY
    Set(t, 0x0018, 25);  // ZC_NOTIFY_STANDENTRY_NPC
    Set(t, 0x0019, 2);   // CZ_NOTIFY_ACTORINIT
    Set(t, 0x001A, 6);   // CZ_REQUEST_TIME
    Set(t, 0x001B, 6);   // ZC_NOTIFY_TIME
    Set(t, 0x001C, 7);   // ZC_NOTIFY_VANISH
    Set(t, 0x001D, 3);   // SC_NOTIFY_BAN
    Set(t, 0x001E, 2);   // CZ_REQUEST_QUIT
    Set(t, 0x001F, 2);   // ZC_ACCEPT_QUIT
    Set(t, 0x0020, 2);   // ZC_REFUSE_QUIT
    Set(t, 0x0021, 5);   // CZ_REQUEST_MOVE
    Set(t, 0x0022, 16);  // ZC_NOTIFY_MOVE
    Set(t, 0x0023, 12);  // ZC_NOTIFY_PLAYERMOVE
    Set(t, 0x0024, 10);  // ZC_STOPMOVE
    Set(t, 0x0025, 7);   // CZ_REQUEST_ACT
    Set(t, 0x0026, 27);  // ZC_NOTIFY_ACT

    // Beta1 inserts ZC_NOTIFY_ACT_POSITION at 0x0027.
    Set(t, 0x0027, 23);  // ZC_NOTIFY_ACT_POSITION

    // Alpha 0x0027-0x009B shifted +1.
    Set(t, 0x0028, kVariablePacketSize); // CZ_REQUEST_CHAT
    Set(t, 0x0029, kVariablePacketSize); // ZC_NOTIFY_CHAT
    Set(t, 0x002A, kVariablePacketSize); // ZC_NOTIFY_PLAYERCHAT
    Set(t, 0x002B, kVariablePacketSize); // SERVER_ENTRY_ACK
    Set(t, 0x002C, 7);   // CZ_CONTACTNPC
    Set(t, 0x002D, 22);  // ZC_NPCACK_MAPMOVE
    Set(t, 0x002E, 28);  // ZC_NPCACK_SERVERMOVE
    Set(t, 0x002F, 2);   // ZC_NPCACK_ENABLE
    Set(t, 0x0030, 6);   // CZ_REQNAME
    Set(t, 0x0031, 54);  // ZC_ACK_REQNAME
    Set(t, 0x0032, kVariablePacketSize); // CZ_WHISPER
    Set(t, 0x0033, kVariablePacketSize); // ZC_WHISPER
    Set(t, 0x0034, 3);   // ZC_ACK_WHISPER
    Set(t, 0x0035, kVariablePacketSize); // CZ_BROADCAST
    Set(t, 0x0036, kVariablePacketSize); // ZC_BROADCAST
    Set(t, 0x0037, 3);   // CZ_CHANGE_DIRECTION
    Set(t, 0x0038, 7);   // ZC_CHANGE_DIRECTION
    Set(t, 0x0039, 38);  // ZC_ITEM_ENTRY
    Set(t, 0x003A, 38);  // ZC_ITEM_FALL_ENTRY
    Set(t, 0x003B, 6);   // CZ_ITEM_PICKUP
    Set(t, 0x003C, 33);  // ZC_ITEM_PICKUP_ACK
    Set(t, 0x003D, 6);   // ZC_ITEM_DISAPPEAR
    Set(t, 0x003E, 6);   // CZ_ITEM_THROW
    Set(t, 0x003F, kVariablePacketSize); // ZC_NORMAL_ITEMLIST
    Set(t, 0x0040, kVariablePacketSize); // ZC_EQUIPMENT_ITEMLIST
    Set(t, 0x0041, kVariablePacketSize); // ZC_STORE_NORMAL_ITEMLIST
    Set(t, 0x0042, kVariablePacketSize); // ZC_STORE_EQUIPMENT_ITEMLIST
    Set(t, 0x0043, 8);   // CZ_USE_ITEM
    Set(t, 0x0044, 7);   // ZC_USE_ITEM_ACK
    Set(t, 0x0045, 5);   // CZ_REQ_WEAR_EQUIP
    Set(t, 0x0046, 6);   // ZC_REQ_WEAR_EQUIP_ACK
    Set(t, 0x0047, 4);   // CZ_REQ_TAKEOFF_EQUIP
    Set(t, 0x0048, 6);   // ZC_REQ_TAKEOFF_EQUIP_ACK
    Set(t, 0x0049, 26);  // CZ_REQ_ITEM_EXPLANATION_BYNAME
    Set(t, 0x004A, kVariablePacketSize); // ZC_REQ_ITEM_EXPLANATION_ACK
    Set(t, 0x004B, 6);   // ZC_ITEM_THROW_ACK
    Set(t, 0x004C, 6);   // ZC_PAR_CHANGE
    Set(t, 0x004D, 8);   // ZC_LONGPAR_CHANGE
    Set(t, 0x004E, 3);   // CZ_RESTART
    Set(t, 0x004F, 3);   // ZC_RESTART_ACK
    Set(t, 0x0050, kVariablePacketSize); // ZC_SAY_DIALOG
    Set(t, 0x0051, 6);   // ZC_WAIT_DIALOG
    Set(t, 0x0052, 6);   // ZC_CLOSE_DIALOG
    Set(t, 0x0053, kVariablePacketSize); // ZC_MENU_LIST
    Set(t, 0x0054, 7);   // CZ_CHOOSE_MENU
    Set(t, 0x0055, 6);   // CZ_REQ_NEXT_SCRIPT
    Set(t, 0x0056, 2);   // CZ_REQ_STATUS
    Set(t, 0x0057, 5);   // CZ_STATUS_CHANGE
    Set(t, 0x0058, 6);   // ZC_STATUS_CHANGE_ACK
    Set(t, 0x0059, 44);  // ZC_STATUS
    Set(t, 0x005A, 5);   // ZC_STATUS_CHANGE
    Set(t, 0x005B, 3);   // CZ_REQ_EMOTION
    Set(t, 0x005C, 7);   // ZC_EMOTION
    Set(t, 0x005D, 2);   // CZ_REQ_USER_COUNT
    Set(t, 0x005E, 6);   // ZC_USER_COUNT
    Set(t, 0x005F, 8);   // ZC_SPRITE_CHANGE
    Set(t, 0x0060, 6);   // ZC_SELECT_DEALTYPE
    Set(t, 0x0061, 7);   // CZ_ACK_SELECT_DEALTYPE
    Set(t, 0x0062, kVariablePacketSize); // ZC_PC_PURCHASE_ITEMLIST
    Set(t, 0x0063, kVariablePacketSize); // ZC_PC_SELL_ITEMLIST
    Set(t, 0x0064, kVariablePacketSize); // CZ_PC_PURCHASE_ITEMLIST
    Set(t, 0x0065, kVariablePacketSize); // CZ_PC_SELL_ITEMLIST
    Set(t, 0x0066, 3);   // ZC_PC_PURCHASE_RESULT
    Set(t, 0x0067, 3);   // ZC_PC_SELL_RESULT  <-- collides with PV23 CH_MAKE_CHAR (37)
    Set(t, 0x0068, 26);  // CZ_DISCONNECT_CHARACTER
    Set(t, 0x0069, 3);   // ZC_ACK_DISCONNECT_CHARACTER
    Set(t, 0x006A, 2);   // CZ_DISCONNECT_ALL_CHARACTER
    Set(t, 0x006B, 27);  // CZ_SETTING_WHISPER_PC
    Set(t, 0x006C, 3);   // CZ_SETTING_WHISPER_STATE
    Set(t, 0x006D, 4);   // ZC_SETTING_WHISPER_PC
    Set(t, 0x006E, 4);   // ZC_SETTING_WHISPER_STATE
    Set(t, 0x006F, 2);   // CZ_REQ_WHISPER_LIST
    Set(t, 0x0070, kVariablePacketSize); // ZC_WHISPER_LIST
    Set(t, 0x0071, kVariablePacketSize); // CZ_CREATE_CHATROOM
    Set(t, 0x0072, 3);   // ZC_ACK_CREATE_CHATROOM
    Set(t, 0x0073, kVariablePacketSize); // ZC_ROOM_NEWENTRY
    Set(t, 0x0074, 6);   // ZC_DESTROY_ROOM
    Set(t, 0x0075, 14);  // CZ_REQ_ENTER_ROOM
    Set(t, 0x0076, 3);   // ZC_REFUSE_ENTER_ROOM
    Set(t, 0x0077, kVariablePacketSize); // ZC_ENTER_ROOM
    Set(t, 0x0078, 28);  // ZC_MEMBER_NEWENTRY
    Set(t, 0x0079, 29);  // ZC_MEMBER_EXIT
    Set(t, 0x007A, kVariablePacketSize); // CZ_CHANGE_CHATROOM
    Set(t, 0x007B, kVariablePacketSize); // ZC_CHANGE_CHATROOM
    Set(t, 0x007C, 30);  // CZ_REQ_ROLE_CHANGE
    Set(t, 0x007D, 30);  // ZC_ROLE_CHANGE
    Set(t, 0x007E, 16);  // CZ_REQ_EXPEL_MEMBER
    Set(t, 0x007F, 2);   // CZ_EXIT_ROOM
    Set(t, 0x0080, 6);   // CZ_REQ_EXCHANGE_ITEM
    Set(t, 0x0081, 26);  // ZC_REQ_EXCHANGE_ITEM
    Set(t, 0x0082, 3);   // CZ_ACK_EXCHANGE_ITEM
    Set(t, 0x0083, 3);   // ZC_ACK_EXCHANGE_ITEM
    Set(t, 0x0084, 8);   // CZ_ADD_EXCHANGE_ITEM
    Set(t, 0x0085, 30);  // ZC_ADD_EXCHANGE_ITEM
    Set(t, 0x0086, 5);   // ZC_ACK_ADD_EXCHANGE_ITEM
    Set(t, 0x0087, 2);   // CZ_CONCLUDE_EXCHANGE_ITEM
    Set(t, 0x0088, 3);   // ZC_CONCLUDE_EXCHANGE_ITEM
    Set(t, 0x0089, 2);   // CZ_CANCEL_EXCHANGE_ITEM
    Set(t, 0x008A, 2);   // ZC_CANCEL_EXCHANGE_ITEM
    Set(t, 0x008B, 2);   // CZ_EXEC_EXCHANGE_ITEM
    Set(t, 0x008C, 3);   // ZC_EXEC_EXCHANGE_ITEM
    Set(t, 0x008D, 2);   // ZC_EXCHANGEITEM_UNDO
    Set(t, 0x008E, 6);   // ZC_NOTIFY_STOREITEM_COUNTINFO
    Set(t, 0x008F, 8);   // CZ_MOVE_ITEM_FROM_BODY_TO_STORE
    Set(t, 0x0090, 32);  // ZC_ADD_ITEM_TO_STORE
    Set(t, 0x0091, 8);   // CZ_MOVE_ITEM_FROM_STORE_TO_BODY
    Set(t, 0x0092, 8);   // ZC_DELETE_ITEM_FROM_STORE
    Set(t, 0x0093, 2);   // CZ_CLOSE_STORE
    Set(t, 0x0094, 2);   // ZC_CLOSE_STORE
    Set(t, 0x0095, 26);  // CZ_MAKE_GROUP
    Set(t, 0x0096, 3);   // ZC_ACK_MAKE_GROUP
    Set(t, 0x0097, kVariablePacketSize); // ZC_GROUP_LIST
    Set(t, 0x0098, 6);   // CZ_REQ_JOIN_GROUP
    Set(t, 0x0099, 27);  // ZC_ACK_REQ_JOIN_GROUP
    Set(t, 0x009A, 30);  // ZC_REQ_JOIN_GROUP
    Set(t, 0x009B, 10);  // CZ_JOIN_GROUP
    Set(t, 0x009C, 2);   // CZ_REQ_LEAVE_GROUP

    // Beta1 inserts ZC_GROUPINFO_CHANGE at 0x009D and CZ_CHANGE_GROUPEXPOPTION at 0x009E,
    // shifting Alpha 0x009C-0x00AA by +3 net.
    Set(t, 0x009D, 6);   // ZC_GROUPINFO_CHANGE (Beta1 new)
    Set(t, 0x009E, 6);   // CZ_CHANGE_GROUPEXPOPTION (Beta1 new)

    // Alpha 0x009C-0x00AA shifted +3.
    Set(t, 0x009F, 30);  // CZ_REQ_EXPEL_GROUP_MEMBER
    Set(t, 0x00A0, 79);  // ZC_ADD_MEMBER_TO_GROUP
    Set(t, 0x00A1, 31);  // ZC_DELETE_MEMBER_FROM_GROUP
    Set(t, 0x00A2, 10);  // ZC_NOTIFY_HP_TO_GROUPM
    Set(t, 0x00A3, 10);  // ZC_NOTIFY_POSITION_TO_GROUPM
    Set(t, 0x00A4, kVariablePacketSize); // CZ_REQUEST_CHAT_PARTY
    Set(t, 0x00A5, kVariablePacketSize); // ZC_NOTIFY_CHAT_PARTY
    Set(t, 0x00A6, 18);  // ZC_MVP_GETTING_ITEM
    Set(t, 0x00A7, 6);   // ZC_MVP_GETTING_SPECIAL_EXP
    Set(t, 0x00A8, 6);   // ZC_MVP
    Set(t, 0x00A9, 2);   // ZC_THROW_MVPITEM
    Set(t, 0x00AA, 9);   // ZC_SKILLINFO_UPDATE
    Set(t, 0x00AB, kVariablePacketSize); // ZC_SKILLINFO_LIST
    Set(t, 0x00AC, 10);  // ZC_ACK_TOUSESKILL (Beta1 size)
    Set(t, 0x00AD, 39);  // ZC_ADD_SKILL (Beta1 size)

    // Beta1 new opcodes appended (no shift since past existing range).
    Set(t, 0x00AE, 4);   // CZ_UPGRADE_SKILLLEVEL
    Set(t, 0x00AF, 10);  // CZ_USE_SKILL
    Set(t, 0x00B0, 31);  // ZC_NOTIFY_SKILL
    Set(t, 0x00B1, 35);  // ZC_NOTIFY_SKILL_POSITION
    Set(t, 0x00B2, 10);  // CZ_USE_SKILL_TOGROUND
    Set(t, 0x00B3, 18);  // ZC_NOTIFY_GROUNDSKILL
    Set(t, 0x00B4, 2);   // CZ_CANCEL_LOCKON
    Set(t, 0x00B5, 13);  // ZC_STATE_CHANGE
    Set(t, 0x00B6, 15);  // ZC_USE_SKILL
    Set(t, 0x00B7, 20);  // CZ_SELECT_WARPPOINT
    Set(t, 0x00B8, 52);  // ZC_WARPLIST
    Set(t, 0x00B9, 2);   // CZ_REMEMBER_WARPPOINT
    Set(t, 0x00BA, 3);   // ZC_ACK_REMEMBER_WARPPOINT
    Set(t, 0x00BB, 11);  // ZC_SKILL_ENTRY
    Set(t, 0x00BC, 6);   // ZC_SKILL_DISAPPEAR
    Set(t, 0x00BD, 14);  // ZC_NOTIFY_CARTITEM_COUNTINFO
    Set(t, 0x00BE, kVariablePacketSize); // ZC_CART_EQUIPMENT_ITEMLIST
    Set(t, 0x00BF, kVariablePacketSize); // ZC_CART_NORMAL_ITEMLIST
    Set(t, 0x00C0, 32);  // ZC_ADD_ITEM_TO_CART
    Set(t, 0x00C1, 8);   // ZC_DELETE_ITEM_FROM_CART
    Set(t, 0x00C2, 8);   // CZ_MOVE_ITEM_FROM_BODY_TO_CART
    Set(t, 0x00C3, 8);   // CZ_MOVE_ITEM_FROM_CART_TO_BODY
    Set(t, 0x00C4, 8);   // CZ_MOVE_ITEM_FROM_STORE_TO_CART
    Set(t, 0x00C5, 8);   // CZ_MOVE_ITEM_FROM_CART_TO_STORE
    Set(t, 0x00C6, 2);   // CZ_REQ_CARTOFF
    Set(t, 0x00C7, 2);   // ZC_CARTOFF
    Set(t, 0x00C8, 3);   // ZC_ACK_ADDITEM_TO_CART
    Set(t, 0x00C9, 4);   // ZC_OPENSTORE
    Set(t, 0x00CA, 2);   // CZ_REQ_CLOSESTORE
    Set(t, 0x00CB, kVariablePacketSize); // CZ_REQ_OPENSTORE
    Set(t, 0x00CC, 6);   // CZ_REQ_BUY_FROMMC
    Set(t, 0x00CD, 86);  // ZC_STORE_ENTRY
    Set(t, 0x00CE, 6);   // ZC_DISAPPEAR_ENTRY
    Set(t, 0x00CF, kVariablePacketSize); // ZC_PC_PURCHASE_ITEMLIST_FROMMC
    Set(t, 0x00D0, kVariablePacketSize); // CZ_PC_PURCHASE_ITEMLIST_FROMMC
    Set(t, 0x00D1, 7);   // ZC_PC_PURCHASE_RESULT_FROMMC
    Set(t, 0x00D2, kVariablePacketSize); // ZC_PC_PURCHASE_MYITEMLIST
    Set(t, 0x00D3, 6);   // ZC_DELETEITEM_FROM_MCSTORE
    Set(t, 0x00D4, 3);   // CZ_PKMODE_CHANGE
    Set(t, 0x00D5, 16);  // ZC_ATTACK_FAILURE_FOR_DISTANCE
    Set(t, 0x00D6, 4);   // ZC_ATTACK_RANGE
    Set(t, 0x00D7, 4);   // ZC_ACTION_FAILURE
    Set(t, 0x00D8, 4);   // ZC_EQUIP_ARROW
    Set(t, 0x00D9, 6);   // ZC_RECOVERY
    Set(t, 0x00DA, 24);  // ZC_USESKILL_ACK
    Set(t, 0x00DB, 26);  // CZ_ITEM_CREATE
    Set(t, 0x00DC, 18);  // CZ_MOVETO_MAP
}

//===========================================================================
// Shared semantic mapping for every packet that is identical between PV22
// and PV23 (account/char chain, most map-receive opcodes, and the gameplay
// send packets PV22 didn't reshuffle). Version-specific overrides apply on
// top of this in PopulatePv23Semantics / PopulatePv22Semantics.
//===========================================================================
void BindSharedPv2xSemantics(VersionTable& t)
{
    // Account login chain
    Bind(t, 0x0064, SemanticPacket::AccountLogin);
    Bind(t, 0x02B0, SemanticPacket::AccountLoginChannel);
    Bind(t, 0x01DB, SemanticPacket::RequestPasswordHash);
    Bind(t, 0x01DC, SemanticPacket::PasswordHashChallenge);
    Bind(t, 0x01DD, SemanticPacket::PasswordHashLogin);
    Bind(t, 0x0200, SemanticPacket::ConnectInfoChanged);
    Bind(t, 0x0204, SemanticPacket::ExeHashCheck);
    Bind(t, 0x01F1, SemanticPacket::NotifyError);

    // Character server
    Bind(t, 0x0065, SemanticPacket::CharServerEnter);
    Bind(t, 0x0066, SemanticPacket::SelectCharacter);
    Bind(t, 0x0067, SemanticPacket::MakeCharacter);
    Bind(t, 0x0068, SemanticPacket::DeleteCharacter);

    // Misc / gameplay common
    Bind(t, 0x018A, SemanticPacket::QuitGame);
    Bind(t, 0x00BF, SemanticPacket::ReqEmotion);
    Bind(t, 0x00C0, SemanticPacket::Emotion);

    // Send-side packets shared between PV22 and PV23
    Bind(t, 0x012A, SemanticPacket::CartOff);
    Bind(t, 0x01AF, SemanticPacket::ChangeCart);
    Bind(t, 0x017A, SemanticPacket::ItemCompositionList);
    Bind(t, 0x017C, SemanticPacket::ItemComposition);
    Bind(t, 0x0178, SemanticPacket::ItemIdentify);
    Bind(t, 0x0113, SemanticPacket::UseSkillToPos);
    Bind(t, 0x0116, SemanticPacket::DropItem);
    Bind(t, 0x011B, SemanticPacket::UseSkillMap);
    Bind(t, 0x0112, SemanticPacket::SkillUp);
    Bind(t, 0x00F5, SemanticPacket::TakeItem);
    Bind(t, 0x00A9, SemanticPacket::EquipItem);
    Bind(t, 0x00AB, SemanticPacket::UnequipItem);
    Bind(t, 0x00A7, SemanticPacket::WalkToXY);
    Bind(t, 0x0085, SemanticPacket::ChangeDir);
    Bind(t, 0x0089, SemanticPacket::TickSend);
    Bind(t, 0x008C, SemanticPacket::GetCharNameRequest);
    Bind(t, 0x0096, SemanticPacket::Whisper);
    Bind(t, 0x00F3, SemanticPacket::GlobalMessage);
    Bind(t, 0x007D, SemanticPacket::NotifyActorInit);

    // Receive-side: lifecycle / world state (PV22/PV23 share these).
    Bind(t, 0x0073, SemanticPacket::AcceptEnterLegacy);
    Bind(t, 0x02EB, SemanticPacket::AcceptEnterModern);
    Bind(t, 0x007F, SemanticPacket::NotifyTime);
    Bind(t, 0x0091, SemanticPacket::MapChangeBasic);
    Bind(t, 0x0092, SemanticPacket::MapChangeServerMove);
    Bind(t, 0x008A, SemanticPacket::ActorActionNotifyBasic);
    Bind(t, 0x02E1, SemanticPacket::ActorActionNotifyExtended);
    Bind(t, 0x0088, SemanticPacket::ActorSetPositionBasic);
    Bind(t, 0x01FF, SemanticPacket::ActorSetPositionHighJump);
    Bind(t, 0x0087, SemanticPacket::SelfMoveAck);
    Bind(t, 0x009A, SemanticPacket::BroadcastBasic);
    Bind(t, 0x01C3, SemanticPacket::BroadcastColored);

    // Receive-side: ground items
    Bind(t, 0x009D, SemanticPacket::GroundItemEntryExisting);
    Bind(t, 0x009E, SemanticPacket::GroundItemEntryDropped);
    Bind(t, 0x00A0, SemanticPacket::ItemPickupAckBasic);
    Bind(t, 0x02D4, SemanticPacket::ItemPickupAckExtended);

    // Receive-side: inventory / storage lists
    Bind(t, 0x00A3, SemanticPacket::NormalInventoryListBasic);
    Bind(t, 0x01EE, SemanticPacket::NormalInventoryListCardSlots);
    Bind(t, 0x02E8, SemanticPacket::NormalInventoryListTimed);
    Bind(t, 0x00A4, SemanticPacket::EquipInventoryListBasic);
    Bind(t, 0x01EF, SemanticPacket::EquipInventoryListTimed);
    Bind(t, 0x02D0, SemanticPacket::EquipInventoryListTimedOwned);
    Bind(t, 0x00A5, SemanticPacket::NormalStorageListBasic);
    Bind(t, 0x01F0, SemanticPacket::NormalStorageListCardSlots);
    Bind(t, 0x02EA, SemanticPacket::NormalStorageListTimed);
    Bind(t, 0x00A6, SemanticPacket::EquipStorageListBasic);
    Bind(t, 0x02D1, SemanticPacket::EquipStorageListTimedOwned);
    Bind(t, 0x00F4, SemanticPacket::StorageItemAddedBasic);
    Bind(t, 0x01C4, SemanticPacket::StorageItemAddedTyped);

    // Receive-side: item interactions
    Bind(t, 0x00A8, SemanticPacket::UseItemAckBasic);
    Bind(t, 0x01C8, SemanticPacket::UseItemAckExtended);
    Bind(t, 0x00AF, SemanticPacket::ItemRemoveBasic);
    Bind(t, 0x07FA, SemanticPacket::ItemRemoveExtended);

    // Receive-side: party
    Bind(t, 0x00FD, SemanticPacket::PartyInviteAckBasic);
    Bind(t, 0x02C5, SemanticPacket::PartyInviteAckExtended);
    Bind(t, 0x00FE, SemanticPacket::PartyInviteRequestBasic);
    Bind(t, 0x02C6, SemanticPacket::PartyInviteRequestExtended);
    Bind(t, 0x0104, SemanticPacket::PartyMemberAddedBasic);
    Bind(t, 0x01E9, SemanticPacket::PartyMemberAddedExtended);
    Bind(t, 0x0106, SemanticPacket::PartyHpUpdateBasic);
    Bind(t, 0x080E, SemanticPacket::PartyHpUpdateExtended);

    // Receive-side: skills
    Bind(t, 0x0115, SemanticPacket::SkillDamagePositionNotify);
    Bind(t, 0x0117, SemanticPacket::GroundSkillNotify);
    Bind(t, 0x011A, SemanticPacket::SkillNoDamageNotify);
    Bind(t, 0x011F, SemanticPacket::SkillUnitSetBasic);
    Bind(t, 0x01C9, SemanticPacket::SkillUnitSetExtended);
    Bind(t, 0x0114, SemanticPacket::SkillDamageNotifyBasic);
    Bind(t, 0x01DE, SemanticPacket::SkillDamageNotifyExtended);
    Bind(t, 0x019B, SemanticPacket::NotifyEffectBasic);
    Bind(t, 0x01F3, SemanticPacket::NotifyEffectDirect);

    // Receive-side: actor spawn / move
    Bind(t, 0x0078, SemanticPacket::ActorSpawnLegacyIdle);
    Bind(t, 0x0079, SemanticPacket::ActorSpawnLegacySpawn);
    Bind(t, 0x007A, SemanticPacket::ActorSpawnLegacyAlt);
    Bind(t, 0x007C, SemanticPacket::ActorSpawnLegacyNpc);
    Bind(t, 0x01D8, SemanticPacket::ActorSpawnLegacyIdleShifted);
    Bind(t, 0x01D9, SemanticPacket::ActorSpawnLegacySpawnShifted);
    Bind(t, 0x007B, SemanticPacket::ActorMoveLegacy);
    Bind(t, 0x01DA, SemanticPacket::ActorMoveLegacyShifted);
    Bind(t, 0x07F9, SemanticPacket::ActorSpawnVariableIdle);
    Bind(t, 0x07F8, SemanticPacket::ActorSpawnVariableSpawn);
    Bind(t, 0x0857, SemanticPacket::ActorSpawnVariableIdleRobe);
    Bind(t, 0x0858, SemanticPacket::ActorSpawnVariableSpawnRobe);
    Bind(t, 0x07F7, SemanticPacket::ActorMoveVariable);
    Bind(t, 0x0856, SemanticPacket::ActorMoveVariableRobe);
    Bind(t, 0x022A, SemanticPacket::ActorSpawnModernIdle);
    Bind(t, 0x022B, SemanticPacket::ActorSpawnModernSpawn);
    Bind(t, 0x02EE, SemanticPacket::ActorSpawnModernIdleFont);
    Bind(t, 0x02ED, SemanticPacket::ActorSpawnModernSpawnFont);
    Bind(t, 0x022C, SemanticPacket::ActorMoveModern);
    Bind(t, 0x02EC, SemanticPacket::ActorMoveModernFont);

    // Receive-side: actor name / state
    Bind(t, 0x0095, SemanticPacket::ActorNameAckBasic);
    Bind(t, 0x0194, SemanticPacket::ActorNameAckParty);
    Bind(t, 0x0195, SemanticPacket::ActorNameAckFull);
    Bind(t, 0x0119, SemanticPacket::ActorStateChangeBasic);
    Bind(t, 0x0229, SemanticPacket::ActorStateChangeExtended);
}

void PopulatePv23Semantics(VersionTable& t)
{
    BindSharedPv2xSemantics(t);

    // Send-side packets PV23 reshuffled to the 0x04xx range.
    Bind(t, 0x0436, SemanticPacket::WantToConnection);
    Bind(t, 0x0437, SemanticPacket::ActionRequest);
    Bind(t, 0x0438, SemanticPacket::UseSkillToId);
    Bind(t, 0x0439, SemanticPacket::UseItem);
}

void PopulatePv22Semantics(VersionTable& t)
{
    BindSharedPv2xSemantics(t);

    // PV22 keeps the pre-0x0436 layout for the four packets PV23 moved.
    Bind(t, 0x009B, SemanticPacket::WantToConnection);
    Bind(t, 0x0190, SemanticPacket::ActionRequest);
    Bind(t, 0x0072, SemanticPacket::UseSkillToId);
    Bind(t, 0x009F, SemanticPacket::UseItem);
}

void PopulatePv200Semantics(VersionTable& t)
{
    // Sabine Beta1 (iRO Ragexe 2002-02-20). Opcodes mirror MakePacketVer200*
    // in MapSendProfile.cpp (login/zone/gameplay) and MakePacketVer200ReceiveProfile
    // (receive). Standalone — does not inherit from the PV2x shared block.

    // Account login chain (Sabine Beta1)
    Bind(t, 0x0000, SemanticPacket::AccountLogin);
    Bind(t, 0x0006, SemanticPacket::NotifyError);

    // Character server (Sabine Beta1)
    Bind(t, 0x0001, SemanticPacket::CharServerEnter);
    Bind(t, 0x0002, SemanticPacket::SelectCharacter);
    Bind(t, 0x0003, SemanticPacket::MakeCharacter);
    Bind(t, 0x0004, SemanticPacket::DeleteCharacter);

    // Zone handshake / disconnect (Beta1 has no modern CZ_QUITGAME)
    Bind(t, 0x000E, SemanticPacket::WantToConnection);
    Bind(t, 0x001E, SemanticPacket::QuitGame);

    // Map gameplay send (Sabine Beta1 / Alpha-shifted opcodes)
    Bind(t, 0x0025, SemanticPacket::ActionRequest);
    Bind(t, 0x00AF, SemanticPacket::UseSkillToId);
    Bind(t, 0x00C6, SemanticPacket::CartOff);
    Bind(t, 0x00B2, SemanticPacket::UseSkillToPos);
    Bind(t, 0x0043, SemanticPacket::UseItem);
    Bind(t, 0x003B, SemanticPacket::TakeItem);
    Bind(t, 0x003E, SemanticPacket::DropItem);
    Bind(t, 0x00AE, SemanticPacket::SkillUp);
    Bind(t, 0x0045, SemanticPacket::EquipItem);
    Bind(t, 0x0047, SemanticPacket::UnequipItem);
    Bind(t, 0x0021, SemanticPacket::WalkToXY);
    Bind(t, 0x0037, SemanticPacket::ChangeDir);
    Bind(t, 0x001A, SemanticPacket::TickSend);
    Bind(t, 0x0019, SemanticPacket::NotifyActorInit);
    Bind(t, 0x0030, SemanticPacket::GetCharNameRequest);
    Bind(t, 0x0032, SemanticPacket::Whisper);
    Bind(t, 0x0028, SemanticPacket::GlobalMessage);

    // Map receive (Sabine Beta1)
    Bind(t, 0x000F, SemanticPacket::AcceptEnterLegacy);
    Bind(t, 0x001B, SemanticPacket::NotifyTime);
    Bind(t, 0x002D, SemanticPacket::MapChangeBasic);
    Bind(t, 0x002E, SemanticPacket::MapChangeServerMove);
    Bind(t, 0x0026, SemanticPacket::ActorActionNotifyBasic);
    Bind(t, 0x0027, SemanticPacket::ActorActionNotifyExtended);
    Bind(t, 0x0023, SemanticPacket::SelfMoveAck);
    Bind(t, 0x0036, SemanticPacket::BroadcastBasic);
    Bind(t, 0x0039, SemanticPacket::GroundItemEntryExisting);
    Bind(t, 0x003A, SemanticPacket::GroundItemEntryDropped);
    Bind(t, 0x003C, SemanticPacket::ItemPickupAckBasic);
    Bind(t, 0x0044, SemanticPacket::UseItemAckBasic);
    Bind(t, 0x004B, SemanticPacket::ItemRemoveBasic);
    Bind(t, 0x0014, SemanticPacket::ActorSpawnLegacyIdle);
    Bind(t, 0x0015, SemanticPacket::ActorSpawnLegacySpawn);
    Bind(t, 0x0018, SemanticPacket::ActorSpawnLegacyNpc);
    Bind(t, 0x0017, SemanticPacket::ActorMoveLegacy);
    Bind(t, 0x0022, SemanticPacket::ActorMoveLegacyShifted);
    Bind(t, 0x0031, SemanticPacket::ActorNameAckBasic);
    Bind(t, 0x005A, SemanticPacket::ActorStateChangeBasic);
    Bind(t, 0x00B5, SemanticPacket::ActorStateChangeExtended);
    Bind(t, 0x00B1, SemanticPacket::SkillDamagePositionNotify);
    Bind(t, 0x00B3, SemanticPacket::GroundSkillNotify);
    Bind(t, 0x00B6, SemanticPacket::SkillNoDamageNotify);
    Bind(t, 0x00BB, SemanticPacket::SkillUnitSetBasic);
    Bind(t, 0x00B0, SemanticPacket::SkillDamageNotifyBasic);
    Bind(t, 0x003F, SemanticPacket::NormalInventoryListBasic);
    Bind(t, 0x0040, SemanticPacket::EquipInventoryListBasic);
    Bind(t, 0x0041, SemanticPacket::NormalStorageListBasic);
    Bind(t, 0x0042, SemanticPacket::EquipStorageListBasic);
    Bind(t, 0x0090, SemanticPacket::StorageItemAddedBasic);
}

VersionTable Build(PacketVersionId id,
    void (*populateSizes)(VersionTable&),
    void (*populateSemantics)(VersionTable&))
{
    VersionTable t;
    t.id = id;
    populateSizes(t);
    if (populateSemantics) {
        populateSemantics(t);
    }
    return t;
}

const VersionTable& Pv23()
{
    static const VersionTable t = Build(PacketVersionId::PacketVer23, PopulatePv23, PopulatePv23Semantics);
    return t;
}

const VersionTable& Pv22()
{
    // PV22 shares PV23's size table (matches prior FillPacketSizeTable behavior),
    // but the send-side opcodes for WantToConnection / ActionRequest / UseSkillToId /
    // UseItem differ, so semantic mapping is PV22-specific.
    static const VersionTable t = Build(PacketVersionId::PacketVer22, PopulatePv23, PopulatePv22Semantics);
    return t;
}

const VersionTable& Pv200()
{
    static const VersionTable t = Build(PacketVersionId::PacketVer200, PopulatePv200, PopulatePv200Semantics);
    return t;
}

} // namespace

const VersionTable& GetVersionTable(PacketVersionId id)
{
    switch (id) {
    case PacketVersionId::PacketVer22:  return Pv22();
    case PacketVersionId::PacketVer200: return Pv200();
    case PacketVersionId::PacketVer23:
    default:                            return Pv23();
    }
}

const VersionTable& GetActiveVersionTable()
{
    return GetVersionTable(GetActiveMapReceiveProfile().id);
}

u16 GetOpcode(const VersionTable& table, SemanticPacket sem)
{
    if (sem == SemanticPacket::kUnknown) {
        return 0;
    }
    const auto it = table.byName.find(sem);
    return it == table.byName.end() ? 0 : it->second;
}

u16 GetActiveOpcode(SemanticPacket sem)
{
    return GetOpcode(GetActiveVersionTable(), sem);
}

SemanticPacket GetSemantic(const VersionTable& table, u16 opcode)
{
    return table.byOpcode[opcode].semantic;
}

SemanticPacket GetActiveSemantic(u16 opcode)
{
    return GetSemantic(GetActiveVersionTable(), opcode);
}

} // namespace ro::net
