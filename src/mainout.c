#include "common/common.h"

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>

#include "blckthrn.h"
#include "cast2.h"
#include "comsubs.h"
#include "mainout.h"
#include "outsubs.h"

static u16 capeToggle; // this object's _BSS (DS:a524)

// defined in mainoutb.c, the part of MAINOUT.OVL built with MSC 5.0
extern u8 landMonsters[];
extern u8 deepLandMonsters[];
extern u8 seaMonsters[];
extern u8 deepSeaMonsters[];
extern u8 landWeights[];
extern u8 deepLandWeights[];
extern u8 seaWeights[];
extern u8 deepSeaWeights[];
int MonstersTurn(void);
void DamageVehicle(void);
void BridgeTrolls(void);

// enters the overworld: clears the sky, sets the view origin around the party, loads the map chunks, the light map, wind, sun and moons, the underworld's items
void SetUpOverworld(void)
{
    ULTIMA_4be8_ClearSky();
    ULTIMA_4f7c_AdvanceTime(0);

    viewOn = newPrompt = D_24e6_viewDirty = 1;
    sailDir = 0;
    D_589b_mapOriginX = D_5896_mapX & 0xf0;
    if ((D_5896_mapX & 0xf) < 8)
    {
        D_589b_mapOriginX = (D_589b_mapOriginX - 0x10) & 0xf0;
    }

    // 0034
    D_589c_mapOriginY = D_5897_mapY & 0xf0;
    if ((D_5897_mapY & 0xf) < 8)
    {
        D_589c_mapOriginY = (D_589c_mapOriginY - 0x10) & 0xf0;
    }

    // 004f
    LoadViewChunks(0, 1);
    LoadViewChunks(0, -1);
    ULTIMA_4f7c_AdvanceTime(0);
    ULTIMA_5e4a_BuildLightMap();
    ULTIMA_2e96_SetWindDirection(-1);
    ULTIMA_4a84_DrawSunAndMoons();
    PlaceAmuletShards();
}

// one frame's wait when an actor of the table is within five cells (only actor 1 is looked at: the loop breaks at once)
void WaitIfActorNear(void)
{
    int distX;
    int distY;

    int index;
    ActorFmt* actor_h;

    for (index = 1; index < 32; index++)
    {
        actor_h = &D_5c5a_actors[index];

        distX = abs(D_5896_mapX - actor_h->x);
        distY = abs(D_5897_mapY - actor_h->y);

        if (actor_h->tile != 0 && actor_h->z == D_5895_mapLevel && distX < 6 && distY < 6)
        {
            WaitFrames(1);
        }

        break;
    }
}

// "Ride/Fly/Row/Head": turns the party's tile to the direction; 1 when the party only turned (no step), also a frigate under no wind
static int TurnVehicle(int dir)
{
    int oldTile;
    int turned = 0;

    switch (D_587c_partyTile & 0xfc)
    {
    case TILE_ACTOR_HORSE:
        // 010a
        ULTIMA_1850_PrintString(/*0x2946*/ "Ride ");
        if (dir == 1)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE;
        }
        else if (dir == 3)
        {
            D_587c_partyTile = TILE_ACTOR_RIDING_HORSE + 1;
        }
        break;

    case TILE_ACTOR_FLYING_CARPET:
        // 0130
        ULTIMA_1850_PrintString(/*0x294c*/ "Fly ");
        if (dir == 1)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET;
        }
        else if (dir == 3)
        {
            D_587c_partyTile = TILE_ACTOR_FLYING_CARPET + 1;
        }
        break;

    case TILE_ACTOR_SKIFF:
        // 0152
        ULTIMA_1850_PrintString(/*0x2951*/ "Row ");
        D_587c_partyTile = (char)dir + (D_587c_partyTile & 0xfc);
        break;

    case TILE_ACTOR_FRIGATE_20:
    case TILE_ACTOR_FRIGATE_24:
        // 016a
        oldTile = D_587c_partyTile;
        D_587c_partyTile = (char)dir + (D_587c_partyTile & 0xfc);
        if (oldTile != D_587c_partyTile)
        {
            ULTIMA_1850_PrintString(/*0x2956*/ "Head ");

            switch (dir)
            {
            case 0:
                ULTIMA_1850_PrintString(/*0x295c*/ "North\n");
                break;
            case 2:
                ULTIMA_1850_PrintString(/*0x2963*/ "South\n");
                break;
            case 1:
                ULTIMA_1850_PrintString(/*0x296a*/ "East\n");
                break;
            case 3:
                ULTIMA_1850_PrintString(/*0x2970*/ "West\n");
                break;
            }

            // 01ad
            newPrompt = turned = 1;
            if (D_5c5a_actors[0].value < 0x32)
            {
                ULTIMA_1850_PrintString(/*0x2976*/ "Hull weak!\n");
            }
        }
        // 01dc
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_24 && windDir == 0)
        {
            turned = 1;
        }

        break;
    }

    // 0129
    return turned;
}

