#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "cast.h"
#include "cast2.h"
#include "combat.h"
#include "comsubs.h"
#include "dnglook.h"
#include "dungeon.h"
#include "lookobj.h"
#include "town.h"
#include "zstats.h"
#include "time/time.h"

void MAINOUT_0000(void);

u8 D_4592[4] = {0x35, 0x33, 0x34, 0x36};

u8 D_4596[4] = {0x82, 0x81, 0x80, 0x83};



static int CAST_0000(int param_1)
{
    int local_4 = D_ba14[param_1].entityIdx;
    if (local_4 == 47 || local_4 == 0xe || local_4 == 0xf)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void CAST_0032(int param_1)
{
    D_589d = param_1;
    COMSUBS_0c52(D_589e, param_1);
}

static int CAST_004c(int param_1)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    if (D_5893_map_id < 0x80)
    {
        CAST2_0000_CastEffect(param_1 == 3 ? 4 : 3);
        local_4_ab = (D_24d6[D_6603] + D_5896_map_x) & 7;
        local_6 = (D_24de[D_6603] + D_5897_map_y) & 7;

        local_a = GetDungeonMap(local_4_ab, local_6, D_5895_map_level);
        if ((local_a & 0xf7) != 0)
        {
            local_8 = 0;
        }
        else
        {
            GetDungeonMap(local_4_ab, local_6, D_5895_map_level) = (local_a & 8) | D_4596[param_1];
            local_8 = -1;
        }
    }
    else
    {
        D_589d = D_4592[param_1];
        COMSUBS_0c52(D_589e, D_589d);
        local_8 = -1;
    }

    return local_8;
}

static int CAST_0114_AnZu(void)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = CAST2_009e_OnWho();
    if (local_8 < 0)
    {
        local_4_ab = -1;
    }
    else if (D_55a8_party[local_8].status == STATUS_SLEEP)
    {
        D_55a8_party[local_8].status = STATUS_GOOD;
        if (D_5893_map_id > 0x7f)
        {
            for (local_6 = 0; local_6 < 0x20; local_6++)
            {
                if ((D_ba14[local_6].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) ==
                    (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_ASLEEP) &&
                    D_ba14[local_6].entityIdx == local_8)
                {
                    ULTIMA_6800(local_6);
                    break;
                }
            }

            if (local_6 == 0x20)
            {
                local_4_ab = 0;
            }
        }

        CAST2_0000_CastEffect(1);
        local_4_ab = -1;
        D_a9fa = 1;
    }
    else
    {
        local_4_ab = 0;
    }

    return local_4_ab;
}

static int CAST_01ae_AnNox(void)
{
    int local_6_an;
    int local_4;

    local_6_an = CAST2_009e_OnWho();
    if (local_6_an < 0)
    {
        local_4 = -1;
    }
    else
    {
        if (D_55a8_party[local_6_an].status == STATUS_POISONED)
        {
            D_55a8_party[local_6_an].status = STATUS_GOOD;
            CAST2_0000_CastEffect(1);
            D_a9fa = 1;
            local_4 = 1;
        }
        else
        {
            local_4 = 0;
        }
    }

    return local_4;
}

static int CAST_01fa_Mani(void)
{
    int local_6_an;
    int local_4;

    local_6_an = CAST2_009e_OnWho();
    if (local_6_an < 0)
    {
        local_4 = -1;
    }
    else
    {
        local_4 = CAST2_03c2(local_6_an);
    }

    if (local_4 != 0)
    {
        CAST2_0000_CastEffect(1);
    }

    return local_4;
}

static int CAST_0230_AnYlem(void)
{
    int local_6_an;
    byte* local_4;

    if (CAST2_0306() == 0)
    {
        return -1;
    }

    CAST2_0000_CastEffect(1);
    local_4 = ULTIMA_4402_GetTileAddr(D_5876, D_5878);
    switch (*local_4)
    {
    case TILE_MAP_5B:
    case TILE_MAP_CHAIR_90:
    case TILE_MAP_CHAIR_91:
    case TILE_MAP_CHAIR_92:
    case TILE_MAP_CHAIR_93:
    case TILE_MAP_MIRROR:
    case TILE_MAP_DESK:
    case TILE_MAP_BARREL:
    case TILE_MAP_VANITY:
    case TILE_MAP_A9:
    case TILE_MAP_DRESSER:
    case TILE_MAP_AE:
    case TILE_MAP_TRUNK:
        *local_4 = TILE_MAP_44;
        ULTIMA_1850_PrintString(_TEXT(0x459a, "POOF!\n"));
        D_24e6 |= 2;
        ULTIMA_5910_UpdateFrame();
        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
        local_6_an = -1;
        break;

    default:
        local_6_an = 0;
        break;
    }

    return local_6_an;
}

static int CAST_02d2_AnSanct(void)
{
    ActorFmt* local_6;
    byte* local_4_c;
    int local_8;
    int local_a;
    int local_c;
    int local_e;

    if (D_5893_map_id > 0x20 && D_5893_map_id < 0x80)
    {
        CAST2_0000_CastEffect(2);

        local_c = D_24d6[D_6603];
        local_e = D_24de[D_6603];

        local_4_c = &GetDungeonMap(D_5896_map_x, D_5897_map_y, D_5895_map_level);

        if ((*local_4_c & 0xf0) != 0x40)
        {
            local_4_c =
                &GetDungeonMap((local_c + D_5896_map_x) & 7, (local_e + D_5897_map_y) & 7, D_5895_map_level);
        }

        if ((*local_4_c & 0xf0) == 0x40)
        {
            if ((*local_4_c & 1) != 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x45a1, "Disarmed!\n"));
            }

            *local_4_c = (*local_4_c & 8) | 0x70;

            ULTIMA_1850_PrintString(_TEXT(0x45ac, "Chest opened!\n"));

            local_8 = -1;
        }
        else
        {
            local_8 = 0;
        }
    }
    else
    {
        if (CAST2_0306() == 0)
        {
            return -1;
        }

        local_8 = 0;

        local_4_c = ULTIMA_4402_GetTileAddr(D_5876, D_5878);

        if (*local_4_c == TILE_MAP_DOOR_B9 || *local_4_c == TILE_MAP_DOOR_BB)
        {
            --*local_4_c;

            D_24e6 |= 2;

            CAST2_0000_CastEffect(2);

            local_8 = 1;
        }
        else
        {
            for (local_a = 0; local_a < 0x20; local_a++)
            {
                local_6 = &D_5c5a[local_a];

                if (local_6->_0_tile == TILE_ACTOR_CHEST && local_6->_2_x == D_5876 && local_6->_3_y == D_5878 &&
                    (D_5893_map_id > 0x7f || local_6->_4_z == D_5895_map_level))
                {
                    local_6->_5 &= 0x7f;

                    CAST2_0000_CastEffect(2);

                    local_8 = 1;
                    break;
                }
            }
        }
    }

    return local_8;
}

static void CAST_043e_AnCorpXen(void)
{
    CombatEntity* local_4;
    int local_6_p;

    CAST2_0000_CastEffect(2);

    for (local_6_p = 0; local_6_p < 0x20; local_6_p++)
    {
        local_4 = &D_ba14[local_6_p];
        if (CAST_0000(local_6_p) == 0 && (local_4->flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) == COMBAT_FLAGS_MONSTER &&
            ((byte)D_153c_enemyFlags[local_4->entityIdx] & ENEMY_FLAG_UNDEAD) != 0)
        {
            if (COMSUBS_0000(D_589e, local_6_p, 0) == 0)
            {
                local_4->hp = 1;
                local_4->flags |= COMBAT_FLAGS_2;
            }
        }
    }
}

