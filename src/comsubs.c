#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "combat.h"
#include "comsubs.h"
#include "sjog.h"

// this object's _BSS (DS:a728-a9bc): two lists of 0x14a bytes (COMSUBS_0e26 fills them,
// 0x100 bytes of each are cleared first). Statics are laid out as locals are, by the hash
// of their names; these two share a bucket, where the one declared last comes first.
static u8 D_a872[0x14a];
static u8 D_a728[0x14a];




int COMSUBS_0000(int param_1, int param_2, int param_3)
{
    int local_6_an;
    int local_4;

    if (param_3 == 0x30 || param_3 == 0x31 || param_3 >= 0x33)
        return 0;

    local_6_an = COMBAT_13e2(param_1, QUERY_STAT_INTEL);
    local_4 = COMBAT_13e2(param_2, QUERY_STAT_INTEL);
    if (ULTIMA_3abe_Random1To30() < (local_4 + 0x1e - local_6_an) / 2)
    {
        return 1;
    }

    return 0;
}

void COMSUBS_0056(void)
{
    int iVar2;

    for (iVar2 = 0; iVar2 < 0x20; iVar2++)
    {
        if ((D_5c5a_actors[iVar2].tile & 0xfc) == TILE_ACTOR_E8)
        {
            if (ULTIMA_2092_RandomRange(0, 0xff) < 0x10)
            {
                COMBAT_1236(iVar2 + 1);
            }
        }
    }
}

void COMSUBS_0094_PrintName(int param_1)
{
    if ((D_ba14_combatants[param_1].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        ULTIMA_1850_PrintString(D_55a8_party[D_ba14_combatants[param_1].entityIdx].name);
    }
    else
    {
        ULTIMA_1850_PrintString(D_1856_enemyNames[D_ba14_combatants[param_1].entityIdx]);
    }
}

static void COMSUBS_00d2(int param_1)
{
    if (D_588f != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x99a0, "Failed!\n"));
    }
    else
    {
        COMSUBS_0094_PrintName(param_1);
        ULTIMA_1850_PrintString(_TEXT(0x99aa, " missed!\n"));
    }
}

int COMSUBS_00f4(int param_1)
{
    int local_e_am;
    int local_c;
    int local_8;
    int local_6;
    int local_4_ab;
    ActorFmt* local_a;

    local_4_ab = D_ba14_combatants[param_1].entityIdx;
    local_c = D_ba14_combatants[param_1].actorIdx;
    if (D_587a_statusIcon == 0x1c || D_587a_statusIcon == 'N' || (D_ba14_combatants[param_1].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        return 0;
    }

    if (((byte)D_153c_enemyFlags[local_4_ab] & ENEMY_FLAG_CHARM) != 0)
    {
        local_e_am = ULTIMA_2092_RandomRange(0, 0x1f);

        local_6 = D_ba14_combatants[local_e_am].flags;
        if ((local_6 & COMBAT_FLAGS_PLAYER) != 0 &&
            (local_6 & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_INVISIBLE | COMBAT_FLAGS_ASLEEP | COMBAT_FLAGS_4 | COMBAT_FLAGS_CHARMED)) == 0)
        {
            if (COMSUBS_0000(param_1, local_e_am, 0) == 0)
            {
                ULTIMA_16ba_PrintChar('\n');

                D_ba14_combatants[local_e_am].flags |= COMBAT_FLAGS_CHARMED;
                if (D_ba14_combatants[local_e_am].entityIdx == D_587b_activePlayer)
                {
                    D_587b_activePlayer = 0xff;
                }

                ULTIMA_2900_UpdateVitalsDisplay();
                COMSUBS_0094_PrintName(local_e_am);
                ULTIMA_1850_PrintString(_TEXT(0x99b4, " possessed!\n"));
                ULTIMA_2192_AudioPulse(0xc1c, 1, 30000, 1000, 2);
                if (local_4_ab == 0x26)
                {
                    COMBAT_1236(-param_1 - 1);
                }
            }

            return 1;
        }
    }

    if ((D_153c_enemyFlags[local_4_ab] & ENEMY_FLAG_DISAPPEAR) != 0 && ULTIMA_2092_RandomRange(0, 0xff) < 0x20)
    {
        ULTIMA_16ba_PrintChar('\n');
        COMSUBS_0094_PrintName(param_1);
        if (D_5c5a_actors[local_c].animTile == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x99c2, " reappears!"));
            D_ba14_combatants[param_1].flags &= ~COMBAT_FLAGS_INVISIBLE;
            D_5c5a_actors[local_c].animTile = D_5c5a_actors[local_c].tile;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x99ce, " disappears!"));
            D_ba14_combatants[param_1].flags |= COMBAT_FLAGS_INVISIBLE;
            D_5c5a_actors[local_c].animTile = 0;
        }

        return 1;
    }

    if ((D_153c_enemyFlags[local_4_ab] & ENEMY_FLAG_SUMMON) != 0 && ULTIMA_2092_RandomRange(0, 0xff) < 0x20)
    {
        if (COMBAT_120e() != 0)
        {
            if (COMBAT_0000(TILE_ACTOR_D8, D_5876_tempX, D_5878_tempY) != 0)
            {
                local_8 = ULTIMA_6506_PlaceCombatant(0x26, 0, D_5876_tempX, D_5878_tempY, D_5895_mapLevel);
                if (local_8 != -1)
                {
                    ULTIMA_16ba_PrintChar('\n');
                    COMSUBS_0094_PrintName(param_1);
                    ULTIMA_1850_PrintString(_TEXT(0x99dc, " gates in a daemon!\n"));
                    ULTIMA_2192_AudioPulse(0xac8, 1, 5000, 1000, 0xf);

                    local_a = &D_5c5a_actors[D_ba14_combatants[local_8].actorIdx];

                    local_a->tile = local_a->animTile = TILE_ACTOR_CIRCLE;

                    ULTIMA_1068_RevealTile(0x1d8, D_5876_tempX, D_5878_tempY);

                    local_a->tile = local_a->animTile = TILE_ACTOR_D8;

                    return 1;
                }
            }
        }
    }

    return 0;
}

