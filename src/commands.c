#include "common/common.h"

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

int MainoutAttackCmd(void);
int EnterCmd(void);

// DS:55a6-6a0a: the saved game (55a6-6606), the map buffers and the game state, zero in the
// original's DATA.OVL. Initialised data, so in some module's _DATA between party.c's (ends
// at 55a6) and this module's tables (6a0a); which of the two is not known (the bytes are
// the same either way).
u16 saveStart = 0; // +0x0
S_55a8 party[0x10] = {0}; // ~57a8)
s16 food = 0; // +0x0202 Food // int (ReceiveItem, MOVSX)
s16 gold = 0; // +0x0204 Gold // int (TrollToll, MOVSX)
u8 keys = 0; // +0x0206 #Keys
u8 gems = 0; // +0x0207 #Gems
u8 torches = 0; // +0x0208 #Torches
u8 grapple = 0; // +0x0209 Grapple
u8 carpets = 0; // +0x020A #Magic Carpets
u8 skullKeys = 0; // +0x020B #Skull Keys
u8 skullKeysDay = 0; // +0x020C Last day Minoc skull keys were taken
u8 amulet = 0; // +0x020D Amulet of Lord British
u8 crown = 0; // +0x020E Crown of Lord British
u8 sceptre = 0; // +0x020F Sceptre of Lord British
u8 shards[4] = {0}; // +0x0210 Shards   // [57b6..57ba)
u8 spyglasses = 0; // +0x0214 #Spy Glass
u8 hmsCapePlans = 0; // +0x0215 HMS Cape Plans
u8 sextants = 0; // +0x0216 #Sextant
u8 pocketWatch = 0; // +0x0217 Pocket Watch
u8 blackBadge = 0; // +0x0218 Black Badge
u8 sandalwoodBox = 0; // +0x0219 Sandalwood Box
u8 equipment[0x30] = {0}; // +0x021A Equipment(Quantity owned by the party) [57c0..57f0)
u8 mixedSpells[0x30] = {0}; // +0x024A Spell Mixtures [57f0..5820)
u8 scrolls[8] = {0}; // +0x027A Scrolls
u8 potions[8] = {0}; // +0x0282 Potions (Blue, Yellow, Red, Green, Orange, Purple, Black, White)
u8 moonstoneX[8] = {0}; // +0x028A 0-0xFF Moonstone X-coordinates (valid only if buried)
u8 moonstoneY[8] = {0}; // +0x0292 0-0xFF Moonstone Y-coordinates (valid only if buried)
u8 moonstoneMap[8] = {0}; // +0x029A 0=buried,0xFF=Inventory moonstone flags
u8 moonstoneZ[8] = {0}; // +0x02A2 0=Britannia,0xFF=Underworld moonstone Z-coordinates (valid only if buried)
u8 D_5850_reagents[8] = {0}; // +0x02AA Reagents (A..H)
u8 harvestDay[3] = {0}; // +0x02B2 Day each of the three nightshade/mandrake spots was last harvested [5858..585b)
u8 partyCount = 0; // +0x02B5 party member count
u8 takenItemFlags[0x18] = {0}; // [585c..5874)
u16 year = 0; // +0x02CE Current year (TODO: int?)
s16 tempX = 0; // selected x direction? // +0x02D0 first party member that is not dead or sleeping? # of active monsters & charmed PCs[2]?
s16 tempY = 0; // selected y direction? // +0x02D2 # of active PCs and charmed monsters[2]?
u8 statusIcon = 0; // +0x02D4 Icon
u8 activePlayer = 0; // +0x02D5 Active character
u8 partyTile = 0; // +0x02D6 party tile
u8 D_587d_month = 0; // +0x02D7 Current month
u8 D_587e_day = 0; // +0x02D8 Current day
u8 D_587f_hour = 0; // +0x02D9 Current hour
u8 lastHour = 0; // +0x02DA copy of 0x2D9
u8 minute = 0; // +0x02DB Current minute
u8 combatTick = 0;
u8 sailCounter = 0; // ?
u8 clockChimes = 0;
u8 trammelPhase = 0; // +0x02DF Phase of Trammel
u8 feluccaPhase = 0; // +0x02E0 Phase of Felucca
u8 moongateHeight = 0; // +0x02E1 Current moongate height (in pixels)
u8 karma = 0; // +0x02E2 Karma
u8 D_5889_savedUnused = 0;
u8 D_588a_savedUnused = 0;
u8 turnCount = 0; // +0x02E5 Current turn count
u8 campHealWait = 0;
u8 apparitionMonth = 0;
u8 protectTurns = 0; // +0x02E8 Protective spell duration
u8 castCircle = 0;
u8 D_5890_spellAttack = 0;
u8 animate = 0; // +0x02EB animate flag
u8 windDir = 0; // +0x02EC wind direction
u8 mapId = 0; // +0x02ED map_id (21h~: dungeon, 40: unloaded)
u8 D_5894_savedMapId = 0; // +0x02EE Combat backup of 0x2ED
u8 D_5895_mapLevel = 0; // +0x02EF level (depth)
u8 D_5896_mapX = 0; // +0x02F0 map_x
u8 D_5897_mapY = 0; // +0x02F1 map_y
u8 crosshairOn = 0; // crosshair visible
u8 crosshairX = 0; // crosshair x
u8 crosshairY = 0; // crosshair y
u8 mapOriginX = 0; // x upper left chunk
u8 mapOriginY = 0; // y upper left chunk
u8 attackWeapon = 0; // +0x02F7 Attacker's weapon
u8 curCombatant = 0; // +0x02F8 ?
u8 focusBlink = 0; // +0x02F9 show focus rectangle & crosshair?
u8 combatExitDir = 0; // +0x02FA exit direction from combat map
u8 combatFlags = 0; // +0x02FB combat type flags
u8 combatEvents = 0;
u8 combatOver = 0; // +0x02FD battle successful flag (win?)
u8 viewOn = 0; // +0x02FE Update/animate 2D map?
u8 D_58a5_lightLevel = 0; // +0x02FF Current light intensity
u8 lightSpellTurns = 0;
u8 torchTurns = 0;
u8 lastAttacker[0x20] = {0}; // [58a8..58c8)
u8 shadowlordTowns[3] = {0}; // 58cb)
u8 summonedLord = 0;
u16 questsOrdained = 0; // shrine quest active flags TODO: u8?
u16 questsDone = 0; // shrine quest completed flags TODO: u8?
u8 dungeonOpen[8] = {0};
u8 shrineDestroyed[8] = {0};
u8 roomsCleared[0xe] = {0}; // ..58ee)
u8 annotX[0x20] = {0}; // ..590e) spell counts
u8 annotY[0x20] = {0}; // ..592e)
u8 annotTile[0x20] = {0}; // .594e)
u8 annotCount = 0;
u8 openDoorTile = 0; // +0x03A9 opened door
u8 openDoorX = 0; // +0x03AA open_door_x
u8 openDoorY = 0; // +0x03AB open_door_y
u8 openDoorTurns = 0; // +0x03AC # of turns the door stays open
u8 boughtShipX = 0; // +0x03AD ship_x
u8 boughtShipY = 0; // +0x03AE ship_y
u8 sailDir = 0;
u8 newPrompt = 0;
u8 drunkTurns = 0; // +0x03B1 remaining drunk turn count (GetTownKey)
u8 townShadowlord = 0;
u8 fewerEnemies = 0;
u8 dungeonMap[0x200] = {0}; // +0x03B4 dungeon map data ..5b5a)
u8 npcKilled[0x80] = {0}; // +0x05B4 NPC killed flags (4 bytes / 32 bits per settlement) [..5bda) // TODO: u32
u8 npcMet[0x80] = {0}; // +0x0634 NPC met flags (4 bytes / 32 bits per settlement) [..5c5a)
ActorFmt actors[32] = {0}; // ~5d5a) "Object" struct
u16 D_5d5a_unused = 0; // unused?
u16 D_5d5c_unused = 0; // unused?
NpcScheduleFmt npcSchedules[32] = {0}; // +0x07B8 NPC Schedules [..5f5e) 0x200
NpcFmt npcs[32] = {0}; // +0x09B8 Character/monster states (16bytes * 32) [..615e) 0x200
MovementFmt moveLists[0x20] = {0}; // +0x0BB8 Movement list table [..655e)
s16 moveListPos[0x20] = {0}; // +0x0FB8 Movement list pointers [..659e)
u8 npcTypes[0x20] = {0}; // +0x0FF8 NPC types [..65be)
u8 npcApproach = 0;
u8 npcApproaching = 0;
u16 D_65c0_unused = 0; // unused?
s16 npcStuckCount[0x20] = {0};
u8 dungeonMoveDir = 0;
u8 dungeonFacing = 0; // +0x105D orientation in a dungeon
u8 dungeonStyle = 0; // +0x105E graphics for the current dungeon
u8 boughtShip = 0; // +0x105F purchased ship/skiff
u16 saveEnd = 0; // end
S_6608_Map D_6608_map = {0}; // ~6a08) map data: layer4 * y16 * x16 / y32 * x32
// (D_6708_map, DS:6708, is a place inside D_6608_map: vars.h)
u8 songPos = 0; // some audio related
u8 D_6a09_pad[1] = {0}; // no name in vars.h

