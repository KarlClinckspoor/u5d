#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "cast2.h"
#include "cmds.h"
#include "combat.h"
#include "comsubs.h"
#include "dnglook.h"
#include "npc.h"
#include "outsubs.h"
#include "town.h"

void MainoutMoveParty(int stepX, int stepY);
int IsMovingActor(int tile);

// CMDS.OVL's MSC 5.1 code is two objects: cmds.c (HoleUpRest-YellShadowlord) and this one, from
// RestoreShrine (its _DATA starts at DS:4450, an even address, after shadowlordNames). Compiled after
// the first part's functions, MSC 5.1 drops loop optimizations in YellWordOfPower (its loop
// registers); as an object of its own it gives the original's code.

// defined in cmds.c
int YellShadowlord(char* word);

// At ruins: asks the virtue and three mantras; right answers at the shrine's place restore the destroyed
// shrine.
static void RestoreShrine(int virtue, int x, int y)
{
    bool right;
    int i;
    char spoken[16];

    right = 1;

    PrintString(/*0x4450*/ "\nUpon what virtue\ndost thou\nmeditate?\n\n:");
    GetString(spoken, 0xf);

    if (CompareWord((byte*)virtueNames[virtue], spoken) == -1)
    {
        right = 0;
    }

    for (i = 0; i < 3; i++)
    {
        PrintString(/*0x4479*/ "\nMantra:");
        GetString(spoken, 0xf);

        if (CompareWord((byte*)mantras[virtue], spoken) == -1)
        {
            right = 0;
        }
    }

    if (right && shrineX[virtue] == x && shrineY[virtue] == y)
    {
        shrineDestroyed[virtue] &= 0x7f;
        PrintString(/*0x4482*/ "\n\nThe Shrine is\nrestored!\n");
        ShakeScreen();
        *GetTileAddr(x, y) = TILE_MAP_SHRINE;
        viewDirty |= 2;
    }
    else
    {
        PrintChar('\n');
    }
}

char* vileWords[8] =
{
    /*0x449d*/ "FALLAX",
    /*0x44a4*/ "VILIS",
    /*0x44aa*/ "INOPIA",
    /*0x44b1*/ "MALUM",
    /*0x44b7*/ "AVIDUS",
    /*0x44be*/ "INFAMA",
    /*0x44c5*/ "IGNAVUS",
    /*0x44cd*/ "VERAMOCOR"
};

u8 wordTile[8] = {0x18, 0x16, 0x16, 0x18, 0x18, 0x17, 0x17, 0x16};

