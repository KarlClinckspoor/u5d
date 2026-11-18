#include "common/common.h"
#include "time/time.h"

#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include "cast.h"
#include "cast2.h"
#include "cmds.h"
#include "dnglook.h"
#include "dungeon.h"
#include "lookobj.h"
#include "sjog.h"
#include "talk.h"
#include "town.h"
#include "zstats.h"

int MAINOUT_06ec_AttackCmd(void);
int MAINOUT_08de_EnterCmd(void);

// DS:55a6-6a0a: the saved game (55a6-6606), the map buffers and the game state, zero in the
// original's DATA.OVL. Initialised data, so in some module's _DATA between 2000.c's (ends
// at 55a6) and this module's tables (6a0a); which of the two is not known (the bytes are
// the same either way).
u16 D_55a6_saveStart = 0; // +0x0
S_55a8 D_55a8_party[0x10] = {0}; // ~57a8)
s16 D_57a8_food = 0; // +0x0202 Food // int (TALK_0682_ReceiveItem, MOVSX)
s16 D_57aa_gold = 0; // +0x0204 Gold // int (MAINOUT_1b3e, MOVSX)
u8 D_57ac_keys = 0; // +0x0206 #Keys
u8 D_57ad_gems = 0; // +0x0207 #Gems
u8 D_57ae_torches = 0; // +0x0208 #Torches
u8 D_57af_grapple = 0; // +0x0209 Grapple
u8 D_57b0_carpets = 0; // +0x020A #Magic Carpets
u8 D_57b1_skullKeys = 0; // +0x020B #Skull Keys
u8 D_57b2_skullKeysDay = 0; // +0x020C Last day Minoc skull keys were taken
u8 D_57b3_amulet = 0; // +0x020D Amulet of Lord British
u8 D_57b4_crown = 0; // +0x020E Crown of Lord British
u8 D_57b5_sceptre = 0; // +0x020F Sceptre of Lord British
u8 D_57b6_shards[4] = {0}; // +0x0210 Shards   // [57b6..57ba)
u8 D_57ba_spyglasses = 0; // +0x0214 #Spy Glass
u8 D_57bb_hmsCapePlans = 0; // +0x0215 HMS Cape Plans
u8 D_57bc_sextants = 0; // +0x0216 #Sextant
u8 D_57bd_pocketWatch = 0; // +0x0217 Pocket Watch
u8 D_57be_blackBadge = 0; // +0x0218 Black Badge
u8 D_57bf_sandalwoodBox = 0; // +0x0219 Sandalwood Box
u8 D_57c0_equipment[0x30] = {0}; // +0x021A Equipment(Quantity owned by the party) [57c0..57f0)
u8 D_57f0_mixedSpells[0x30] = {0}; // +0x024A Spell Mixtures [57f0..5820)
u8 D_5820_scrolls[8] = {0}; // +0x027A Scrolls
u8 D_5828_potions[8] = {0}; // +0x0282 Potions (Blue, Yellow, Red, Green, Orange, Purple, Black, White)
u8 D_5830_moonstoneX[8] = {0}; // +0x028A 0-0xFF Moonstone X-coordinates (valid only if buried)
u8 D_5838_moonstoneY[8] = {0}; // +0x0292 0-0xFF Moonstone Y-coordinates (valid only if buried)
u8 D_5840_moonstoneMap[8] = {0}; // +0x029A 0=buried,0xFF=Inventory moonstone flags
u8 D_5848_moonstoneZ[8] = {0}; // +0x02A2 0=Britannia,0xFF=Underworld moonstone Z-coordinates (valid only if buried)
u8 D_5850_reagents[8] = {0}; // +0x02AA Reagents (A..H)
u8 D_5858_harvestDay[3] = {0}; // +0x02B2 Day each of the three nightshade/mandrake spots was last harvested [5858..585b)
u8 D_585b_partyCount = 0; // +0x02B5 party member count
u8 D_585c_takenItemFlags[0x18] = {0}; // [585c..5874)
u16 D_5874_year = 0; // +0x02CE Current year (TODO: int?)
s16 D_5876_tempX = 0; // selected x direction? // +0x02D0 first party member that is not dead or sleeping? # of active monsters & charmed PCs[2]?
s16 D_5878_tempY = 0; // selected y direction? // +0x02D2 # of active PCs and charmed monsters[2]?
u8 D_587a_statusIcon = 0; // +0x02D4 Icon
u8 D_587b_activePlayer = 0; // +0x02D5 Active character
u8 D_587c_partyTile = 0; // +0x02D6 party tile
u8 D_587d_month = 0; // +0x02D7 Current month
u8 D_587e_day = 0; // +0x02D8 Current day
u8 D_587f_hour = 0; // +0x02D9 Current hour
u8 D_5880_lastHour = 0; // +0x02DA copy of 0x2D9
u8 D_5881_minute = 0; // +0x02DB Current minute
u8 D_5882 = 0;
u8 D_5883 = 0; // ?
u8 D_5884_clockChimes = 0;
u8 D_5885_trammelPhase = 0; // +0x02DF Phase of Trammel
u8 D_5886_feluccaPhase = 0; // +0x02E0 Phase of Felucca
u8 D_5887_moongateHeight = 0; // +0x02E1 Current moongate height (in pixels)
u8 D_5888_karma = 0; // +0x02E2 Karma
u8 D_5889 = 0;
u8 D_588a = 0;
u8 D_588b_turnCount = 0; // +0x02E5 Current turn count
u8 D_588c_campHealWait = 0;
u8 D_588d = 0;
u8 D_588e_protectTurns = 0; // +0x02E8 Protective spell duration
u8 D_588f = 0;
u8 D_5890 = 0;
u8 D_5891_animate = 0; // +0x02EB animate flag
u8 D_5892_windDir = 0; // +0x02EC wind direction
u8 D_5893_mapId = 0; // +0x02ED map_id (21h~: dungeon, 40: unloaded)
u8 D_5894_savedMapId = 0; // +0x02EE Combat backup of 0x2ED
u8 D_5895_mapLevel = 0; // +0x02EF level (depth)
u8 D_5896_mapX = 0; // +0x02F0 map_x
u8 D_5897_mapY = 0; // +0x02F1 map_y
u8 D_5898_crosshairOn = 0; // crosshair visible
u8 D_5899_crosshairX = 0; // crosshair x
u8 D_589a_crosshairY = 0; // crosshair y
u8 D_589b_mapOriginX = 0; // x upper left chunk
u8 D_589c_mapOriginY = 0; // y upper left chunk
u8 D_589d_attackWeapon = 0; // +0x02F7 Attacker's weapon
u8 D_589e_curCombatant = 0; // +0x02F8 ?
u8 D_589f_focusBlink = 0; // +0x02F9 show focus rectangle & crosshair?
u8 D_58a0_combatExitDir = 0; // +0x02FA exit direction from combat map
u8 D_58a1_combatFlags = 0; // +0x02FB combat type flags
u8 D_58a2 = 0;
u8 D_58a3_combatOver = 0; // +0x02FD battle successful flag (win?)
u8 D_58a4_viewOn = 0; // +0x02FE Update/animate 2D map?
u8 D_58a5_lightLevel = 0; // +0x02FF Current light intensity
u8 D_58a6_lightSpellTurns = 0;
u8 D_58a7_torchTurns = 0;
u8 D_58a8[0x20] = {0}; // [58a8..58c8)
u8 D_58c8_shadowlordTowns[3] = {0}; // 58cb)
u8 D_58cb = 0;
u16 D_58cc_questsOrdained = 0; // shrine quest active flags TODO: u8?
u16 D_58ce_questsDone = 0; // shrine quest completed flags TODO: u8?
u8 D_58d0_dungeonOpen[8] = {0};
u8 D_58d8_shrineDestroyed[8] = {0};
u8 D_58e0_roomsCleared[0xe] = {0}; // ..58ee)
u8 D_58ee_annotX[0x20] = {0}; // ..590e) spell counts
u8 D_590e_annotY[0x20] = {0}; // ..592e)
u8 D_592e_annotTile[0x20] = {0}; // .594e)
u8 D_594e_annotCount = 0;
u8 D_594f_openDoorTile = 0; // +0x03A9 opened door
u8 D_5950_openDoorX = 0; // +0x03AA open_door_x
u8 D_5951_openDoorY = 0; // +0x03AB open_door_y
u8 D_5952_openDoorTurns = 0; // +0x03AC # of turns the door stays open
u8 D_5953_boughtShipX = 0; // +0x03AD ship_x
u8 D_5954_boughtShipY = 0; // +0x03AE ship_y
u8 D_5955_sailDir = 0;
u8 D_5956_newPrompt = 0;
u8 D_5957_drunkTurns = 0; // +0x03B1 remaining drunk turn count (TOWN_0dc4_GetTownKey)
u8 D_5958_townShadowlord = 0;
u8 D_5959 = 0;
u8 D_595a_dungeonMap[0x200] = {0}; // +0x03B4 dungeon map data ..5b5a)
u8 D_5b5a_npcKilled[0x80] = {0}; // +0x05B4 NPC killed flags (4 bytes / 32 bits per settlement) [..5bda) // TODO: u32
u8 D_5bda_npcMet[0x80] = {0}; // +0x0634 NPC met flags (4 bytes / 32 bits per settlement) [..5c5a)
ActorFmt D_5c5a_actors[32] = {0}; // ~5d5a) "Object" struct
u16 D_5d5a = 0; // unused?
u16 D_5d5c = 0; // unused?
NpcScheduleFmt D_5d5e_npcSchedules[32] = {0}; // +0x07B8 NPC Schedules [..5f5e) 0x200
NpcFmt D_5f5e_npcs[32] = {0}; // +0x09B8 Character/monster states (16bytes * 32) [..615e) 0x200
MovementFmt D_615e_moveLists[0x20] = {0}; // +0x0BB8 Movement list table [..655e)
s16 D_655e_moveListPos[0x20] = {0}; // +0x0FB8 Movement list pointers [..659e)
u8 D_659e_npcTypes[0x20] = {0}; // +0x0FF8 NPC types [..65be)
u8 D_65be = 0;
u8 D_65bf = 0;
u16 D_65c0 = 0; // unused?
s16 D_65c2[0x20] = {0};
u8 D_6602 = 0;
u8 D_6603_dungeonFacing = 0; // +0x105D orientation in a dungeon
u8 D_6604_dungeonStyle = 0; // +0x105E graphics for the current dungeon
u8 D_6605_boughtShip = 0; // +0x105F purchased ship/skiff
u16 D_6606_saveEnd = 0; // end
S_6608_Map D_6608_map = {0}; // ~6a08) map data: layer4 * y16 * x16 / y32 * x32
// (D_6708_map, DS:6708, is a place inside D_6608_map: vars.h)
u8 D_6a08 = 0; // some audio related
u8 D_6a09_pad[1] = {0}; // no name in vars.h

