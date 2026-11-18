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

static u8 D_a526_lastX; // this object's _BSS (DS:a526)
static u8 D_a527_lastY;

// The part of MAINOUT.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").
// Where this part starts is not certain: MAINOUT_0e4e_MonsterForTile to 105c use no strings and compile
// the same with both compilers, so the original boundary may be anywhere from 0e4e to 109e.

int MAINOUT_0d8c_SpawnChance(void);
int MAINOUT_105c_IsMovingActor(int tile);
void MAINOUT_0000_SetUpOverworld(void);
void MAINOUT_0fc4_SpawnMonster(void);

u8 D_2bc0_landMonsters[0xc] = { 0xc0, 0xc8, 0x90, 0x98, 0xbc, 0xc4, 0xd0, 0xe4, 0xcc, 0xd4, 0xdc, 0xd8 };

u8 D_2bcc_deepLandMonsters[0x8] = { 0x94, 0x90, 0x98, 0xf0, 0xf4, 0xd8, 0xdc, 0x00 };

u8 D_2bd4_seaMonsters[0x6] = { 0x8c, 0x84, 0x88, 0x80, 0x2c, 0x00 };

u8 D_2bda_deepSeaMonsters[0x2] = { 0x84, 0x88 };

u8 D_2bdc_landWeights[0xc] = { 0x3c, 0x32, 0x28, 0x1e, 0x14, 0x0f, 0x0f, 0x0a, 0x0a, 0x03, 0x02, 0x01 };

u8 D_2be8_deepLandWeights[0x8] = { 0x40, 0x38, 0x38, 0x20, 0x20, 0x08, 0x08, 0x00 };

u8 D_2bf0_seaWeights[0x6] = { 0x48, 0x48, 0x28, 0x26, 0x22, 0x00 };

u8 D_2bf6_deepSeaWeights[0x2] = { 0x80, 0x80 };

u16 D_2bf8_sailDelay[4][4] = { 2, 3, 4, 4, 4, 4, 2, 3, 3, 2, 4, 4, 4, 4, 3, 2 };

u8 D_2c18_chaseRange[0x3d] =
{
    0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// u8 2c54 = 0;
u8 D_2c55_quickToggle = 0;

static u8 D_2c56 = 0; // unused
u8 D_2c57_mountToggle = 0;

// CHECKED
// a blow to the party's vehicle: a ship loses 1 to 30 hull points and sinks (abandon ship, or drowning); on foot the party is hurt
void MAINOUT_109e_DamageVehicle(void)
{
    int skiffs;
    int damage;

    if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
    {
        damage = ULTIMA_2092_RandomRange(1, 0x1e);
        if (damage < D_5c5a_actors[0].value)
        {
            D_5c5a_actors[0].value -= damage;
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x6ada*/ "Ship sunk!\n");
            skiffs = D_5c5a_actors[0].extra;
            if (skiffs > 0 || D_57b0_carpets != 0)
            {
                ULTIMA_1850_PrintString(/*0x6ae6*/ "Abandon ship!\n");
                if (skiffs > 0)
                {
                    D_587c_partyTile = (D_587c_partyTile & 3) + TILE_ACTOR_SKIFF;
                }
                else
                {
                    D_57b0_carpets--;
                    D_587c_partyTile = ULTIMA_2092_RandomRange(0, 1) + TILE_ACTOR_FLYING_CARPET;
                }

                D_a9fa_vitalsDirty = 1;
            }
            else
            {
                D_587c_partyTile = 0;
                ULTIMA_2900_UpdateVitalsDisplay();
                ULTIMA_5910_UpdateFrame();
                ULTIMA_43ae_AudioSweepTone(0x294, 0x96, 0x28, 0x1e78);
                ULTIMA_1850_PrintString(/*0x6af6*/ "DROWNING!!!\n");

                while (ULTIMA_39fc_GetFirstActivePartyMember() != -1)
                {
                    ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                    ULTIMA_2aa8_DamageParty();
                }
                return; // needed to match: drops the jump over the else branch
            }
        }
    }
    else
    {
        ULTIMA_2aa8_DamageParty();
    }
}

