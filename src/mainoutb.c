#include "common/common.h"

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <stdlib.h>

#include "audio/aud_mus.h"

#include "blckthrn.h"
#include "cast2.h"
#include "comsubs.h"
#include "mainout.h"
#include "outsubs.h"

static u8 D_a526; // this object's _BSS (DS:a526)
static u8 D_a527;

// The part of MAINOUT.OVL built with MSC 5.0 (docs/MATCHING.md, "Two compilers").
// Where this part starts is not certain: MAINOUT_0e4e to 105c use no strings and compile
// the same with both compilers, so the original boundary may be anywhere from 0e4e to 109e.

int MAINOUT_0d8c(void);
int MAINOUT_105c(int param_1);
void MAINOUT_0000(void);
void MAINOUT_0fc4(void);

u8 D_2bc0[0xc] = { 0xc0, 0xc8, 0x90, 0x98, 0xbc, 0xc4, 0xd0, 0xe4, 0xcc, 0xd4, 0xdc, 0xd8 };

u8 D_2bcc[0x8] = { 0x94, 0x90, 0x98, 0xf0, 0xf4, 0xd8, 0xdc, 0x00 };

u8 D_2bd4[0x6] = { 0x8c, 0x84, 0x88, 0x80, 0x2c, 0x00 };

u8 D_2bda[0x2] = { 0x84, 0x88 };

u8 D_2bdc[0xc] = { 0x3c, 0x32, 0x28, 0x1e, 0x14, 0x0f, 0x0f, 0x0a, 0x0a, 0x03, 0x02, 0x01 };

u8 D_2be8[0x8] = { 0x40, 0x38, 0x38, 0x20, 0x20, 0x08, 0x08, 0x00 };

u8 D_2bf0[0x6] = { 0x48, 0x48, 0x28, 0x26, 0x22, 0x00 };

u8 D_2bf6[0x2] = { 0x80, 0x80 };

u16 D_2bf8[4][4] = { 2, 3, 4, 4, 4, 4, 2, 3, 3, 2, 4, 4, 4, 4, 3, 2 };

u8 D_2c18[0x3d] =
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
u8 D_2c55 = 0;

static u8 D_2c56 = 0; // unused
u8 D_2c57 = 0;

// CHECKED
void MAINOUT_109e(void)
{
    int local_6_an;
    int local_4;

    if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
    {
        local_4 = ULTIMA_2092_RandomRange(1, 0x1e);
        if (local_4 < D_5c5a_actors[0].value)
        {
            D_5c5a_actors[0].value -= local_4;
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x6ada, "Ship sunk!\n"));
            local_6_an = D_5c5a_actors[0].extra;
            if (local_6_an > 0 || D_57b0_carpets != 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x6ae6, "Abandon ship!\n"));
                if (local_6_an > 0)
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
                ULTIMA_1850_PrintString(_TEXT(0x6af6, "DROWNING!!!\n"));

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

static void MAINOUT_1168(int param_1, int param_2, int param_3)
{
    byte local_8;
    byte local_6;
    int local_4_ab;

    if (param_2 == 0)
    {
        if (D_5c5a_actors[param_1].animTile == TILE_ACTOR_PIRATE || D_5c5a_actors[param_1].animTile == TILE_ACTOR_2E)
        {
            D_5c5a_actors[param_1].animTile = (ULTIMA_2092_RandomRange(0, 3) & 2) + TILE_ACTOR_2D;
        }
    }

    if (param_3 == 0)
    {
        if (D_5c5a_actors[param_1].animTile == TILE_ACTOR_2D || D_5c5a_actors[param_1].animTile == TILE_ACTOR_2F)
        {
            D_5c5a_actors[param_1].animTile = (ULTIMA_2092_RandomRange(0, 3) & 2) + TILE_ACTOR_PIRATE;
        }
    }

    ULTIMA_5910_UpdateFrame();
    ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);

    local_6 = D_5c5a_actors[param_1].x - D_5896_mapX + 5;
    local_8 = D_5c5a_actors[param_1].y - D_5897_mapY + 5;
    local_4_ab = COMSUBS_12de_FlyMissile(local_6, local_8, 5, 5, 1);
    if (local_4_ab != 0)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
        MAINOUT_109e();
    }
}