u16 D_6a0a_powersOfTen[5] = { 1, 10, 100, 1000, 10000 };

u8 D_6a14[0x20] =
{
    0xff, 0xf3, 0xc3, 0x8f, 0xff, 0xff, 0xff, 0xc0,
    0xdd, 0xf8, 0x03, 0xdf, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff
};

// shake screen
void ULTIMA_3072_ShakeScreen(void)
{
    int i;
    int local_8;

    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(0, 1);
    ULTIMA_0c22_GRAP_0f_SelectPage(1);

#if defined(TARGET_SDL)
    ULTIMA_223c_AudioWhiteNoise(0x13, 16000, 0x96);
#endif

    for (local_8 = 0; local_8 < 8; local_8++)
    {
        for (i = 8; i <= 0xb3; i += 3)
        {
            ULTIMA_71ca_DRV_27_ScrollTextWindow1(8, i, 0xb7, i + 4, 2);
#if defined(MATCHING_BUILD)
            ULTIMA_22e2_PcspkOn(ULTIMA_2092_RandomRange(0x13, 0x96));
#endif
        }

#if defined(TARGET_DOS32)
        ULTIMA_223c_AudioWhiteNoise(0x13, 700, 0x96);
#elif defined(ENABLE_SPEED_FIX)
        TIME_SleepMs(70);
#endif

        for (i = 8; i <= 0xb3; i += 3)
        {
            ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 8, i, 0xb7, i + 4);
#if defined(MATCHING_BUILD)
            ULTIMA_22e2_PcspkOn(ULTIMA_2092_RandomRange(0x13, 0x96));
#endif
        }

#if defined(TARGET_DOS32)
        ULTIMA_223c_AudioWhiteNoise(0x13, 300, 0x96);
#elif defined(ENABLE_SPEED_FIX)
        TIME_SleepMs(30);
#endif

        for (i = 0xb3; i >= 8; i -= 3)
        {
            ULTIMA_7200_DRV_27_ScrollTextWindow2(8, i, 0xb7, i + 4, 2);
#if defined(MATCHING_BUILD)
            ULTIMA_22e2_PcspkOn(ULTIMA_2092_RandomRange(0x13, 0x96));
#endif
        }

#if defined(TARGET_DOS32)
        ULTIMA_223c_AudioWhiteNoise(0x13, 700, 0x96);
#elif defined(ENABLE_SPEED_FIX)
        TIME_SleepMs(70);
#endif

        for (i = 0xb3; i >= 8; i -= 3)
        {
            ULTIMA_0ace_GRAP_18_TransferArea(1, 0, 8, i, 0xb7, i + 4);
#if defined(MATCHING_BUILD)
            ULTIMA_22e2_PcspkOn(ULTIMA_2092_RandomRange(0x13, 0x96));
#endif
        }

#if defined(TARGET_DOS32)
        ULTIMA_223c_AudioWhiteNoise(0x13, 300, 0x96);
#elif defined(ENABLE_SPEED_FIX)
        TIME_SleepMs(30);
#endif
    }

    ULTIMA_0c22_GRAP_0f_SelectPage(0);