void COMSUBS_0312(int param_1, int param_2)
{
    int local_8; // unused
    int local_6;
    int local_4_ab;
    
    local_4_ab = D_ba14_combatants[param_1].flags;
    D_58a2 &= 0xfe;
    if ((D_58a2 & 0x20) != 0)
    {
        COMSUBS_0094_PrintName(param_1);
        ULTIMA_1850_PrintString(_TEXT(0x99f2, " grazed!\n"));
        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
    }

    if ((D_58a2 & 0x22) == 0)
    {
        if (local_4_ab == 0 || (local_4_ab & COMBAT_FLAGS_DEAD) != 0)
        {
            COMSUBS_0094_PrintName(param_1);
            ULTIMA_1850_PrintString(_TEXT(0x99fc, " killed!\n"));
            D_58a2 |= 1;
        }
        else if ((D_58a2 & 4) != 0)
        {
            COMSUBS_0094_PrintName(param_1);
            ULTIMA_1850_PrintString(_TEXT(0x9a06, " slept!\n"));
        }
        else if ((D_58a2 & 8) == 0)
        {
            COMSUBS_0094_PrintName(param_1);
            if ((local_4_ab & COMBAT_FLAGS_PLAYER) != 0)
            {
                if (param_2 != 0xff && D_ba14_combatants[param_2].entityIdx == 0x2d)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9a10, " dragged under!\n"));
                    ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
                    D_ba14_combatants[param_1].flags |= COMBAT_FLAGS_4;
                    D_5c5a_actors[D_ba14_combatants[param_1].actorIdx].animTile = 0;
                    ULTIMA_3ae6_WaitFrames(4);
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9a22, " hit!\n"));
                }
            }
            else
            {
                local_6 = COMBAT_1a5c(param_1);
                switch (local_6)
                {
                case 4:
                    ULTIMA_1850_PrintString(_TEXT(0x9a2a, " barely wounded!\n"));
                    break;
                case 3:
                    ULTIMA_1850_PrintString(_TEXT(0x9a3c, " lightly wounded!\n"));
                    break;
                case 2:
                    ULTIMA_1850_PrintString(_TEXT(0x9a50, " heavily wounded!\n"));
                    break;
                case 1:
                    ULTIMA_1850_PrintString(_TEXT(0x9a64, " critical!\n"));
                    break;
                }
            }
        }

        if ((local_4_ab & COMBAT_FLAGS_PLAYER) != 0)
        {
            ULTIMA_2900_UpdateVitalsDisplay();
            ULTIMA_5910_UpdateFrame();
        }

        D_58a2 &= 0xf3;
    }
    else
    {
        D_58a2 &= 0xdd;
    }
}