// a pirate ship fires (cannon sound, a missile to the party); a hit hurts the vehicle
static void MAINOUT_1168_PirateFire(int actor, int distX, int distY)
{
    byte shotY;
    byte shotX;
    int hit;

    if (distX == 0)
    {
        if (D_5c5a_actors[actor].animTile == TILE_ACTOR_PIRATE || D_5c5a_actors[actor].animTile == TILE_ACTOR_2E)
        {
            D_5c5a_actors[actor].animTile = (ULTIMA_2092_RandomRange(0, 3) & 2) + TILE_ACTOR_2D;
        }
    }

    if (distY == 0)
    {
        if (D_5c5a_actors[actor].animTile == TILE_ACTOR_2D || D_5c5a_actors[actor].animTile == TILE_ACTOR_2F)
        {
            D_5c5a_actors[actor].animTile = (ULTIMA_2092_RandomRange(0, 3) & 2) + TILE_ACTOR_PIRATE;
        }
    }

    ULTIMA_5910_UpdateFrame();
    ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);

    shotX = D_5c5a_actors[actor].x - D_5896_mapX + 5;
    shotY = D_5c5a_actors[actor].y - D_5897_mapY + 5;
    hit = COMSUBS_12de_FlyMissile(shotX, shotY, 5, 5, 1);
    if (hit != 0)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
        MAINOUT_109e_DamageVehicle();
    }
}

// an actor next to the party: a whirlpool pulls the ship into the underworld, else "Attacked!" and combat (not on a skiff or carpet over water)
static void MAINOUT_1248_ActorTouchesParty(int actor)
{
    int savedTile;

    if ((D_5c5a_actors[actor].tile & 0xfc) == TILE_ACTOR_WHIRLPOOL)
    {
        if (D_587c_partyTile == TILE_ACTOR_AVATAR)
        {
            MAINOUT_109e_DamageVehicle();
            return;
        }
        else
        {
            D_5c5a_actors[actor].tile = D_5c5a_actors[actor].animTile = 0;
            ULTIMA_1850_PrintString(/*0x6b04*/ "\nWHIRLPOOL!\n");

            savedTile = D_587c_partyTile;
            D_587c_partyTile = TILE_ACTOR_WHIRLPOOL;
            ULTIMA_5910_UpdateFrame();
            ULTIMA_43ae_AudioSweepTone(0x294, 0x96, 0x28, 0x1e78);

            D_587c_partyTile = savedTile;
            MAINOUT_109e_DamageVehicle();

            D_5895_mapLevel = 0xff;
            D_5896_mapX = 0x22;
            D_5897_mapY = 0x12;
            MAINOUT_0000_SetUpOverworld();
        }
    }
    else if ((D_5c5a_actors[actor].tile & 0xfc) != TILE_ACTOR_E0)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_1850_PrintString(/*0x6b12*/ "\nAttacked!\n");

        if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON &&
            ((D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET || (D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF))
        {
            MAINOUT_109e_DamageVehicle();
            return;
        }
        else
        {
            ULTIMA_6150_Combat(actor);
        }
    }
    else
    {
        MAINOUT_109e_DamageVehicle();
        return;
    }
}

// next to the party: touches it; sea monsters fire from three cells at 1 in 8; a pirate ship in line says "BOOOM!"; 1 when it acted
static int MAINOUT_131a_ActorAttack(int actor)
{
    byte shotY_g;
    int tile;
    byte shotX_h;
    int distY;
    int distX;
    int hit;

    tile = D_5c5a_actors[actor].tile;
    distX = abs(D_5c5a_actors[actor].x - D_5896_mapX);
    if (distX > 0x7f)
    {
        distX = 0x100 - distX;
    }

    distY = abs(D_5c5a_actors[actor].y - D_5897_mapY);
    if (distY > 0x7f)
    {
        distY = 0x100 - distY;
    }

    if ((distX == 1 && distY == 0) || (distX == 0 && distY == 1))
    {
        MAINOUT_1248_ActorTouchesParty(actor);
        return 1;
    }

    if (tile == TILE_ACTOR_88 || tile == TILE_ACTOR_DC)
    {
        if (distX <= 3 && distY <= 3 && ULTIMA_2092_RandomRange(0, 7) == 0)
        {
            ULTIMA_5910_UpdateFrame();
            ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);

            shotX_h = D_5c5a_actors[actor].x - D_5896_mapX + 5;
            shotY_g = D_5c5a_actors[actor].y - D_5897_mapY + 5;

            hit = COMSUBS_12de_FlyMissile(shotX_h, shotY_g, 5, 5, 3);
            if (hit != 0)
            {
                ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                MAINOUT_109e_DamageVehicle();
            }

            return 1;
        }
    }
    else if ((tile & 0xfc) == TILE_ACTOR_PIRATE && ((distX == 0 && distY < 4) || (distY == 0 && distX < 4)))
    {
        ULTIMA_1850_PrintString(/*0x6b1e*/ "* BOOOM! *\n\n");
        MAINOUT_1168_PirateFire(actor, distX, distY);
        return 1;
    }

    return 0;
}