#if defined(MATCHING_BUILD)
    ULTIMA_230e_PcspkOff();
#endif
}

static void ULTIMA_3c9a_HoleUpCmd(void);

int ULTIMA_3178_ProcessCommand(int param_1)
{
    int result;
    int local_6;

    result = 1;
    switch (param_1)
    {
    case U5_KEY_CTRL_B:
        ULTIMA_1850_PrintString(_TEXT(0xa110, "Buffer O"));
        D_538c_keyBufferOff = !D_538c_keyBufferOff;
        if (D_538c_keyBufferOff != 0) 
        {
            ULTIMA_1850_PrintString(_TEXT(0xa11a, "ff\n"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0xa11e, "n\n"));
        }

        result = 0;
        break;
    case U5_KEY_SPACE:
        /* ' ' Pass */
        if (D_5893_mapId == 0 && D_5955_sailDir != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa122, "Sheets in irons!\n"));
            D_5955_sailDir = 0;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0xa134, "Pass\n"));
        }
        break;
    case 0x41:
        /* 'A' */
        // 3216
        result = D_5893_mapId == 0 ?
            MAINOUT_06ec_AttackCmd() :
                D_5893_mapId < 0x21 ?
            TOWN_09e6_AttackCmd() :
            DUNGEON_1d4a_AttackCmd();
        break;
    case 0x42:
        /* 'B' Board */
        ULTIMA_1850_PrintString(_TEXT(0xa13a, "Board "));
        result = CMDS_07f6_BoardCmd();
        break;
    case 0x43:
        /* 'C' Cast */
        ULTIMA_1850_PrintString(_TEXT(0xa142, "Cast...\n"));
        result = CAST_0dba_CastSpellCmd();
        break;
    case 0x44:
        /* 'D' What? */
        ULTIMA_1850_PrintString(_TEXT(0xa14c, "D-What?\n"));
        result = 0;
        break;
    case 0x45:
        /* 'E' Enter */
        if (D_5893_mapId == 0)
        {
            result = MAINOUT_08de_EnterCmd();
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0xa156, "Enter what?\n"));
        break;
    case 0x46:
        /* 'F' Fire */
        ULTIMA_1850_PrintString(_TEXT(0xa164, "Fire-"));
        CMDS_0aea_FireCmd();
        break;
    case 0x47:
        /* 'G' Get */
        if (D_5893_mapId < 0x21)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa16a, "Get-"));
        }
        SJOG_18ce_GetCmd();
        break;
    case 0x48:
        /* 'H' Hole up */
        if (D_5893_mapId == 0 || D_5893_mapId > 0x20)
        {
            // outmap_hole_up_cmd
            ULTIMA_3c9a_HoleUpCmd();
            break;
        }
        local_6 = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
        ULTIMA_1850_PrintString(_TEXT(0xa170, "Hole up- "));
        if (local_6 != TILE_MAP_BED)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa17a, "Only in bed!\n"));
        }
        else
        {
            CMDS_0552_HoleUpCmd();
        }
        break;
    case 0x49:
        /* 'I' Ignite torch */
        ULTIMA_1850_PrintString(_TEXT(0xa188, "Ignite torch!\n"));
        CMDS_0d98_IgniteTorchCmd();
        break;
    case 0x4a:
        /* 'J' Jimmy */
        ULTIMA_1850_PrintString(_TEXT(0xa198, "Jimmy-"));
        SJOG_0d4a_JimmyCmd();
        break;
    case 0x4b:
        /* 'K' Klimb */
        if (D_5893_mapId == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa1a0, "Klimb-"));
            CMDS_1c20_KlimbCmd();
        }
        else if (D_5893_mapId < 0x21)
        {
            result = TOWN_0b82_KlimbCmd();
        }
        else
        {
            result = DUNGEON_1e10_KlimbCmd();
        }
        break;
    case 0x4c:
        /* 'L' Look */
        ULTIMA_1850_PrintString(_TEXT(0xa1a8, "Look"));
        if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
        {
            /* 0x21..0x28 (Dungeon) */
            ULTIMA_1850_PrintString(_TEXT(0xa1ae, "...\n"));
            DNGLOOK_0000_LookCmdInDungeon();
        }
        else
        {
            ULTIMA_16ba_PrintChar('-');
            LOOKOBJ_099c_LookCmd();
        }
        break;
    case 0x4d:
        /* 'M' Mix */
        ULTIMA_1850_PrintString(_TEXT(0xa1b4, "Mix Reagents\n\n"));
        CMDS_1ad8_MixCmd();
        break;
    case 0x4e:
        /* 'N' New order */
        ULTIMA_1850_PrintString(_TEXT(0xa1c4, "New Order"));
        CMDS_0ddc_NewOrderCmd();
        break;
    case 0x4f:
        /* 'O' Open */
        ULTIMA_1850_PrintString(_TEXT(0xa1ce, "Open-"));
        SJOG_1374_OpenCmd();
        break;
    case 0x50:
        /* 'P' Push */
        if (D_5893_mapId > 0x20 && D_5893_mapId < 0x29)
        {
            /* Cannot push in dungeons */
            ULTIMA_1850_PrintString(_TEXT(0xa1d4, "Push\nNot here!\n"));
            result = 0;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0xa1e4, "Push-"));
            CMDS_161a_PushCmd();
        }
        break;
    case 0x51:
        /* 'Q' Quit */
        ULTIMA_1850_PrintString(_TEXT(0xa1ea, "Quit:"));
        CAST2_10fe_SaveGame();
        result = 0;
        break;
    case 0x52:
        /* 'R' Ready */
        ULTIMA_1850_PrintString(_TEXT(0xa1f0, "Ready...\n\n"));
        ZSTATS_1296_ReadyCmd();
        break;
    case 0x53:
        /* 'S' Search */
        if (D_5893_mapId < 0x21)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa1fc, "Search-"));
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0xa204, "Search...\n"));
        }
        SJOG_095c_SearchCmd();
        break;
    case 0x54:
        /* 'T' Talk */
        if (D_5893_mapId == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa210, "Talk-"));
            if (ULTIMA_35ec_SelectDirection() == 0)
                break;
            ULTIMA_1850_PrintString(_TEXT(0xa216, "Funny, no response!\n"));
        }
        else
        {
            if (D_5893_mapId > 0x20)
            {
                ULTIMA_1850_PrintString(_TEXT(0xa22c, "Talk-Funny, no response!\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0xa246, "Talk-"));
                if (TALK_041c_TalkCmd() != 0)
                {
                    result = 2;
                }
            }
        }
        break;
    case 0x55:
        /* 'U' Use item */
        ULTIMA_1850_PrintString(_TEXT(0xa24c, "Use item\n\n"));
        CAST_1792_UseCmd();
        break;
    case 0x56:
        /* 'V' View a gem */
        ULTIMA_1850_PrintString(_TEXT(0xa258, "View a gem!\n"));
        if (D_57ad_gems != 0)
        {
            D_57ad_gems--;
            if (D_5893_mapId < 0x21)
            {
                LOOKOBJ_10fc_ViewCmd(D_5896_mapX, D_5897_mapY);
            }
            else
            {
                DNGLOOK_06a8_ViewCmd();
            }
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0xa266, "You have none!\n"));
        break;
    case 0x57:
        /* 'W' What? */
        ULTIMA_1850_PrintString(_TEXT(0xa276, "W-What?\n"));
        result = 0;
        break;
    case 0x58:
        /* 'X' X-it */
        ULTIMA_1850_PrintString(_TEXT(0xa280, "X-it "));
        CMDS_0eb4_XitCmd();
        break;
    case 0x59:
        /* 'Y' Yell */
        ULTIMA_1850_PrintString(_TEXT(0xa286, "Yell "));
        result = CMDS_1418_YellCmd();
        break;
    case 0x5a:
        /* 'Z' Z-stats */
        ULTIMA_1850_PrintString(_TEXT(0xa28c, "Z-stats...\n"));
        ZSTATS_0a3a_ZstatsCmd();
        break;
    default:
        /* default What? */
        ULTIMA_1850_PrintString(_TEXT(0xa298, "What?\n"));
        result = 0;
        break;
    }
    return result;
}