static int COMSUBS_0458(int param_1, int param_2, int param_3, int param_4)
{
    int local_6 = param_1 - param_3;
    int local_8 = param_2 - param_4;
    int local_4_ab = local_6 * local_6 + local_8 * local_8;
    return local_4_ab;
}

int COMSUBS_048a(int param_1, int param_2, int param_3, int param_4)
{
    int local_8;
    int local_6;
    int local_4_ab;

    local_8 = 0;
    local_4_ab = 1;

    local_6 = 0;

    local_8 = COMSUBS_0458(param_1, param_2, param_3, param_4);

    for (; local_8 >= local_4_ab;)
    {
        local_8 -= local_4_ab;
        local_4_ab++;
        local_4_ab++;
        local_6++;
    }

    return local_6;
}

int COMSUBS_04d4(int param_1, int param_2)
{
    return COMSUBS_048a(D_ba14_combatants[param_1].x, D_ba14_combatants[param_1].y, D_ba14_combatants[param_2].x, D_ba14_combatants[param_2].y);
}

int COMSUBS_0504(int param_1, int param_2)
{
    int local_14_am;
    int local_12_a;
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    int local_6;
    int local_4_ab;
    ActorFmt* local_8;

    D_5898_crosshairOn = 1;
    local_c = D_5c5a_actors[D_ba14_combatants[param_1].actorIdx].extra;
    local_8 = &D_5c5a_actors[D_ba14_combatants[local_c].actorIdx];

    if (local_c > 0x1f || (D_ba14_combatants[local_c].flags & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_INVISIBLE)) != 0 || D_ba14_combatants[local_c].flags == 0 ||
        local_8->tile == 0 || COMSUBS_04d4(param_1, local_c) > param_2)
    {
        // e742
        local_c = param_1;
    }

    // e748
    D_5899_crosshairX = D_ba14_combatants[local_c].x;
    D_589a_crosshairY = D_ba14_combatants[local_c].y;

    local_a = local_e_am = 0;

    while (!local_a && !local_e_am)
    {
        // e816
        local_10_aa = local_12_a = 0;

        ULTIMA_5910_UpdateFrame();

        switch (ULTIMA_266c_GetChar())
        {
        case U5_KEY_HOME: // 05c2
            local_10_aa = -1;
            local_12_a = -1;
            break;

        case U5_KEY_PGDN: // 066c
            local_10_aa = 1;
            local_12_a = 1;
            break;

        case U5_KEY_PGUP: // 0676
            local_12_a = -1;
            local_10_aa = 1;
            break;

        case U5_KEY_END: // 0680
            local_12_a = 1;
            local_10_aa = -1;
            break;

        case U5_KEY_UP: // 05c5
            local_12_a = -1;
            break;

        case U5_KEY_DOWN: // 066f
            local_12_a = 1;
            break;

        case U5_KEY_RIGHT: // 0679
            local_10_aa = 1;
            break;

        case U5_KEY_LEFT: // 0683
            local_10_aa = -1;
            break;

        case U5_KEY_SPACE: // 068a
            if ((D_ba14_combatants[param_1].x == D_5899_crosshairX) && (D_ba14_combatants[param_1].y == D_589a_crosshairY))
            {
                local_e_am = 1;
            }
            // fallthrough

        case U5_KEY_ENTER:  // 06a3
        case 0x41: // 06a3
            if (D_ba14_combatants[param_1].x != D_5899_crosshairX || D_ba14_combatants[param_1].y != D_589a_crosshairY)
            {
                ULTIMA_16ba_PrintChar('\n');
                if (D_588f != 0)
                {
                    ULTIMA_223c_AudioWhiteNoise(800, (uint)D_588f * 0x640 + 8000, 700);
                }
                local_a = 1;
            }
            break;

        case U5_KEY_ESC: // 06ea
            local_e_am = 1;
            //local_10_aa = local_12_a;
            break;
        }

        // 05c8
        local_4_ab = local_10_aa + D_5899_crosshairX;
        local_6 = local_12_a + D_589a_crosshairY;

        local_14_am = COMSUBS_048a(local_4_ab, local_6, D_ba14_combatants[param_1].x, D_ba14_combatants[param_1].y);
        if (local_14_am <= param_2 &&
            local_4_ab >= 0 && local_4_ab < 0xb &&
            local_6 >= 0 && local_6 < 0xb)
        {
            D_5899_crosshairX = local_4_ab;
            D_589a_crosshairY = local_6;
        }
    }

    D_5898_crosshairOn = 0;
    if (local_e_am)
    {
        ULTIMA_16ba_PrintChar('\n');
        return 0;
    }

    return local_14_am;
}