// the cell is walkable for the actor and holds no other actor
static int MAINOUT_1482_CanActorEnter(int actor, int destX, int destY)
{
    if (ULTIMA_2c4c_IsWalkableTile(D_5c5a_actors[actor].tile, *ULTIMA_4402_GetTileAddr(destX, destY)) == 0)
        return 0;

    if (ULTIMA_3702_FindLastActorTileAt(destX, destY, D_5895_mapLevel) != 0)
        return 0;

    return 1;
}

// 0 for the cell the last moved actor came from (D_a526/D_a527)
static int MAINOUT_14c8_NotLastCell(int destX, int destY)
{
    if (destX == D_a526_lastX && destY == D_a527_lastY)
    {
        return 0;
    }

    return 1;
}

// the actor is within six cells and in the pattern D_2c18
static int MAINOUT_14ea_InChaseRange(int actor)
{
    int distY;
    int distX;

    distX = abs(D_5c5a_actors[actor].x - D_5896_mapX);
    if (distX > 0x7f)
    {
        distX = 0x100 - distX;
    }

    distY = abs(D_5c5a_actors[actor].y - D_5897_mapY);
    if (distY > 0x7f)
    {
        distY = 0x100 - distY;
    }

    if (distX < 6 && distY < 6)
    {
        return D_2c18_chaseRange[distX + distY * 0xb];
    }

    return 0;
}

// moves an actor by (x, y): a pirate ship turns to face it; rough terrain may stop it; it leaves a moongate's cell empty
static void MAINOUT_1578_MoveActor(int actor, int stepX, int stepY)
{
    int tile_b;
    int destTile;
    int destY;
    int destX;
    int facing_h;

    tile_b = D_5c5a_actors[actor].tile;
    destX = D_5c5a_actors[actor].x + stepX;
    destY = D_5c5a_actors[actor].y + stepY;
    destTile = *ULTIMA_4402_GetTileAddr(destX, destY);
    if ((tile_b & 0xfc) == TILE_ACTOR_PIRATE)
    {
        if (stepX == 0 && stepY == -1)
        {
            facing_h = 0;
        }
        else if (stepX == 1 && stepY == 0)
        {
            facing_h = 1;
        }
        else if (stepX == 0 && stepY == 1)
        {
            facing_h = 2;
        }
        else if (stepX == -1 && stepY == 0)
        {
            facing_h = 3;
        }

        D_5c5a_actors[actor].tile = D_5c5a_actors[actor].animTile = facing_h + TILE_ACTOR_PIRATE;
    }
    else if (tile_b != TILE_ACTOR_DC && tile_b != TILE_ACTOR_94 && tile_b != TILE_ACTOR_D8 && tile_b != TILE_ACTOR_F0)
    {
        switch (destTile)
        {
        case TILE_MAP_POISON:
        case TILE_MAP_6:
        case TILE_MAP_7:
        case TILE_MAP_8:
        case TILE_MAP_1E:
        case TILE_MAP_1F:
            if (ULTIMA_2092_RandomRange(0, 1) == 0)
            {
                return;
            }
            break;
        case TILE_MAP_9:
        case TILE_MAP_A:
        case TILE_MAP_B:
        case TILE_MAP_C:
        case TILE_MAP_D:
        case TILE_MAP_E:
        case TILE_MAP_F:
            if (ULTIMA_2092_RandomRange(0, 2) != 2)
            {
                return;
            }
        }
    }

    D_a526_lastX = D_5c5a_actors[actor].x;
    D_a527_lastY = D_5c5a_actors[actor].y;

    D_5c5a_actors[actor].x = destX;
    D_5c5a_actors[actor].y = destY;
    D_24e6_viewDirty |= 2;

    if (*ULTIMA_4402_GetTileAddr(destX, destY) == TILE_MAP_MOONGATE)
    {
        D_5c5a_actors[actor].tile = D_5c5a_actors[actor].animTile = 0;
    }
}