static void CAST_04a4_InWis(void)
{
    CAST2_0000_CastEffect(2);
    CAST2_06ec();
}

// CHECKED
static int CAST_04b0_KalXen(void)
{
    int local_10_ad;
    u8 local_e;
    int local_c;
    int local_a;
    int local_8;
    ActorFmt* local_6;
    int local_4_c;

    CAST2_0000_CastEffect(2);

    local_c = ULTIMA_2092_RandomRange(0, 0xf);

    if (local_c < 6)
    {
        local_4_c = 0x14;
    }
    else if (local_c < 0xb)
    {
        local_4_c = 0x16;
    }
    else if (local_c < 0xe)
    {
        local_4_c = 0x15;
    }
    else
    {
        local_4_c = 0x22;
    }

    // 04f9
    local_a = 0;
    local_c = 0;

    local_e = local_4_c * 4 + 0x40;
    local_10_ad = local_4_c * 4 + 0x140;

    // 0522
    while (local_c < 8)
    {
        if (COMBAT_120e() == 0 || COMBAT_0000(TILE_ACTOR_90, D_5876, D_5878) == 0)
        {
            local_c++;
            continue;
        }

        local_8 = ULTIMA_6506(local_4_c, 0, D_5876, D_5878, D_5895_map_level);
        if (local_8 < 0)
        {
            break;
        }

        local_6 = &D_5c5a[D_ba14[local_8].actorIdx];

        // 056c
        local_6->_1_animTile = local_6->_0_tile = TILE_ACTOR_CIRCLE;

        // 0588
        ULTIMA_1068(local_10_ad, D_5876, D_5878);

        local_6->_1_animTile = local_6->_0_tile = local_e;
        D_ba14[local_8].flags |= COMBAT_FLAGS_CHARMED;

        local_a = 1;
        break;
    }
    // 05aa
    return local_a;
}

static int CAST_05b4_InManiXen(void)
{
    CAST2_0000_CastEffect(2);
    ULTIMA_3f14_IncreaseInt(&D_57a8, ULTIMA_2092_RandomRange(1, 3), 9999);
    D_a9fa = 1;
    return 1;
}

static int CAST_05dc_InPor(void)
{
    int local_14;
    int local_12_ab;
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    ActorFmt* local_4_c;

    local_c = 0;
    if (D_5893_map_id > 0x7f)
    {
        local_4_c = &D_5c5a[D_ba14[D_589e].actorIdx];
        if ((D_58a1 & 2) != 0)
        {
            local_c = 0;
        }
        else
        {
            CAST2_0000_CastEffect(3);
            for (local_6 = 0; local_6 < 7; local_6++)
            {
                if (COMBAT_120e() == 0)
                    continue;

                if (COMBAT_0000(local_4_c->_0_tile, D_5876, D_5878) == 0)
                    continue;

                local_4_c->_2_x = D_ba14[D_589e].x = D_5876;
                local_4_c->_3_y = D_ba14[D_589e].y = D_5878;

                local_c = 1;
                break;
            }
        }
    }
    else
    {
        // 0680
        if (CAST2_0306() == 0)
        {
            return -1;
        }
        else
        {
            // 068e
            CAST2_0000_CastEffect(3);

            local_12_ab = D_5876 - D_5896_map_x;
            local_14 = D_5878 - D_5897_map_y;

            local_e = D_589b + 0x20;
            if (local_e > 0x100)
            {
                local_e = 0x100;
            }

            local_10_ad = D_589c + 0x20;
            if (local_10_ad > 0x100)
            {
                local_10_ad = 0x100;
            }

            // 06d9
            local_c = 0;
            local_8 = (byte)D_5876;
            for (local_a = (byte)D_5878; D_589b <= local_8 && local_8 < local_e && D_589c <= local_a && local_a < local_10_ad;
                 local_a += local_14)
            {
                if (*ULTIMA_4402_GetTileAddr(local_8, local_a) == TILE_MAP_GRASS)
                {
                    local_c = -1;
                    D_5896_map_x = local_8;
                    D_5897_map_y = local_a;
                    D_24e6 = 1;
                }
                local_8 += local_12_ab;
            }

            if (local_c != 0)
            {
                MAINOUT_0000();
            }
        }
    }

    return local_c;
}

static void CAST_074c_QuasWis(void)
{
    int local_6_p;
    ActorFmt* local_4;

    CAST2_0000_CastEffect(4);

    for (local_6_p = 0; local_6_p < 0x20; local_6_p++)
    {
        if (D_ba14[local_6_p].flags != 0 &&
            (D_ba14[local_6_p].flags & COMBAT_FLAGS_PLAYER) == 0 &&
            (D_ba14[local_6_p].flags & COMBAT_FLAGS_INVISIBLE) != 0)
        {
            D_ba14[local_6_p].flags &= ~COMBAT_FLAGS_INVISIBLE;
            local_4 = &D_5c5a[D_ba14[local_6_p].actorIdx];
            local_4->_1_animTile = local_4->_0_tile;

            ULTIMA_5394();
            ULTIMA_56ac_DrawMap();
        }
    }
}

static int CAST_07b4_BetInXen(void)
{
    int local_8;
    int local_6;
    int local_4_ab;

    CAST2_0000_CastEffect(5);

    local_6 = 0;

    // 07c7
    for (local_8 = 0; local_8 < 8; local_8++)
    {
        if (COMBAT_120e() == 0 || COMBAT_0000(TILE_ACTOR_BC, D_5876, D_5878) == 0)
            continue;

        local_6 = 1;
        break;
    }

    if (local_6 != 0)
    {
        local_6 = 0;

        for (local_8 = 0; local_8 < 4; local_8++)
        {
            local_4_ab = ULTIMA_6506(0x1f, 0, D_5876, D_5878, D_5895_map_level);
            if (local_4_ab < 0)
            {
                break;
            }

            D_ba14[local_4_ab].flags |= COMBAT_FLAGS_CHARMED;
            local_6 = 1;
        }
    }

    return local_6;
}

static int CAST_0846_AnExPor(void)
{
    int local_6_an;
    byte* local_4;

    if (CAST2_0306() == 0)
    {
        return -1;
    }
    else
    {
        CAST2_0000_CastEffect(5);
        local_4 = ULTIMA_4402_GetTileAddr(D_5876, D_5878);
        local_6_an = 0;
        switch (*local_4)
        {
        case TILE_MAP_DOOR_B8:
        case TILE_MAP_DOOR_B9:
            *local_4 = TILE_MAP_97;
            local_6_an = 1;
            D_24e6 |= 2;
            break;

        case TILE_MAP_DOOR_BA:
        case TILE_MAP_DOOR_BB:
            *local_4 = TILE_MAP_98;
            local_6_an = 1;
            D_24e6 |= 2;
            break;
        }

        return local_6_an;
    }
}

static int CAST_08ac_ManiVas(void)
{
    int local_6_an;
    int local_4;

    local_6_an = CAST2_009e_OnWho();
    if (local_6_an < 0)
    {
        local_4 = -1;
    }
    else if (D_55a8_party[local_6_an].status == STATUS_DEAD || (D_5893_map_id > 0x7f && D_58a3 == 0))
    {
        local_4 = 0;
    }
    else if (D_5893_map_id > 0x7f && D_58a3 == 0) // sic
    {
        local_4 = 0;
    }
    else
    {
        D_55a8_party[local_6_an].hp = D_55a8_party[local_6_an].maxHp;
        CAST2_0000_CastEffect(5);
        D_a9fa = 1;
        local_4 = 1;
    }

    return local_4;
}