int COMSUBS_0748(int param_1, int param_2)
{
    int local_6;
    int local_4_ab; // flags
    int local_8;

    local_8 = -1;

    for (local_6 = 0; local_6 < 0x20; local_6++)
    {
        if (D_ba14_combatants[local_6].x == param_1 && D_ba14_combatants[local_6].y == param_2)
        {
            local_4_ab = D_ba14_combatants[local_6].flags;

            if (D_5c5a_actors[D_ba14_combatants[local_6].actorIdx].animTile != TILE_ACTOR_F4)
            {
                if ((local_4_ab & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) != 0 && (local_4_ab & COMBAT_FLAGS_DEAD) == 0)
                {
                    if ((local_4_ab & COMBAT_FLAGS_4) != 0)
                    {
                        // noop
                    }
                    else
                    {
                        local_8 = local_6;
                        break;
                    }
                }
            }

            local_8 = -1;
        }
    }

    return local_8;
}

void COMSUBS_07d4(int param_1, int param_2)
{
    do
    {
        D_5876_tempX = ULTIMA_2092_RandomRange(1, 3) + param_1 - 2;
        D_5878_tempY = ULTIMA_2092_RandomRange(1, 3) + param_2 - 2;
    } while (D_5876_tempX < 0 || D_5876_tempX > 10 || D_5878_tempY < 0 || D_5878_tempY > 10);
}

int COMSUBS_0822(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = param_2;
    local_c = param_3;
    local_4_ab = D_ba14_combatants[param_1].x;
    local_a = D_ba14_combatants[param_1].y;

    if (param_4 == 0)
    {
        do
        {
            COMSUBS_07d4(local_6, local_c);
            if (local_4_ab != D_5876_tempX)
                break;

        } while (local_a == D_5878_tempY);
        local_6 = D_5876_tempX;
        local_c = D_5878_tempY;
    }

    if (D_589d_attackWeapon == 25 || D_589d_attackWeapon == 34)
    {
        if (param_4 != 0)
        {
            local_6 = param_2;
            local_c = param_3;
            local_e_am = 1;
        }
    }
    else
    {
        local_e_am = COMSUBS_12de(local_4_ab, local_a, local_6, local_c, param_5);
        if (local_e_am != 0)
        {
            COMBAT_111a(local_6, local_c);
        }
        else
        {
            local_6 = D_5876_tempX;
            local_c = D_5878_tempY;
        }
    }

    if (local_e_am != 0)
    {
        local_8 = COMSUBS_0748(local_6, local_c);

        switch (D_589d_attackWeapon)
        {
        case 0x13:
        case 53:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_EA, 2, local_6, local_c, D_5895_mapLevel);
            break;
        case 51:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_E8, 2, local_6, local_c, D_5895_mapLevel);
            break;
        case 52:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_E9, 2, local_6, local_c, D_5895_mapLevel);
            break;
        case 54:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_EB, 2, local_6, local_c, D_5895_mapLevel);
            break;
        }

        if ((D_588f == 0 || param_4 != 0) && local_8 >= 0 && local_8 != D_589e_curCombatant)
        {
            D_5876_tempX = local_6;
            D_5878_tempY = local_c;
            return local_8;
        }
    }

    D_5876_tempX = local_6;
    D_5878_tempY = local_c;
    return -1;
}

static void COMSUBS_097c(int param_1, int param_2)
{
    if (D_588f != 0)
    {
        return;
    }

    switch (param_1)
    {
    case 0x1a:
    case 0x24:
        if (--D_57db_arrows == 0)
        {
            D_57c0_equipment[param_1] += SJOG_1b34(param_1);
        }
        break;

    case 0x1c:
        if (--D_57dd_quarrels == 0)
        {
            D_57c0_equipment[param_1] += SJOG_1b34(param_1);
        }
        break;

    case 0x10:
    case 0x15:
    case 0x16:
        if (param_2 <= 1)
        {
            break;
        }

        if (D_57c0_equipment[param_1] != 0)
        {
            D_57c0_equipment[param_1]--;
            break;
        }

        ULTIMA_6e60_Unequip(D_ba14_combatants[D_589e_curCombatant].entityIdx, param_1);
        break;
    }
}