u16 D_6a0a_powersOfTen[5] = { 1, 10, 100, 1000, 10000 };

u8 viewTileFlags[0x20] =
{
    0xff, 0xf3, 0xc3, 0x8f, 0xff, 0xff, 0xff, 0xc0,
    0xdd, 0xf8, 0x03, 0xdf, 0xff, 0xff, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f,
    0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff
};

// shake screen
void ShakeScreen(void)
{
    int i;
    int shakeRound;

    GRAP_1b_TransferFullscreen(0, 1);
    GRAP_0f_SelectPage(1);

    for (shakeRound = 0; shakeRound < 8; shakeRound++)
    {
        for (i = 8; i <= 0xb3; i += 3)
        {
            DRV_27_ScrollTextWindow1(8, i, 0xb7, i + 4, 2);
            PcspkOn(RandomRange(0x13, 0x96));
        }

        for (i = 8; i <= 0xb3; i += 3)
        {
            GRAP_18_TransferArea(1, 0, 8, i, 0xb7, i + 4);
            PcspkOn(RandomRange(0x13, 0x96));
        }

        for (i = 0xb3; i >= 8; i -= 3)
        {
            DRV_27_ScrollTextWindow2(8, i, 0xb7, i + 4, 2);
            PcspkOn(RandomRange(0x13, 0x96));
        }

        for (i = 0xb3; i >= 8; i -= 3)
        {
            GRAP_18_TransferArea(1, 0, 8, i, 0xb7, i + 4);
            PcspkOn(RandomRange(0x13, 0x96));
        }
    }

    GRAP_0f_SelectPage(0);
    PcspkOff();
}