// tries random directions, up to three times, and takes the first free cell
static void MAINOUT_16fc_MoveRandomly(int actor)
{
    int northY_i;
    int eastX_h;
    int southY;
    int westX;
    int y;
    int x;
    int tries;

    x = D_5c5a_actors[actor].x;
    y = D_5c5a_actors[actor].y;

    tries = 0;
    westX = x - 1;
    southY = y + 1;
    eastX_h = x + 1;
    northY_i = y - 1;

    // bug?
    for (; tries < 3; tries++)
    {
        switch (ULTIMA_2092_RandomRange(0, 3))
        {
        case 0:
            // 1760
            if (MAINOUT_1482_CanActorEnter(actor, x, northY_i) != 0)
            {
                MAINOUT_1578_MoveActor(actor, 0, -1);
            }
            return;

        case 1:
            // 177c
            if (MAINOUT_1482_CanActorEnter(actor, eastX_h, y) != 0)
            {
                MAINOUT_1578_MoveActor(actor, 1, 0);
            }
            return;

        case 2:
            // 1790
            if (MAINOUT_1482_CanActorEnter(actor, x, southY) != 0)
            {
                MAINOUT_1578_MoveActor(actor, 0, 1);
            }
            return;

        case 3:
            // 17a8
            if (MAINOUT_1482_CanActorEnter(actor, westX, y) != 0)
            {
                MAINOUT_1578_MoveActor(actor, -1, 0);
            }
            return;
        }
    }
}

// The original stores the constants to mode through AX ("mov ax,1; mov [bp+4],ax"), MSC's
// form for a chained assignment (see SHOPPES2_00ac_PrintHonorific): mode = mode = 1 gives it.
#define P2_SET(v) mode = mode = (v) // two stores between sequence points: DOS build only