static void MAINOUT_1248(int param_1)
{
    int local_4;

    if ((D_5c5a_actors[param_1].tile & 0xfc) == TILE_ACTOR_WHIRLPOOL)
    {
        if (D_587c_partyTile == TILE_ACTOR_AVATAR)
        {
            MAINOUT_109e();
            return;
        }
        else
        {
            D_5c5a_actors[param_1].tile = D_5c5a_actors[param_1].animTile = 0;
            ULTIMA_1850_PrintString(_TEXT(0x6b04, "\nWHIRLPOOL!\n"));

            local_4 = D_587c_partyTile;
            D_587c_partyTile = TILE_ACTOR_WHIRLPOOL;
            ULTIMA_5910_UpdateFrame();
            ULTIMA_43ae_AudioSweepTone(0x294, 0x96, 0x28, 0x1e78);

            D_587c_partyTile = local_4;
            MAINOUT_109e();

#if defined(ENABLE_BGM)
            AUDIO_StopBgm();
#endif

            D_5895_mapLevel = 0xff;
            D_5896_mapX = 0x22;
            D_5897_mapY = 0x12;
            MAINOUT_0000();

#if defined(ENABLE_BGM)
            AUDIO_PlayBgmPerMap();
#endif
        }
    }
    else if ((D_5c5a_actors[param_1].tile & 0xfc) != TILE_ACTOR_E0)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_1850_PrintString(_TEXT(0x6b12, "\nAttacked!\n"));

        if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) < TILE_MAP_POISON &&
            ((D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET || (D_587c_partyTile & 0xfc) == TILE_ACTOR_SKIFF))
        {
            MAINOUT_109e();
            return;
        }
        else
        {
            ULTIMA_6150_Combat(param_1);
        }
    }
    else
    {
        MAINOUT_109e();
        return;
    }
}

static int MAINOUT_131a(int param_1)
{
    byte local_e_am;
    int local_c;
    byte local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_c = D_5c5a_actors[param_1].tile;
    local_6 = abs(D_5c5a_actors[param_1].x - D_5896_mapX);
    if (local_6 > 0x7f)
    {
        local_6 = 0x100 - local_6;
    }

    local_8 = abs(D_5c5a_actors[param_1].y - D_5897_mapY);
    if (local_8 > 0x7f)
    {
        local_8 = 0x100 - local_8;
    }

    if ((local_6 == 1 && local_8 == 0) || (local_6 == 0 && local_8 == 1))
    {
        MAINOUT_1248(param_1);
        return 1;
    }

    if (local_c == TILE_ACTOR_88 || local_c == TILE_ACTOR_DC)
    {
        if (local_6 <= 3 && local_8 <= 3 && ULTIMA_2092_RandomRange(0, 7) == 0)
        {
            ULTIMA_5910_UpdateFrame();
            ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);

            local_a = D_5c5a_actors[param_1].x - D_5896_mapX + 5;
            local_e_am = D_5c5a_actors[param_1].y - D_5897_mapY + 5;

            local_4_ab = COMSUBS_12de_FlyMissile(local_a, local_e_am, 5, 5, 3);
            if (local_4_ab != 0)
            {
                ULTIMA_3522_HitEffectAt(D_5896_mapX, D_5897_mapY);
                MAINOUT_109e();
            }

            return 1;
        }
    }
    else if ((local_c & 0xfc) == TILE_ACTOR_PIRATE && ((local_6 == 0 && local_8 < 4) || (local_8 == 0 && local_6 < 4)))
    {
        ULTIMA_1850_PrintString(_TEXT(0x6b1e, "* BOOOM! *\n\n"));
        MAINOUT_1168(param_1, local_6, local_8);
        return 1;
    }

    return 0;
}

static int MAINOUT_1482(int param_1, int param_2, int param_3)
{
    if (ULTIMA_2c4c_IsWalkableTile(D_5c5a_actors[param_1].tile, *ULTIMA_4402_GetTileAddr(param_2, param_3)) == 0)
        return 0;

    if (ULTIMA_3702_FindLastActorTileAt(param_2, param_3, D_5895_mapLevel) != 0)
        return 0;

    return 1;
}

static int MAINOUT_14c8(int param_1, int param_2)
{
    if (param_1 == D_a526 && param_2 == D_a527)
    {
        return 0;
    }

    return 1;
}

static int MAINOUT_14ea(int param_1)
{
    int local_6_an;
    int local_4;

    local_4 = abs(D_5c5a_actors[param_1].x - D_5896_mapX);
    if (local_4 > 0x7f)
    {
        local_4 = 0x100 - local_4;
    }

    local_6_an = abs(D_5c5a_actors[param_1].y - D_5897_mapY);
    if (local_6_an > 0x7f)
    {
        local_6_an = 0x100 - local_6_an;
    }

    if (local_4 < 6 && local_6_an < 6)
    {
        return D_2c18[local_4 + local_6_an * 0xb];
    }

    return 0;
}