// whether the party may step by (x, y): the actor there and the walkable tile decide; prints Blocked!, OUCH!, COLLISION!, BREAKING UP!, Docked!
static int MainoutCanStepTo(int stepX, int stepY)
{
    int destTile;
    int blockerTile;
    bool canStep_c;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_24)
    {
        ULTIMA_1850_PrintString(/*0x2982*/ "Rowing!\n");
    }

    canStep_c = 1;
    blockerTile = ULTIMA_368e_FindActorTileAtPos(stepX + (uint)D_5896_mapX, stepY + (uint)D_5897_mapY, D_5895_mapLevel);
    if (blockerTile != 0)
    {
        canStep_c = 0;
        if (D_587c_partyTile >= TILE_ACTOR_30 || D_587c_partyTile < TILE_ACTOR_FRIGATE_20)
        {
            if ((blockerTile >= TILE_ACTOR_FRIGATE_24 && blockerTile < TILE_ACTOR_PIRATE) || blockerTile == TILE_ACTOR_CARPET || (blockerTile & 0xfe) == TILE_ACTOR_HORSE)
            {
                canStep_c = 1;
            }
        }
        else
        {
            if (D_587c_partyTile >= TILE_ACTOR_SKIFF && blockerTile >= TILE_ACTOR_FRIGATE_24 && blockerTile < TILE_ACTOR_SKIFF)
            {
                canStep_c = 1;
            }
        }
    }

    destTile = GetMapViewport(stepX + 5, stepY + 5);

    canStep_c = canStep_c && ULTIMA_2c4c_IsWalkableTile(D_587c_partyTile, destTile) != 0 ? 1 : 0;
    if (canStep_c == 0)
    {
        if (sailDir != 0)
        {
            if (destTile == TILE_MAP_WATER_3)
            {
                ULTIMA_1850_PrintString(/*0x298b*/ "BREAKING UP!\n");
            }
            else if (destTile != TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(/*0x2999*/ "COLLISION!\n");
            }

            if (destTile == TILE_MAP_47)
            {
                ULTIMA_1850_PrintString(/*0x29a5*/ "Docked!\n");
                D_587c_partyTile += 4;
            }
            else
            {
                ULTIMA_223c_AudioWhiteNoise(100, 2000, 300);
                DamageVehicle();
            }

            sailDir = 0;
            newPrompt = 1;
        }
        else if (D_587c_partyTile < TILE_ACTOR_FRIGATE_20 || (blockerTile & 0xfc) != TILE_ACTOR_WHIRLPOOL)
        {
            ULTIMA_1850_PrintString(/*0x29ae*/ "Blocked!\n");
            if (destTile == TILE_MAP_2F)
            {
                ULTIMA_1850_PrintString(/*0x29b8*/ "OUCH!\n");
                ULTIMA_2aa8_DamageParty();
            }
            else
            {
                ULTIMA_22c0_AudioTone(0xa5, 200);
            }
            ULTIMA_1b16_ClearKbdBuffer();
        }
    }

    return canStep_c;
}

// the party moves by (x, y); near the edge of the 32x32 view the chunks are shifted and loaded
void MainoutMoveParty(int stepX, int stepY)
{
    int relY;
    int relX;

    D_5896_mapX += stepX;
    D_5897_mapY += stepY;
    D_24e6_viewDirty = 1;
    relX = D_5896_mapX - D_589b_mapOriginX & 0x1f;
    relY = D_5897_mapY - D_589c_mapOriginY & 0x1f;

    if (relX < 5 || relX > 0x1a || relY < 5 || relY > 0x1a)
    {
        ShiftChunks(stepX, stepY); // 7bd2
        D_589b_mapOriginX = (char)stepX * 16 + D_589b_mapOriginX & 0xf0; // wrap x?
        D_589c_mapOriginY = (char)stepY * 16 + D_589c_mapOriginY & 0xf0; // wrap y?
        LoadViewChunks(stepX, stepY); // 7b8a
        ULTIMA_5e4a_BuildLightMap();
    }
}