// CHECKED
// MSC places a block that nothing falls into after the first unconditional jump that follows
// it in the source. The original has the two "if (x > 0) mode = 0;" blocks after the
// local_e_am == 0 branch's jump and after the first MAINOUT_1578_MoveActor call's, jumping back to the
// next test: they were reached only by a goto from the else, and sat (here in an if (0)) in
// the source after the code that ends with those jumps. Written in place, as an else-if, MSC
// keeps them inline and the "x < 0" blocks jump over them. The end is an if/else chain with
// MAINOUT_16fc_MoveRandomly in each last else, not early returns: the second MAINOUT_1578_MoveActor call ends a
// then-branch, so tail merging keeps its copy and the first one jumps forward into it.
// one step toward the party, along x or y by chance; otherwise at random (the second parameter is only written)
static void MAINOUT_17d4_MoveTowardParty(int actor, int mode)
{
    int relY;
    int y;
    int relX_m;
    int x;
    int stepY;
    int stepX;

    x = D_5c5a_actors[actor].x;
    y = D_5c5a_actors[actor].y;
    relX_m = (D_5c5a_actors[actor].x - D_5896_mapX) & 0xff;
    if (relX_m > 0x7f)
    {
        relX_m -= 0x100;
    }

    relY = (D_5c5a_actors[actor].y - D_5897_mapY) & 0xff;
    if (relY > 0x7f)
    {
        relY -= 0x100;
    }

    // 1828
    if (relX_m == 0)
    {
        stepX = 0;
    }
    else if (relX_m > 0) // 1836
    {
        P2_SET(1);
        stepX = -1;
        // -> 1864
    }
    else
    {
        if (relX_m < 0) // 184a
        {
            P2_SET(0);
        }

        if (relX_m < 0) // 1855
        {
            stepX = mode = 1;
        }
        else
        {
            goto L_1872;
        }
    }

L_1864:
    if (relY == 0) // 1864
    {
        stepY = 0;

        if (0)
        {
L_1872:
            if (relX_m > 0)
            {
                P2_SET(0);
            }
            goto L_1864;
        }
    }
    else if (relY > 0) // 1880
    {
        P2_SET(1);
        stepY = -1;
    }
    else
    {
        if (relY < 0) // 1894
        {
            P2_SET(0);
        }

        if (relY < 0) // 189f
        {
            stepY = mode = 1;
        }
        else
        {
            goto L_18ee;
        }
    }

    if (0)
    {
L_18ee:
        if (relY > 0)
        {
            P2_SET(0);
        }
        goto L_18ae;
    }

L_18ae:
    if (ULTIMA_2092_RandomRange(0, 1) == 1)
    {
        // 18bd
        if (stepX != 0 && MAINOUT_1482_CanActorEnter(actor, x + stepX, y) != 0 &&
            MAINOUT_14c8_NotLastCell(x + stepX, y) != 0)
        {
            // 18e2: -> 192a
            MAINOUT_1578_MoveActor(actor, stepX, 0);
        }
        else if (stepY != 0 && MAINOUT_1482_CanActorEnter(actor, x, y + stepY) != 0 &&
                 MAINOUT_14c8_NotLastCell(x, y + stepY) != 0)
        {
            // 1921
            MAINOUT_1578_MoveActor(actor, 0, stepY);
        }
        else
        {
            MAINOUT_16fc_MoveRandomly(actor);
        }
    }
    else
    {
        // 1930
        if (stepY != 0 && MAINOUT_1482_CanActorEnter(actor, x, y + stepY) != 0 &&
            MAINOUT_14c8_NotLastCell(x, y + stepY) != 0)
        {
            // -> 1921
            MAINOUT_1578_MoveActor(actor, 0, stepY);
        }
        else if (stepX != 0 && MAINOUT_1482_CanActorEnter(actor, x + stepX, y) != 0 &&
                 MAINOUT_14c8_NotLastCell(x + stepX, y) != 0)
        {
            // -> 18e2
            MAINOUT_1578_MoveActor(actor, stepX, 0);
        }
        else
        {
            // 197d
            MAINOUT_16fc_MoveRandomly(actor);
        }
    }
}
#undef P2_SET

// one actor's move by kind: a whirlpool every other turn, the Shadowlord after the party (20 moves), a ship by the wind, else toward the party
static void MAINOUT_198c_ActorTurn(int actor)
{
    int tile;
    int facing;
    uint delay;

    tile = D_5c5a_actors[actor].tile;
    if ((tile & 0xfc) == TILE_ACTOR_WHIRLPOOL)
    {
        D_5c5a_actors[actor].value ^= 1;
        if (D_5c5a_actors[actor].value == 0)
        {
            return;
        }

        if (ULTIMA_2092_RandomRange(0, 1) == 0)
        {
            MAINOUT_16fc_MoveRandomly(actor);
            return;
        }
    }
    else if (tile == TILE_ACTOR_SHADOWLORD)
    {
        if (MAINOUT_14ea_InChaseRange(actor) != 0 && D_5c5a_actors[actor].value++ < 0x14)
        {
            MAINOUT_17d4_MoveTowardParty(actor, 0);
            return;
        }
    }
    else if ((tile & 0xfc) == TILE_ACTOR_PIRATE)
    {
        if (D_5892_windDir == 0)
        {
            return;
        }

        facing = D_5c5a_actors[actor].tile - TILE_ACTOR_PIRATE;

        delay = D_2bf8_sailDelay[facing][D_5892_windDir - 1];
        if (delay != 4 && delay < ++D_5c5a_actors[actor].extra)
        {
            D_5c5a_actors[actor].extra = 0;
            return;
        }
    }

    MAINOUT_17d4_MoveTowardParty(actor, 1);
}