// "explosion effect"
// The hit effect on the map cell (x, y): the flash tile and noise.
void ULTIMA_3522_HitEffectAt(int param_1, int param_2)
{
    if (D_5893_mapId < 0x80)
    {
        param_1 -= D_5896_mapX - 5;
        param_2 -= D_5897_mapY - 5;
    }

    ULTIMA_10e0_GRAP_51_PutTile(0, param_1, param_2);
    ULTIMA_223c_AudioWhiteNoise(10, 3000, 2000);
    ULTIMA_5910_UpdateFrame();
}

// The hit effect on an actor (in combat a combatant; a party member's stats row flashes too).
void ULTIMA_3564_HitEffectOnActor(int param_1)
{
    CombatEntity* local_4;

#if !defined(MATCHING_BUILD)
    local_4 = 0;
    ASSERT(D_5893_mapId > 0x7f);
#endif

    if (D_5893_mapId > 0x7f)
    {
        local_4 = &D_ba14_combatants[param_1];
        param_1 = local_4->actorIdx;
    }

    ULTIMA_10e0_GRAP_51_PutTile(0, D_5c5a_actors[param_1].x, D_5c5a_actors[param_1].y);

    if (D_5893_mapId > 0x7f && (local_4->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        ULTIMA_2a28_InvertPlayerRow(local_4->entityIdx);
        ULTIMA_223c_AudioWhiteNoise(0x28, 3000, 500);
        ULTIMA_2a28_InvertPlayerRow(local_4->entityIdx);
    }
    else
    {
        ULTIMA_223c_AudioWhiteNoise(10, 3000, 2000);
    }

    ULTIMA_5910_UpdateFrame();
}

int ULTIMA_35ec_SelectDirection(void)
{
    u8 local_4;

    D_5876_tempX = 0;
    D_5878_tempY = 0;

    while ((local_4 = ULTIMA_266c_GetChar()) != U5_KEY_SPACE &&
           local_4 != U5_KEY_UP && local_4 != U5_KEY_DOWN &&
           local_4 != U5_KEY_LEFT && local_4 != U5_KEY_RIGHT)
    {
    }

    switch (local_4)
    {
    case U5_KEY_ESC:
    case U5_KEY_SPACE:
        ULTIMA_1850_PrintString(_TEXT(0xa2a0, "Pass\n"));
        return 0;
    case U5_KEY_UP:
        ULTIMA_1850_PrintString(_TEXT(0xa2a6, "North\n"));
        D_5878_tempY--;
        return 1;
    case U5_KEY_DOWN:
        ULTIMA_1850_PrintString(_TEXT(0xa2ae, "South\n"));
        D_5878_tempY++;
        return 1;
    case U5_KEY_LEFT:
        ULTIMA_1850_PrintString(_TEXT(0xa2b6, "West\n"));
        D_5876_tempX--;
        return 1;
    case U5_KEY_RIGHT:
        ULTIMA_1850_PrintString(_TEXT(0xa2bc, "East\n"));
        D_5876_tempX++;
        return 1;
    }

#if !defined(MATCHING_BUILD)
    return local_4;
#endif
}

// CHECKED
int ULTIMA_368e_FindActorTileAtPos(int param_1_x, int param_2_y, int param_3_level)
{
    for (D_5876_tempX = 1; D_5876_tempX < 32; D_5876_tempX++)
    {
        if (D_5c5a_actors[D_5876_tempX].x == param_1_x && D_5c5a_actors[D_5876_tempX].y == param_2_y &&
            (D_5893_mapId > 0x7f || (D_5893_mapId < 0x80 && D_5c5a_actors[D_5876_tempX].z == param_3_level)))
        {
            return D_5c5a_actors[D_5876_tempX].tile;
        }
    }

    return 0;
}

// CHECKED
// The tile of the actor at (x, y, level), searching from slot 31 down to the party (slot 0); D_5876_tempX is its slot. 0 none.
int ULTIMA_3702_FindLastActorTileAt(int param_1, int param_2, int param_3)
{
    for (D_5876_tempX = 31; D_5876_tempX >= 0; D_5876_tempX--)
    {
        if (D_5c5a_actors[D_5876_tempX].x == param_1 && D_5c5a_actors[D_5876_tempX].y == param_2 &&
            (D_5893_mapId > 0x7f || (D_5893_mapId < 0x80 && D_5c5a_actors[D_5876_tempX].z == param_3)))
        {
            return D_5c5a_actors[D_5876_tempX].tile;
        }
    }

    return 0;
}

// CHECKED
// NOTE: UNUSED
static int ULTIMA_3776(byte param_1)
{
    int local_6;
    int local_8;
    int local_4_ab;

    for (local_8 = 0x1f; local_8 >= 0; local_8--)
    {
        if (D_5c5a_actors[local_8].tile == 0)
        {
            return local_8;
        }

        if (D_5c5a_actors[local_8].tile == param_1)
        {
            if (D_5896_mapX >= D_5c5a_actors[local_8].x)
            {
                local_4_ab = D_5896_mapX - D_5c5a_actors[local_8].x;
            }
            else
            {
                local_4_ab = ~(D_5896_mapX - D_5c5a_actors[local_8].x) + 1;
            }

            if (D_5897_mapY >= D_5c5a_actors[local_8].y)
            {
                local_6 = D_5897_mapY - D_5c5a_actors[local_8].y;
            }
            else
            {
                local_6 = ~(D_5897_mapY - D_5c5a_actors[local_8].y) + 1;
            }

            if (D_5c5a_actors[local_8].z != D_5895_mapLevel || (D_5c5a_actors[local_8].z == D_5895_mapLevel && local_4_ab >= 6 && local_6 >= 6))
            {
                return local_8;
            }
        }
    }

    return -1;
}

// CHECKED
static int ULTIMA_3868(int param_1, int param_2, int param_3)
{
    int local_8;
    byte local_6;
    byte local_4_ab;

    for (local_8 = 1; local_8 < 0x18; local_8++)
    {
        if (D_5c5a_actors[local_8].tile >= param_1 && D_5c5a_actors[local_8].tile <= param_2 && D_5c5a_actors[local_8].tile != TILE_ACTOR_CROWN)
        {
            if (param_3 != 0)
            {
                local_4_ab = D_5c5a_actors[local_8].x - D_5896_mapX + 5;
                local_6 = D_5c5a_actors[local_8].y - D_5897_mapY + 5;
                if (local_4_ab > 10 || local_6 > 10)
                {
                    return local_8;
                }
            }
            else
            {
                return local_8;
            }
        }
    }

    return 0;
}

// A slot of D_5c5a_actors for a new actor: a free one, else one out of sight, the least important kind first. 0 none.
int ULTIMA_38e4_FindActorSlot(void)
{
    int ret;

    ret = ULTIMA_3868(0, 0, 0);
    if (ret == 0)
        ret = ULTIMA_3868(1, 0xf, 1);
    if (ret == 0)
        ret = ULTIMA_3868(0x80, 0xff, 1);
    if (ret == 0)
        ret = ULTIMA_3868(0x10, 0x11, 1);
    if (ret == 0)
        ret = ULTIMA_3868(0x30, 0x7f, 1);
    if (ret == 0)
        ret = ULTIMA_3868(1, 0xf, 0);
    if (ret == 0)
        ret = ULTIMA_3868(0x80, 0xff, 0);
    if (ret == 0)
        ret = ULTIMA_3868(0x10, 0x11, 0);
    if (ret == 0)
        ret = ULTIMA_3868(0x30, 0x7f, 0);
    if (ret == 0)
        ret = ULTIMA_3868(0, 0xff, 0);

    return ret;
}

void ULTIMA_39cc_SetTile(byte new_tile_id, byte x, byte y)
{
    if (D_5893_mapId != 0 && D_5893_mapId < 0x21 && new_tile_id != 0)
    {
        *ULTIMA_4402_GetTileAddr(x, y) = new_tile_id;
    }
}

// CHECKED
int ULTIMA_39fc_GetFirstActivePartyMember(void)
{
    byte local_6;
    int local_4_ab;
    int local_8;

    local_8 = 0;

    for (local_4_ab = 0; local_4_ab < D_585b_partyCount; local_4_ab++)
    {
        if ((local_6 = D_55a8_party[local_4_ab].status) == STATUS_GOOD || local_6 == STATUS_POISONED)
        {
            D_5876_tempX = local_4_ab;
            return 0;
        }
        else if (local_6 == STATUS_SLEEP)
        {
            local_8++;
        }
    }

    if (local_8 != 0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// Fill slot `slot` of D_5c5a_actors: tile, animTile, x, y, z, value.
void ULTIMA_3a74_SetActor(byte a, byte b, byte c, byte d, byte e, byte f, int g)
{
    D_5c5a_actors[g].tile = a;
    D_5c5a_actors[g].animTile = b;
    D_5c5a_actors[g].x = c;
    D_5c5a_actors[g].y = d;
    D_5c5a_actors[g].z = e;
    D_5c5a_actors[g].value = f;
}

int ULTIMA_3aae_Random(int param_1)
{
    return ULTIMA_2092_RandomRange(0, param_1);
}

// A random number from 1 to 30.
int ULTIMA_3abe_Random1To30(void)
{
    int local_4 = 0;

    local_4 = ULTIMA_3aae_Random(0x3c) / 2;
    if (local_4 == 0)
    {
        local_4++;
    }
    return local_4;
}

// sleep_ticks
// Show `count` frames of the map, a timer tick apart (nothing while the view is off).
void ULTIMA_3ae6_WaitFrames(int param_1)
{
    int local_4;

    if (D_58a4_viewOn == 0)
        return;

    for (local_4 = 0; local_4 < param_1; local_4++)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_20fa_WaitTicks(1);
    }
}

// gets
void ULTIMA_3b1c_GetString(char* param_1, int param_2)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_4_ab = 0; // di
    local_8 = D_538c_keyBufferOff;
    D_538c_keyBufferOff = 0;
    do
    {
        local_6 = ULTIMA_266c_GetChar();
        if ((local_6 == U5_KEY_BACKSPACE || local_6 == U5_KEY_LEFT) && local_4_ab != 0)
        {
            // 3b43
            ULTIMA_1fa0_Backspace(1);
            local_4_ab--;
        }
        else if ((local_6 == U5_KEY_ESC) && (local_4_ab != 0))
        {
            ULTIMA_1fa0_Backspace(local_4_ab);
            local_4_ab = 0;
        }
        else if (0x1f < local_6 && local_6 < 0x80 && local_4_ab < param_2)
        {
            param_1[local_4_ab++] = (char)local_6;
            ULTIMA_16ba_PrintChar(local_6);
        }
    } while (local_6 != U5_KEY_ENTER);
    param_1[local_4_ab] = 0;
    D_538c_keyBufferOff = local_8;
}

// Read a number of up to `maxLen` (at most 5) digits from the keyboard, with a sign; Enter ends it.
int ULTIMA_3b9e_GetNumber(int param_1_maxLen)
{
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    byte local_8[6];

    local_c = 0;
    local_10_aa = 0;
    local_a = 0;
    if (param_1_maxLen > 5)
    {
        param_1_maxLen = 5;
    }

    do
    {
        local_e_am = ULTIMA_266c_GetChar();
        if ((uint)local_e_am >= '0' && (uint)local_e_am <= '9')
        {
            if (local_c < param_1_maxLen)
            {
                local_8[local_c] = local_e_am;
                local_c++;
                ULTIMA_16ba_PrintChar(local_e_am);
            }
        }
        else
        {
            if (((uint)local_e_am == U5_KEY_MINUS || (uint)local_e_am == U5_KEY_PLUS) && local_c == 0)
            {
                local_8[local_c] = local_e_am;
                local_c++;
                ULTIMA_16ba_PrintChar(local_e_am);
            }
            else if ((local_e_am == U5_KEY_BACKSPACE || local_e_am == U5_KEY_LEFT) && local_c != 0)
            {
                local_c--;
                ULTIMA_1fa0_Backspace(1);
            }
            else if (local_e_am == U5_KEY_ESC && local_c != 0)
            {
                ULTIMA_1fa0_Backspace(local_c);
                local_c = 0;
            }
        }
    } while (local_e_am != U5_KEY_ENTER);

#if !defined(MATCHING_BUILD)
    if (local_c == 0)
    {
        return 0;
    }
#endif

    while (--local_c > 0)
    {
        local_10_aa += (local_8[local_c] - 0x30) * D_6a0a_powersOfTen[local_a];
        local_a++;
    }

    if (local_8[0] == U5_KEY_MINUS)
    {
        local_10_aa = -local_10_aa;
    }
    else if (local_8[0] >= 0x30)
    {
        local_10_aa += D_6a0a_powersOfTen[local_a] * (local_8[0] - 0x30);
    }

    return local_10_aa;
}

void DNGLOOK_109e(void); // NOTE: SIC
void MAINOUT_007a(void);
int MAINOUT_1a60(void);

// hole up from outside
static void ULTIMA_3c9a_HoleUpCmd(void)
{
    int local_10_aa;
    byte local_e_am;
    int local_c;
    ActorFmt* local_a;
    int local_8;
    int local_6;
    byte local_4_aa;

    local_a = &D_5c5a_actors[0];
    local_e_am = D_5c5a_actors[0].animTile;

    ULTIMA_1850_PrintString(_TEXT(0xa2c2, "Hole up & "));

    if ((local_e_am & 0xf8) == TILE_ACTOR_FRIGATE_20)
    {
        ULTIMA_1850_PrintString(_TEXT(0xa2ce, "\nrepair...\n\n"));

        if (D_5c5a_actors[0].animTile < TILE_ACTOR_FRIGATE_24)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa2dc, "Sails must be\n"));
            ULTIMA_1850_PrintString(_TEXT(0xa2ec, "lowered!\n\n"));
        }
        else
        {
            for (local_c = 0; local_c < 5; local_c++)
            {
                MAINOUT_007a();
                MAINOUT_1a60();
                if ((D_587c_partyTile & 0xfc) != TILE_ACTOR_FRIGATE_24)
                {
                    return;
                }
                ULTIMA_4f7c_AdvanceTime(5);
            }

            do
            {
                local_a->value += ULTIMA_2092_RandomRange(1, 3);
                if (local_a->value > 99)
                {
                    local_a->value = 99;
                }
            } while (local_a->value < 10);

            ULTIMA_1850_PrintString(_TEXT(0xa2f8, "Hull now "));
            ULTIMA_1a3e_PrintNumber(local_a->value, 2, 0x20);
            ULTIMA_1850_PrintString(_TEXT(0xa302, "!\n\n"));
            D_a9fa_vitalsDirty = 1;
        }
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0xa306, "camp!\n\n"));

        if (D_5893_mapId < 0x21)
        {
            local_e_am = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
        }

        if (D_5893_mapId < 0x21 && local_e_am != 0 && local_e_am < 4)
        {
            ULTIMA_1850_PrintString(_TEXT(0xa30e, "On land or ship!\n\n"));
        }
        else
        {
            if (D_5893_mapId < 0x21 && local_a->animTile != TILE_ACTOR_AVATAR)
            {
                ULTIMA_1850_PrintString(_TEXT(0xa322, "On foot!\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0xa32c, "For how many hours? (1-9) "));

                while ((local_4_aa = ULTIMA_266c_GetChar()) != U5_KEY_SPACE && (local_4_aa < 0x30 || local_4_aa > 0x39))
                {
                }

                ULTIMA_16ba_PrintChar(local_4_aa);
                ULTIMA_16ba_PrintChar('\n');

                if (local_4_aa == U5_KEY_SPACE || local_4_aa == 0x30)
                {
                    return;
                }

                local_10_aa = local_4_aa - 0x30;

                local_6 = 0;
                for (local_c = 0; local_c != D_585b_partyCount; local_c++)
                {
                    if (D_55a8_party[local_c].status == STATUS_GOOD || D_55a8_party[local_c].status == STATUS_POISONED)
                    {
                        local_6++;
                    }
                }

                if (local_6 > 1)
                {
                    ULTIMA_1850_PrintString(_TEXT(0xa348, "\nWilt thou set a watch? "));

                    while ((local_4_aa = ULTIMA_266c_GetChar()) != 'Y' && local_4_aa != 'N')
                    {
                    }

                    if (local_4_aa == 'N')
                    {
                        ULTIMA_1850_PrintString(_TEXT(0xa362, "No\n\n"));
                        local_8 = -1;
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0xa368, "Yes\n\n"));
                        ULTIMA_1850_PrintString(_TEXT(0xa36e, "Who will stand guard? "));
                        local_8 = ULTIMA_2e8e_SelectPlayer();
                        ULTIMA_16ba_PrintChar('\n');
                        if (local_8 == -1 || D_55a8_party[local_8].status != STATUS_GOOD)
                        {
                            local_8 = -1;
                            ULTIMA_1850_PrintString(_TEXT(0xa386, "None posted!\n\n"));
                        }
                    }
                }
                else
                {
                    local_8 = -1;
                }

                if (D_5893_mapId > 0x20)
                {
                    D_58a1_combatFlags = 6;
                    DNGLOOK_1130();
                    DNGLOOK_0d3e();
                    ULTIMA_5f86_SpecialMapHandler(D_58a1_combatFlags, local_8, local_10_aa);
                    ULTIMA_251e_SwitchDisks(2);
                    DNGLOOK_109e(); // NOTE: sic (bug?)

                    D_5c5a_actors[1] = D_a9fc_savedActors[1];

                    DUNGEON_0134(0);
                }
                else
                {
                    ULTIMA_6360_Camping(local_8, local_10_aa);
                }
            }
        }
    }

    D_24e6_viewDirty = 1;
}