// after a step: on rough terrain (forest, hills, mountains) the monsters move once or twice more, "Slow progress!" / "Very slow!"
static void SlowTerrain(void)
{
    int tile;
    int terrainClass;
    int moved;

    moved = 0;

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (tile == TILE_MAP_GRASS)
    {
        // 0406
        terrainClass = 0;
    }
    else if (tile == TILE_MAP_1E || tile == TILE_MAP_1F)
    {
        // 041a
        terrainClass = 1;
    }
    else if (tile < TILE_MAP_POISON || tile >= TILE_MAP_HUT) // 042e
    {
        // -> 0406
        terrainClass = 0;
    }
    else if (tile >= TILE_MAP_9)
    {
        terrainClass = 2;
    }
    else
    {
        terrainClass = 1;
    }

    // 041f
    switch (terrainClass)
    {
    case 1:
        // 0448
        moved += MonstersTurn();
        WaitIfActorNear();

        if (moved == 0)
        {
            ULTIMA_1850_PrintString(/*0x29bf*/ "Slow progress!\n");
        }

        ULTIMA_4f7c_AdvanceTime(2);
        break;

    case 2:
        // 0468
        moved += MonstersTurn();
        WaitIfActorNear();

        moved += MonstersTurn();
        WaitIfActorNear();

        if (moved == 0)
        {
            ULTIMA_1850_PrintString(/*0x29cf*/ "Very slow!\n");
        }

        ULTIMA_4f7c_AdvanceTime(4);
        break;
    }
}

u8 windDx[4] = { 0x00, 0x00, 0xFF, 0x01 };

u8 windDy[4] = { 0x01, 0xFF, 0x00, 0x00 };

// a direction key: prints the direction, turns the vehicle, steps when allowed (and the frigate sets sail); 1 when it stepped
static int WalkInDirection(int dir, int noMove)
{
    int stepY;
    int stepX;
    int result_b;

    if ((D_587c_partyTile & 0xfc) == TILE_ACTOR_FRIGATE_20)
    {
        if (dir != sailDir)
        {
            sailDir = dir;
            sailCounter = 0;
        }
        newPrompt = 0;
    }

    // 04b8
    if (D_587c_partyTile == TILE_ACTOR_AVATAR || (D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
    {
        ULTIMA_433e_AudioFootstep();
    }

    // 04cb
    stepX = stepY = 0;

    switch (dir)
    {
    case DIR_UP:
        // 04f0
        stepY--;
        if (TurnVehicle(0))
        {
            return;
        }

        // 0500
        if (sailDir == 0)
        {
            ULTIMA_1850_PrintString(/*0x29db*/ "North\n");
        }
        break;

    case DIR_DOWN:
        // 054e
        stepY++;
        if (TurnVehicle(2))
        {
            return;
        }
        if (sailDir == 0)
        {
            ULTIMA_1850_PrintString(/*0x29e2*/ "South\n");
        }
        break;

    case DIR_RIGHT:
        // 055c
        stepX++;
        if (TurnVehicle(1))
        {
            return;
        }
        if (sailDir == 0)
        {
            ULTIMA_1850_PrintString(/*0x29e9*/ "East\n");
        }
        break;

    case DIR_LEFT:
        // 0576
        stepX--;
        if (TurnVehicle(3))
        {
            return;
        }
        if (sailDir == 0)
        {
            ULTIMA_1850_PrintString(/*0x29ef*/ "West\n");
        }
        break;
    }

    // 050e
    result_b = MainoutCanStepTo(stepX, stepY);
    if (noMove == 0 && result_b != 0)
    {
        if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE)
        {
            ULTIMA_433e_AudioFootstep();
        }
        // 0530
        MainoutMoveParty(stepX, stepY);
        SlowTerrain();
    }
    return result_b;
}

// TODO
// u8 D_29f5[4]; // wind-releated
// u8 D_29f9[4]; // wind-releated

// redraws, handles the falling tile, prints the prompt; a frigate under sail moves with the wind until a key; returns the key
static int MainoutWaitForKey(void)
{
    int windStepY;
    int windStepX;
    int key_h;
    int windScore_c;

    ULTIMA_5910_UpdateFrame();
    if (D_a9fa_vitalsDirty != 0)
    {
        ULTIMA_2900_UpdateVitalsDisplay();
        D_a9fa_vitalsDirty = 0;
    }

    // 05b2
    if ((GetMapViewport(5, 6) & 0xfc) == 0xd4)
    {
        Falls();
        key_h = 0;
    }
    else
    {
        // 05c6
        if (newPrompt != 0)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
        }

        // 05d7
        newPrompt = 1;
        while (sailDir != 0)
        {
            key_h = ToUpperChar(PollKeyWithCursor());
            if (key_h != 0 && key_h != sailDir)
                break;

            // 05e6
            key_h = (uint)sailDir;
            if (windDir != 0)
            {
                windStepX = windStepY = 0;

                switch (key_h)
                {
                case DIR_UP:
                    // 0614
                    windStepY = (byte)-1;
                    break;
                case DIR_DOWN:
                    // 0658
                    windStepY = 1;
                    break;
                case DIR_RIGHT:
                    // 0660
                    windStepX = 1;
                    break;
                case DIR_LEFT:
                    // 0668
                    windStepX = (byte)-1;
                    break;
                }

                // 0619
                windScore_c = 1;
                if (windDx[windDir - 1] != windStepX)
                {
                    windScore_c++;
                }
                // 062e
                if (windDy[windDir - 1] != windStepY)
                {
                    windScore_c++;
                }
                // 0640
                if (windScore_c % 3 <= (uint)sailCounter)
                {
                    sailCounter = 0;
                    break;
                }
            }

            // 0670
            if (hmsCapePlans > 0x7f)
            {
                capeToggle = (uint)(capeToggle == 0);
                ULTIMA_4f7c_AdvanceTime(1);
            }
            else
            {
                ULTIMA_4f7c_AdvanceTime(2);
            }

            if (capeToggle == 0 || hmsCapePlans < 0x80)
            {
                MonstersTurn();
            }

            WaitFrames(1);
            sailCounter = sailCounter + 1;
        }

        if (sailDir == 0)
        {
            key_h = ULTIMA_266c_GetChar();
        }
    }

    return key_h;
}