// the monsters' turn after a step: may spawn one, each moving actor attacks or moves, those out of the view vanish; the number that attacked
int MAINOUT_1a60_MonstersTurn(void)
{
    int index;
    int acted;

    acted = 0;
    if (D_587a_statusIcon == 'T')
    {
        return 0;
    }

    if (D_587a_statusIcon == 'Q')
    {
        D_2c55_quickToggle = D_2c55_quickToggle ^ 1;
        if (D_2c55_quickToggle != 0)
        {
            return 0;
        }
    }

    if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET)
    {
        D_2c57_mountToggle = D_2c57_mountToggle ^ 1;
        if (D_2c57_mountToggle != 0)
        {
            return 0;
        }
    }

    if (MAINOUT_0d8c_SpawnChance() > ULTIMA_2092_RandomRange(1, 0x1e))
    {
        MAINOUT_0fc4_SpawnMonster();
    }

    for (index = 0x1f; 0 < index; index--)
    {
        if (MAINOUT_105c_IsMovingActor(D_5c5a_actors[index].tile) != 0)
        {
            acted += MAINOUT_131a_ActorAttack(index);
            if (acted == 0)
            {
                MAINOUT_198c_ActorTurn(index);
            }
        }
    }

    for (index = 0x1f; 0 < index; index--)
    {
        if (MAINOUT_105c_IsMovingActor(D_5c5a_actors[index].tile) != 0 &&
            ((byte)(D_5c5a_actors[index].x - D_589b_mapOriginX) > 0x1f || (byte)(D_5c5a_actors[index].y - D_589c_mapOriginY) > 0x1f))
        {
            ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, index);
        }
    }

    return acted;
}

// the trolls ask for a toll (99 - 3 x strength of the first, gold); not paid, they fight
static void MAINOUT_1b3e_TrollToll(void)
{
    int toll;
    int slot;
    byte answer;

    ULTIMA_1850_PrintString(/*0x6b2c*/ "Caught!\n\nThe trolls demand a ");
    ULTIMA_39fc_GetFirstActivePartyMember();
    toll = -(D_55a8_party[D_5876_tempX].str * 3 - 99);
    ULTIMA_1a3e_PrintNumber(toll, 2, 0x20);
    ULTIMA_1850_PrintString(/*0x6b4a*/ " gp toll!\n\nDost thou pay?");

    while ((answer = ULTIMA_266c_GetChar()) != 'Y' && answer != 'N')
    {
    }

    ULTIMA_16ba_PrintChar(answer);
    ULTIMA_16ba_PrintChar('\n');
    if (answer == 'Y')
    {
        D_57aa_gold -= toll;
        D_a9fa_vitalsDirty = 1;
        if (D_57aa_gold >= 0)
        {
            return;
        }

        D_57aa_gold += toll;
    }

    slot = ULTIMA_38e4_FindActorSlot();
    ULTIMA_3a74_SetActor(TILE_ACTOR_E4, 0, D_5896_mapX, D_5897_mapY, 0, 0, slot);
    ULTIMA_6150_Combat(slot);
}

// crossing a bridge: one in eight, each party member's dexterity tests whether the trolls under it see them
void MAINOUT_1be8_BridgeTrolls(void)
{
    int dot;
    int member_h;

    if (ULTIMA_2092_RandomRange(0, 7) == 0 && D_587c_partyTile == TILE_ACTOR_AVATAR)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_1850_PrintString(/*0x6b64*/ "\nThou spieth trolls under the bridge!\n\n");
        ULTIMA_3ae6_WaitFrames(10);

        for (member_h = 0; member_h < D_585b_partyCount; member_h++)
        {
            if (D_55a8_party[member_h].status != STATUS_DEAD && D_55a8_party[member_h].status != STATUS_SLEEP)
            {
                ULTIMA_1850_PrintString(D_55a8_party[member_h].name);
                ULTIMA_1850_PrintString(/*0x6b8c*/ " sneaks across");

                for (dot = 0; dot < 3; dot++)
                {
                    ULTIMA_3ae6_WaitFrames(5);
                    ULTIMA_16ba_PrintChar('.');
                }

                ULTIMA_1850_PrintString(/*0x6b9c*/ "\n\n");

                if (D_55a8_party[member_h].dex < ULTIMA_2092_RandomRange(1, 0x1e))
                {
                    MAINOUT_1b3e_TrollToll();
                    return;
                }
            }
        }

        ULTIMA_1850_PrintString(/*0x6ba0*/ "Trolls evaded!\n");
    }
}