// TODO: int? byte?
void ULTIMA_3ef0_IncreaseByte(byte* param_1, int param_2, int param_3)
{
    if (*param_1 + param_2 < param_3)
    {
        *param_1 += param_2;
    }
    else
    {
        *param_1 = param_3;
    }
}

// TODO: u16? s16? int? (FMT)
void ULTIMA_3f14_IncreaseInt(s16* param_1, int param_2, int param_3)
{
    if (*param_1 + param_2 < param_3)
    {
        *param_1 += param_2;
    }
    else
    {
        *param_1 = param_3;
    }
}

void ULTIMA_3f36_DecreaseByte(byte* param_1, int param_2)
{
    if (*param_1 > param_2)
    {
        *param_1 = *param_1 - param_2;
    }
    else
    {
        *param_1 = 0;
    }
}

void ULTIMA_3f54_DecreaseInt(s16* param_1, int param_2)
{
    if (*param_1 > param_2)
    {
        *param_1 = *param_1 - param_2;
    }
    else
    {
        *param_1 = 0;
    }
}

// Test the tile in view cell (x, y) against the bit table D_6a14 (CAST and COMSUBS: probably whether a missile passes).
bool ULTIMA_3f6e_TestViewTileFlag(int param_1, int param_2)
{
    int local_4 = GetMapViewport(param_1, param_2);
    return (0x80 >> (local_4 & 7) & D_6a14[local_4 >> 3]) != 0;
}

// Pixel coordinates to the view cell, in D_5876_tempX and D_5878_tempY; -1 outside the view.
void ULTIMA_3fb4_PixelToViewCell(int param_1, int param_2)
{
    if (param_1 < 8 || param_1 > 0xb7 || param_2 < 8 || param_2 > 0xb7)
    {
        D_5876_tempX = D_5878_tempY = -1;
    }
    else
    {
        D_5876_tempX = (param_1 - 8) / 0x10;
        D_5878_tempY = (param_2 - 8) / 0x10;
    }
}