// the Attack command: a direction, then combat with what is there, if it can be fought (not on a skiff or carpet over water)
int MainoutAttackCmd(void)
{
    int targetX_b;
    int targetY_b;
    int result_b = 0;
    int targetTile;

    ULTIMA_1850_PrintString(/*0x29fe*/ "Attack-");
    if ((*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON) &&
        ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
    {
        ULTIMA_1850_PrintString(/*0x2a06*/ "On foot!\n");
        result_b = 0;
    }
    else
    {
        if (ULTIMA_35ec_SelectDirection() != 0)
        {
            targetX_b = (uint)D_5896_mapX + D_5876_tempX;
            targetY_b = (uint)D_5897_mapY + D_5878_tempY;
            targetTile = ULTIMA_368e_FindActorTileAtPos(targetX_b, targetY_b, D_5895_mapLevel) & 0xfc;
            if (targetTile == TILE_ACTOR_PIRATE || (targetTile != TILE_ACTOR_SHARD && targetTile != TILE_ACTOR_E8 && targetTile >= TILE_ACTOR_WIZARD))
            {
                ULTIMA_6150_Combat(D_5876_tempX);
            }
            else
            {
                ULTIMA_1850_PrintString(/*0x2a10*/ "Nothing to attack!\n");
            }
        }
    }
    return result_b;
}

// prints what is entered, finds the dungeon at the party's position (D_1e8a/D_1eb2, entries 0x20-0x27), saves the actors and reads its DUNGEON.DAT level; Doom's entrance may be guarded
static int LoadDungeon(char* verb)
{
    int slot;
    int dungeonIndex_i;

    ULTIMA_1850_PrintString(verb);
    for (dungeonIndex_i = 0x20; dungeonIndex_i < 0x28; dungeonIndex_i++)
    {
        if (D_1e8a_placeX[dungeonIndex_i] == D_5896_mapX && D_1eb2_placeY[dungeonIndex_i] == D_5897_mapY)
            break;
    }

    if (dungeonIndex_i < 0x28)
    {
        if (D_587c_partyTile != TILE_ACTOR_AVATAR)
        {
            ULTIMA_1850_PrintString(/*0x2a24*/ "\nOn foot!\n");
            return; // sic
        }

        if (dungeonIndex_i == 0x27)
        {
            if ((D_58c8_shadowlordTowns[0] & D_58c8_shadowlordTowns[1] & D_58c8_shadowlordTowns[2]) < 0x80)
            {
                ULTIMA_1850_PrintString(/*0x2a2f*/ "\nAttacked at entrance!\n");
                slot = ULTIMA_38e4_FindActorSlot();
                D_5c5a_actors[slot].tile = TILE_ACTOR_SHADOWLORD;
                ULTIMA_6150_Combat(slot);
                return; // sic
            }
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x2a47*/ "\n\n");
            ULTIMA_16ba_PrintChar(CTRL_CHAR_CENTER_TEXT);
            ULTIMA_1850_PrintString(mapNames[dungeonIndex_i]);
            ULTIMA_16ba_PrintChar(CTRL_CHAR_UNCENTER_TEXT);
        }

        ULTIMA_16ba_PrintChar('\n');
        if (curDisk[0] != 1)
        {
            SwitchDisks(1);
            while (TestOpenFile(/*0x2a4a*/ "BRIT.DAT") == 0) {}
        }

        WriteFileToDisk(GetWorldSavefile(), D_5c5a_actors, 0x100);
        SwitchDisks(2);
        ULTIMA_256e_ReadFileFromDisk(/*0x2a53*/ "DUNGEON.DAT", dungeonMap, 0x200, dungeonIndex_i * 0x200 + -0x4000);
        D_5893_mapId = dungeonIndex_i + 1;
        if (D_5895_mapLevel != 0 && (byte)(dungeonIndex_i + 1) != 0x28)
        {
            D_5895_mapLevel = 7;
            dungeonFacing = 3;
            dungeonMoveDir = 4;
            D_5896_mapX = D_5897_mapY = 7;
        }
        else
        {
            D_5895_mapLevel = 0;
            D_5896_mapX = D_5897_mapY = 1;
            dungeonFacing = 1;
            dungeonMoveDir = 5;
        }
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x2a5f*/ "\nWhat dungeon?\n");
    }

    return 1;
}