// An overworld word of power (FALLAX ... VERAMOCOR, one per virtue) opens or closes the shrine or dungeon
// entrance beside the party, or goes to the shrine restoring at ruins; else "No effect!".
// CHECKED
// local_4_ab and local_a are stack locals that the loop optimizer keeps in SI and DI.
static void YellWordOfPower(char* word)
{
    char** wordPtr_m;
    byte tile;
    int dy; // <- di
    bool found;
    int dx;
    int virtue_b; // <- si

    found = 0;

    for (virtue_b = 0, wordPtr_m = vileWords; virtue_b < 8; virtue_b++)
    {
        if (CompareWord((byte*)*wordPtr_m, word) > -1)
        {
            PrintString(/*0x44d7*/ "\nA word of power is uttered\n");
            ShakeScreen();

            tile = GetMapViewport(4, 5);
            if (wordTile[virtue_b] == tile || tile == TILE_MAP_DF || tile == TILE_MAP_RUINS)
            {
                dx = -1;
                dy = 0;
            }
            else
            {
                tile = GetMapViewport(5, 6);
                if (wordTile[virtue_b] == tile || tile == TILE_MAP_DF || tile == TILE_MAP_RUINS)
                {
                    dx = 0;
                    dy = 1;
                }
                else
                {
                    tile = GetMapViewport(6, 5);
                    if (wordTile[virtue_b] == tile || tile == TILE_MAP_DF || tile == TILE_MAP_RUINS)
                    {
                        dx = 1;
                        dy = 0;
                    }
                    else
                    {
                        tile = GetMapViewport(5, 4);
                        if (wordTile[virtue_b] == tile || tile == TILE_MAP_DF || tile == TILE_MAP_RUINS)
                        {
                            dx = 0;
                            dy = -1;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            if (tile == TILE_MAP_RUINS)
            {
                RestoreShrine(virtue_b, dx + D_5896_mapX, dy + D_5897_mapY);
                return;
            }

            if (dx + D_5896_mapX == placeX[virtue_b + 0x20] &&
                dy + D_5897_mapY == placeY[virtue_b + 0x20])
            {
                dungeonOpen[virtue_b] ^= 0x80;
                *GetTileAddr(dx + D_5896_mapX, dy + D_5897_mapY) ^= wordTile[virtue_b] ^ TILE_MAP_DF;
                viewDirty |= 2;
            }

            found = 1;
            break;
        }
        wordPtr_m++;
    }

    if (!found)
    {
        PrintString(/*0x44f4*/ "\nNo effect!\n");
    }
}

// The Yell command: on a frigate "FURL!" and "HOIST!" (the sails); else asks a word: a shadowlord's name, or
// a word of power on the overworld.
int YellCmd(void)
{
    int result;
    char word[32];

    result = 1;

    if ((partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20 && mapId < 0x80)
    {
        if ((partyTile & 0xfc) == TILE_ACTOR_FRIGATE_20)
        {
            PrintString(/*0x451a*/ "FURL!\n");
            partyTile += 4;
            return; // sic
        }
        else
        {
            PrintString(/*0x4521*/ "HOIST!\n");
            partyTile -= 4;
            return; // sic
        }
    }
    else
    {
        PrintString(/*0x4529*/ "what?\n:");
        GetString(word, 0x1e);
        if (word[0] == 0)
        {
            PrintString(/*0x4531*/ "Nothing\n");
            return; // sic
        }
        else
        {
            PrintChar('\n');
            if (mapId >= 1 && mapId <= 0x20)
            {
                result = YellShadowlord(word);
            }
            else if (mapId == 0)
            {
                YellWordOfPower(word);
            }
            else
            {
                PrintString(/*0x453a*/ "\nNo effect!\n");
            }
        }
    }

    return result;
}

// 1 for the tiles that can be pushed or pulled: chairs, desk, barrel, vanity, dresser, trunk, cannons and a
// few others.
static int IsPushable(byte tile)
{
    switch (tile)
    {
    case TILE_MAP_5B:
    case TILE_MAP_CHAIR_90:
    case TILE_MAP_CHAIR_91:
    case TILE_MAP_CHAIR_92:
    case TILE_MAP_CHAIR_93:
    case TILE_MAP_DESK:
    case TILE_MAP_BARREL:
    case TILE_MAP_VANITY:
    case TILE_MAP_A9:
    case TILE_MAP_DRESSER:
    case TILE_MAP_AE:
    case TILE_MAP_TRUNK:
    case TILE_MAP_CANNON_B4:
    case TILE_MAP_CANNON_B5:
    case TILE_MAP_CANNON_B6:
    case TILE_MAP_CANNON_B7:
        return 1;
    }

    return 0;
}

// The chair or cannon tile turned for a push (or, with reverse, a pull) in the direction (dx, dy).
static int FacingTile(int dx, int dy, int tile, int reverse)
{
    if (dx == 1 && dy == 0)
    {
        tile++;
    }
    if (dx == 0 && dy == 1)
    {
        tile += 2;
    }
    if (dx == -1 && dy == 0)
    {
        tile += 3;
    }
    if (reverse != 0)
    {
        tile ^= 2;
    }

    return tile;
}

// Puts a pushed object on its new cell and the tile that lay there on the old one, turning chairs and
// cannons; "Pushed!".
static void PutPushed(int objTile, int beyondTile, int objX, int objY, int beyondX, int beyondY, int dx,
                 int dy)
{
    PrintString(/*0x4547*/ "Pushed!\n");
    *GetTileAddr(beyondX, beyondY) = objTile;
    *GetTileAddr(objX, objY) = beyondTile;
    objTile &= 0xfc;
    if (objTile == TILE_MAP_CHAIR_90 || objTile == TILE_MAP_CANNON_B4)
    {
        *GetTileAddr(beyondX, beyondY) = FacingTile(dx, dy, objTile, 0);
    }
}

// The same for a pulled object (it takes the party's cell and the floor under the party goes to the old
// one); "Pulled!".
static void PutPulled(int floorTile, int objTile, int partyX, int partyY, int objX, int objY, int dx,
                 int dy)
{
    PrintString(/*0x4550*/ "Pulled!\n");
    *GetTileAddr(partyX, partyY) = objTile;
    *GetTileAddr(objX, objY) = floorTile;
    objTile &= 0xfc;
    if (objTile == TILE_MAP_CHAIR_90 || objTile == TILE_MAP_CANNON_B4)
    {
        *GetTileAddr(partyX, partyY) = FacingTile(dx, dy, objTile, 1);
    }
}

// The Push command (also in combat): an object in the chosen direction moves on when the cell beyond is
// free, else it is pulled to the party when the party stands on free floor; "Won't budge!" otherwise.
void PushCmd(void)
{
    int floorUnder_p;
    int beyondY_f;
    int dy_c;
    int objY_l;
    int beyondX_g;
    int dx_d;
    int objX_m;
    int savedY_d;
    int savedX_e;
    int emptyTile_c;
    int entity_c;
    int beyondTile;
    int objTile;

    SetTile(openDoorTile, openDoorX, openDoorY);
    if (SelectDirection() == 0)
    {
        return;
    }

    dx_d = tempX;
    dy_c = tempY;

    if (mapId > 0x7f)
    {
        savedX_e = D_5896_mapX;
        savedY_d = D_5897_mapY;
        D_5896_mapX = combatants[curCombatant].x;
        D_5897_mapY = combatants[curCombatant].y;
    }

    objX_m = dx_d + (uint)D_5896_mapX;
    objY_l = dy_c + (uint)D_5897_mapY;

    if (mapId > 0x7f && FireMapTrigger(objX_m, objY_l) != 0)
    {
        return;
    }

    objTile = *GetTileAddr(objX_m, objY_l);
    if (FindActorTileAtPos(objX_m, objY_l, D_5895_mapLevel) != 0 || IsPushable(objTile) == 0)
    {
        PrintString(/*0x4559*/ "Won't budge!\n");
    }
    else
    {
        emptyTile_c = (objTile & 0xfc) == TILE_MAP_CANNON_B4 ? TILE_MAP_45 : TILE_MAP_44;
        beyondX_g = objX_m + dx_d;
        beyondY_f = objY_l + dy_c;
        beyondTile = *GetTileAddr(beyondX_g, beyondY_f);
        floorUnder_p = *GetTileAddr(D_5896_mapX, D_5897_mapY);
        if (FindActorTileAtPos(beyondX_g, beyondY_f, D_5895_mapLevel) == 0 && beyondTile == emptyTile_c)
        {
            PutPushed(objTile, beyondTile, objX_m, objY_l, beyondX_g, beyondY_f, dx_d, dy_c);
        }
        else
        {
            if (floorUnder_p == emptyTile_c)
            {
                PutPulled(floorUnder_p, objTile, D_5896_mapX, D_5897_mapY, objX_m, objY_l, dx_d, dy_c);
            }
            else
            {
                PrintString(/*0x4567*/ "Won't budge\n");
                return;
            }
        }

        D_5896_mapX += dx_d;
        D_5897_mapY += dy_c;
        viewDirty = 1;

        if (mapId > 0x7f)
        {
            entity_c = combatants[curCombatant].entityIdx;

            combatants[curCombatant].x += dx_d;
            combatants[curCombatant].y += dy_c;

            actors[entity_c].x += dx_d;
            actors[entity_c].y += dy_c;

            D_5896_mapX = savedX_e;
            D_5897_mapY = savedY_d;

            UpdateFrame();
        }
    }
}

// The Escape command in combat: refused while players are alive and the fight is not yet over or cannot be
// left; else all combatants and actors are removed; 1 when refused.
int Escape(void)
{
    int i;
    int refused_h;
    bool anyPlayer;

    PrintString(/*0x4574*/ "Escape");

    anyPlayer = refused_h = 0;
    for (i = 0; i < 0x20; i++)
    {
        if ((combatants[i].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_DEAD)) == COMBAT_FLAGS_PLAYER)
        {
            anyPlayer = 1;
            break;
        }
    }

    if (anyPlayer)
    {
        if ((combatFlags & 0x80) != 0)
        {
            PrintString(/*0x457b*/ "-Not here!\n");
            refused_h = 1;
        }
        else if (combatOver == 0)
        {
            PrintString(/*0x4587*/ "-Not yet!\n");
            refused_h = 1;
        }
    }

    if (refused_h == 0)
    {
        PrintChar('!');

        for (i = 0; i < 0x20; i++)
        {
            if (combatants[i].flags != 0)
            {
                RemoveEntity(-i - 1);
                UpdateFrame();
            }
        }

        for (i = 0; i < 0x20; i++)
        {
            if (actors[i].tile != 0)
            {
                RemoveEntity(i + 1);
                UpdateFrame();
            }
        }

        AudioSweepTone(0x4b0, 2000, 1, 0x28);
    }

    vitalsDirty = 1;
    return refused_h;
}

// CMDS_MIX.C?