static void MAINOUT_1578(int param_1, int param_2, int param_3)
{
    int local_6;
    int local_c;
    int local_a;
    int local_8;
    int local_4_ab;

    local_6 = D_5c5a_actors[param_1].tile;
    local_8 = D_5c5a_actors[param_1].x + param_2;
    local_a = D_5c5a_actors[param_1].y + param_3;
    local_c = *ULTIMA_4402_GetTileAddr(local_8, local_a);
    if ((local_6 & 0xfc) == TILE_ACTOR_PIRATE)
    {
        if (param_2 == 0 && param_3 == -1)
        {
            local_4_ab = 0;
        }
        else if (param_2 == 1 && param_3 == 0)
        {
            local_4_ab = 1;
        }
        else if (param_2 == 0 && param_3 == 1)
        {
            local_4_ab = 2;
        }
        else if (param_2 == -1 && param_3 == 0)
        {
            local_4_ab = 3;
        }

        D_5c5a_actors[param_1].tile = D_5c5a_actors[param_1].animTile = local_4_ab + TILE_ACTOR_PIRATE;
    }
    else if (local_6 != TILE_ACTOR_DC && local_6 != TILE_ACTOR_94 && local_6 != TILE_ACTOR_D8 && local_6 != TILE_ACTOR_F0)
    {
        switch (local_c)
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

    D_a526 = D_5c5a_actors[param_1].x;
    D_a527 = D_5c5a_actors[param_1].y;

    D_5c5a_actors[param_1].x = local_8;
    D_5c5a_actors[param_1].y = local_a;
    D_24e6_viewDirty |= 2;

    if (*ULTIMA_4402_GetTileAddr(local_8, local_a) == TILE_MAP_MOONGATE)
    {
        D_5c5a_actors[param_1].tile = D_5c5a_actors[param_1].animTile = 0;
    }
}

static void MAINOUT_16fc(int param_1)
{
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = D_5c5a_actors[param_1].x;
    local_8 = D_5c5a_actors[param_1].y;

    local_4_ab = 0;
    local_a = local_6 - 1;
    local_c = local_8 + 1;
    local_e_am = local_6 + 1;
    local_10_aa = local_8 - 1;

    // bug?
    for (; local_4_ab < 3; local_4_ab++)
    {
        switch (ULTIMA_2092_RandomRange(0, 3))
        {
        case 0:
            // 1760
            if (MAINOUT_1482(param_1, local_6, local_10_aa) != 0)
            {
                MAINOUT_1578(param_1, 0, -1);
            }
            return;

        case 1:
            // 177c
            if (MAINOUT_1482(param_1, local_e_am, local_8) != 0)
            {
                MAINOUT_1578(param_1, 1, 0);
            }
            return;

        case 2:
            // 1790
            if (MAINOUT_1482(param_1, local_6, local_c) != 0)
            {
                MAINOUT_1578(param_1, 0, 1);
            }
            return;

        case 3:
            // 17a8
            if (MAINOUT_1482(param_1, local_a, local_8) != 0)
            {
                MAINOUT_1578(param_1, -1, 0);
            }
            return;
        }
    }
}

// The original stores the constants to param_2 through AX ("mov ax,1; mov [bp+4],ax"), MSC's
// form for a chained assignment (see SHOPPES2_00ac_PrintHonorific): param_2 = param_2 = 1 gives it.
#if defined(TARGET_DOS16)
#define P2_SET(v) param_2 = param_2 = (v) // two stores between sequence points: DOS build only
#else
#define P2_SET(v) param_2 = (v)
#endif

// CHECKED
// MSC places a block that nothing falls into after the first unconditional jump that follows
// it in the source. The original has the two "if (x > 0) param_2 = 0;" blocks after the
// local_e_am == 0 branch's jump and after the first MAINOUT_1578 call's, jumping back to the
// next test: they were reached only by a goto from the else, and sat (here in an if (0)) in
// the source after the code that ends with those jumps. Written in place, as an else-if, MSC
// keeps them inline and the "x < 0" blocks jump over them. The end is an if/else chain with
// MAINOUT_16fc in each last else, not early returns: the second MAINOUT_1578 call ends a
// then-branch, so tail merging keeps its copy and the first one jumps forward into it.
static void MAINOUT_17d4(int param_1, int param_2)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = D_5c5a_actors[param_1].x;
    local_c = D_5c5a_actors[param_1].y;
    local_a = (D_5c5a_actors[param_1].x - D_5896_mapX) & 0xff;
    if (local_a > 0x7f)
    {
        local_a -= 0x100;
    }

    local_e_am = (D_5c5a_actors[param_1].y - D_5897_mapY) & 0xff;
    if (local_e_am > 0x7f)
    {
        local_e_am -= 0x100;
    }

    // 1828
    if (local_a == 0)
    {
        local_4_ab = 0;
    }
    else if (local_a > 0) // 1836
    {
        P2_SET(1);
        local_4_ab = -1;
        // -> 1864
    }
    else
    {
        if (local_a < 0) // 184a
        {
            P2_SET(0);
        }

        if (local_a < 0) // 1855
        {
            local_4_ab = param_2 = 1;
        }
        else
        {
            goto L_1872;
        }
    }

L_1864:
    if (local_e_am == 0) // 1864
    {
        local_6 = 0;

        if (0)
        {
L_1872:
            if (local_a > 0)
            {
                P2_SET(0);
            }
            goto L_1864;
        }
    }
    else if (local_e_am > 0) // 1880
    {
        P2_SET(1);
        local_6 = -1;
    }
    else
    {
        if (local_e_am < 0) // 1894
        {
            P2_SET(0);
        }

        if (local_e_am < 0) // 189f
        {
            local_6 = param_2 = 1;
        }
        else
        {
            goto L_18ee;
        }
    }

    if (0)
    {
L_18ee:
        if (local_e_am > 0)
        {
            P2_SET(0);
        }
        goto L_18ae;
    }

L_18ae:
    if (ULTIMA_2092_RandomRange(0, 1) == 1)
    {
        // 18bd
        if (local_4_ab != 0 && MAINOUT_1482(param_1, local_8 + local_4_ab, local_c) != 0 &&
            MAINOUT_14c8(local_8 + local_4_ab, local_c) != 0)
        {
            // 18e2: -> 192a
            MAINOUT_1578(param_1, local_4_ab, 0);
        }
        else if (local_6 != 0 && MAINOUT_1482(param_1, local_8, local_c + local_6) != 0 &&
                 MAINOUT_14c8(local_8, local_c + local_6) != 0)
        {
            // 1921
            MAINOUT_1578(param_1, 0, local_6);
        }
        else
        {
            MAINOUT_16fc(param_1);
        }
    }
    else
    {
        // 1930
        if (local_6 != 0 && MAINOUT_1482(param_1, local_8, local_c + local_6) != 0 &&
            MAINOUT_14c8(local_8, local_c + local_6) != 0)
        {
            // -> 1921
            MAINOUT_1578(param_1, 0, local_6);
        }
        else if (local_4_ab != 0 && MAINOUT_1482(param_1, local_8 + local_4_ab, local_c) != 0 &&
                 MAINOUT_14c8(local_8 + local_4_ab, local_c) != 0)
        {
            // -> 18e2
            MAINOUT_1578(param_1, local_4_ab, 0);
        }
        else
        {
            // 197d
            MAINOUT_16fc(param_1);
        }
    }
}
#undef P2_SET

