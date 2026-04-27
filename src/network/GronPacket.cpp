#include "GronPacket.h"

#include "MapSendProfile.h"

#include <array>
#include <mutex>

namespace ro::net {
namespace {

using PacketSizeTable = std::array<s16, 0x10000>;

PacketSizeTable g_packetSizePacketVer23{};
PacketSizeTable g_packetSizePacketVer22{};
PacketSizeTable g_packetSizePacketVer200{};
PacketSizeTable* g_buildPacketSizeTable = nullptr;
std::once_flag g_initOnce;

void SetPacketSize(u16 packetId, s16 size)
{
    if (g_buildPacketSizeTable) {
        (*g_buildPacketSizeTable)[packetId] = size;
    }
}

void FillPacketSizeTable(PacketSizeTable& packetSizeTable)
{
    g_buildPacketSizeTable = &packetSizeTable;
    packetSizeTable.fill(0);

    // Table aligned to the observed pre-Renewal receive family used by the active
    // server chain, starting from the packet_ver 23 reference set and keeping the
    // older actor-stream packet family that the live server still emits.
    SetPacketSize(0x0000, 54);              // CA_LOGIN (beta1)
    SetPacketSize(0x0001, 17);              // CH_ENTER (beta1)
    SetPacketSize(0x0002, 3);               // CH_SELECT_CHAR (beta1)
    SetPacketSize(0x0004, 46);              // CH_DELETE_CHAR (beta1)
    SetPacketSize(0x0005, kVariablePacketSize); // AC_ACCEPT_LOGIN (beta1)
    SetPacketSize(0x0006, 3);               // AC_REFUSE_LOGIN (beta1)
    SetPacketSize(0x0007, kVariablePacketSize); // HC_ACCEPT_ENTER (beta1)
    SetPacketSize(0x0008, 3);               // HC_REFUSE_ENTER (beta1)
    SetPacketSize(0x0009, 92);              // HC_ACCEPT_MAKECHAR (beta1)
    SetPacketSize(0x000A, 3);               // HC_REFUSE_MAKECHAR (beta1)
    SetPacketSize(0x000B, 2);               // HC_ACCEPT_DELETECHAR (beta1)
    SetPacketSize(0x000C, 3);               // HC_REFUSE_DELETECHAR (beta1)
    SetPacketSize(0x000D, 28);              // HC_NOTIFY_ZONESVR (beta1)
    SetPacketSize(0x000E, 19);              // CZ_ENTER (beta1)
    SetPacketSize(0x000F, 11);              // ZC_ACCEPT_ENTER (beta1)
    SetPacketSize(0x0010, 3);               // ZC_REFUSE_ENTER (beta1)
    SetPacketSize(0x001D, 3);               // SC_NOTIFY_BAN (beta1)

    // --- Sabine Alpha/Beta1 receive stream (opcodes 0x0014 – 0x004C) ---
    // Sizes per PacketTable.100_Alpha.cs + PacketTable.200_Beta1.cs overrides.
    // These opcodes are unused by eAthena/packet_ver 23 servers, so wiring them
    // into the shared size table does not disturb the modern profile.
    SetPacketSize(0x0011, kVariablePacketSize); // ZC_NOTIFY_INITCHAR (unused in alpha, var)
    SetPacketSize(0x0012, 9);               // ZC_NOTIFY_UPDATECHAR
    SetPacketSize(0x0013, 5);               // ZC_NOTIFY_UPDATEPLAYER
    SetPacketSize(0x0014, 26);              // ZC_NOTIFY_STANDENTRY (beta1)
    SetPacketSize(0x0015, 25);              // ZC_NOTIFY_NEWENTRY (beta1)
    SetPacketSize(0x0016, 30);              // ZC_NOTIFY_ACTENTRY (beta1)
    SetPacketSize(0x0017, 32);              // ZC_NOTIFY_MOVEENTRY (beta1)
    SetPacketSize(0x0018, 25);              // ZC_NOTIFY_STANDENTRY_NPC (beta1)
    SetPacketSize(0x001B, 6);               // ZC_NOTIFY_TIME (alpha/beta1)
    SetPacketSize(0x001C, 7);               // ZC_NOTIFY_VANISH
    SetPacketSize(0x001F, 2);               // ZC_ACCEPT_QUIT
    SetPacketSize(0x0020, 2);               // ZC_REFUSE_QUIT
    SetPacketSize(0x0022, 16);              // ZC_NOTIFY_MOVE (monster/NPC, alpha/beta1)
    SetPacketSize(0x0023, 12);              // ZC_NOTIFY_PLAYERMOVE
    SetPacketSize(0x0024, 10);              // ZC_STOPMOVE
    SetPacketSize(0x0026, 27);              // ZC_NOTIFY_ACT (beta1)
    SetPacketSize(0x0027, 23);              // ZC_NOTIFY_ACT_POSITION (beta1-only)
    SetPacketSize(0x0028, kVariablePacketSize); // ZC_NOTIFY_CHAT
    SetPacketSize(0x0029, kVariablePacketSize); // ZC_NOTIFY_PLAYERCHAT
    SetPacketSize(0x002C, 22);              // ZC_NPCACK_MAPMOVE
    SetPacketSize(0x002D, 28);              // ZC_NPCACK_SERVERMOVE
    SetPacketSize(0x002E, 2);               // ZC_NPCACK_ENABLE
    SetPacketSize(0x0030, 54);              // ZC_ACK_REQNAME (beta1)
    SetPacketSize(0x0032, kVariablePacketSize); // ZC_WHISPER
    SetPacketSize(0x0033, 3);               // ZC_ACK_WHISPER
    SetPacketSize(0x0035, kVariablePacketSize); // ZC_BROADCAST
    SetPacketSize(0x0037, 7);               // ZC_CHANGE_DIRECTION
    SetPacketSize(0x0038, 38);              // ZC_ITEM_ENTRY (beta1)
    SetPacketSize(0x0039, 38);              // ZC_ITEM_FALL_ENTRY (beta1)
    SetPacketSize(0x003B, 33);              // ZC_ITEM_PICKUP_ACK (beta1)
    SetPacketSize(0x003C, 6);               // ZC_ITEM_DISAPPEAR
    SetPacketSize(0x003E, kVariablePacketSize); // ZC_NORMAL_ITEMLIST
    SetPacketSize(0x003F, kVariablePacketSize); // ZC_EQUIPMENT_ITEMLIST
    SetPacketSize(0x0040, kVariablePacketSize); // ZC_STORE_NORMAL_ITEMLIST
    SetPacketSize(0x0041, kVariablePacketSize); // ZC_STORE_EQUIPMENT_ITEMLIST
    SetPacketSize(0x0043, 7);               // ZC_USE_ITEM_ACK
    SetPacketSize(0x0045, 6);               // ZC_REQ_WEAR_EQUIP_ACK
    SetPacketSize(0x0047, 6);               // ZC_REQ_TAKEOFF_EQUIP_ACK
    SetPacketSize(0x0049, kVariablePacketSize); // ZC_REQ_ITEM_EXPLANATION_ACK
    SetPacketSize(0x004A, 6);               // ZC_ITEM_THROW_ACK
    SetPacketSize(0x004B, 6);               // ZC_PAR_CHANGE
    SetPacketSize(0x004C, 8);               // ZC_LONGPAR_CHANGE
    SetPacketSize(0x004E, 3);               // ZC_RESTART_ACK
    SetPacketSize(0x004F, kVariablePacketSize); // ZC_SAY_DIALOG
    SetPacketSize(0x0050, 6);               // ZC_WAIT_DIALOG
    SetPacketSize(0x0051, 6);               // ZC_CLOSE_DIALOG
    SetPacketSize(0x0052, kVariablePacketSize); // ZC_MENU_LIST
    // Beta1-only opcodes (0x00B0 – 0x00BC range) that collide with PV23 sizes
    // are applied later in ApplyPacketVer200Overrides.
    // End Sabine Alpha/Beta1 block

    SetPacketSize(0x0057, 6);               // ZC_STATUS_CHANGE_ACK (alpha/beta1)
    SetPacketSize(0x0058, 20);              // ZC_STATUS (alpha); Beta1 override (44) applied in ApplyPacketVer200Overrides
    SetPacketSize(0x0059, 5);               // ZC_STATUS_CHANGE (alpha/beta1)
    SetPacketSize(0x005B, 7);               // ZC_EMOTION (alpha/beta1)
    SetPacketSize(0x005D, 6);               // ZC_USER_COUNT (alpha/beta1)
    SetPacketSize(0x005E, 8);               // ZC_SPRITE_CHANGE (alpha/beta1)
    SetPacketSize(0x005F, 6);               // ZC_SELECT_DEALTYPE (alpha/beta1)
    SetPacketSize(0x0064, 55);              // CA_LOGIN  (client sends)
    SetPacketSize(0x0065, 17);              // CA_ENTER  (client sends)
    SetPacketSize(0x0066, 3);               // CZ_SELECT_CHAR (classic client)
    SetPacketSize(0x0067, 37);
    SetPacketSize(0x0069, kVariablePacketSize); // AC_ACCEPT_LOGIN (variable, has PacketLength)
    SetPacketSize(0x006A, 23);              // AC_REFUSE_LOGIN
    SetPacketSize(0x006B, kVariablePacketSize); // HC_ACCEPT_ENTER (variable char list)
    SetPacketSize(0x006C, 3);               // HC_REFUSE_ENTER
    SetPacketSize(0x006D, 110);             // HC_ACCEPT_MAKECHAR
    SetPacketSize(0x006E, 3);               // HC_REFUSE_MAKECHAR
    SetPacketSize(0x006F, 2);               // HC_ACCEPT_DELETECHAR
    SetPacketSize(0x0070, 3);               // HC_REFUSE_DELETECHAR
    SetPacketSize(0x0071, 28);
    SetPacketSize(0x0072, 25);
    SetPacketSize(0x0073, 11);
    SetPacketSize(0x0078, 55);
    SetPacketSize(0x0079, 53);
    SetPacketSize(0x007A, 58);
    SetPacketSize(0x007B, 60);
    SetPacketSize(0x007C, 42);
    SetPacketSize(0x01DB, 2);
    SetPacketSize(0x01DC, kVariablePacketSize);
    SetPacketSize(0x01DD, 47);
    SetPacketSize(0x01D8, 54);
    SetPacketSize(0x01D9, 53);
    SetPacketSize(0x01DA, 60);
    SetPacketSize(0x01F1, kVariablePacketSize);
    SetPacketSize(0x0204, 18);
    SetPacketSize(0x007E, 6);
    SetPacketSize(0x007F, 6);
    SetPacketSize(0x0080, 7);
    SetPacketSize(0x0085, 11);
    SetPacketSize(0x0081, 3);
    SetPacketSize(0x0086, 16);
    SetPacketSize(0x0087, 12);
    SetPacketSize(0x0088, 10);
    SetPacketSize(0x0089, 8);
    SetPacketSize(0x008A, 29);
    SetPacketSize(0x008D, kVariablePacketSize);
    SetPacketSize(0x008E, kVariablePacketSize);
    SetPacketSize(0x0091, 22);
    SetPacketSize(0x0092, 28);
    SetPacketSize(0x0097, kVariablePacketSize);
    SetPacketSize(0x0098, 3);
    SetPacketSize(0x0099, kVariablePacketSize);
    SetPacketSize(0x009A, kVariablePacketSize);
    SetPacketSize(0x009B, 26);
    SetPacketSize(0x009C, 9);
    SetPacketSize(0x009D, 17);
    SetPacketSize(0x009E, 17);
    SetPacketSize(0x0095, 30);
    SetPacketSize(0x00A0, 23);
    SetPacketSize(0x00A1, 6);
    SetPacketSize(0x00A3, kVariablePacketSize);
    SetPacketSize(0x00A4, kVariablePacketSize);
    SetPacketSize(0x00A7, 8);
    SetPacketSize(0x00A8, 7);
    SetPacketSize(0x00A9, 6);
    SetPacketSize(0x00AA, 7);
    SetPacketSize(0x00AC, 7);
    SetPacketSize(0x00AF, 6);
    SetPacketSize(0x00B0, 8);
    SetPacketSize(0x00B1, 8);
    SetPacketSize(0x00B2, 3);
    SetPacketSize(0x00B3, 3);
    SetPacketSize(0x00B4, kVariablePacketSize);
    SetPacketSize(0x00B5, 6);
    SetPacketSize(0x00B6, 6);
    SetPacketSize(0x00B7, kVariablePacketSize);
    SetPacketSize(0x00BA, 2);
    SetPacketSize(0x00BB, 5);
    SetPacketSize(0x00BC, 6);
    SetPacketSize(0x00BD, 44);
    SetPacketSize(0x00BE, 5);
    SetPacketSize(0x00BF, 3);
    SetPacketSize(0x00C0, 7);
    SetPacketSize(0x00C1, 2);
    SetPacketSize(0x00C2, 6);
    SetPacketSize(0x00C3, 8);
    SetPacketSize(0x00C4, 6);
    SetPacketSize(0x00C5, 7);
    SetPacketSize(0x00C6, kVariablePacketSize);
    SetPacketSize(0x00C7, kVariablePacketSize);
    SetPacketSize(0x00C8, kVariablePacketSize);
    SetPacketSize(0x00C9, kVariablePacketSize);
    SetPacketSize(0x00CA, 3);
    SetPacketSize(0x00CB, 3);
    SetPacketSize(0x00D1, 4);
    SetPacketSize(0x00D2, 4);
    SetPacketSize(0x00D3, 2);
    SetPacketSize(0x00D4, kVariablePacketSize);
    SetPacketSize(0x00D6, 3);
    SetPacketSize(0x00D7, kVariablePacketSize);
    SetPacketSize(0x00D8, 6);
    SetPacketSize(0x00D9, 14);
    SetPacketSize(0x00DA, 3);
    SetPacketSize(0x00DC, 28);
    SetPacketSize(0x00DD, 29);
    SetPacketSize(0x00DE, kVariablePacketSize);
    SetPacketSize(0x00DF, kVariablePacketSize);
    SetPacketSize(0x00E0, 30);
    SetPacketSize(0x00E1, 30);
    SetPacketSize(0x00E2, 26);
    SetPacketSize(0x00E3, 2);
    SetPacketSize(0x00E4, 6);
    SetPacketSize(0x00E5, 26);
    SetPacketSize(0x00E6, 3);
    SetPacketSize(0x00E7, 3);
    SetPacketSize(0x00E8, 8);
    SetPacketSize(0x00E9, 19);
    SetPacketSize(0x00EA, 5);
    SetPacketSize(0x00EB, 2);
    SetPacketSize(0x00EC, 3);
    SetPacketSize(0x00EE, 2);
    SetPacketSize(0x00EF, 2);
    SetPacketSize(0x00F2, 6);
        SetPacketSize(0x00A5, kVariablePacketSize);
        SetPacketSize(0x00A6, kVariablePacketSize);
    SetPacketSize(0x00F3, kVariablePacketSize);
    SetPacketSize(0x00F4, 21);
    SetPacketSize(0x00F5, 8);
    SetPacketSize(0x00F6, 8);
    SetPacketSize(0x01E9, 81);
    SetPacketSize(0x00F7, 22);
    SetPacketSize(0x00F8, 2);
    SetPacketSize(0x00F9, 26);
    SetPacketSize(0x00FA, 3);
    SetPacketSize(0x00FB, kVariablePacketSize);
    SetPacketSize(0x00FC, 6);
    SetPacketSize(0x00FD, 27);
    SetPacketSize(0x00FE, 30);
    SetPacketSize(0x00FF, 10);
    SetPacketSize(0x02B0, 85);
    SetPacketSize(0x02B1, kVariablePacketSize);
    SetPacketSize(0x02B2, kVariablePacketSize);
    SetPacketSize(0x02C5, 30);
    SetPacketSize(0x02C6, 30);
    SetPacketSize(0x0100, 2);
    SetPacketSize(0x0101, 6);
    SetPacketSize(0x0102, 6);
    SetPacketSize(0x0103, 30);
    SetPacketSize(0x0104, 79);
    SetPacketSize(0x0105, 31);
    SetPacketSize(0x0106, 10);
    SetPacketSize(0x080E, 14);
    SetPacketSize(0x0107, 10);              // ZC_NOTIFY_POSITION_TO_GROUPM (party minimap dot)
    SetPacketSize(0x0109, kVariablePacketSize);
    SetPacketSize(0x010E, 11);
    SetPacketSize(0x010F, kVariablePacketSize);
    SetPacketSize(0x0110, 10);
    SetPacketSize(0x0111, 39);
    SetPacketSize(0x0113, 22);              // CZ useskilltopos (packet_ver >= 22; client send)
    SetPacketSize(0x0116, 10);              // dropitem / legacy useskilltopos size on older profiles
    SetPacketSize(0x0114, 31);
    SetPacketSize(0x0115, 35);
    SetPacketSize(0x0117, 18);
    SetPacketSize(0x0119, 13);
    SetPacketSize(0x011A, 15);
    SetPacketSize(0x011F, 16);
    SetPacketSize(0x0120, 6);
    SetPacketSize(0x0121, 14);             // ZC_UPDATE_CARTINFO
    SetPacketSize(0x012B, 2);              // ZC_CLEAR_CART
    SetPacketSize(0x012F, kVariablePacketSize);
    SetPacketSize(0x0147, 39);              // ZC_AUTORUN_SKILL (e.g. Magnifier -> MC_IDENTIFY)
    SetPacketSize(0x0130, 6);
    SetPacketSize(0x0131, 86);
    SetPacketSize(0x0132, 6);
    SetPacketSize(0x0139, 16);
    SetPacketSize(0x013A, 4);
    SetPacketSize(0x013B, 4);
    SetPacketSize(0x013C, 4);
    SetPacketSize(0x013D, 6);
    SetPacketSize(0x013E, 24);
    SetPacketSize(0x013F, 26);
    SetPacketSize(0x0140, 22);
    SetPacketSize(0x0141, 14);
    SetPacketSize(0x0142, 6);
    SetPacketSize(0x0148, 8);
    SetPacketSize(0x014C, kVariablePacketSize); // ZC_MYGUILD_BASIC_INFO (ally list)
    SetPacketSize(0x014E, 6);                   // ZC_ACK_CREATE_GUILD
    SetPacketSize(0x0152, kVariablePacketSize); // ZC_GUILD_EMBLEM_IMG
    SetPacketSize(0x0154, kVariablePacketSize); // ZC_MEMBERMGR_INFO (entry 104)
    SetPacketSize(0x0156, kVariablePacketSize); // ZC_ACK_REQ_CHANGE_MEMBERS (entry 12)
    SetPacketSize(0x015A, 66);                  // ZC_ACK_BAN_GUILD
    SetPacketSize(0x015C, 66);                  // ZC_ACK_LEAVE_GUILD (mes 40)
    SetPacketSize(0x015E, 6);                   // ZC_ACK_DISORGANIZE_GUILD
    SetPacketSize(0x0160, kVariablePacketSize); // ZC_POSITION_ID_NAME_INFO
    SetPacketSize(0x0162, kVariablePacketSize); // ZC_GUILD_SKILLINFO
    SetPacketSize(0x0163, kVariablePacketSize);
    SetPacketSize(0x0164, kVariablePacketSize);
    SetPacketSize(0x0165, 30);
    SetPacketSize(0x0166, kVariablePacketSize);
    SetPacketSize(0x0167, 3);
    SetPacketSize(0x0168, 14);
    SetPacketSize(0x016A, 14);                  // ZC_REQ_JOIN_GUILD
    SetPacketSize(0x016C, 114);                 // ZC_UPDATE_GDID (basic info pre-renewal)
    SetPacketSize(0x016D, 14);                  // ZC_UPDATE_CHARSTAT
    SetPacketSize(0x016F, 182);                 // ZC_GUILD_NOTICE
    SetPacketSize(0x0170, 14);
    SetPacketSize(0x0171, 30);
    SetPacketSize(0x0172, 10);
    SetPacketSize(0x0173, 3);
    SetPacketSize(0x0174, kVariablePacketSize);
    SetPacketSize(0x0175, 6);
    SetPacketSize(0x017B, kVariablePacketSize);
    SetPacketSize(0x017C, 6);
    SetPacketSize(0x017D, 7);
    SetPacketSize(0x0177, kVariablePacketSize);
    SetPacketSize(0x0178, 4);
    SetPacketSize(0x0179, 5);
    SetPacketSize(0x0193, 2);
    SetPacketSize(0x0194, 30);
    SetPacketSize(0x0195, 102);
    SetPacketSize(0x0196, 9);
        SetPacketSize(0x0199, 4);               // ZC_NOTIFY_MAPPROPERTY
    SetPacketSize(0x0192, 24);
    SetPacketSize(0x019B, 10);
    SetPacketSize(0x01A2, 37);
    SetPacketSize(0x01A3, 5);
    SetPacketSize(0x01A4, 11);
    SetPacketSize(0x01A5, 26);
    SetPacketSize(0x01A6, kVariablePacketSize);
    SetPacketSize(0x01A7, 4);
    SetPacketSize(0x01A8, 4);
    SetPacketSize(0x01A9, 6);
    SetPacketSize(0x01AA, 10);
    SetPacketSize(0x01AB, 12);
    SetPacketSize(0x01AC, 6);
    SetPacketSize(0x01AD, kVariablePacketSize);
    SetPacketSize(0x01AE, 4);
    SetPacketSize(0x01AF, 4);
    SetPacketSize(0x01B0, 11);
    SetPacketSize(0x01B1, 7);
    SetPacketSize(0x01B2, kVariablePacketSize);
    SetPacketSize(0x01B3, 67);
    SetPacketSize(0x01B4, 12);
    SetPacketSize(0x01B5, 18);
    SetPacketSize(0x01B6, 114);
    SetPacketSize(0x01B7, 6);
    SetPacketSize(0x01B8, 3);
    SetPacketSize(0x01B9, 6);
    SetPacketSize(0x01BA, 26);
    SetPacketSize(0x01C3, kVariablePacketSize);
    SetPacketSize(0x01C4, 22);
    SetPacketSize(0x01C8, 13);
    SetPacketSize(0x01C9, 97);
    SetPacketSize(0x01CF, 28);
    SetPacketSize(0x01D0, 8);
    SetPacketSize(0x01D6, 4);
    SetPacketSize(0x01D4, 6);
    SetPacketSize(0x01D7, 11);
    SetPacketSize(0x01DE, 33);
    SetPacketSize(0x01E1, 8);
    SetPacketSize(0x01EE, kVariablePacketSize);
    SetPacketSize(0x01EF, kVariablePacketSize);
    SetPacketSize(0x01F0, kVariablePacketSize);
    SetPacketSize(0x01F3, 10);
    SetPacketSize(0x01F8, 2);
    SetPacketSize(0x01FF, 10);              // ZC_HIGHJUMP (clif_slide)
    SetPacketSize(0x0201, kVariablePacketSize);
    SetPacketSize(0x0502, 11);              // Observed in the live Beta1 stream; keep the framer aligned.
    SetPacketSize(0x0206, 11);
    SetPacketSize(0x0207, 34);
    SetPacketSize(0x0209, 36);
    SetPacketSize(0x020A, 10);
    SetPacketSize(0x0214, 42);
    SetPacketSize(0x0220, 10);
    SetPacketSize(0x0229, 15);
    SetPacketSize(0x022A, 58);
    SetPacketSize(0x022B, 57);
    SetPacketSize(0x022C, 65);
    SetPacketSize(0x0235, kVariablePacketSize);
    SetPacketSize(0x0239, 11);
    SetPacketSize(0x0283, 6);
    SetPacketSize(0x02EA, kVariablePacketSize);
    SetPacketSize(0x02EB, 13);
    SetPacketSize(0x02EC, 67);
    SetPacketSize(0x8482, 4);
    SetPacketSize(0x8483, 4);
    SetPacketSize(0x02ED, 59);
    SetPacketSize(0x02EE, 60);
    SetPacketSize(0x02EF, 8);
    SetPacketSize(0x02DD, 32);
    SetPacketSize(0x02D0, kVariablePacketSize);
    SetPacketSize(0x02D1, kVariablePacketSize);
    SetPacketSize(0x02D2, kVariablePacketSize);
    SetPacketSize(0x02D3, 4);
    SetPacketSize(0x02D4, 29);
    SetPacketSize(0x02D5, 2);
    SetPacketSize(0x029D, kVariablePacketSize);
    SetPacketSize(0x029E, 11);
    SetPacketSize(0x07F7, kVariablePacketSize);
    SetPacketSize(0x07F8, kVariablePacketSize);
    SetPacketSize(0x07F9, kVariablePacketSize);
    SetPacketSize(0x0856, kVariablePacketSize);
    SetPacketSize(0x0857, kVariablePacketSize);
    SetPacketSize(0x0858, kVariablePacketSize);
    SetPacketSize(0x0814, 86);
    SetPacketSize(0x0816, 6);
    SetPacketSize(0x02C9, 3);
    SetPacketSize(0x02D7, kVariablePacketSize);
    SetPacketSize(0x02D9, 10);              // ZC_CONFIG
    SetPacketSize(0x02DA, 3);
    SetPacketSize(0x02B9, 191);
    SetPacketSize(0x02BA, 11);
    SetPacketSize(0x06B3, 14);
    SetPacketSize(0x06B4, 14);
    SetPacketSize(0x0477, 8);
    SetPacketSize(0x0569, 8);
    SetPacketSize(0x02E1, 33);
    SetPacketSize(0x02E7, kVariablePacketSize);
    SetPacketSize(0x02E9, kVariablePacketSize);
    SetPacketSize(0x02E8, kVariablePacketSize);
    SetPacketSize(0x0459, 8);
    SetPacketSize(0x045A, 10);
    SetPacketSize(0x045B, kVariablePacketSize);
    SetPacketSize(0x045C, kVariablePacketSize);
    SetPacketSize(0x0463, kVariablePacketSize);
    SetPacketSize(0x045A, 10);
    SetPacketSize(0x045B, kVariablePacketSize);
    SetPacketSize(0x045C, kVariablePacketSize);
    SetPacketSize(0x0461, kVariablePacketSize);
    SetPacketSize(0x0465, 10);
    SetPacketSize(0x0466, 10);
    SetPacketSize(0x0467, 12);
    SetPacketSize(0x0468, 12);
    SetPacketSize(0x0469, 10);
    SetPacketSize(0x046F, 12);
    SetPacketSize(0x0470, 12);
    SetPacketSize(0x0471, 10);
    SetPacketSize(0x0477, 8);
    SetPacketSize(0x043F, 25);
    SetPacketSize(0x05EA, 10);
    SetPacketSize(0x06B3, 14);
    SetPacketSize(0x06B4, 14);
    SetPacketSize(0x0517, 4);
    SetPacketSize(0x0518, 4);
    SetPacketSize(0x0519, 6);
    SetPacketSize(0x06C8, 6);
    SetPacketSize(0x06C9, 7);
    SetPacketSize(0x06CA, 8);
    SetPacketSize(0x06CE, 10);
    SetPacketSize(0x07FA, 8);
    SetPacketSize(0x02DC, kVariablePacketSize);

    g_buildPacketSizeTable = nullptr;
}

// Overrides on top of the shared fill, applied only to the Sabine Beta1 profile.
// These opcodes collide with PV23 semantics (different payloads, same opcode).
//
// Beta1 inserts three new packets into the Alpha opcode list:
//   ZC_NOTIFY_ACT_POSITION at 0x0027 → shifts Alpha 0x0027-0x009B by +1
//   ZC_GROUPINFO_CHANGE    at 0x009D → additionally shifts Alpha 0x009C-0x00AA by +1
//   CZ_CHANGE_GROUPEXPOPTION at 0x009E → additionally shifts Alpha 0x009C-0x00AA by +1
// Net: Alpha 0x0027-0x009B → Beta1 +1. Alpha 0x009C-0x00AA → Beta1 +3.
void ApplyPacketVer200Overrides(PacketSizeTable& t)
{
    g_buildPacketSizeTable = &t;

    // --- Opcodes shifted +1 from their Alpha positions (alpha 0x0027-0x009B) ---
    SetPacketSize(0x002D, 22);              // ZC_NPCACK_MAPMOVE    (alpha 0x002C)
    SetPacketSize(0x002E, 28);              // ZC_NPCACK_SERVERMOVE (alpha 0x002D)
    SetPacketSize(0x002F, 2);               // ZC_NPCACK_ENABLE     (alpha 0x002E) — framer alignment
    SetPacketSize(0x0031, 54);              // ZC_ACK_REQNAME       (alpha 0x0030, beta1 size 54)
    SetPacketSize(0x0036, kVariablePacketSize); // ZC_BROADCAST     (alpha 0x0035)
    SetPacketSize(0x0039, 38);              // ZC_ITEM_ENTRY        (alpha 0x0038, beta1 size 38)
    SetPacketSize(0x003A, 38);              // ZC_ITEM_FALL_ENTRY   (alpha 0x0039, beta1 size 38)
    SetPacketSize(0x003C, 33);              // ZC_ITEM_PICKUP_ACK   (alpha 0x003B, beta1 size 33)
    SetPacketSize(0x003F, kVariablePacketSize); // ZC_NORMAL_ITEMLIST (alpha 0x003E)
    SetPacketSize(0x0040, kVariablePacketSize); // ZC_EQUIPMENT_ITEMLIST (alpha 0x003F)
    SetPacketSize(0x0041, kVariablePacketSize); // ZC_STORE_NORMAL_ITEMLIST (alpha 0x0040)
    SetPacketSize(0x0042, kVariablePacketSize); // ZC_STORE_EQUIPMENT_ITEMLIST (alpha 0x0041)
    SetPacketSize(0x0044, 7);               // ZC_USE_ITEM_ACK      (alpha 0x0043)
    SetPacketSize(0x004B, 6);               // ZC_ITEM_THROW_ACK    (alpha 0x004A)
    SetPacketSize(0x004C, 6);               // ZC_PAR_CHANGE        (alpha 0x004B — overrides standard 8-byte LONGPAR_CHANGE)
    SetPacketSize(0x004D, 8);               // ZC_LONGPAR_CHANGE    (alpha 0x004C — overrides standard 3-byte CZ_RESTART)
    SetPacketSize(0x0059, 44);              // ZC_STATUS            (alpha 0x0058, beta1 size 44)
    SetPacketSize(0x005A, 5);               // ZC_STATUS_CHANGE     (alpha 0x0059)

    // --- Opcodes shifted +3 (alpha 0x009C-0x00AA) ---
    SetPacketSize(0x00AA, 9);               // ZC_SKILLINFO_UPDATE  (alpha 0x00A7)
    SetPacketSize(0x00AB, kVariablePacketSize); // ZC_SKILLINFO_LIST (alpha 0x00A8)
    SetPacketSize(0x00AC, 6);               // ZC_ACK_TOUSESKILL   (alpha 0x00A9)
    SetPacketSize(0x00AD, 39);              // ZC_ADD_SKILL         (alpha 0x00AA, beta1 size 39)

    // --- Beta1 member/party packets (alpha-derived, sizes changed) ---
    SetPacketSize(0x0078, 28);              // ZC_MEMBER_NEWENTRY   (alpha 0x0077, beta1 size 28)
    SetPacketSize(0x0079, 29);              // ZC_MEMBER_EXIT       (alpha 0x0078, beta1 size 29)

    // --- Beta1 new packets inserted at final positions (not shifted, appended) ---
    SetPacketSize(0x009D, 6);               // ZC_GROUPINFO_CHANGE  (beta1 new)
    SetPacketSize(0x00B0, 31);              // ZC_NOTIFY_SKILL
    SetPacketSize(0x00B1, 35);              // ZC_NOTIFY_SKILL_POSITION
    SetPacketSize(0x00B3, 18);              // ZC_NOTIFY_GROUNDSKILL
    SetPacketSize(0x00B5, 13);              // ZC_STATE_CHANGE
    SetPacketSize(0x00B6, 15);              // ZC_USE_SKILL
    SetPacketSize(0x00BB, 11);              // ZC_SKILL_ENTRY
    SetPacketSize(0x00BD, 14);              // ZC_NOTIFY_CARTITEM_COUNTINFO (beta1)
    SetPacketSize(0x00D9, 6);               // ZC_RECOVERY (beta1)
    SetPacketSize(0x00DA, 24);              // ZC_USESKILL_ACK (beta1)

    g_buildPacketSizeTable = nullptr;
}

const PacketSizeTable& GetActivePacketSizeTable()
{
    switch (GetActiveMapReceiveProfile().id) {
    case PacketVersionId::PacketVer200:
        return g_packetSizePacketVer200;
    case PacketVersionId::PacketVer22:
        return g_packetSizePacketVer22;
    case PacketVersionId::PacketVer23:
    default:
        return g_packetSizePacketVer23;
    }
}

} // namespace

void InitializePacketSize()
{
    std::call_once(g_initOnce, []() {
        FillPacketSizeTable(g_packetSizePacketVer23);
        FillPacketSizeTable(g_packetSizePacketVer22);
        FillPacketSizeTable(g_packetSizePacketVer200);
        ApplyPacketVer200Overrides(g_packetSizePacketVer200);
    });
}

s16 GetPacketSize(u16 packetId)
{
    InitializePacketSize();
    return GetActivePacketSizeTable()[packetId];
}

bool IsVariableLengthPacket(u16 packetId)
{
    return GetPacketSize(packetId) == kVariablePacketSize;
}

} // namespace ro::net