static void CAST_091e_InPorVasYlem(int param_1)
{
    int local_4;

    CAST2_0000_CastEffect(6);
    ULTIMA_3072_ShakeScreen();
    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        if (D_ba14[local_4].flags != 0)
        {
            if (ULTIMA_5646(local_4) != 0)
            {
                if (COMBAT_13e2(local_4, QUERY_STAT_DEX) <= ULTIMA_3abe())
                {
                    ULTIMA_3564(local_4);
                    ULTIMA_3f14_IncreaseInt(&D_55a8_party[param_1].exp, COMBAT_1574(local_4, ULTIMA_2092_RandomRange(1, 0x14)), 9999); // FMT: &local_8 ??
                    COMSUBS_0312(local_4, D_589e);
                }
            }
        }
    }
}

static int CAST_09a0_AnExXen(void)
{
    int local_4;

    ULTIMA_1850_PrintString(_TEXT(0x45bb, "Creature: "));
    if (COMSUBS_0504(D_589e, 0xf) == 0)
    {
        return -1;
    }

    // 09c6
    CAST2_0000_CastEffect(6);
    if ((*ULTIMA_4402_GetTileAddr(D_5899, D_589a) & 0xfe) == TILE_MAP_84)
    {
        return 0;
    }

    local_4 = COMSUBS_0748(D_5899, D_589a);
    if (local_4 >= 0)
    {
        if (CAST_0000(local_4) == 0 && ULTIMA_5646(local_4) != 0 && COMSUBS_0000(D_589e, local_4, 0) == 0)
        {
            D_ba14[local_4].flags ^= COMBAT_FLAGS_CHARMED;
            if ((D_ba14[local_4].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                D_55a8_party[D_ba14[local_4].entityIdx].status = STATUS_GOOD;
                ULTIMA_2900_UpdateVitalsDisplay();
            }

            COMSUBS_0094_PrintName(local_4);
            ULTIMA_1850_PrintString(_TEXT(0x45c6, " charmed!\n"));

            return -1;
        }
    }

    return 0;
}

static int CAST_0a5c_BetRelXen(void)
{
    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x45d1, "Creature: "));
    if (COMSUBS_0504(D_589e, 0xf) == 0)
    {
        return -1;
    }

    // 0a82
    CAST2_0000_CastEffect(6);
    local_4_ab = COMSUBS_0748(D_5899, D_589a);
    if (local_4_ab < 0)
    {
        return 0;
    }

    if (CAST_0000(local_4_ab) == 0 && COMSUBS_0000(D_589e, local_4_ab, 0) == 0)
    {
        local_6 = D_ba14[local_4_ab].x;
        local_8 = D_ba14[local_4_ab].y;
        COMBAT_1236(-local_4_ab - 1);
        ULTIMA_6506(0x14, 0, local_6, local_8, D_5895_map_level);
        return 1;
    }

    return 0;
}

static int CAST_0afe_LorSanct(void)
{
    D_5c5a[D_ba14[D_589e].actorIdx]._1_animTile = TILE_ACTOR_1D;
    D_ba14[D_589e].flags |= COMBAT_FLAGS_INVISIBLE;
    CAST2_0000_CastEffect(7);
    return 1;
}

static int CAST_0b28_InQuasXen(void)
{
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    ULTIMA_1850_PrintString(_TEXT(0x45dc, "Creature: "));
    if (COMSUBS_0504(D_589e, 0xf) == 0)
    {
        return -1;
    }

    CAST2_0000_CastEffect(7);
    local_6 = COMSUBS_0748(D_5899, D_589a);
    if (local_6 < 0)
    {
        local_8 = 0;
    }
    else
    {
        // 0b72
        local_4_ab = local_c = 0xff;
        for (local_a = 0; local_a < 0x20; local_a++)
        {
            if (D_5c5a[local_a]._0_tile == 0)
            {
                local_c = local_a;
                break;
            }
        }

        if (local_c != 0xff)
        {
            for (local_a = 0; local_a < 0x20; local_a++)
            {
                if (D_ba14[local_a].flags == 0)
                {
                    local_4_ab = local_a;
                    break;
                }
            }

            if (local_4_ab != 0xff)
            {
                // 0bbc
                D_ba14[local_4_ab] = D_ba14[local_6];
                
                D_ba14[local_4_ab].actorIdx = local_c;

                D_5c5a[local_c] = D_5c5a[D_ba14[local_6].actorIdx];

                do
                {
                    if (COMBAT_120e() == 0 || COMBAT_0000(TILE_ACTOR_AVATAR, D_5876, D_5878) == 0)
                    {
                        local_8 = 0;
                    }
                    else
                    {
                        D_5c5a[local_c]._2_x = D_ba14[local_4_ab].x = D_5876;
                        D_5c5a[local_c]._3_y = D_ba14[local_4_ab].y = D_5878;
                        local_8 = 1;
                    }
                } while (local_8 == 0);
            }
        }
    }

    return local_8;
}

static void CAST_0c98_CorpInQuas(void)
{
    int local_6_p;
    CombatEntity* local_4;

    CAST2_0000_CastEffect(7);

    for (local_6_p = 0; local_6_p < 0x20; local_6_p++)
    {
        local_4 = &D_ba14[local_6_p];
        if ((local_4->flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) == COMBAT_FLAGS_MONSTER)
        {
            if (CAST_0000(local_6_p) == 0)
            {
                if (COMSUBS_0000(D_589e, local_6_p, 0) == 0)
                {
                    local_4->hp = 1;
                    local_4->flags |= COMBAT_FLAGS_2;
                }
            }
        }
    }
}

static int CAST_0cf0_PorRelVas(void)
{
    byte local_4;

    if ((D_587c_partyTile & 0xf0) != TILE_ACTOR_FRIGATE_20)
    {
        ULTIMA_1850_PrintString(_TEXT(0x45e7, "To phase: "));
        local_4 = ULTIMA_266c_GetChar();
        if (local_4 >= 0x20)
        {
            ULTIMA_16ba_PrintChar(local_4);
        }

        ULTIMA_16ba_PrintChar('\n');

        if (local_4 >= 0x31 && local_4 <= 0x38)
        {
            local_4 -= 0x31;
            CAST2_0000_CastEffect(8);
            if (ULTIMA_47f4(local_4) != 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

static int CAST_0d4c_AnTym(void)
{
    int local_6_an;
    int local_4;

    CAST2_0000_CastEffect(8);
    local_6_an = -1;

    for (local_4 = 0; local_4 < 0x20; local_4++)
    {
        if (D_5c5a[local_4]._0_tile == TILE_ACTOR_SHADOWLORD)
        {
            ULTIMA_1850_PrintString(_TEXT(0x45f2, "Magic absorbed!\n"));
            ULTIMA_2192_AudioPulse(0x2648, 1, 28000, 1000, 2);
            local_6_an = 0;
            break;
        }
    }

    if (local_6_an != 0)
    {
        D_587a = 0x54;
        D_588e = 10;
        ULTIMA_2900_UpdateVitalsDisplay();
    }

    return local_6_an;
}

static void CAST_1f60(int param_1, int param_2, int param_3);

#if defined(TARGET_DOS16)
static char S_0x466a[] = "Scroll\n\n";
#endif

int CAST_0dba_CastSpellCmd(void)
{
    int local_e;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_c = local_e = D_5890 = D_588f = local_8 = 1;

    local_6 = ULTIMA_4988();
    if (local_6 < 0)
    {
        return local_6;
    }

    ULTIMA_1850_PrintString(_TEXT(0x4603, "Spell name:\n:"));
    local_4_ab = CAST2_00de_SelectSpell();
    if (local_4_ab == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4611, "None!\n"));
#if !defined(MATCHING_BUILD)
        return local_8; // TODO: validate return value
#else
        return; // sic
#endif
    }

    if (local_4_ab == -2)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4618, "No effect!\n"));
#if !defined(MATCHING_BUILD)
        return local_8; // TODO: validate return value
#else
        return; // sic
#endif
    }

    D_588f = local_a = local_4_ab / 6 + 1;

    if (D_5893_map_id == 0)
    {
        if ((D_1c90[local_4_ab] & 8) == 0)
        {
            local_e = 0;
        }
    }
    else if (D_5893_map_id > 0x7f)
    {
        if ((D_1c90[local_4_ab] & 1) == 0)
        {
            local_e = 0;
        }
    }
    else
    {
        if (((D_5893_map_id == 0x12) && (D_57b4 == 0)) || (D_5893_map_id == 0x1d)) // LB crown / at STONEGATE
        {
            ULTIMA_1850_PrintString(_TEXT(0x4624, "Absorbed!\n"));
            ULTIMA_2192_AudioPulse(0x2648, 1, 28000, 1000, 2);
#if !defined(MATCHING_BUILD)
            return local_8; // TODO: validate return value
#else
            return; // sic
#endif
        }

        if (D_5893_map_id < 0x21)
        {
            if ((D_1c90[local_4_ab] & 4) == 0)
            {
                local_e = 0;
            }
        }
        else
        {
            if ((D_1c90[local_4_ab] & 2) == 0)
            {
                local_e = 0;
            }
        }
    }

    if (local_e == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x462f, "Not here!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
#if !defined(MATCHING_BUILD)
        return local_8; // TODO: validate return value
#else
        return; // sic
#endif
    }

    if (D_57f0[local_4_ab] == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x463a, "None mixed!\n"));