int COMSUBS_09fc(int param_1)
{
    int uVar1;

    uVar1 = D_58a8[param_1];
    if (uVar1 != 0xff && D_ba14_combatants[uVar1].flags != 0 && ULTIMA_5646_IsEnemy(uVar1) != 0 && (D_ba14_combatants[uVar1].flags & (COMBAT_FLAGS_ASLEEP | COMBAT_FLAGS_4)) == 0 &&
        D_587a_statusIcon != 'T' && COMSUBS_04d4(param_1, uVar1) == 1)
    {
        ULTIMA_16ba_PrintChar('\n');
        COMSUBS_0094_PrintName(uVar1);
        ULTIMA_1850_PrintString(_TEXT(0x9a70, " interferes!\n"));
        return 1;
    }

    return 0;
}

static void COMSUBS_0a68(int param_1, int param_2, int param_3)
{
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_8;
    int local_6;
    int local_4_ab;
    ActorFmt* local_a;

    if (param_2 != 0x1a && param_2 != 0x1c && param_2 != 0x24 && param_2 != 0x13 && param_2 != 0x11 ||
        COMSUBS_09fc(param_1) == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9a7e, "Aim! "));

        local_10_aa = COMSUBS_0504(param_1, D_1664[param_2]);
        if (local_10_aa != 0)
        {
            ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);
            if (param_2 == 0x13)
            {
                if (D_57c0_equipment[param_2] != 0)
                {
                    D_57c0_equipment[param_2]--;
                }
                else
                {
                    ULTIMA_6e60_Unequip(D_ba14_combatants[D_589e_curCombatant].entityIdx, param_2);
                }
            }

            local_a = &D_5c5a_actors[D_ba14_combatants[param_1].actorIdx];
            local_a->extra = 0xff;
            local_c = local_e_am = COMSUBS_0748(D_5899_crosshairX, D_589a_crosshairY);
            if (local_c > -1)
            {
                local_a->extra = local_c;
                COMSUBS_097c(param_2, local_10_aa);
                local_4_ab = COMBAT_14d6(local_c, param_1, -D_588f, param_2);
                local_c = COMSUBS_0822(param_1, D_5899_crosshairX, D_589a_crosshairY, local_4_ab, param_3);
            }
            else
            {
                local_c = COMSUBS_0822(param_1, D_5899_crosshairX, D_589a_crosshairY, 1, param_3);
            }

            local_6 = D_5876_tempX;
            local_8 = D_5878_tempY;
            if (local_c > -1)
            {
                ULTIMA_3564_HitEffectOnActor(local_c);
                ULTIMA_16ba_PrintChar('\n');
                COMBAT_194a(local_c, param_1);
                COMSUBS_0312(local_c, param_1);
            }
            else if (local_e_am > -1)
            {
                ULTIMA_16ba_PrintChar('\n');
                COMSUBS_00d2(local_e_am);
            }

            if (param_2 == 0x26)
            {
                COMSUBS_12de(local_6, local_8, D_ba14_combatants[param_1].x, D_ba14_combatants[param_1].y, param_3);
            }
        }
    }
}

void COMSUBS_0bf8(int param_1, int param_2, int param_3)
{
    ULTIMA_43ae_AudioSweepTone(400, 0x2ee, 5, 0x96);
    ULTIMA_16ba_PrintChar('\n');
    if (COMBAT_14d6(param_2, param_1, -D_588f, param_3) != 0)
    {
        ULTIMA_3564_HitEffectOnActor(param_2);
        COMBAT_194a(param_2, param_1);
        COMSUBS_0312(param_2, param_1);
    }
    else
    {
        COMSUBS_00d2(param_2);
    }
}