static void MAINOUT_198c(int param_1)
{
    int local_8;
    int local_6;
    uint local_4_ab;

    local_8 = D_5c5a_actors[param_1].tile;
    if ((local_8 & 0xfc) == TILE_ACTOR_WHIRLPOOL)
    {
        D_5c5a_actors[param_1].value ^= 1;
        if (D_5c5a_actors[param_1].value == 0)
        {
            return;
        }

        if (ULTIMA_2092_RandomRange(0, 1) == 0)
        {
            MAINOUT_16fc(param_1);
            return;
        }
    }
    else if (local_8 == TILE_ACTOR_SHADOWLORD)
    {
        if (MAINOUT_14ea(param_1) != 0 && D_5c5a_actors[param_1].value++ < 0x14)
        {
            MAINOUT_17d4(param_1, 0);
            return;
        }
    }
    else if ((local_8 & 0xfc) == TILE_ACTOR_PIRATE)
    {
        if (D_5892_windDir == 0)
        {
            return;
        }

        local_6 = D_5c5a_actors[param_1].tile - TILE_ACTOR_PIRATE;

        local_4_ab = D_2bf8[local_6][D_5892_windDir - 1];
        if (local_4_ab != 4 && local_4_ab < ++D_5c5a_actors[param_1].extra)
        {
            D_5c5a_actors[param_1].extra = 0;
            return;
        }
    }

    MAINOUT_17d4(param_1, 1);
}