// the Enter command: by the tile under the party, a shrine, a town, castle, keep, hut, lighthouse, dungeon, cave, mine or the palace of Blackthorn; 1 when it went in
int EnterCmd(void)
{
    int shrineIndex;
    int result_b;
    int tile;

    result_b = 1;

    ULTIMA_1850_PrintString(/*0x2a6f*/ "Enter ");

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    switch (tile)
    {
    case TILE_MAP_SHRINE:
        ULTIMA_1850_PrintString(/*0x2a76*/ "the shrine of\n");
        for (shrineIndex = 0; shrineIndex < 8; shrineIndex++)
        {
            if (shrineX[shrineIndex] == D_5896_mapX && shrineY[shrineIndex] == D_5897_mapY)
                break;
        }

        ULTIMA_1850_PrintString(virtueNames[shrineIndex]);
        ULTIMA_16ba_PrintChar('\n');
        Shrine();
        break;

    case TILE_MAP_HUT:
        result_b = EnterTown(/*0x2a85*/ "hut");
        break;

    case TILE_MAP_CODEX:
        ULTIMA_1850_PrintString(/*0x2a89*/ "the Shrine of the Codex!\n");
        Shrine();
        break;

    case TILE_MAP_KEEP:
        result_b = EnterTown(/*0x2aa3*/ "keep");
        break;

    case TILE_MAP_VILLAGE:
        result_b = EnterTown(/*0x2aa8*/ "village");
        break;

    case TILE_MAP_TOWNE:
        result_b = EnterTown(/*0x2ab0*/ "towne");
        break;

    case TILE_MAP_CASTLE:
        result_b = EnterTown(/*0x2ab6*/ "castle");
        break;

    case TILE_MAP_CAVE:
        result_b = LoadDungeon(/*0x2abd*/ "cave");
        break;

    case TILE_MAP_MINE:
        result_b = LoadDungeon(/*0x2ac2*/ "mine");
        break;

    case TILE_MAP_DUNGEON:
        result_b = LoadDungeon(/*0x2ac7*/ "dungeon");
        break;

    case TILE_MAP_RUINS:
        ULTIMA_1850_PrintString(/*0x2acf*/ "ruins");
        break;

    case TILE_MAP_LIGHTHOUSE:
        result_b = EnterTown(/*0x2ad5*/ "lighthouse");
        break;

    case TILE_MAP_PALACEBT:
        result_b = EnterTown(/*0x2ae0*/ "the palace of Blackthorn!");
        break;

    case TILE_MAP_CASTLELB:
        result_b = EnterTown(/*0x2afa*/ "the Castle of Lord British!");
        break;

    default:
        ULTIMA_1850_PrintString(/*0x2b16*/ "What?\n");
        result_b = 0;
        break;
    }

    return result_b;
}