#if !defined(MATCHING_BUILD)
        return local_8; // TODO: validate return value
#else
        return; // sic
#endif
    }

    D_57f0[local_4_ab]--;
    if (D_55a8_party[local_6].mag < local_a)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4647, "M.P. too low!\n"));
        local_c = 0;
    }
    else
    {
        D_55a8_party[local_6].mag -= local_a;
        local_c = -1;
        if (D_55a8_party[local_6].level < local_a)
        {
            local_c = 0;
        }
        else
        {
            // D_1c30 idx
            switch (local_4_ab)
            {
            case 0: // IL
                CAST2_08ea(100);
                CAST2_0000_CastEffect(1);
                break;
            case 1: // GP
                CAST_0032(0x30);
                break;
            case 2: // AZ
                local_c = CAST_0114_AnZu();
                break;
            case 3: // AN
                local_c = CAST_01ae_AnNox();
                break;
            case 4: // M
                local_c = CAST_01fa_Mani();
                break;
            case 5: // AY
                local_c = CAST_0230_AnYlem();
                break;
            case 6: // AS
                local_c = CAST_02d2_AnSanct();
                break;
            case 7: // ACX
                CAST_043e_AnCorpXen();
                break;
            case 8: // HR
                CAST2_040a_ChangeWindDirection(CAST2_0306(), 0);
                break;
            case 9: // IW
                CAST_04a4_InWis();
                break;
            case 10: // KX
                local_c = CAST_04b0_KalXen();
                break;
            case 0xb: // IMX
                local_c = CAST_05b4_InManiXen();
                break;
            case 0xc: // LV
                CAST2_08ea(0xff);
                CAST2_0000_CastEffect(3);
                break;
            case 0xd: // FV
                CAST_0032(0x31);
                break;
            case 0xe: // FGI
                local_c = CAST_004c(0);
                break;
            case 0xf: // GIN
                local_c = CAST_004c(1);
                break;
            case 0x10: // GIZ
                local_c = CAST_004c(2);
                break;
            case 0x11: // IP
                local_c = CAST_05dc_InPor();
                break;
            case 0x12: // AG
                local_c = CAST2_07bc(1);
                break;
            case 0x13: // IS
                CAST2_08f8(0x50, 0x14, 4);
                break;
            case 0x14: // GIS
                local_c = CAST_004c(3);
                break;
            case 0x15: // PU
                if (D_5893_map_id == 40)
                {
                    local_c = 0;
                    break;
                }

                CAST2_0000_CastEffect(4);
                if (DUNGEON_1c6a_Klimb(-1, 1) != 0)
                {
                    DUNGEON_1d08_Exit();
                }
                break;
            case 0x16: // DP
                if (D_5893_map_id == 40)
                {
                    local_c = 0;
                    break;
                }

                CAST2_0000_CastEffect(4);
                if (DUNGEON_1c6a_Klimb(1, 1) != 0)
                {
                    DUNGEON_1d08_Exit();
                }
                break;
            case 0x17: // QW
                CAST_074c_QuasWis();
                break;
            case 0x18: // BIX
                local_c = CAST_07b4_BetInXen();
                break;
            case 0x19: // AEP
                local_c = CAST_0846_AnExPor();
                break;
            case 0x1a: // EIP
                local_c = CAST2_0768();
                if (local_c == -1)
                    break;
                CAST2_0000_CastEffect(5);
                break;
            case 0x1b: // MV
                local_c = CAST_08ac_ManiVas();
                break;
            case 0x1c: // IZ
                CAST_1f60(D_589e, 1, D_13b6_magentaColor);
                break;
            case 0x1d: // RT
                CAST2_08f8(0x51, 0x1e, 5);
                break;
            case 0x1e: // IPVY
                CAST_091e_InPorVasYlem(local_6);
                break;
            case 0x1f: // AQW
                CAST2_08f8(0x43, 0x14, 6);
                break;
            case 0x20: // AI
                CAST2_08f8(0x4e, 10, 6);
                break;
            case 0x21: // AWY
                CAST2_0000_CastEffect(6);
                CAST2_046c();
                break;
            case 0x22: // AEX
                local_c = CAST_09a0_AnExXen();
                break;
            case 0x23: // BRX
                local_c = CAST_0a5c_BetRelXen();
                break;
            case 0x24: // LS
                local_c = CAST_0afe_LorSanct();
                break;
            case 0x25: // CX
                CAST_0032(0x32);
                break;
            case 0x26: // IQX
                local_c = CAST_0b28_InQuasXen();
                break;
            case 0x27: // IQW view?
                CAST2_0000_CastEffect(7);
                if (D_5893_map_id < 0x21)
                {
                    LOOKOBJ_10fc_ViewCmd(D_5896_map_x, D_5897_map_y);
                }
                else
                {
                    DNGLOOK_06a8_ViewCmd();
                }
                break;
            case 0x28: // HIN
                CAST_1f60(D_589e, 2, D_13b4_greenColor);
                break;
            case 0x29: // CIQ
                CAST_0c98_CorpInQuas();
                break;
            case 0x2a: // CIM
                local_c = CAST2_05e0_Resurrect(CAST2_009e_OnWho(), 0);
                ULTIMA_2900_UpdateVitalsDisplay();
                break;
            case 0x2b: // CKX
                local_c = CAST2_04c2(0);
                break;
            case 0x2c: // CGIV
                CAST_1f60(D_589e, 4, D_13b2_blueColor);
                break;
            case 0x2d: // FHI
                CAST_1f60(D_589e, 3, D_13ae_redColor);
                break;
            case 0x2e: // PRV
                local_c = CAST_0cf0_PorRelVas();
                if (local_c != 0)
                {
                    local_8 = 0;
                }
                break;
            case 0x2f: // AT
                local_c = CAST_0d4c_AnTym();
                break;
            }
        }
    }

    if (local_c == 1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4656, "Success!\n"));
    }
    else if (local_c == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4660, "Failed!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
    }

    return local_8;
}