void COMSUBS_0c52(int param_1, int param_2)
{
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    if (param_2 >= 0x23)
    {
        D_5890 = 1;
    }

    if ((D_ba14_combatants[param_1].flags & COMBAT_FLAGS_PLAYER) == 0)
    {
        local_a = D_159c[D_ba14_combatants[param_1].entityIdx];
        if (local_a == 1)
        {
            local_a = 0;
        }

        local_4_ab = D_15cc[D_ba14_combatants[param_1].entityIdx];
    }
    else
    {
        if (param_2 != 0xff)
        {
            local_a = D_1664[param_2];
        }
        else
        {
            local_a = 0;
        }

#if !defined(MATCHING_BUILD)
        // fix OOB when param_2 == 0xff (bare hands)
        if (param_2 != 0xff)
        {
            local_4_ab = D_169c[param_2];
        }
        else
        {
            local_4_ab = 0;
        }
#else
        local_4_ab = D_169c[param_2];
#endif
    }

    if (local_a == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9a84, "Aim! "));
        local_6 = COMSUBS_0504(param_1, 1);
        local_8 = COMSUBS_0748(D_5899_crosshairX, D_589a_crosshairY);
        if (local_6 == 0 || local_8 == -1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x9a8a, "Nothing!\n"));
        }
        else
        {
            D_5c5a_actors[D_ba14_combatants[param_1].actorIdx].extra = local_8;
            COMSUBS_0bf8(param_1, local_8, param_2);
        }
    }
    else
    {
        COMSUBS_0a68(param_1, param_2, local_4_ab);
    }
}

static void COMSUBS_0d3c(int param_1, int param_2)
{
    if (param_1 != 0xff && D_15fc[param_1] != 0)
    {
        if (param_2 > 1)
        {
            D_5890 = D_588f = 0;
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_1850_PrintString(D_17f6_equipNames[param_1]);
            ULTIMA_1850_PrintString(_TEXT(0x9a94, ":\n"));
        }

        D_589d_attackWeapon = (u8)param_1;
        ULTIMA_1850_PrintString(_TEXT(0x9a98, "Attack-"));
        COMSUBS_0c52(D_589e_curCombatant, param_1);
    }
}

// attack
void COMSUBS_0d96(int param_1, int param_2)
{
    int local_4;

    if ((D_ba14_combatants[param_1].flags & COMBAT_FLAGS_PLAYER) == 0 || param_2 == 0)
    {
        D_589d_attackWeapon = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x9aa0, "Attack-"));
        COMSUBS_0c52(param_1, 0xff);
    }
    else
    {
        local_4 = D_ba14_combatants[param_1].entityIdx;
        if (param_2 > 1)
        {
            ULTIMA_1bf2_SetTextPosition(0, ULTIMA_1cee_GetCurrentTextY());
            ULTIMA_16ba_PrintChar(' ');
        }

        COMSUBS_0d3c(D_55a8_party[local_4].equips[0], param_2);
        COMSUBS_0d3c(D_55a8_party[local_4].equips[2], param_2);
        COMSUBS_0d3c(D_55a8_party[local_4].equips[3], param_2);
    }
}

static void COMSUBS_0e26(int param_1, int param_2, int param_3, int param_4, u8* param_5, u8* param_6)
{
    int local_10_aa;
    int local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_6 = 1;
    local_8 = 1;
    local_c = 0x148;

    if (param_3 != param_1)
    {
        local_a = ((param_4 - param_2) * 100) / (param_3 - param_1);
    }
    else
    {
        local_a = 0x4b00;
    }

    if (local_a < 0)
    {
        local_a = -local_a;
    }

    if (param_3 < param_1)
    {
        local_6 = -1;
    }

    if (param_4 < param_2)
    {
        local_8 = -1;
    }

    local_10_aa = param_3 - param_1;
    if (local_10_aa < 0)
    {
        local_10_aa = -local_10_aa;
    }

    local_4_ab = param_4 - param_2;
    if (local_4_ab < 0)
    {
        local_4_ab = -local_4_ab;
    }

    // f08f
    local_e_am = local_a;

    *param_5 = param_1;
    param_5++;

    *param_6 = param_2;
    param_6++;

    while (local_10_aa > 0 || local_4_ab > 0)
    {
        while (local_e_am > 0 && local_4_ab > 0)
        {
            local_e_am -= 100;
            param_2 += local_8;

            *param_6 = param_2;
            param_6++;
            local_4_ab--;

            *param_5 = param_1;
            param_5++;
            local_c--;

            if (local_c == 0)
                break;
        }

        if (local_c == 0)
            break;

        local_10_aa--;
        local_e_am += local_a;

        param_1 += local_6;

        *param_6 = param_2;
        *param_5 = param_1;

        param_6++;
        param_5++;
        local_c--;
    }

    *param_5 = 0xff;
    *param_6 = 0xff;
}