// on a blank (0xff) tile without the status 0xe the light level is 0, the view is redrawn once
static int UpdateDarkness(int wasDark)
{
    if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) == 0xff && statusIcon != 0xe)
    {
        D_58a5_lightLevel = 0;
        if (wasDark == 0)
        {
            ULTIMA_5910_UpdateFrame();
            wasDark = 1;
        }
    }
    else
    {
        wasDark = 0;
        ULTIMA_4f7c_AdvanceTime(0);
    }

    return wasDark;
}

// underground: one in 256 turns "EARTHQUAKE!" damages the party
static void CheckEarthquake(void)
{
    if (D_5895_mapLevel != 0)
    {
        if (ULTIMA_2092_RandomRange(0, 0xff) == 0x69)
        {
            ULTIMA_1850_PrintString(/*0x2b1d*/ "EARTHQUAKE!\n");
            ULTIMA_3072_ShakeScreen();
            ULTIMA_2aa8_DamageParty();
        }
    }
}

// the overworld's command loop: sleeping, death, moongates, a key (Ctrl-K karma, E exit, V version, S sound, direction, commands), then the tile's effects and the monsters' turn
static void MainoutMainLoop(void)
{
    int tileUnder;
    int unused1_h; // not used
    int unused2_g; // not used
    bool done_g;
    int turnTaken;
    int awake;
    byte key;
    int dark = 0;
    do
    {
        // 0a8f
        done_g = 0;
        turnTaken = 1;
        dark = UpdateDarkness(dark);
        awake = GetFirstActivePartyMember();
        if (awake == 1)
        {
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_4c2a_PrintLeftCap();
            // 0aba
            ULTIMA_1850_PrintString(/*0x2b2a*/ "Zzzzzz...\n");
        }
        else if (awake == -1) // 0ac2
        {
            if (curDisk[0] != 1)
            {
                SwitchDisks(1);
                // 0ad6
                while (TestOpenFile(/*0x2b35*/ "BRIT.DAT") == 0) {}
            }
            // 0ae1
            WriteFileToDisk(GetWorldSavefile(), D_5c5a_actors, 0x100);
            Death();
            done_g = 1;
            turnTaken = 0;
        }
        else
        {
            // 0b00
            if (EnterMoongate() != 0)
            {
                Shrine();
            }
            // 0b0a
            if (D_5893_mapId != 0)
            {
                return;
            }
            // 0b14
            key = MainoutWaitForKey();
            if (key < 0x20)
            {
                switch (key & 0xff)
                {
                case U5_KEY_CTRL_K: // 0b34
                    PrintNumber(D_5888_karma, 1, 0x20);
                    // -> 0b75
                    ULTIMA_16ba_PrintChar('\n');
                    // -> 0af8
                    turnTaken = 0;
                    break; // -> 0c12

                case U5_KEY_CTRL_E: // 0b48
                    ULTIMA_1850_PrintString(/*0x2b3e*/ "Exit to DOS? ");
                    key = ULTIMA_266c_GetChar();
                    if (key == 'Y')
                    {
                        RestoreVideoMode();
                        exit(0);
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(/*0x2b4c*/ "N\n");
                    }
                    break;

                case U5_KEY_CTRL_V: // 0b6e
                    ULTIMA_1850_PrintString(/*0x2b4f*/ "1.16");
                    // 0b75
                    ULTIMA_16ba_PrintChar('\n');
                    turnTaken = 0;
                    break;

                case U5_KEY_CTRL_S: // 0b80
                    ULTIMA_1850_PrintString(/*0x2b54*/ "Sound ");
                    if (D_a9ce_soundOn != 0)
                    {
                        ULTIMA_1850_PrintString(/*0x2b5b*/ "Off\n");
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(/*0x2b60*/ "On\n");
                    }
                    // ?
                    D_a9ce_soundOn = !D_a9ce_soundOn;
                    break;

                case U5_KEY_LEFT: // 0baa
                case U5_KEY_RIGHT:
                case U5_KEY_UP:
                case U5_KEY_DOWN:
                    turnTaken = WalkInDirection(key, dark);
                    break;

                case 0: // 0af8
                    turnTaken = 0;
                    break;

                default: // 0bb8
                    ULTIMA_1850_PrintString(/*0x2b64*/ "What?\n");
                    break;
                }
            }
            else
            {
                // 0bee
                if (key < 0x30 || key > 0x39)
                {
                    // 0bfa
                    turnTaken = ProcessCommand(key & 0xff);
                }
                else
                {
                    // 0c06
                    turnTaken = ULTIMA_4080_SetActivePlayer(key);
                }
            }
        }
        // 0c12
        if ((D_587c_partyTile & 0xfc) != TILE_ACTOR_FRIGATE_20)
        {
            sailDir = 0;
        }
        // 0c20
        if (D_5893_mapId != 0)
        {
            done_g = 1;
        }
        else
        {
            if (turnTaken != 0)
            {
                // 0c39
                ULTIMA_4f7c_AdvanceTime(2);
                tileUnder = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
                if ((tileUnder & 0xfe) == TILE_MAP_6A)
                {
                    BridgeTrolls();
                }
                // 0c64
                else if (tileUnder == TILE_MAP_POISON && D_587c_partyTile == TILE_ACTOR_AVATAR)
                {
                    StepOnPoisonTile();
                    WaitFrames(1);
                }
                else if (tileUnder == TILE_MAP_LAVA)
                {
                    StepOnBurningTile();
                }
                else if (D_5896_mapX == 0xe9 && D_5897_mapY == 0xeb && D_5895_mapLevel == 0 && D_5893_mapId == 0)
                {
                    ULTIMA_1850_PrintString(/*0x2b6b*/ "\n\"");
                    if ((byte)questsOrdained != 0)
                    {
                        ULTIMA_1850_PrintString(/*0x2b6e*/ "Pass, Seeker!\"\n");
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(/*0x2b7e*/ "Thou art not upon a Sacred Quest!\n");
                        ULTIMA_1850_PrintString(/*0x2ba1*/ "Passage denied!\"\n");
                        D_5897_mapY++;
                    }
                }

                // 0cd0
                CheckEarthquake();
                ULTIMA_2ae8_EndTurn();
                if (tileUnder == 1 && ((D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET))
                {
                    ULTIMA_1850_PrintString(/*0x2bb3*/ "Rough seas!\n");
                    ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                    DamageVehicle();
                }
                if ((tileUnder & 0xfc) == TILE_MAP_WATERFALL)
                {
                    Falls();
                }
                MonstersTurn();
            }
        }
    } while (!done_g);
}

// the overlay's entry: sets up the overworld, puts a bought ship at its place, runs the main loop
void MainoutEntry(void)
{
    int shipTile;
    int slot;
    ActorFmt* ship_m;

    SetUpOverworld();

    if (boughtShip >= 0x40)
    {
        slot = ULTIMA_38e4_FindActorSlot();
        ship_m = &D_5c5a_actors[slot]; // get some actor
        ship_m->extra = boughtShip & 0x3f;
        ship_m->x = boughtShipX; // ship_x
        ship_m->y = boughtShipY; // ship_y
        ship_m->z = 0;

        if (boughtShip > 0x7f)
        {
            shipTile = TILE_ACTOR_FRIGATE_25;
        }
        else
        {
            shipTile = TILE_ACTOR_SKIFF_29;
        }

        ship_m->tile = ship_m->animTile = shipTile;
        ship_m->value = 99;
        boughtShip = 0;
    }

    MainoutMainLoop();
}

// how likely a monster spawn is, 0 (towns, castles) to 5: by the tile under the party, + 3 at night; 3 in the underworld
int SpawnChance(void)
{
    int tile;
    int chance;

    if (D_5895_mapLevel > 0x7f)
    {
        return 3;
    }

    tile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    if (tile >= TILE_MAP_20 && tile <= TILE_MAP_26)
    {
        chance = 0;
    }
    else
    {
        if (tile == TILE_MAP_POISON || (tile >= TILE_MAP_9 && tile <= TILE_MAP_F))
        {
            chance = 2;
        }
        else
        {
            chance = 1;
        }
    }

    if (D_587f_hour >= 0x20 || D_587f_hour < 5)
    {
        return chance + 3;
    }

    return chance;
}

// The original adds the index and the pointer as integers (mov bx,cx / add bx,si),
// not with the [bx][si] addressing mode that param_1[i] gives.
#define AT(i) (*(byte*)((uint)(i) + (uint)weights))

// an index picked by weights that add up to 256 (a roll of 0-255 minus each weight)
static int PickWeighted(byte* weights)
{
    int index;
    int roll_h;

    roll_h = ULTIMA_2092_RandomRange(0, 0xff);

    for (index = 0; AT(index) <= roll_h; index++)
    {
        roll_h -= AT(index);
    }
    return index;
}

// the monster (actor tile) that may appear on that terrain: sea, land or underworld tables, whirlpools in deep water; 0 for none
static byte MonsterForTile(int tile)
{
    int unused;

    if (tile < TILE_MAP_POISON || (tile >= TILE_MAP_60 && tile <= TILE_MAP_6F) || (tile >= TILE_MAP_WATERFALL && tile <= TILE_MAP_WATERFALL + 3) ||
        (tile >= TILE_MAP_WATER_E4 && tile <= TILE_MAP_WATER_E7))
    {
        if (ULTIMA_2092_RandomRange(0, 0x40) < 0x10)
        {
            if (D_5895_mapLevel < 0x80)
            {
                if (tile == 1 && ULTIMA_2092_RandomRange(0, 7) == 7)
                {
                    return TILE_ACTOR_WHIRLPOOL;
                }
                else
                {
                    return seaMonsters[PickWeighted(seaWeights)];
                }
            }
            else
            {
                return deepSeaMonsters[PickWeighted(deepSeaWeights)];
            }
        }
    }
    else if (tile == TILE_MAP_7)
    {
        if (ULTIMA_2092_RandomRange(0, 3) == 0)
        {
            return TILE_ACTOR_E0;
        }
    }
    else
    {
        if (tile == TILE_MAP_POISON && D_5895_mapLevel == 0xff)
        {
            return TILE_ACTOR_F8;
        }

        if (tile == TILE_MAP_C || tile == TILE_MAP_D)
        {
            return 0;
        }

        if (tile < TILE_MAP_HUT || ((byte)tile & 0xfc) == TILE_MAP_30)
        {
            if (D_5895_mapLevel < 0x80)
            {
                return landMonsters[PickWeighted(landWeights)];
            }

            return deepLandMonsters[PickWeighted(deepLandWeights)];
        }
    }

    return 0;
}

// a random cell of the 32x32 view more than six away from the party, into D_5876/D_5878
static void PickSpawnCell(void)
{
    do
    {
        D_5876_tempX = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589b_mapOriginX);
        D_5878_tempY = (byte)(ULTIMA_2092_RandomRange(0, 0x1f) + D_589c_mapOriginY);
    } while (abs(D_5876_tempX - D_5896_mapX) <= 6 || abs(D_5878_tempY - D_5897_mapY) <= 6 ||
        abs(D_5876_tempX - D_5896_mapX) >= 0xfa || abs(D_5878_tempY - D_5897_mapY) >= 0xfa);
}