// use scroll
static int CAST_11de_UseScroll(int param_1)
{
    int local_6_an;
    int local_4 = 1;

    D_5820[param_1]--;
    ULTIMA_1850_PrintString(_XTEXT(0x466a, "Scroll\n\n"));
    switch (param_1)
    {
    case 0:
        CAST2_08ea(0xf0);
        ULTIMA_1850_PrintString(_TEXT(0x4673, "Light!\n"));
        CAST2_0000_CastEffect(0);
        break;

    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x467b, "Wind change!\n"));
        local_6_an = CAST2_0306();
        if (D_5893_map_id < 0x21)
        {
            CAST2_040a_ChangeWindDirection(local_6_an, 1);
            break;
        }
        local_4 = 0;
        break;

    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x4689, "Protection!\n"));
        CAST2_08f8(0x50, 100, 2);
        break;

    case 3:
        ULTIMA_1850_PrintString(_TEXT(0x4696, "Negate magic!\n"));
        CAST2_08f8(0x4e, 0x14, 3);
        break;

    case 4:
        // 1278
        ULTIMA_1850_PrintString(_TEXT(0x46a5, "View!\n"));
        if (D_5893_map_id > 0x7f)
        {
            ULTIMA_1850_PrintString(_TEXT(0x46ac, "Not here!\n"));
            break;
        }
        else
        {
            // 1290
            CAST2_0000_CastEffect(4);
            if (D_5893_map_id < 0x21)
            {
                LOOKOBJ_10fc_ViewCmd(D_5896_map_x, D_5897_map_y);
                break;
            }
            DNGLOOK_06a8_ViewCmd();
            break;
        }

    case 5:
        // 12b4
        ULTIMA_1850_PrintString(_TEXT(0x46b7, "Summon Daemon!\n"));
        if (D_5893_map_id > 0x7f)
        {
            local_4 = CAST2_04c2(1);
            break;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x46c7, "Not here!\n"));
            break;
        }

    case 6:
        // 12d2
        ULTIMA_1850_PrintString(_TEXT(0x46d2, "Resurrection!\n"));
        if (D_5893_map_id < 0x80)
        {
            local_4 = CAST2_05e0_Resurrect(CAST2_009e_OnWho(), 1);
            ULTIMA_2900_UpdateVitalsDisplay();
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0x46e1, "Not here!\n"));
        break;

    case 7:
        if ((D_5893_map_id == 0x1d) || (D_5893_map_id == 0x28))
        {
            ULTIMA_1850_PrintString(_TEXT(0x46ec, "No effect!\n"));
            ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
            break;
        }
        ULTIMA_1850_PrintString(_TEXT(0x46f8, "Negate time!\n"));
        CAST2_08f8(0x54, 0x14, 7);
        break;
    }

    return local_4;
}

#if defined(TARGET_DOS16)
static char S_0x474c[] = "\nNo noticeable effect now!\n";
#endif

// CHECKED
// use potion
static int CAST_135a_UsePotion(int param_1)
{
    ActorFmt* local_6;
    int local_4_c;
    int local_8;
    int local_a;

    local_8 = 1;
    D_5828[param_1]--;
    ULTIMA_1850_PrintString(_TEXT(0x4706, "Potion\n"));
    if (D_5893_map_id > 0x7f)
    {
        local_a = D_ba14[D_589e].entityIdx;
    }
    else
    {
        local_a = CAST2_009e_OnWho();
    }

    if (local_a < 0)
    {
        return local_a;
    }

    CAST2_0000_CastEffect(param_1);
    local_4_c = ULTIMA_2092_RandomRange(0, 0xf);
    if (local_4_c == 0)
    {
        param_1 = 4;
    }
    else if (local_4_c == 1)
    {
        param_1 = ULTIMA_2092_RandomRange(0, 7);
    }

    switch (param_1)
    {
    case 0:
        if (D_55a8_party[local_a].status == STATUS_SLEEP)
        {
            D_55a8_party[local_a].status = STATUS_GOOD;

            if (D_5893_map_id > 0x7f)
            {
                if (local_a == D_ba14[D_589e].entityIdx)
                {
                    if ((D_ba14[D_589e].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) ==
                        (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_ASLEEP))
                    {
                        ULTIMA_6800(D_589e);
                    }
                    else
                    {
                        local_8 = 0;
                    }
                }
                else
                {
                    local_8 = 0;
                }
            }
            else
            {
                ULTIMA_2900_UpdateVitalsDisplay();
            }
        }
        else
        {
            local_8 = 0;
        }
        break;

    case 1:
        local_8 = CAST2_03c2(local_a);
        if (local_8 != 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x470e, "Healed!\n"));
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        break;

    case 2:
        if (D_55a8_party[local_a].status == STATUS_POISONED)
        {
            D_55a8_party[local_a].status = STATUS_GOOD;

            ULTIMA_1850_PrintString(_TEXT(0x4717, "Poison cured!\n"));
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            local_8 = 0;
        }
        break;

    case 3:
        if (D_55a8_party[local_a].status == STATUS_GOOD)
        {
            D_55a8_party[local_a].status = STATUS_POISONED;

            ULTIMA_1850_PrintString(_TEXT(0x4726, "POISONED!\n"));
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            local_8 = 0;
        }
        break;

    case 4:
        if (D_55a8_party[local_a].status == STATUS_GOOD)
        {
            if (D_5893_map_id < 0x80)
            {
                D_55a8_party[local_a].status = STATUS_SLEEP;
            }
            else
            {
                ULTIMA_68ae(local_a);
            }

            ULTIMA_1850_PrintString(_TEXT(0x4731, "Slept!\n"));
            ULTIMA_2900_UpdateVitalsDisplay();
        }
        else
        {
            local_8 = 0;
        }
        break;

    case 5:
        if (D_5893_map_id > 0x7f)
        {
            ULTIMA_1850_PrintString(_TEXT(0x4739, "Poof!\n"));
            local_6 = &D_5c5a[D_ba14[D_589e].actorIdx];
            local_6->_0_tile = local_6->_1_animTile = TILE_ACTOR_90;
        }
        else
        {
            ULTIMA_1850_PrintString(_XTEXT(0x474c, "\nNo noticeable effect now!\n"));
        }
        break;

    case 6:
        if (D_5893_map_id > 0x7f)
        {
            D_ba14[D_589e].flags |= COMBAT_FLAGS_INVISIBLE;
            local_6 = &D_5c5a[D_ba14[D_589e].actorIdx];
            ULTIMA_1850_PrintString(_TEXT(0x4740, "Invisible!\n"));
            local_6->_0_tile = local_6->_1_animTile = TILE_ACTOR_1D;
        }
        else
        {
            ULTIMA_1850_PrintString(_XTEXT(0x474c, "\nNo noticeable effect now!\n"));
        }
        break;

    case 7:
        if (D_5893_map_id < 0x21)
        {
            CAST2_046c();
        }
        else
        {
            ULTIMA_1850_PrintString(_XTEXT(0x474c, "\nNo noticeable effect now!\n"));
        }
        break;
    }

    return local_8;
}