static void HoleUpCmd(void);

int ProcessCommand(int key)
{
    int result;
    int tileUnder;

    result = 1;
    switch (key)
    {
    case U5_KEY_CTRL_B:
        PrintString(/*0xa110*/ "Buffer O");
        keyBufferOff = !keyBufferOff;
        if (keyBufferOff != 0) 
        {
            PrintString(/*0xa11a*/ "ff\n");
        }
        else
        {
            PrintString(/*0xa11e*/ "n\n");
        }

        result = 0;
        break;
    case U5_KEY_SPACE:
        /* ' ' Pass */
        if (mapId == 0 && sailDir != 0)
        {
            PrintString(/*0xa122*/ "Sheets in irons!\n");
            sailDir = 0;
        }
        else
        {
            PrintString(/*0xa134*/ "Pass\n");
        }
        break;
    case 0x41:
        /* 'A' */
        // 3216
        result = mapId == 0 ?
            MainoutAttackCmd() :
                mapId < 0x21 ?
            AttackCmd() :
            DungeonAttackCmd();
        break;
    case 0x42:
        /* 'B' Board */
        PrintString(/*0xa13a*/ "Board ");
        result = BoardCmd();
        break;
    case 0x43:
        /* 'C' Cast */
        PrintString(/*0xa142*/ "Cast...\n");
        result = CastSpellCmd();
        break;
    case 0x44:
        /* 'D' What? */
        PrintString(/*0xa14c*/ "D-What?\n");
        result = 0;
        break;
    case 0x45:
        /* 'E' Enter */
        if (mapId == 0)
        {
            result = EnterCmd();
            break;
        }
        PrintString(/*0xa156*/ "Enter what?\n");
        break;
    case 0x46:
        /* 'F' Fire */
        PrintString(/*0xa164*/ "Fire-");
        FireCmd();
        break;
    case 0x47:
        /* 'G' Get */
        if (mapId < 0x21)
        {
            PrintString(/*0xa16a*/ "Get-");
        }
        GetCmd();
        break;
    case 0x48:
        /* 'H' Hole up */
        if (mapId == 0 || mapId > 0x20)
        {
            // outmap_hole_up_cmd
            HoleUpCmd();
            break;
        }
        tileUnder = *GetTileAddr(D_5896_mapX, D_5897_mapY);
        PrintString(/*0xa170*/ "Hole up- ");
        if (tileUnder != TILE_MAP_BED)
        {
            PrintString(/*0xa17a*/ "Only in bed!\n");
        }
        else
        {
            CmdsHoleUpCmd();
        }
        break;
    case 0x49:
        /* 'I' Ignite torch */
        PrintString(/*0xa188*/ "Ignite torch!\n");
        IgniteTorchCmd();
        break;
    case 0x4a:
        /* 'J' Jimmy */
        PrintString(/*0xa198*/ "Jimmy-");
        JimmyCmd();
        break;
    case 0x4b:
        /* 'K' Klimb */
        if (mapId == 0)
        {
            PrintString(/*0xa1a0*/ "Klimb-");
            CmdsKlimbCmd();
        }
        else if (mapId < 0x21)
        {
            result = KlimbCmd();
        }
        else
        {
            result = DungeonKlimbCmd();
        }
        break;
    case 0x4c:
        /* 'L' Look */
        PrintString(/*0xa1a8*/ "Look");
        if (mapId > 0x20 && mapId < 0x29)
        {
            /* 0x21..0x28 (Dungeon) */
            PrintString(/*0xa1ae*/ "...\n");
            LookCmdInDungeon();
        }
        else
        {
            PrintChar('-');
            LookCmd();
        }
        break;
    case 0x4d:
        /* 'M' Mix */
        PrintString(/*0xa1b4*/ "Mix Reagents\n\n");
        MixCmd();
        break;
    case 0x4e:
        /* 'N' New order */
        PrintString(/*0xa1c4*/ "New Order");
        NewOrderCmd();
        break;
    case 0x4f:
        /* 'O' Open */
        PrintString(/*0xa1ce*/ "Open-");
        OpenCmd();
        break;
    case 0x50:
        /* 'P' Push */
        if (mapId > 0x20 && mapId < 0x29)
        {
            /* Cannot push in dungeons */
            PrintString(/*0xa1d4*/ "Push\nNot here!\n");
            result = 0;
        }
        else
        {
            PrintString(/*0xa1e4*/ "Push-");
            PushCmd();
        }
        break;
    case 0x51:
        /* 'Q' Quit */
        PrintString(/*0xa1ea*/ "Quit:");
        SaveGame();
        result = 0;
        break;
    case 0x52:
        /* 'R' Ready */
        PrintString(/*0xa1f0*/ "Ready...\n\n");
        ReadyCmd();
        break;
    case 0x53:
        /* 'S' Search */
        if (mapId < 0x21)
        {
            PrintString(/*0xa1fc*/ "Search-");
        }
        else
        {
            PrintString(/*0xa204*/ "Search...\n");
        }
        SearchCmd();
        break;
    case 0x54:
        /* 'T' Talk */
        if (mapId == 0)
        {
            PrintString(/*0xa210*/ "Talk-");
            if (SelectDirection() == 0)
                break;
            PrintString(/*0xa216*/ "Funny, no response!\n");
        }
        else
        {
            if (mapId > 0x20)
            {
                PrintString(/*0xa22c*/ "Talk-Funny, no response!\n");
            }
            else
            {
                PrintString(/*0xa246*/ "Talk-");
                if (TalkCmd() != 0)
                {
                    result = 2;
                }
            }
        }
        break;
    case 0x55:
        /* 'U' Use item */
        PrintString(/*0xa24c*/ "Use item\n\n");
        UseCmd();
        break;
    case 0x56:
        /* 'V' View a gem */
        PrintString(/*0xa258*/ "View a gem!\n");
        if (gems != 0)
        {
            gems--;
            if (mapId < 0x21)
            {
                LookobjViewCmd(D_5896_mapX, D_5897_mapY);
            }
            else
            {
                DnglookViewCmd();
            }
            break;
        }
        PrintString(/*0xa266*/ "You have none!\n");
        break;
    case 0x57:
        /* 'W' What? */
        PrintString(/*0xa276*/ "W-What?\n");
        result = 0;
        break;
    case 0x58:
        /* 'X' X-it */
        PrintString(/*0xa280*/ "X-it ");
        XitCmd();
        break;
    case 0x59:
        /* 'Y' Yell */
        PrintString(/*0xa286*/ "Yell ");
        result = YellCmd();
        break;
    case 0x5a:
        /* 'Z' Z-stats */
        PrintString(/*0xa28c*/ "Z-stats...\n");
        ZstatsCmd();
        break;
    default:
        /* default What? */
        PrintString(/*0xa298*/ "What?\n");
        result = 0;
        break;
    }
    return result;
}