static void COMSUBS_0f4a(byte param_1, byte param_2, int param_3, int param_4, u8* param_5, u8* param_6)
{
    int local_1e_ao;
    int local_1c_aa;
    int local_1a_ac;
    byte* local_18_al;
    int local_16_an;
    int local_14_ap;
    int local_12_ab;
    byte* local_10_ad;
    s8* local_e_a;
    int local_c_ac;
    s8* local_a_ae;
    int local_8_an;
    int local_6;
    int local_4_ab;

    local_1c_aa = 1;
    local_1e_ao = 1;
    local_4_ab = -1;

#if !defined(MATCHING_BUILD)
    local_e_a = NULL;
    local_a_ae = NULL;
#endif

    if (param_4 == 2)
    {
        local_a_ae = local_e_a = D_2188;
    }

    if (param_4 > 2)
    {
        local_a_ae = local_e_a = D_2188 + 0x10;
    }

    switch (param_3)
    {
    case 1: // f178
        local_e_a++;
        break;
    case 2: // f196
        local_a_ae++;
        local_1c_aa = -1;
        break;
    case 3: // f1a0
        local_e_a++;
        local_1c_aa = local_1e_ao = -1;
        break;
    case 4: // f1ae
        local_a_ae++;
        local_1e_ao = -1;
        break;
    }

    // f17b
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    switch (param_4)
    {
    case 0: // f1b8
        local_10_ad = param_5 - 4;
        local_18_al = param_6 - 4;
        for (local_16_an = 0; local_16_an < 9; local_16_an++)
        {
            if (*local_10_ad == 0xff)
                break;

            ULTIMA_0b10_GRAP_Line(*local_10_ad, *local_18_al, *local_10_ad + 1, *local_18_al);
            local_10_ad++;
            local_18_al++;
        }
        break;

    case 1: // f206
        ULTIMA_0b10_GRAP_Line(param_1 - 1, param_2 - 2, param_1 + 1, param_2 - 2);
        ULTIMA_0b10_GRAP_Line(param_1 - 2, param_2 - 1, param_1 + 2, param_2 - 1);
        ULTIMA_0b10_GRAP_Line(param_1 - 2, param_2, param_1 + 2, param_2);
        ULTIMA_0b10_GRAP_Line(param_1 - 2, param_2 + 1, param_1 + 2, param_2 + 1);
        ULTIMA_0b10_GRAP_Line(param_1 - 1, param_2 + 2, param_1 + 1, param_2 + 2);
        break;

    case 2: // f27e
        for (local_16_an = 0; local_16_an < 4; local_16_an++)
        {
            local_c_ac = *local_a_ae * local_1c_aa + param_1;
            local_a_ae += 2;
            local_14_ap = *local_e_a * local_1e_ao + param_2;
            local_e_a += 2;
            local_12_ab = *local_a_ae * local_1c_aa + param_1;
            local_a_ae += 2;
            local_1a_ac = *local_e_a * local_1e_ao + param_2;
            local_e_a += 2;
            ULTIMA_0b10_GRAP_Line(local_c_ac, local_14_ap, local_12_ab, local_1a_ac);
        }
        break;

    case 3: // f2f4
        local_4_ab = D_13ae_redColor + 8;
        break;

    case 4: // f300
        local_4_ab = D_13b2_blueColor + 8;
        break;

    case 5: // f306
        local_4_ab = D_13b4_greenColor + 8;
        break;

    case 6: // f30c
        local_4_ab = D_13b6_magentaColor + 8;
        break;

    case 7: // f312
        for (local_16_an = 0; local_16_an < 7; local_16_an++)
        {
            local_c_ac = *local_a_ae * local_1c_aa + param_1;
            local_a_ae += 2;
            local_14_ap = *local_e_a * local_1e_ao + param_2;
            local_e_a += 2;
            local_12_ab = *local_a_ae * local_1c_aa + param_1;
            local_a_ae += 2;
            local_1a_ac = *local_e_a * local_1e_ao + param_2;
            local_e_a += 2;
            ULTIMA_0b10_GRAP_Line(local_c_ac, local_14_ap, local_12_ab, local_1a_ac);
        }

        // f384
        ULTIMA_0c64_GRAP_30_Pset(*local_a_ae * local_1c_aa + param_1, *local_e_a * local_1e_ao + param_2);
        local_a_ae += 2;
        local_e_a += 2;
        ULTIMA_0c64_GRAP_30_Pset(*local_a_ae * local_1c_aa + param_1, *local_e_a * local_1e_ao + param_2);
        local_a_ae += 2;
        local_e_a += 2;
        ULTIMA_0c64_GRAP_30_Pset(*local_a_ae * local_1c_aa + param_1, *local_e_a * local_1e_ao + param_2);
        break;
    }

    if (local_4_ab > -1)
    {
        // f40d
        local_1c_aa = -1;
        local_1e_ao = 1;
        for (local_16_an = 0; local_16_an < 0x30; local_16_an++)
        {
            // f42b
            local_10_ad = local_18_al = D_21ba;

            local_10_ad += ULTIMA_2092_RandomRange(0, 0xf);
            local_18_al += ULTIMA_2092_RandomRange(0, 0xf);

            local_6 = ULTIMA_2092_RandomRange(0, 1);
            local_8_an = ULTIMA_2092_RandomRange(0, 1);

            if (local_6 != 0)
            {
                local_1c_aa = -local_1c_aa;
            }

            if (local_8_an != 0)
            {
                local_1e_ao = -local_1e_ao;
            }

            ULTIMA_0a70_GRAP_2d_SetPenColor(local_4_ab);
            ULTIMA_0c64_GRAP_30_Pset(*local_10_ad * local_1c_aa + param_1, *local_18_al * local_1e_ao + param_2);
        }
    }
}