// use / bury moonstone
static void CAST_153c_UseMoonstone(int param_1)
{
    int local_4;

    local_4 = *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y);
    ULTIMA_1850_PrintString(_TEXT(0x4768, "Moonstone "));
    if (D_5893_map_id < 0x21 && (local_4 == TILE_MAP_CROPS_PICKED || local_4 == TILE_MAP_CROPS || (local_4 > TILE_MAP_WATER_3 && local_4 < TILE_MAP_B)))
    {
        ULTIMA_1850_PrintString(_TEXT(0x4773, "buried!\n"));
        D_5830[param_1] = D_5896_map_x;
        D_5838[param_1] = D_5897_map_y;
        D_5840[param_1] = D_5893_map_id;
        D_5848[param_1] = D_5895_map_level;
    }
    else
    {
        ULTIMA_1850_PrintString(_TEXT(0x477c, "cannot be buried here!\n"));
    }
}


u8 D_4882[4] = {0x0f, 0x0f, 0x0f};

u8 D_4886[4] = {0x09, 0x03, 0x10};

u8 D_488a[4] = {0x1e, 0x1f, 0x20};

u8 D_488e[4] = {0x02, 0x01, 0xff};

u8 D_4892[3] = {0x02, 0x04, 0x08};

// CHECKED
// use gem shard -> defeat shadowlord
static void CAST_15b4_UseGemShard(int param_1)
{
    int local_4;

    ULTIMA_1850_PrintString(_TEXT(0x4794, "Gem Shard\n\nThou dost hold above thee the evil Shard of "));
    switch (param_1)
    {
    case 0:
        ULTIMA_1850_PrintString(_TEXT(0x47cc, "Falsehood..."));
        break;
    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x47d9, "Hatred..."));
        break;
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x47e3, "Cowardice..."));
        break;
    }

    for (local_4 = 2000; local_4 < 25000; local_4 += 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, local_4, 0);
    }

    for (local_4 = 25000; local_4 > 2000; local_4 -= 0x32)
    {
        ULTIMA_2192_AudioPulse(0xa50, 1, 200, local_4, 0);
    }

    if (D_4882[param_1] != D_5896_map_x || D_4886[param_1] != D_5897_map_y || D_488a[param_1] != D_5893_map_id ||
        D_488e[param_1] != D_5895_map_level)
    {
        ULTIMA_1850_PrintString(_TEXT(0x47f0, "\n\nNo effect!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
        return;
    }

    ULTIMA_3ae6(7);
    ULTIMA_1850_PrintString(_TEXT(0x47fe, "\n\n...and cast it into the Flame of "));
    switch (param_1)
    {
    case 0:
        ULTIMA_1850_PrintString(_TEXT(0x4822, "Truth!\n"));
        break;
    case 1:
        ULTIMA_1850_PrintString(_TEXT(0x482a, "Love!\n"));
        break;
    case 2:
        ULTIMA_1850_PrintString(_TEXT(0x4831, "Courage!\n"));
        break;
    }

    ULTIMA_3072_ShakeScreen();
    ULTIMA_3072_ShakeScreen();
    ULTIMA_3072_ShakeScreen();
    ULTIMA_3ae6(3);

    if (ULTIMA_368e_FindActorTileAtPos(D_5896_map_x, D_5897_map_y - 1, D_5895_map_level) == TILE_ACTOR_SHADOWLORD &&
        param_1 == D_58cb)
    {
        TOWN_00b0_DespawnNpc(TOWN_011e(D_5876));

        for (local_4 = 0; local_4 < 7; local_4++)
        {
            ULTIMA_3522(D_5896_map_x, D_5897_map_y - 1);
        }

        D_58c8[param_1] = 0xff;
        D_57b6[param_1] = 0;

        *(u32*)&D_5b5a[0x70] |= D_4892[param_1];

        ULTIMA_1850_PrintString(_TEXT(0x483b, "\nThe doom of the Shadowlord "));
        switch (param_1)
        {
        case 0:
            ULTIMA_1850_PrintString(_TEXT(0x4858, "Faulinei"));
            break;
        case 1:
            ULTIMA_1850_PrintString(_TEXT(0x4861, "Astaroth"));
            break;
        case 2:
            ULTIMA_1850_PrintString(_TEXT(0x486a, "Nosfentor"));
            break;
        }

        ULTIMA_1850_PrintString(_TEXT(0x4874, " is wrought!\n"));
        ULTIMA_4368_AudioVictoryTune();
    }
}

// remove item
static bool CAST_1764(int param_1)
{
    if (param_1 == D_587a)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4895, "Removed!\n"));

        D_587a = D_588e = 0;
        D_a9fa = 1;

        return 0;
    }

    return 1;
}

#if defined(TARGET_DOS16)
static char S_0x4a84[] = " of Lord British...\n";
#endif