// looks up to 128 times for a cell and a monster that fit, and puts it there (pirates get hull value 100)
void SpawnMonster(void)
{
    int spawnY;
    int spawnX;
    int tries_j;
    byte monsterTile_k;

    for (tries_j = 0; tries_j < 0x80; tries_j++)
    {
        PickSpawnCell();

        spawnX = D_5876_tempX;
        spawnY = D_5878_tempY;

        monsterTile_k = MonsterForTile(*ULTIMA_4402_GetTileAddr(spawnX, spawnY));
        if (monsterTile_k != 0)
        {
            if (monsterTile_k != TILE_ACTOR_PIRATE || (*ULTIMA_4402_GetTileAddr(spawnX, spawnY) & 0xf0) != TILE_MAP_60)
            {
                break;
            }
        }
    }

    if (tries_j != 0x80)
    {
        tries_j = ULTIMA_38e4_FindActorSlot();
        SetActor(monsterTile_k, monsterTile_k, spawnX, spawnY, D_5895_mapLevel, 0, tries_j);
        if (monsterTile_k == TILE_ACTOR_PIRATE)
        {
            D_5c5a_actors[tries_j].value = 100;
        }
    }
}

// whether an actor tile is one that moves by itself: 0x2c-0x2f and 0x80 up except 0xb4-0xb7 and 0xe8-0xeb
int IsMovingActor(int tile)
{
    if (tile >= 0x2c && tile <= 0x2f)
    {
        return 1;
    }
    
    if (tile < 0x80)
    {
        return 0;
    }

    if (tile >= 0xb4 && tile <= 0xb7 || tile >= 0xe8 && tile <= 0xeb)
    {
        return 0;
    }

    return 1;
}