int MAINOUT_1a60(void)
{
    int local_6_an;
    int local_4;

    local_4 = 0;
    if (D_587a_statusIcon == 'T')
    {
        return 0;
    }

    if (D_587a_statusIcon == 'Q')
    {
        D_2c55 = D_2c55 ^ 1;
        if (D_2c55 != 0)
        {
            return 0;
        }
    }

    if ((D_587c_partyTile & 0xfe) == TILE_ACTOR_RIDING_HORSE || (D_587c_partyTile & 0xfe) == TILE_ACTOR_FLYING_CARPET)
    {
        D_2c57 = D_2c57 ^ 1;
        if (D_2c57 != 0)
        {
            return 0;
        }
    }

    if (MAINOUT_0d8c() > ULTIMA_2092_RandomRange(1, 0x1e))
    {
        MAINOUT_0fc4();
    }

    for (local_6_an = 0x1f; 0 < local_6_an; local_6_an--)
    {
        if (MAINOUT_105c(D_5c5a_actors[local_6_an].tile) != 0)
        {
            local_4 += MAINOUT_131a(local_6_an);
            if (local_4 == 0)
            {
                MAINOUT_198c(local_6_an);
            }
        }
    }

    for (local_6_an = 0x1f; 0 < local_6_an; local_6_an--)
    {
        if (MAINOUT_105c(D_5c5a_actors[local_6_an].tile) != 0 &&
            ((byte)(D_5c5a_actors[local_6_an].x - D_589b_mapOriginX) > 0x1f || (byte)(D_5c5a_actors[local_6_an].y - D_589c_mapOriginY) > 0x1f))
        {
            ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, local_6_an);
        }
    }

    return local_4;
}

static void MAINOUT_1b3e(void)
{
    int local_8;
    int local_6;
    byte local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x6b2c, "Caught!\n\nThe trolls demand a "));
    ULTIMA_39fc_GetFirstActivePartyMember();
    local_8 = -(D_55a8_party[D_5876_tempX].str * 3 - 99);
    ULTIMA_1a3e_PrintNumber(local_8, 2, 0x20);
    ULTIMA_1850_PrintString(_TEXT(0x6b4a, " gp toll!\n\nDost thou pay?"));

    while ((local_4_ab = ULTIMA_266c_GetChar()) != 'Y' && local_4_ab != 'N')
    {
    }

    ULTIMA_16ba_PrintChar(local_4_ab);
    ULTIMA_16ba_PrintChar('\n');
    if (local_4_ab == 'Y')
    {
        D_57aa_gold -= local_8;
        D_a9fa_vitalsDirty = 1;
        if (D_57aa_gold >= 0)
        {
            return;
        }

        D_57aa_gold += local_8;
    }

    local_6 = ULTIMA_38e4_FindActorSlot();
    ULTIMA_3a74_SetActor(TILE_ACTOR_E4, 0, D_5896_mapX, D_5897_mapY, 0, 0, local_6);
    ULTIMA_6150_Combat(local_6);
}

void MAINOUT_1be8(void)
{
    int local_6_an;
    int local_4;

    if (ULTIMA_2092_RandomRange(0, 7) == 0 && D_587c_partyTile == TILE_ACTOR_AVATAR)
    {
        ULTIMA_5910_UpdateFrame();
        ULTIMA_1850_PrintString(_TEXT(0x6b64, "\nThou spieth trolls under the bridge!\n\n"));
        ULTIMA_3ae6_WaitFrames(10);

        for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
        {
            if (D_55a8_party[local_4].status != STATUS_DEAD && D_55a8_party[local_4].status != STATUS_SLEEP)
            {
                ULTIMA_1850_PrintString(D_55a8_party[local_4].name);
                ULTIMA_1850_PrintString(_TEXT(0x6b8c, " sneaks across"));

                for (local_6_an = 0; local_6_an < 3; local_6_an++)
                {
                    ULTIMA_3ae6_WaitFrames(5);
                    ULTIMA_16ba_PrintChar('.');
                }

                ULTIMA_1850_PrintString(_TEXT(0x6b9c, "\n\n"));

                if (D_55a8_party[local_4].dex < ULTIMA_2092_RandomRange(1, 0x1e))
                {
                    MAINOUT_1b3e();
                    return;
                }
            }
        }

        ULTIMA_1850_PrintString(_TEXT(0x6ba0, "Trolls evaded!\n"));
    }
}