int COMSUBS_12de(int param_1, int param_2, int param_3, int param_4, int param_5)
{
    byte* local_12_a;
    int local_10_aa;
    byte* local_e_am;
    int local_c;
    int local_a;
    int local_8;
    int local_6;
    int local_4_ab;

    local_c = 0xd;
    local_a = 0;
    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(0, 1);

    memset(D_a728, 0xff, 0x100);
    memset(D_a872, 0xff, 0x100);

    local_10_aa = param_1 * 0x10 + 0x10;
    local_6 = param_2 * 0x10 + 0x10;
    local_4_ab = param_3 * 0x10 + 0x10;
    local_8 = param_4 * 0x10 + 0x10;
    COMSUBS_0e26(local_10_aa, local_6, local_4_ab, local_8, D_a728, D_a872);

    local_e_am = D_a728;
    local_12_a = D_a872;

    if (D_5893_mapId != 0 && D_5893_mapId < 0x21)
    {
        local_c = 6;
    }
    else
    {
        if (param_5 == 7)
        {
            local_c = 8;
        }
    }

    if (param_5 == 0)
    {
        local_e_am += 4;
        local_12_a += 4;
    }

    while (*local_e_am != 0xff)
    {
        local_a = (local_a & 3) + 1;
        ULTIMA_3fb4_PixelToViewCell(*local_e_am, *local_12_a);
        if (D_5876_tempX == -1)
        {
            break;
        }

        COMSUBS_0f4a(*local_e_am, *local_12_a, local_a, param_5, local_e_am, local_12_a);
        ULTIMA_20c8_Delay(1, 0x28);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, (*local_e_am & 0xf8) - 8, *local_12_a - 8, (*local_e_am & 0xf8) + 0xf, *local_12_a + 0xf);

        local_e_am += local_c;
        local_12_a += local_c;

#if defined(MATCHING_BUILD)
        if (D_a872 < local_e_am) // the end of D_a728 (as D_a728 + sizeof(D_a728) the code is 2 bytes shorter)
        {
            break;
        }
#else
        if (D_a728 + sizeof(D_a728) < local_e_am)
        {
            break;
        }
#endif

        if (*local_e_am == 0xff)
        {
            break;
        }

        if (ULTIMA_3f6e_TestViewTileFlag(D_5876_tempX, D_5878_tempY) == 0 && (param_1 != D_5876_tempX || param_2 != D_5878_tempY))
        {
            return 0;
        }
    }

    return 1;
}