// "explosion effect"
// The hit effect on the map cell (x, y): the flash tile and noise.
void HitEffectAt(int cellX, int cellY)
{
    if (mapId < 0x80)
    {
        cellX -= D_5896_mapX - 5;
        cellY -= D_5897_mapY - 5;
    }

    GRAP_51_PutTile(0, cellX, cellY);
    AudioWhiteNoise(10, 3000, 2000);
    UpdateFrame();
}

// The hit effect on an actor (in combat a combatant; a party member's stats row flashes too).
void HitEffectOnActor(int index)
{
    CombatEntity* combatant;

    if (mapId > 0x7f)
    {
        combatant = &combatants[index];
        index = combatant->actorIdx;
    }

    GRAP_51_PutTile(0, actors[index].x, actors[index].y);

    if (mapId > 0x7f && (combatant->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        InvertPlayerRow(combatant->entityIdx);
        AudioWhiteNoise(0x28, 3000, 500);
        InvertPlayerRow(combatant->entityIdx);
    }
    else
    {
        AudioWhiteNoise(10, 3000, 2000);
    }

    UpdateFrame();
}

int SelectDirection(void)
{
    u8 key;

    tempX = 0;
    tempY = 0;

    while ((key = GetChar()) != U5_KEY_SPACE &&
           key != U5_KEY_UP && key != U5_KEY_DOWN &&
           key != U5_KEY_LEFT && key != U5_KEY_RIGHT)
    {
    }

    switch (key)
    {
    case U5_KEY_ESC:
    case U5_KEY_SPACE:
        PrintString(/*0xa2a0*/ "Pass\n");
        return 0;
    case U5_KEY_UP:
        PrintString(/*0xa2a6*/ "North\n");
        tempY--;
        return 1;
    case U5_KEY_DOWN:
        PrintString(/*0xa2ae*/ "South\n");
        tempY++;
        return 1;
    case U5_KEY_LEFT:
        PrintString(/*0xa2b6*/ "West\n");
        tempX--;
        return 1;
    case U5_KEY_RIGHT:
        PrintString(/*0xa2bc*/ "East\n");
        tempX++;
        return 1;
    }
}

// CHECKED
int FindActorTileAtPos(int mapX, int mapY, int mapLevel)
{
    for (tempX = 1; tempX < 32; tempX++)
    {
        if (actors[tempX].x == mapX && actors[tempX].y == mapY &&
            (mapId > 0x7f || (mapId < 0x80 && actors[tempX].z == mapLevel)))
        {
            return actors[tempX].tile;
        }
    }

    return 0;
}

// CHECKED
// The tile of the actor at (x, y, level), searching from slot 31 down to the party (slot 0); tempX is its slot. 0 none.
int FindLastActorTileAt(int mapX, int mapY, int mapLevel)
{
    for (tempX = 31; tempX >= 0; tempX--)
    {
        if (actors[tempX].x == mapX && actors[tempX].y == mapY &&
            (mapId > 0x7f || (mapId < 0x80 && actors[tempX].z == mapLevel)))
        {
            return actors[tempX].tile;
        }
    }

    return 0;
}

// CHECKED
// NOTE: UNUSED
// UNUSED: a free actor slot, or one holding the given tile out of range of the party.
static int FindFarActorSlot(byte tile)
{
    int distY;
    int slot_m;
    int distX;

    for (slot_m = 0x1f; slot_m >= 0; slot_m--)
    {
        if (actors[slot_m].tile == 0)
        {
            return slot_m;
        }

        if (actors[slot_m].tile == tile)
        {
            if (D_5896_mapX >= actors[slot_m].x)
            {
                distX = D_5896_mapX - actors[slot_m].x;
            }
            else
            {
                distX = ~(D_5896_mapX - actors[slot_m].x) + 1;
            }

            if (D_5897_mapY >= actors[slot_m].y)
            {
                distY = D_5897_mapY - actors[slot_m].y;
            }
            else
            {
                distY = ~(D_5897_mapY - actors[slot_m].y) + 1;
            }

            if (actors[slot_m].z != D_5895_mapLevel || (actors[slot_m].z == D_5895_mapLevel && distX >= 6 && distY >= 6))
            {
                return slot_m;
            }
        }
    }

    return -1;
}

// CHECKED
// An actor slot (from 1) whose tile lies in minTile..maxTile; with outOfViewOnly only one out of the 11x11 view. 0 none.
static int FindActorInRange(int minTile, int maxTile, int outOfViewOnly)
{
    int slot_c;
    byte viewY;
    byte viewX;

    for (slot_c = 1; slot_c < 0x18; slot_c++)
    {
        if (actors[slot_c].tile >= minTile && actors[slot_c].tile <= maxTile && actors[slot_c].tile != TILE_ACTOR_CROWN)
        {
            if (outOfViewOnly != 0)
            {
                viewX = actors[slot_c].x - D_5896_mapX + 5;
                viewY = actors[slot_c].y - D_5897_mapY + 5;
                if (viewX > 10 || viewY > 10)
                {
                    return slot_c;
                }
            }
            else
            {
                return slot_c;
            }
        }
    }

    return 0;
}

// A slot of actors for a new actor: a free one, else one out of sight, the least important kind first. 0 none.
int FindActorSlot(void)
{
    int ret;

    ret = FindActorInRange(0, 0, 0);
    if (ret == 0)
        ret = FindActorInRange(1, 0xf, 1);
    if (ret == 0)
        ret = FindActorInRange(0x80, 0xff, 1);
    if (ret == 0)
        ret = FindActorInRange(0x10, 0x11, 1);
    if (ret == 0)
        ret = FindActorInRange(0x30, 0x7f, 1);
    if (ret == 0)
        ret = FindActorInRange(1, 0xf, 0);
    if (ret == 0)
        ret = FindActorInRange(0x80, 0xff, 0);
    if (ret == 0)
        ret = FindActorInRange(0x10, 0x11, 0);
    if (ret == 0)
        ret = FindActorInRange(0x30, 0x7f, 0);
    if (ret == 0)
        ret = FindActorInRange(0, 0xff, 0);

    return ret;
}

void SetTile(byte new_tile_id, byte x, byte y)
{
    if (mapId != 0 && mapId < 0x21 && new_tile_id != 0)
    {
        *GetTileAddr(x, y) = new_tile_id;
    }
}

// CHECKED
int GetFirstActivePartyMember(void)
{
    byte status_l;
    int member_h;
    int sleepingCount;

    sleepingCount = 0;

    for (member_h = 0; member_h < partyCount; member_h++)
    {
        if ((status_l = party[member_h].status) == STATUS_GOOD || status_l == STATUS_POISONED)
        {
            tempX = member_h;
            return 0;
        }
        else if (status_l == STATUS_SLEEP)
        {
            sleepingCount++;
        }
    }

    if (sleepingCount != 0)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

// Fill slot `slot` of actors: tile, animTile, x, y, z, value.
void SetActor(byte tile, byte animTile, byte mapX, byte mapY, byte mapLevel, byte value, int slot)
{
    actors[slot].tile = tile;
    actors[slot].animTile = animTile;
    actors[slot].x = mapX;
    actors[slot].y = mapY;
    actors[slot].z = mapLevel;
    actors[slot].value = value;
}

int Random(int maxValue)
{
    return RandomRange(0, maxValue);
}

// A random number from 1 to 30.
int Random1To30(void)
{
    int value = 0;

    value = Random(0x3c) / 2;
    if (value == 0)
    {
        value++;
    }
    return value;
}

// sleep_ticks
// Show `count` frames of the map, a timer tick apart (nothing while the view is off).
void WaitFrames(int frameCount)
{
    int frame;

    if (viewOn == 0)
        return;

    for (frame = 0; frame < frameCount; frame++)
    {
        UpdateFrame();
        WaitTicks(1);
    }
}

// gets
void GetString(char* buffer, int maxLength)
{
    int savedKeyBufferOff;
    int key_j;
    int length;

    length = 0; // di
    savedKeyBufferOff = keyBufferOff;
    keyBufferOff = 0;
    do
    {
        key_j = GetChar();
        if ((key_j == U5_KEY_BACKSPACE || key_j == U5_KEY_LEFT) && length != 0)
        {
            // 3b43
            Backspace(1);
            length--;
        }
        else if ((key_j == U5_KEY_ESC) && (length != 0))
        {
            Backspace(length);
            length = 0;
        }
        else if (0x1f < key_j && key_j < 0x80 && length < maxLength)
        {
            buffer[length++] = (char)key_j;
            PrintChar(key_j);
        }
    } while (key_j != U5_KEY_ENTER);
    buffer[length] = 0;
    keyBufferOff = savedKeyBufferOff;
}

// Read a number of up to `maxLen` (at most 5) digits from the keyboard, with a sign; Enter ends it.
int GetNumber(int maxLen)
{
    int value;
    int key;
    int charCount;
    int power_h;
    byte digits[6];

    charCount = 0;
    value = 0;
    power_h = 0;
    if (maxLen > 5)
    {
        maxLen = 5;
    }

    do
    {
        key = GetChar();
        if ((uint)key >= '0' && (uint)key <= '9')
        {
            if (charCount < maxLen)
            {
                digits[charCount] = key;
                charCount++;
                PrintChar(key);
            }
        }
        else
        {
            if (((uint)key == U5_KEY_MINUS || (uint)key == U5_KEY_PLUS) && charCount == 0)
            {
                digits[charCount] = key;
                charCount++;
                PrintChar(key);
            }
            else if ((key == U5_KEY_BACKSPACE || key == U5_KEY_LEFT) && charCount != 0)
            {
                charCount--;
                Backspace(1);
            }
            else if (key == U5_KEY_ESC && charCount != 0)
            {
                Backspace(charCount);
                charCount = 0;
            }
        }
    } while (key != U5_KEY_ENTER);

    while (--charCount > 0)
    {
        value += (digits[charCount] - 0x30) * D_6a0a_powersOfTen[power_h];
        power_h++;
    }

    if (digits[0] == U5_KEY_MINUS)
    {
        value = -value;
    }
    else if (digits[0] >= 0x30)
    {
        value += D_6a0a_powersOfTen[power_h] * (digits[0] - 0x30);
    }

    return value;
}

void LoadDungeonImages(void); // NOTE: SIC
void WaitIfActorNear(void);
int MonstersTurn(void);

// hole up from outside
static void HoleUpCmd(void)
{
    int hours_m;
    byte tile;
    int i;
    ActorFmt* avatar;
    int guard_m;
    int ableCount_b;
    byte key_g;

    avatar = &actors[0];
    tile = actors[0].animTile;

    PrintString(/*0xa2c2*/ "Hole up & ");

    if ((tile & 0xf8) == TILE_ACTOR_FRIGATE_20)
    {
        PrintString(/*0xa2ce*/ "\nrepair...\n\n");

        if (actors[0].animTile < TILE_ACTOR_FRIGATE_24)
        {
            PrintString(/*0xa2dc*/ "Sails must be\n");
            PrintString(/*0xa2ec*/ "lowered!\n\n");
        }
        else
        {
            for (i = 0; i < 5; i++)
            {
                WaitIfActorNear();
                MonstersTurn();
                if ((partyTile & 0xfc) != TILE_ACTOR_FRIGATE_24)
                {
                    return;
                }
                AdvanceTime(5);
            }

            do
            {
                avatar->value += RandomRange(1, 3);
                if (avatar->value > 99)
                {
                    avatar->value = 99;
                }
            } while (avatar->value < 10);

            PrintString(/*0xa2f8*/ "Hull now ");
            PrintNumber(avatar->value, 2, 0x20);
            PrintString(/*0xa302*/ "!\n\n");
            vitalsDirty = 1;
        }
    }
    else
    {
        PrintString(/*0xa306*/ "camp!\n\n");

        if (mapId < 0x21)
        {
            tile = *GetTileAddr(D_5896_mapX, D_5897_mapY);
        }

        if (mapId < 0x21 && tile != 0 && tile < 4)
        {
            PrintString(/*0xa30e*/ "On land or ship!\n\n");
        }
        else
        {
            if (mapId < 0x21 && avatar->animTile != TILE_ACTOR_AVATAR)
            {
                PrintString(/*0xa322*/ "On foot!\n");
            }
            else
            {
                PrintString(/*0xa32c*/ "For how many hours? (1-9) ");

                while ((key_g = GetChar()) != U5_KEY_SPACE && (key_g < 0x30 || key_g > 0x39))
                {
                }

                PrintChar(key_g);
                PrintChar('\n');

                if (key_g == U5_KEY_SPACE || key_g == 0x30)
                {
                    return;
                }

                hours_m = key_g - 0x30;

                ableCount_b = 0;
                for (i = 0; i != partyCount; i++)
                {
                    if (party[i].status == STATUS_GOOD || party[i].status == STATUS_POISONED)
                    {
                        ableCount_b++;
                    }
                }

                if (ableCount_b > 1)
                {
                    PrintString(/*0xa348*/ "\nWilt thou set a watch? ");

                    while ((key_g = GetChar()) != 'Y' && key_g != 'N')
                    {
                    }

                    if (key_g == 'N')
                    {
                        PrintString(/*0xa362*/ "No\n\n");
                        guard_m = -1;
                    }
                    else
                    {
                        PrintString(/*0xa368*/ "Yes\n\n");
                        PrintString(/*0xa36e*/ "Who will stand guard? ");
                        guard_m = SelectPlayer();
                        PrintChar('\n');
                        if (guard_m == -1 || party[guard_m].status != STATUS_GOOD)
                        {
                            guard_m = -1;
                            PrintString(/*0xa386*/ "None posted!\n\n");
                        }
                    }
                }
                else
                {
                    guard_m = -1;
                }

                if (mapId > 0x20)
                {
                    combatFlags = 6;
                    FreeDungeonImages();
                    SetUpDungeonFight();
                    SpecialMapHandler(combatFlags, guard_m, hours_m);
                    SwitchDisks(2);
                    LoadDungeonImages(); // NOTE: sic (bug?)

                    actors[1] = savedActors[1];

                    SetUpMonster(0);
                }
                else
                {
                    Camping(guard_m, hours_m);
                }
            }
        }
    }

    viewDirty = 1;
}

// TODO: int? byte?
void IncreaseByte(byte* value, int amount, int limit)
{
    if (*value + amount < limit)
    {
        *value += amount;
    }
    else
    {
        *value = limit;
    }
}

// TODO: u16? s16? int? (FMT)
void IncreaseInt(s16* value, int amount, int limit)
{
    if (*value + amount < limit)
    {
        *value += amount;
    }
    else
    {
        *value = limit;
    }
}

void DecreaseByte(byte* value, int amount)
{
    if (*value > amount)
    {
        *value = *value - amount;
    }
    else
    {
        *value = 0;
    }
}

void DecreaseInt(s16* value, int amount)
{
    if (*value > amount)
    {
        *value = *value - amount;
    }
    else
    {
        *value = 0;
    }
}

// Test the tile in view cell (x, y) against the bit table viewTileFlags (CAST and COMSUBS: probably whether a missile passes).
bool TestViewTileFlag(int viewX, int viewY)
{
    int tile = GetMapViewport(viewX, viewY);
    return (0x80 >> (tile & 7) & viewTileFlags[tile >> 3]) != 0;
}

// Pixel coordinates to the view cell, in tempX and tempY; -1 outside the view.
void PixelToViewCell(int pixelX, int pixelY)
{
    if (pixelX < 8 || pixelX > 0xb7 || pixelY < 8 || pixelY > 0xb7)
    {
        tempX = tempY = -1;
    }
    else
    {
        tempX = (pixelX - 8) / 0x10;
        tempY = (pixelY - 8) / 0x10;
    }
}