void CAST_1792_UseCmd(void)
{
    int local_14;
    int local_12_ab;
    int local_10_ad;
    int local_e;
    int local_c;
    int local_a;
    byte* local_8;
    int local_6;
    int local_4_ab;

    local_c = 1;
    ZSTATS_099a();

    local_6 = ZSTATS_05a4(-1, 0x26, D_b9ee, 0xff);
    if (local_6 == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0x489f, "No usable items!\n"));
        return;
    }

    ULTIMA_1850_PrintString(_TEXT(0x48b1, "Item: "));
    ULTIMA_4efc();
    ULTIMA_1b94_SelectTextWindow(1);
    ULTIMA_4e50(_TEXT(0x48b8, "Items:"));
    ZSTATS_045e(8);

    local_14 = ZSTATS_0f2e(local_6, 0xff, 0x55);
    ULTIMA_4daa();
    ULTIMA_4e20();
    ULTIMA_4f3c();
    ULTIMA_2900_UpdateVitalsDisplay();
    if (local_14 < 0)
    {
        return;
    }

    if (local_14 < 8)
    {
        local_c = CAST_11de_UseScroll(local_14);
    }
    else if (local_14 < 0x10)
    {
        local_c = CAST_135a_UsePotion(local_14 - 8);
    }
    else if (local_14 > 0x14 && local_14 < 0x1d)
    {
        CAST_153c_UseMoonstone(local_14 - 0x15);
    }
    else
    {
        switch (local_14)
        {
        case 0x10:
            ULTIMA_1850_PrintString(_TEXT(0x48bf, "Carpet\n\n"));
            if (D_5893_map_id < 0x21 && *ULTIMA_4402_GetTileAddr(D_5896_map_x, D_5897_map_y) != TILE_MAP_C)
            {
                if (D_587c_partyTile == TILE_ACTOR_AVATAR)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x48c8, "Boarded!\n"));
                    D_587c_partyTile = ULTIMA_2092_RandomRange(0, 1) + TILE_ACTOR_FLYING_CARPET;
                    D_57b0--;
                }
                else if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x48d2, "X-it ship first!\n"));
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x48e4, "Only on foot!\n"));
                }
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x48f3, "Not here!\n"));
            }
            break;

        case 0x11:
            D_57b1--;
            ULTIMA_1850_PrintString(_TEXT(0x48fe, "Skull Key\n"));
            if (D_5893_map_id < 0x21 || 0x7f < D_5893_map_id)
            {
                local_c = CAST2_0768();
                if (local_c != 0 && D_5893_map_id < 0x80)
                {
                    ULTIMA_3522(D_5876, D_5878);
                }
                break;
            }

            ULTIMA_1850_PrintString(_TEXT(0x4909, "Not here!\n"));
            break;

        case 0x12:
            ULTIMA_1850_PrintString(_TEXT(0x4914, "Amulet\n\n"));
            if (CAST_1764(0xe) == 0)
                break;

            ULTIMA_1850_PrintString(_TEXT(0x491d, "Wearing the Amulet"));
            ULTIMA_1850_PrintString(_XTEXT(0x4a84, " of Lord British...\n"));
            CAST2_08f8(0xe, 0xff, 9);
            break;

        case 0x13:
            ULTIMA_1850_PrintString(_TEXT(0x4930, "Crown\n\n"));
            if (CAST_1764(0x1c) == 0)
                break;

            ULTIMA_1850_PrintString(_TEXT(0x4938, "Thou dost don the Crown"));
            ULTIMA_1850_PrintString(_XTEXT(0x4a84, " of Lord British...\n"));
            CAST2_08f8(0x1c, 0xff, 9);
            break;

        case 0x14:
            ULTIMA_1850_PrintString(_TEXT(0x4950, "Sceptre\n\n"));
            ULTIMA_1850_PrintString(_TEXT(0x495a, "Wielding the Sceptre"));
            ULTIMA_1850_PrintString(_XTEXT(0x4a84, " of Lord British...\n"));
            ULTIMA_2192_AudioPulse(0x1450, 1, 50000, 5000, 1);

            local_4_ab = 0;
            if (D_5893_map_id < 0x21 || D_5893_map_id > 0x28)
            {
                for (local_e = -1; local_e <= 1; local_e++)
                {
                    for (local_12_ab = -1; local_12_ab <= 1; local_12_ab++)
                    {
                        local_8 = ULTIMA_4402_GetTileAddr(local_e + D_5896_map_x, local_12_ab + D_5897_map_y);
                        if ((*local_8 & 0xf0) == TILE_MAP_70)
                        {
                            *local_8 = TILE_MAP_GRASS;
                            ULTIMA_5910_UpdateFrame();
                            ULTIMA_223c_AudioWhiteNoise(10, 3000, 2000);
                            local_4_ab++;
                        }
                    }
                }
            }

            if (local_4_ab != 0)
                break;

            local_a = CAST2_07bc(0);
            if (local_a == 1)
            {
                ULTIMA_1850_PrintString(_TEXT(0x496f, "Field dissolved!\n"));
            }
            else if (local_a == 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4981, "No effect!\n"));
            }
            break;

        case 0x1d:
        case 0x1e:
        case 0x1f:
            CAST_15b4_UseGemShard(local_14 - 0x1d);
            break;

        case 0x20:
            ULTIMA_1850_PrintString(_TEXT(0x498d, "Spyglass\n\n"));
            if (D_5893_map_id < 0x21 && D_5895_map_level < 0x80)
            {
                if (D_587f < 6 || 0x12 < D_587f)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4998, "Looking...\n"));
                    LOOKOBJ_0366();
                    break;
                }

                ULTIMA_1850_PrintString(_TEXT(0x49a4, "No stars!\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x49af, "Not here!\n"));
            }
            break;

        case 0x21:
            ULTIMA_1850_PrintString(_TEXT(0x49ba, "Plans\n\n"));
            if ((D_587c_partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
            {
                D_57bb |= 0x80;
                ULTIMA_1850_PrintString(_TEXT(0x49c2, "Ship rigged for double speed!\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x49e1, "Only usable on shipboard!\n"));
            }
            break;

        case 0x22:
            ULTIMA_1850_PrintString(_TEXT(0x49fc, "Sextant\n\n"));
            if (D_5895_map_level > 0x7f || D_5893_map_id != 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4a06, "Only outdoors!\n"));
            }
            else
            {
                if (D_587f > 5 && D_587f < 0x13)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4a16, "Only at night!\n"));
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x4a26, "Position:"));
                    CAST2_06ec();
                }
            }
            break;

        case 0x23:
            ULTIMA_1850_PrintString(_TEXT(0x4a30, "Watch\n\nThe pocket watch reads "));
            local_10_ad = (byte)(D_587f % 0xc);
            if (local_10_ad == 0)
            {
                local_10_ad = 0xc;
            }

            ULTIMA_1a3e_PrintNumber(local_10_ad, 1, 0x20);
            ULTIMA_16ba_PrintChar(':');
            ULTIMA_1a3e_PrintNumber(D_5881, 2, 0x30);
            if (D_587f > 0xb)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4a4f, " PM.\n"));
            }
            else
            {
                ULTIMA_1850_PrintString(_TEXT(0x4a55, " AM.\n"));
            }
            break;

        case 0x24:
            ULTIMA_1850_PrintString(_TEXT(0x4a5b, "Badge\n\n"));
            if (CAST_1764(0x1d) != 0)
            {
                ULTIMA_1850_PrintString(_TEXT(0x4a63, "Badge worn!\n"));
                D_587a = 0x1d;
                D_588e = 0xff;
                D_a9fa = 1;
            }
            break;

        case 0x25:
            ULTIMA_1850_PrintString(_TEXT(0x4a70, "Box\n\nHow?\n"));
            break;
        }
    }

    if (local_c == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x4a7b, "Failed!\n"));
        ULTIMA_43ae_AudioSweepTone(800, 2000, 1, 0x32);
    }
}

static int CAST_1bb0(int x, int y, int penColor)
{
    if (x < 8 || x > 0xb6 || y < 8 || y > 0xb6)
    {
        D_5876 = D_5878 = -1;
        return 0;
    }
    else
    {
        ULTIMA_0a70_GRAP_2d_SetPenColor(penColor);
        ULTIMA_0c64_GRAP_30_Pset(x, y);
        ULTIMA_0c64_GRAP_30_Pset(x + 1, y);
        ULTIMA_22e2_PcspkOn(ULTIMA_2092_RandomRange(100, 10000));

        if ((y & 1) != 0)
        {
            ULTIMA_3fb4(x, y);
            if (D_5876 >= 0 && D_5878 >= 0)
            {
                return ULTIMA_3f6e(D_5876, D_5878);
            }

            return 0;
        }
        else
        {
            return 1;
        }
    }
}

// CHECKED
// NOT MATCHING (182 bytes short): the original strength-reduces all four loops (frame BEh:
// pointer temporaries for every local_8c/local_5a/local_30_ac/D_a9d0[local_5c] in the
// inner loops, 2*local_90_ad carried through the do-while and the outer for, "local_90_ad
// < 0x3f" replaced by a compare of a param_2 pointer with param_2+7Eh, local_8e in SI).
// Here only the first loop is optimized (frame 9Ah): the module's earlier functions use up
// pass 2's capacity (see intro2.c, cmds2.c). With the functions before CAST_1bb0 removed
// (or this function alone in a file with minimal declarations) the inner loop is optimized
// (frame B6h, 774 bytes) but not yet the outer for/do-while; removing the first or second
// inner switch, or the "else if" block, then also gives the outer loop's pointers. Still
// different even then: the first loop ends with a down-counter in CX ([bp-9Ah] = 15h)
// where the original compares SI with an end pointer (lea ax,[bp-2Eh]), and SI/DI are
// swapped. Tried without effect: all 24 orders of the first loop's statements, /On and
// other /O letters, unsigned local_90_ad, the CAST_1bb0 test inverted (if != 0 ... else
// local_8c = 1), D_a9d0 as s16 without the cast, a 2D viewport array, 736K or 400K of DOS
// memory, and the OS/2 C2.EXE bound for DOS (same code: the limit is not DOS memory).
static int CAST_1c36(s16* param_1, s16* param_2, int param_3, int param_4, int param_5)
{
    int local_90_ad;
    int local_8e;

    s16 local_8c[21];
    int local_62_ab;
    int local_60_ad;
    int local_5e;
    int local_5c;
    s16 local_5a[21];

    s16 local_30_ac[21];
    int local_6;
    int local_4_ab;

    local_90_ad = 0;

    local_60_ad = D_ba14[param_4].x * 0x10 + 8;
    local_62_ab = D_ba14[param_4].y * 0x10 + 8;

    switch (param_5)
    {
    case DIR_UP:
        // dc04
        local_60_ad += 8;
        break;

    case DIR_RIGHT:
        // dc5a
        local_60_ad += 0x10;
        local_62_ab += 8;
        break;

    case DIR_LEFT:
        // dc5e
        local_62_ab += 8;
        break;

    case DIR_DOWN:
        // dc64
        local_60_ad += 8;
        local_62_ab += 0x10;
        break;
    }

    // dc08
    param_3 += 8;
    for (local_5c = 0; local_5c < 0x15; local_5c++)
    {
        local_8c[local_5c] = 0; // dc1d
        D_a9d0[local_5c] = D_1cf0[local_5c]; // dc2a

        local_30_ac[local_5c] = local_60_ad; // dc44
        local_5a[local_5c] = local_62_ab; // dc49
    }

    // dc6e
    local_4_ab = 0;
    local_90_ad = 0;

    // dc7f
    do
    {
        local_4_ab = 0;

        for (local_5c = 0; local_5c < 0x15; local_5c++)
        {
            // dca7
            if (local_8c[local_5c] == 0)
            {
                // dcb3
                local_6 = ULTIMA_3aae_Random(0xf);

#if defined(ENABLE_SPEED_FIX)
                TIME_SleepMs(3);
#endif

                for (local_5e = 0; local_5e < local_6; local_5e++)
                {
                    if (CAST_1bb0(local_30_ac[local_5c], local_5a[local_5c], param_3) == 0)
                    {
                        local_8c[local_5c] = 1;
                    }
                    else if (local_90_ad < 0x3f && D_5876 > -1 && D_5878 > -1 && D_5876 < 0xb && D_5878 < 0xb &&
                        GetMapViewport(D_5876, D_5878) == 0)
                    {
                        local_90_ad++;

                        param_1[local_90_ad] = D_5876;
                        param_2[local_90_ad] = D_5878;

                        GetMapViewport(D_5876, D_5878) = 0xff;
                    }

                    if (local_8c[local_5c] == 0)
                    {
                        D_a9d0[local_5c] -= 10;

                        switch (param_5)
                        {
                        case DIR_LEFT:
                            local_30_ac[local_5c]--;
                            break;

                        case DIR_RIGHT:
                            local_30_ac[local_5c]++;
                            break;

                        case DIR_UP:
                            local_5a[local_5c]--;
                            break;

                        case DIR_DOWN:
                            local_5a[local_5c]++;
                            break;
                        }

                        if ((s16)D_a9d0[local_5c] < 1)
                        {
                            if (local_5c < 10)
                            {
                                local_8e = -1;
                            }
                            else
                            {
                                local_8e = 1;
                            }

                            switch (param_5)
                            {
                            case DIR_LEFT:
                                local_5a[local_5c] -= local_8e;
                                break;

                            case DIR_RIGHT:
                                local_5a[local_5c] += local_8e;
                                break;

                            case DIR_UP:
                                local_30_ac[local_5c] += local_8e;
                                break;

                            case DIR_DOWN:
                                local_30_ac[local_5c] -= local_8e;
                                break;
                            }

                            D_a9d0[local_5c] += D_1cf0[local_5c];
                        }
                    }

                    if (local_8c[local_5c] != 0)
                    {
                        break;
                    }
                }
            }
            else
            {
                local_4_ab++;
            }
        }
    } while (local_4_ab < 0x15);

    ULTIMA_230e_PcspkOff();
    return local_90_ad;
}

// CHECKED
// NOT MATCHING (36 bytes short): the original optimizes both loops (local_108a in SI,
// param_1 in DI, the target arrays walked by a pointer from &local_82l[1] against an end
// pointer at [bp-116h], frame 11Ah); here neither loop is optimized (frame 10Eh), because
// the earlier functions of the module use up pass 2's capacity (see CAST_1c36). With the
// functions before CAST_1bb0 removed the loops are optimized (508 bytes, frame 118h), but
// the outer loop counts with local_10eg-1 instead of the end pointer, and the first switch
// is laid out differently: the original has "cmp 1; je case1; cmp 2; jne default; jmp
// case2", case 1 falling into the call and case 2 placed far away, after the second
// switch's dispatch. In that setting: switch orders (case 2, default, case 1) and (case 1,
// case 2, default) give 500 bytes, an if/else chain 494; none gives that layout.
static void CAST_1f60(int param_1, int param_2, int param_3)
{
    S_55a8* local_104c;
    CombatEntity* local_106b;
    int local_108a;
    int local_10ai;
    int local_10ch;
    s16 local_102d[64];
    s16 local_82l[64];
    int local_10eg;

    local_10ch = 1;
    local_104c = &D_55a8_party[D_ba14[param_1].entityIdx];
#if !defined(MATCHING_BUILD)
    local_10eg = CAST2_0306();
#else
    local_10eg = CAST2_0306(param_1); // NOTE: original code error?
#endif
#if !defined(MATCHING_BUILD) // added in FMT
    if (local_10eg != 0)
#endif
    {
        switch (param_2)
        {
        default:
            local_10ai = 0x5140;
            break;
        case 1:
            local_10ai = 16000;
            break;
        case 2:
            local_10ai = 0x4b00;
            break;
        }

        // df30
        ULTIMA_223c_AudioWhiteNoise(800, local_10ai, 700);

        local_10eg = CAST_1c36(local_82l, local_102d, param_3, param_1, local_10eg);
        local_10eg++;

        for (local_10ch = 1; local_10ch < local_10eg; local_10ch++)
        {
            for (local_108a = 0x1f; local_108a > -1; local_108a--)
            {
                // dfac?
                local_106b = &D_ba14[local_108a]; // local_114
                if (local_106b->x == local_82l[local_10ch] && local_106b->y == local_102d[local_10ch] &&
                    (local_106b->turnTimer & 0x80) == 0 && (local_106b->flags & COMBAT_FLAGS_DEAD) == 0 && local_106b->flags != 0)
                {
                    local_106b->turnTimer |= 0x80;
                    switch (param_2)
                    {
                    case 1:
                        // e012
                        if (COMSUBS_0000(param_1, local_108a, 0) == 0 && CAST_0000(local_108a) == 0)
                        {
                            ULTIMA_3564(local_108a);
                            ULTIMA_68ae(local_108a);
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        }

                        break;

                    case 2:
                        // e03c
                        if (COMBAT_13e2(local_108a, QUERY_STAT_DEX) <= ULTIMA_3abe())
                        {
                            ULTIMA_3564(local_108a);
                            COMBAT_18ba(local_108a, param_1);
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        }

                        break;

                    case 3:
                        // e05c
                        ULTIMA_3564(local_108a);

                        ULTIMA_3f14_IncreaseInt(&local_104c->exp, COMBAT_1574(local_108a, ULTIMA_3aae_Random(0x1e)), 9999);
                        COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        break;

                    case 4:
                        // e07e
                        if (COMSUBS_0000(param_1, local_108a, 0) == 0 && CAST_0000(local_108a) == 0)
                        {
                            ULTIMA_3564(local_108a);

                            ULTIMA_3f14_IncreaseInt(&local_104c->exp, COMBAT_1574(local_108a, 99), 9999); // -> e070
                            COMSUBS_0312(local_108a, param_1); // -> e034 -> e0b2
                        }
                        break;
                    }

                    break;
                }
                // e0a4
            }
            // e0b2
        }

        // e0c8
        for (local_108a = 0x1f; local_108a >= 0; local_108a--)
        {
            // e0d1
            D_ba14[local_108a].turnTimer &= 0x7f;
        }
    }
}
