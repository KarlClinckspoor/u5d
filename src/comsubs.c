#include "common/common.h"
#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "combat.h"
#include "comsubs.h"
#include "sjog.h"

// this object's _BSS (DS:a728-a9bc): two lists of 0x14a bytes (COMSUBS_0e26_TracePath fills them,
// 0x100 bytes of each are cleared first). Statics are laid out as locals are, by the hash
// of their names; these two share a bucket, where the one declared last comes first.
static u8 D_a872_pathYs[0x14a];
static u8 D_a728_pathXs[0x14a];




// Intelligence contest of attacker and target (the spell kinds 0x30, 0x31 and 0x33 up never fail): 1 when the target
// resists, so that a spell or blow with a side effect (sleep, charm) takes hold on 0.
int COMSUBS_0000_TargetResists(int attacker, int target, int weaponKind)
{
    int attackerInt;
    int targetInt;

    if (weaponKind == 0x30 || weaponKind == 0x31 || weaponKind >= 0x33)
        return 0;

    attackerInt = COMBAT_13e2(attacker, QUERY_STAT_INTEL);
    targetInt = COMBAT_13e2(target, QUERY_STAT_INTEL);
    if (ULTIMA_3abe_Random1To30() < (targetInt + 0x1e - attackerInt) / 2)
    {
        return 1;
    }

    return 0;
}

// Called every turn: each field tile (E8-EB: the spell fields) on the actor table has 16 in 256 to vanish.
void COMSUBS_0056_FadeFields(void)
{
    int actorIdx;

    for (actorIdx = 0; actorIdx < 0x20; actorIdx++)
    {
        if ((D_5c5a_actors[actorIdx].tile & 0xfc) == TILE_ACTOR_E8)
        {
            if (ULTIMA_2092_RandomRange(0, 0xff) < 0x10)
            {
                COMBAT_1236(actorIdx + 1);
            }
        }
    }
}

// Prints the name of a combatant: a party member's name, or the monster's kind.
void COMSUBS_0094_PrintName(int combatant)
{
    if ((D_ba14_combatants[combatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        ULTIMA_1850_PrintString(D_55a8_party[D_ba14_combatants[combatant].entityIdx].name);
    }
    else
    {
        ULTIMA_1850_PrintString(D_1856_enemyNames[D_ba14_combatants[combatant].entityIdx]);
    }
}

// Prints that an attack found nothing: "Failed!" for a spell, otherwise "<name> missed!".
static void COMSUBS_00d2_PrintMiss(int combatant)
{
    if (D_588f != 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x99a0, "Failed!\n"));
    }
    else
    {
        COMSUBS_0094_PrintName(combatant);
        ULTIMA_1850_PrintString(_TEXT(0x99aa, " missed!\n"));
    }
}

// A monster's own tricks before it fights: possessing a party member (ENEMY_FLAG_CHARM), vanishing and reappearing
// (ENEMY_FLAG_DISAPPEAR), gating in a daemon (ENEMY_FLAG_SUMMON). 1 when it used its turn on one.
int COMSUBS_00f4_MonsterSpecial(int combatant)
{
    int victim_c;
    int actorIdx;
    int daemon;
    int victimFlags_h;
    int enemyType;
    ActorFmt* daemonActor;

    enemyType = D_ba14_combatants[combatant].entityIdx;
    actorIdx = D_ba14_combatants[combatant].actorIdx;
    if (D_587a_statusIcon == 0x1c || D_587a_statusIcon == 'N' || (D_ba14_combatants[combatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        return 0;
    }

    if (((byte)D_153c_enemyFlags[enemyType] & ENEMY_FLAG_CHARM) != 0)
    {
        victim_c = ULTIMA_2092_RandomRange(0, 0x1f);

        victimFlags_h = D_ba14_combatants[victim_c].flags;
        if ((victimFlags_h & COMBAT_FLAGS_PLAYER) != 0 &&
            (victimFlags_h & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_INVISIBLE | COMBAT_FLAGS_ASLEEP | COMBAT_FLAGS_4 | COMBAT_FLAGS_CHARMED)) == 0)
        {
            if (COMSUBS_0000_TargetResists(combatant, victim_c, 0) == 0)
            {
                ULTIMA_16ba_PrintChar('\n');

                D_ba14_combatants[victim_c].flags |= COMBAT_FLAGS_CHARMED;
                if (D_ba14_combatants[victim_c].entityIdx == D_587b_activePlayer)
                {
                    D_587b_activePlayer = 0xff;
                }

                ULTIMA_2900_UpdateVitalsDisplay();
                COMSUBS_0094_PrintName(victim_c);
                ULTIMA_1850_PrintString(_TEXT(0x99b4, " possessed!\n"));
                ULTIMA_2192_AudioPulse(0xc1c, 1, 30000, 1000, 2);
                if (enemyType == 0x26)
                {
                    COMBAT_1236(-combatant - 1);
                }
            }

            return 1;
        }
    }

    if ((D_153c_enemyFlags[enemyType] & ENEMY_FLAG_DISAPPEAR) != 0 && ULTIMA_2092_RandomRange(0, 0xff) < 0x20)
    {
        ULTIMA_16ba_PrintChar('\n');
        COMSUBS_0094_PrintName(combatant);
        if (D_5c5a_actors[actorIdx].animTile == 0)
        {
            ULTIMA_1850_PrintString(_TEXT(0x99c2, " reappears!"));
            D_ba14_combatants[combatant].flags &= ~COMBAT_FLAGS_INVISIBLE;
            D_5c5a_actors[actorIdx].animTile = D_5c5a_actors[actorIdx].tile;
        }
        else
        {
            ULTIMA_1850_PrintString(_TEXT(0x99ce, " disappears!"));
            D_ba14_combatants[combatant].flags |= COMBAT_FLAGS_INVISIBLE;
            D_5c5a_actors[actorIdx].animTile = 0;
        }

        return 1;
    }

    if ((D_153c_enemyFlags[enemyType] & ENEMY_FLAG_SUMMON) != 0 && ULTIMA_2092_RandomRange(0, 0xff) < 0x20)
    {
        if (COMBAT_120e() != 0)
        {
            if (COMBAT_0000(TILE_ACTOR_D8, D_5876_tempX, D_5878_tempY) != 0)
            {
                daemon = ULTIMA_6506_PlaceCombatant(0x26, 0, D_5876_tempX, D_5878_tempY, D_5895_mapLevel);
                if (daemon != -1)
                {
                    ULTIMA_16ba_PrintChar('\n');
                    COMSUBS_0094_PrintName(combatant);
                    ULTIMA_1850_PrintString(_TEXT(0x99dc, " gates in a daemon!\n"));
                    ULTIMA_2192_AudioPulse(0xac8, 1, 5000, 1000, 0xf);

                    daemonActor = &D_5c5a_actors[D_ba14_combatants[daemon].actorIdx];

                    daemonActor->tile = daemonActor->animTile = TILE_ACTOR_CIRCLE;

                    ULTIMA_1068_RevealTile(0x1d8, D_5876_tempX, D_5878_tempY);

                    daemonActor->tile = daemonActor->animTile = TILE_ACTOR_D8;

                    return 1;
                }
            }
        }
    }

    return 0;
}

// Says what a blow did to the target, by the result bits in D_58a2: grazed, killed, slept, dragged under (by the
// Corpser, 0x2d), hit or how wounded a monster is (COMBAT_1a5c).
void COMSUBS_0312_ReportHit(int target, int attacker)
{
    int local_8; // unused
    int woundLevel_j;
    int targetFlags_k;
    
    targetFlags_k = D_ba14_combatants[target].flags;
    D_58a2 &= 0xfe;
    if ((D_58a2 & 0x20) != 0)
    {
        COMSUBS_0094_PrintName(target);
        ULTIMA_1850_PrintString(_TEXT(0x99f2, " grazed!\n"));
        ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
    }

    if ((D_58a2 & 0x22) == 0)
    {
        if (targetFlags_k == 0 || (targetFlags_k & COMBAT_FLAGS_DEAD) != 0)
        {
            COMSUBS_0094_PrintName(target);
            ULTIMA_1850_PrintString(_TEXT(0x99fc, " killed!\n"));
            D_58a2 |= 1;
        }
        else if ((D_58a2 & 4) != 0)
        {
            COMSUBS_0094_PrintName(target);
            ULTIMA_1850_PrintString(_TEXT(0x9a06, " slept!\n"));
        }
        else if ((D_58a2 & 8) == 0)
        {
            COMSUBS_0094_PrintName(target);
            if ((targetFlags_k & COMBAT_FLAGS_PLAYER) != 0)
            {
                if (attacker != 0xff && D_ba14_combatants[attacker].entityIdx == 0x2d)
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9a10, " dragged under!\n"));
                    ULTIMA_43ae_AudioSweepTone(0x4b0, 2000, 1, 0x28);
                    D_ba14_combatants[target].flags |= COMBAT_FLAGS_4;
                    D_5c5a_actors[D_ba14_combatants[target].actorIdx].animTile = 0;
                    ULTIMA_3ae6_WaitFrames(4);
                }
                else
                {
                    ULTIMA_1850_PrintString(_TEXT(0x9a22, " hit!\n"));
                }
            }
            else
            {
                woundLevel_j = COMBAT_1a5c(target);
                switch (woundLevel_j)
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

        if ((targetFlags_k & COMBAT_FLAGS_PLAYER) != 0)
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

// Squared distance between two cells.
static int COMSUBS_0458_DistanceSquared(int fromX, int fromY, int toX, int toY)
{
    int dx = fromX - toX;
    int dy = fromY - toY;
    int sum = dx * dx + dy * dy;
    return sum;
}

// Distance between two cells: the integer square root of the squared distance.
int COMSUBS_048a_Distance(int fromX, int fromY, int toX, int toY)
{
    int remainder;
    int root;
    int odd_i;

    remainder = 0;
    odd_i = 1;

    root = 0;

    remainder = COMSUBS_0458_DistanceSquared(fromX, fromY, toX, toY);

    for (; remainder >= odd_i;)
    {
        remainder -= odd_i;
        odd_i++;
        odd_i++;
        root++;
    }

    return root;
}

// Distance between two combatants.
int COMSUBS_04d4_CombatantDistance(int fromIdx, int toIdx)
{
    return COMSUBS_048a_Distance(D_ba14_combatants[fromIdx].x, D_ba14_combatants[fromIdx].y, D_ba14_combatants[toIdx].x, D_ba14_combatants[toIdx].y);
}

// Aiming: the crosshair starts on the target the shooter's actor remembers (else on the shooter), moves by the arrow
// keys within maxRange, and is taken with Enter or Space. Returns the distance to the cell, or 0 when cancelled with Esc.
int COMSUBS_0504_AimCrosshair(int shooter, int maxRange)
{
    int distance;
    int stepY;
    int stepX;
    int cancelled_h;
    int firstTarget_d;
    int confirmed_l;
    int newY;
    int newX;
    ActorFmt* targetActor_c;

    D_5898_crosshairOn = 1;
    firstTarget_d = D_5c5a_actors[D_ba14_combatants[shooter].actorIdx].extra;
    targetActor_c = &D_5c5a_actors[D_ba14_combatants[firstTarget_d].actorIdx];

    if (firstTarget_d > 0x1f || (D_ba14_combatants[firstTarget_d].flags & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_INVISIBLE)) != 0 || D_ba14_combatants[firstTarget_d].flags == 0 ||
        targetActor_c->tile == 0 || COMSUBS_04d4_CombatantDistance(shooter, firstTarget_d) > maxRange)
    {
        // e742
        firstTarget_d = shooter;
    }

    // e748
    D_5899_crosshairX = D_ba14_combatants[firstTarget_d].x;
    D_589a_crosshairY = D_ba14_combatants[firstTarget_d].y;

    confirmed_l = cancelled_h = 0;

    while (!confirmed_l && !cancelled_h)
    {
        // e816
        stepX = stepY = 0;

        ULTIMA_5910_UpdateFrame();

        switch (ULTIMA_266c_GetChar())
        {
        case U5_KEY_HOME: // 05c2
            stepX = -1;
            stepY = -1;
            break;

        case U5_KEY_PGDN: // 066c
            stepX = 1;
            stepY = 1;
            break;

        case U5_KEY_PGUP: // 0676
            stepY = -1;
            stepX = 1;
            break;

        case U5_KEY_END: // 0680
            stepY = 1;
            stepX = -1;
            break;

        case U5_KEY_UP: // 05c5
            stepY = -1;
            break;

        case U5_KEY_DOWN: // 066f
            stepY = 1;
            break;

        case U5_KEY_RIGHT: // 0679
            stepX = 1;
            break;

        case U5_KEY_LEFT: // 0683
            stepX = -1;
            break;

        case U5_KEY_SPACE: // 068a
            if ((D_ba14_combatants[shooter].x == D_5899_crosshairX) && (D_ba14_combatants[shooter].y == D_589a_crosshairY))
            {
                cancelled_h = 1;
            }
            // fallthrough

        case U5_KEY_ENTER:  // 06a3
        case 0x41: // 06a3
            if (D_ba14_combatants[shooter].x != D_5899_crosshairX || D_ba14_combatants[shooter].y != D_589a_crosshairY)
            {
                ULTIMA_16ba_PrintChar('\n');
                if (D_588f != 0)
                {
                    ULTIMA_223c_AudioWhiteNoise(800, (uint)D_588f * 0x640 + 8000, 700);
                }
                confirmed_l = 1;
            }
            break;

        case U5_KEY_ESC: // 06ea
            cancelled_h = 1;
            //stepX = stepY;
            break;
        }

        // 05c8
        newX = stepX + D_5899_crosshairX;
        newY = stepY + D_589a_crosshairY;

        distance = COMSUBS_048a_Distance(newX, newY, D_ba14_combatants[shooter].x, D_ba14_combatants[shooter].y);
        if (distance <= maxRange &&
            newX >= 0 && newX < 0xb &&
            newY >= 0 && newY < 0xb)
        {
            D_5899_crosshairX = newX;
            D_589a_crosshairY = newY;
        }
    }

    D_5898_crosshairOn = 0;
    if (cancelled_h)
    {
        ULTIMA_16ba_PrintChar('\n');
        return 0;
    }

    return distance;
}

// The combatant standing on a cell (not dead, not swallowed), else -1.
int COMSUBS_0748_CombatantAt(int cellX, int cellY)
{
    int combatant;
    int flags_c; // flags
    int found;

    found = -1;

    for (combatant = 0; combatant < 0x20; combatant++)
    {
        if (D_ba14_combatants[combatant].x == cellX && D_ba14_combatants[combatant].y == cellY)
        {
            flags_c = D_ba14_combatants[combatant].flags;

            if (D_5c5a_actors[D_ba14_combatants[combatant].actorIdx].animTile != TILE_ACTOR_F4)
            {
                if ((flags_c & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) != 0 && (flags_c & COMBAT_FLAGS_DEAD) == 0)
                {
                    if ((flags_c & COMBAT_FLAGS_4) != 0)
                    {
                        // noop
                    }
                    else
                    {
                        found = combatant;
                        break;
                    }
                }
            }

            found = -1;
        }
    }

    return found;
}

// A random cell within one step of (centerX, centerY) on the 11x11 combat map, left in D_5876_tempX and D_5878_tempY.
void COMSUBS_07d4_PickNearbyCell(int centerX, int centerY)
{
    do
    {
        D_5876_tempX = ULTIMA_2092_RandomRange(1, 3) + centerX - 2;
        D_5878_tempY = ULTIMA_2092_RandomRange(1, 3) + centerY - 2;
    } while (D_5876_tempX < 0 || D_5876_tempX > 10 || D_5878_tempY < 0 || D_5878_tempY > 10);
}

// Resolves a shot or spell at a cell: a miss lands on a nearby cell, the missile is flown (COMSUBS_12de), the spell
// fields are laid where it ends. Returns the combatant it hits, else -1; the cell it ended on stays in D_5876/D_5878.
int COMSUBS_0822_ResolveShot(int shooter, int aimX, int aimY, int onTarget, int missileKind)
{
    int landed;
    int endY_h;
    int startY;
    int victim_k;
    int endX_h;
    int startX;

    endX_h = aimX;
    endY_h = aimY;
    startX = D_ba14_combatants[shooter].x;
    startY = D_ba14_combatants[shooter].y;

    if (onTarget == 0)
    {
        do
        {
            COMSUBS_07d4_PickNearbyCell(endX_h, endY_h);
            if (startX != D_5876_tempX)
                break;

        } while (startY == D_5878_tempY);
        endX_h = D_5876_tempX;
        endY_h = D_5878_tempY;
    }

    if (D_589d_attackWeapon == 25 || D_589d_attackWeapon == 34)
    {
        if (onTarget != 0)
        {
            endX_h = aimX;
            endY_h = aimY;
            landed = 1;
        }
    }
    else
    {
        landed = COMSUBS_12de_FlyMissile(startX, startY, endX_h, endY_h, missileKind);
        if (landed != 0)
        {
            COMBAT_111a(endX_h, endY_h);
        }
        else
        {
            endX_h = D_5876_tempX;
            endY_h = D_5878_tempY;
        }
    }

    if (landed != 0)
    {
        victim_k = COMSUBS_0748_CombatantAt(endX_h, endY_h);

        switch (D_589d_attackWeapon)
        {
        case 0x13:
        case 53:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_EA, 2, endX_h, endY_h, D_5895_mapLevel);
            break;
        case 51:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_E8, 2, endX_h, endY_h, D_5895_mapLevel);
            break;
        case 52:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_E9, 2, endX_h, endY_h, D_5895_mapLevel);
            break;
        case 54:
            ULTIMA_6506_PlaceCombatant(TILE_ACTOR_EB, 2, endX_h, endY_h, D_5895_mapLevel);
            break;
        }

        if ((D_588f == 0 || onTarget != 0) && victim_k >= 0 && victim_k != D_589e_curCombatant)
        {
            D_5876_tempX = endX_h;
            D_5878_tempY = endY_h;
            return victim_k;
        }
    }

    D_5876_tempX = endX_h;
    D_5878_tempY = endY_h;
    return -1;
}

// Bookkeeping of a shot by a party member: an arrow or a quarrel is used (when the count runs out it is recounted from
// the members who still hold some, SJOG_1b34), a thrown weapon is taken from the stack or unequipped. Nothing is spent for a spell.
static void COMSUBS_097c_SpendAmmo(int weapon, int range)
{
    if (D_588f != 0)
    {
        return;
    }

    switch (weapon)
    {
    case 0x1a:
    case 0x24:
        if (--D_57db_arrows == 0)
        {
            D_57c0_equipment[weapon] += SJOG_1b34(weapon);
        }
        break;

    case 0x1c:
        if (--D_57dd_quarrels == 0)
        {
            D_57c0_equipment[weapon] += SJOG_1b34(weapon);
        }
        break;

    case 0x10:
    case 0x15:
    case 0x16:
        if (range <= 1)
        {
            break;
        }

        if (D_57c0_equipment[weapon] != 0)
        {
            D_57c0_equipment[weapon]--;
            break;
        }

        ULTIMA_6e60_Unequip(D_ba14_combatants[D_589e_curCombatant].entityIdx, weapon);
        break;
    }
}

// 1 when an adjacent enemy that last attacked this combatant (D_58a8) interferes and spoils the attack.
int COMSUBS_09fc_EnemyInterferes(int combatant)
{
    int attacker;

    attacker = D_58a8[combatant];
    if (attacker != 0xff && D_ba14_combatants[attacker].flags != 0 && ULTIMA_5646_IsEnemy(attacker) != 0 && (D_ba14_combatants[attacker].flags & (COMBAT_FLAGS_ASLEEP | COMBAT_FLAGS_4)) == 0 &&
        D_587a_statusIcon != 'T' && COMSUBS_04d4_CombatantDistance(combatant, attacker) == 1)
    {
        ULTIMA_16ba_PrintChar('\n');
        COMSUBS_0094_PrintName(attacker);
        ULTIMA_1850_PrintString(_TEXT(0x9a70, " interferes!\n"));
        return 1;
    }

    return 0;
}

// A ranged attack: aim within the weapon's range, spend the ammunition, roll to hit, fly the missile and report
// the hit or the miss.
static void COMSUBS_0a68_FireMissile(int shooter, int weapon, int missileKind)
{
    int range_b;
    int aimedAt_i;
    int victim_c;
    int hitY;
    int hitX;
    int onTarget;
    ActorFmt* shooterActor;

    if (weapon != 0x1a && weapon != 0x1c && weapon != 0x24 && weapon != 0x13 && weapon != 0x11 ||
        COMSUBS_09fc_EnemyInterferes(shooter) == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9a7e, "Aim! "));

        range_b = COMSUBS_0504_AimCrosshair(shooter, D_1664[weapon]);
        if (range_b != 0)
        {
            ULTIMA_43ae_AudioSweepTone(0x514, 300, 5, 100);
            if (weapon == 0x13)
            {
                if (D_57c0_equipment[weapon] != 0)
                {
                    D_57c0_equipment[weapon]--;
                }
                else
                {
                    ULTIMA_6e60_Unequip(D_ba14_combatants[D_589e_curCombatant].entityIdx, weapon);
                }
            }

            shooterActor = &D_5c5a_actors[D_ba14_combatants[shooter].actorIdx];
            shooterActor->extra = 0xff;
            victim_c = aimedAt_i = COMSUBS_0748_CombatantAt(D_5899_crosshairX, D_589a_crosshairY);
            if (victim_c > -1)
            {
                shooterActor->extra = victim_c;
                COMSUBS_097c_SpendAmmo(weapon, range_b);
                onTarget = COMBAT_14d6(victim_c, shooter, -D_588f, weapon);
                victim_c = COMSUBS_0822_ResolveShot(shooter, D_5899_crosshairX, D_589a_crosshairY, onTarget, missileKind);
            }
            else
            {
                victim_c = COMSUBS_0822_ResolveShot(shooter, D_5899_crosshairX, D_589a_crosshairY, 1, missileKind);
            }

            hitX = D_5876_tempX;
            hitY = D_5878_tempY;
            if (victim_c > -1)
            {
                ULTIMA_3564_HitEffectOnActor(victim_c);
                ULTIMA_16ba_PrintChar('\n');
                COMBAT_194a(victim_c, shooter);
                COMSUBS_0312_ReportHit(victim_c, shooter);
            }
            else if (aimedAt_i > -1)
            {
                ULTIMA_16ba_PrintChar('\n');
                COMSUBS_00d2_PrintMiss(aimedAt_i);
            }

            if (weapon == 0x26)
            {
                COMSUBS_12de_FlyMissile(hitX, hitY, D_ba14_combatants[shooter].x, D_ba14_combatants[shooter].y, missileKind);
            }
        }
    }
}

// A hand-to-hand blow at a victim: roll to hit, damage and report, or the miss.
void COMSUBS_0bf8_MeleeBlow(int attacker, int victim, int weapon)
{
    ULTIMA_43ae_AudioSweepTone(400, 0x2ee, 5, 0x96);
    ULTIMA_16ba_PrintChar('\n');
    if (COMBAT_14d6(victim, attacker, -D_588f, weapon) != 0)
    {
        ULTIMA_3564_HitEffectOnActor(victim);
        COMBAT_194a(victim, attacker);
        COMSUBS_0312_ReportHit(victim, attacker);
    }
    else
    {
        COMSUBS_00d2_PrintMiss(victim);
    }
}

// The attack of one combatant with one weapon (0xff: bare hands): a blow when its range is 0, else a missile attack.
// The range and the missile kind come from the weapon tables for a party member, from the monster tables otherwise.
void COMSUBS_0c52_AttackWith(int attacker, int weapon)
{
    int range;
    int victim;
    int aimed;
    int missileKind_c;

    if (weapon >= 0x23)
    {
        D_5890 = 1;
    }

    if ((D_ba14_combatants[attacker].flags & COMBAT_FLAGS_PLAYER) == 0)
    {
        range = D_159c[D_ba14_combatants[attacker].entityIdx];
        if (range == 1)
        {
            range = 0;
        }

        missileKind_c = D_15cc[D_ba14_combatants[attacker].entityIdx];
    }
    else
    {
        if (weapon != 0xff)
        {
            range = D_1664[weapon];
        }
        else
        {
            range = 0;
        }

#if !defined(MATCHING_BUILD)
        // fix OOB when weapon == 0xff (bare hands)
        if (weapon != 0xff)
        {
            missileKind_c = D_169c[weapon];
        }
        else
        {
            missileKind_c = 0;
        }
#else
        missileKind_c = D_169c[weapon];
#endif
    }

    if (range == 0)
    {
        ULTIMA_1850_PrintString(_TEXT(0x9a84, "Aim! "));
        aimed = COMSUBS_0504_AimCrosshair(attacker, 1);
        victim = COMSUBS_0748_CombatantAt(D_5899_crosshairX, D_589a_crosshairY);
        if (aimed == 0 || victim == -1)
        {
            ULTIMA_1850_PrintString(_TEXT(0x9a8a, "Nothing!\n"));
        }
        else
        {
            D_5c5a_actors[D_ba14_combatants[attacker].actorIdx].extra = victim;
            COMSUBS_0bf8_MeleeBlow(attacker, victim, weapon);
        }
    }
    else
    {
        COMSUBS_0a68_FireMissile(attacker, weapon, missileKind_c);
    }
}

// The attack with one readied weapon (0xff: none); when the member holds several, the weapon is named first.
static void COMSUBS_0d3c_AttackWithWeapon(int weapon, int weaponCount)
{
    if (weapon != 0xff && D_15fc[weapon] != 0)
    {
        if (weaponCount > 1)
        {
            D_5890 = D_588f = 0;
            ULTIMA_16ba_PrintChar('\n');
            ULTIMA_1850_PrintString(D_17f6_equipNames[weapon]);
            ULTIMA_1850_PrintString(_TEXT(0x9a94, ":\n"));
        }

        D_589d_attackWeapon = (u8)weapon;
        ULTIMA_1850_PrintString(_TEXT(0x9a98, "Attack-"));
        COMSUBS_0c52_AttackWith(D_589e_curCombatant, weapon);
    }
}

// The Attack command: a monster or a bare-handed member attacks once; a member attacks with each weapon ready in hand
// (equips 0, 2 and 3).
void COMSUBS_0d96_AttackCmd(int attacker, int weaponCount)
{
    int member;

    if ((D_ba14_combatants[attacker].flags & COMBAT_FLAGS_PLAYER) == 0 || weaponCount == 0)
    {
        D_589d_attackWeapon = 0xff;
        ULTIMA_1850_PrintString(_TEXT(0x9aa0, "Attack-"));
        COMSUBS_0c52_AttackWith(attacker, 0xff);
    }
    else
    {
        member = D_ba14_combatants[attacker].entityIdx;
        if (weaponCount > 1)
        {
            ULTIMA_1bf2_SetTextPosition(0, ULTIMA_1cee_GetCurrentTextY());
            ULTIMA_16ba_PrintChar(' ');
        }

        COMSUBS_0d3c_AttackWithWeapon(D_55a8_party[member].equips[0], weaponCount);
        COMSUBS_0d3c_AttackWithWeapon(D_55a8_party[member].equips[2], weaponCount);
        COMSUBS_0d3c_AttackWithWeapon(D_55a8_party[member].equips[3], weaponCount);
    }
}

// Fills two lists with the pixels of a straight line (0x148 at most, 100 units to a pixel, ended by 0xff).
static void COMSUBS_0e26_TracePath(int fromX, int fromY, int toX, int toY, u8* outXs, u8* outYs)
{
    int absDx_e;
    int balance;
    int stepsLeft_k;
    int slope_c;
    int stepY;
    int stepX;
    int absDy;

    stepX = 1;
    stepY = 1;
    stepsLeft_k = 0x148;

    if (toX != fromX)
    {
        slope_c = ((toY - fromY) * 100) / (toX - fromX);
    }
    else
    {
        slope_c = 0x4b00;
    }

    if (slope_c < 0)
    {
        slope_c = -slope_c;
    }

    if (toX < fromX)
    {
        stepX = -1;
    }

    if (toY < fromY)
    {
        stepY = -1;
    }

    absDx_e = toX - fromX;
    if (absDx_e < 0)
    {
        absDx_e = -absDx_e;
    }

    absDy = toY - fromY;
    if (absDy < 0)
    {
        absDy = -absDy;
    }

    // f08f
    balance = slope_c;

    *outXs = fromX;
    outXs++;

    *outYs = fromY;
    outYs++;

    while (absDx_e > 0 || absDy > 0)
    {
        while (balance > 0 && absDy > 0)
        {
            balance -= 100;
            fromY += stepY;

            *outYs = fromY;
            outYs++;
            absDy--;

            *outXs = fromX;
            outXs++;
            stepsLeft_k--;

            if (stepsLeft_k == 0)
                break;
        }

        if (stepsLeft_k == 0)
            break;

        absDx_e--;
        balance += slope_c;

        fromX += stepX;

        *outYs = fromY;
        *outXs = fromX;

        outYs++;
        outXs++;
        stepsLeft_k--;
    }

    *outXs = 0xff;
    *outYs = 0xff;
}

// Draws one frame of a missile at a pixel: its picture depends on the kind (0 a trail of dots, 1 a ball, 2 and 7 a
// bolt of lines, 3-6 a sparkle of the field colours red, blue, green or magenta), turned by the heading.
static void COMSUBS_0f4a_DrawMissile(byte pixelX, byte pixelY, int heading, int missileKind, u8* pathXs, u8* pathYs)
{
    int signY;
    int signX_b;
    int lineY2_h;
    byte* ptrYs_i;
    int index_c;
    int lineY1_i;
    int lineX2_i;
    byte* ptrXs_j;
    s8* shapeY;
    int lineX1_i;
    s8* shapeX;
    int flipY;
    int flipX;
    int sparkColor;

    signX_b = 1;
    signY = 1;
    sparkColor = -1;

#if !defined(MATCHING_BUILD)
    shapeY = NULL;
    shapeX = NULL;
#endif

    if (missileKind == 2)
    {
        shapeX = shapeY = D_2188;
    }

    if (missileKind > 2)
    {
        shapeX = shapeY = D_2188 + 0x10;
    }

    switch (heading)
    {
    case 1: // f178
        shapeY++;
        break;
    case 2: // f196
        shapeX++;
        signX_b = -1;
        break;
    case 3: // f1a0
        shapeY++;
        signX_b = signY = -1;
        break;
    case 4: // f1ae
        shapeX++;
        signY = -1;
        break;
    }

    // f17b
    ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
    switch (missileKind)
    {
    case 0: // f1b8
        ptrXs_j = pathXs - 4;
        ptrYs_i = pathYs - 4;
        for (index_c = 0; index_c < 9; index_c++)
        {
            if (*ptrXs_j == 0xff)
                break;

            ULTIMA_0b10_GRAP_Line(*ptrXs_j, *ptrYs_i, *ptrXs_j + 1, *ptrYs_i);
            ptrXs_j++;
            ptrYs_i++;
        }
        break;

    case 1: // f206
        ULTIMA_0b10_GRAP_Line(pixelX - 1, pixelY - 2, pixelX + 1, pixelY - 2);
        ULTIMA_0b10_GRAP_Line(pixelX - 2, pixelY - 1, pixelX + 2, pixelY - 1);
        ULTIMA_0b10_GRAP_Line(pixelX - 2, pixelY, pixelX + 2, pixelY);
        ULTIMA_0b10_GRAP_Line(pixelX - 2, pixelY + 1, pixelX + 2, pixelY + 1);
        ULTIMA_0b10_GRAP_Line(pixelX - 1, pixelY + 2, pixelX + 1, pixelY + 2);
        break;

    case 2: // f27e
        for (index_c = 0; index_c < 4; index_c++)
        {
            lineX1_i = *shapeX * signX_b + pixelX;
            shapeX += 2;
            lineY1_i = *shapeY * signY + pixelY;
            shapeY += 2;
            lineX2_i = *shapeX * signX_b + pixelX;
            shapeX += 2;
            lineY2_h = *shapeY * signY + pixelY;
            shapeY += 2;
            ULTIMA_0b10_GRAP_Line(lineX1_i, lineY1_i, lineX2_i, lineY2_h);
        }
        break;

    case 3: // f2f4
        sparkColor = D_13ae_redColor + 8;
        break;

    case 4: // f300
        sparkColor = D_13b2_blueColor + 8;
        break;

    case 5: // f306
        sparkColor = D_13b4_greenColor + 8;
        break;

    case 6: // f30c
        sparkColor = D_13b6_magentaColor + 8;
        break;

    case 7: // f312
        for (index_c = 0; index_c < 7; index_c++)
        {
            lineX1_i = *shapeX * signX_b + pixelX;
            shapeX += 2;
            lineY1_i = *shapeY * signY + pixelY;
            shapeY += 2;
            lineX2_i = *shapeX * signX_b + pixelX;
            shapeX += 2;
            lineY2_h = *shapeY * signY + pixelY;
            shapeY += 2;
            ULTIMA_0b10_GRAP_Line(lineX1_i, lineY1_i, lineX2_i, lineY2_h);
        }

        // f384
        ULTIMA_0c64_GRAP_30_Pset(*shapeX * signX_b + pixelX, *shapeY * signY + pixelY);
        shapeX += 2;
        shapeY += 2;
        ULTIMA_0c64_GRAP_30_Pset(*shapeX * signX_b + pixelX, *shapeY * signY + pixelY);
        shapeX += 2;
        shapeY += 2;
        ULTIMA_0c64_GRAP_30_Pset(*shapeX * signX_b + pixelX, *shapeY * signY + pixelY);
        break;
    }

    if (sparkColor > -1)
    {
        // f40d
        signX_b = -1;
        signY = 1;
        for (index_c = 0; index_c < 0x30; index_c++)
        {
            // f42b
            ptrXs_j = ptrYs_i = D_21ba;

            ptrXs_j += ULTIMA_2092_RandomRange(0, 0xf);
            ptrYs_i += ULTIMA_2092_RandomRange(0, 0xf);

            flipX = ULTIMA_2092_RandomRange(0, 1);
            flipY = ULTIMA_2092_RandomRange(0, 1);

            if (flipX != 0)
            {
                signX_b = -signX_b;
            }

            if (flipY != 0)
            {
                signY = -signY;
            }

            ULTIMA_0a70_GRAP_2d_SetPenColor(sparkColor);
            ULTIMA_0c64_GRAP_30_Pset(*ptrXs_j * signX_b + pixelX, *ptrYs_i * signY + pixelY);
        }
    }
}

// Animates a missile along the line between two cells, frame by frame, redrawing what it covered. Returns 0 when a
// wall or an obstacle ends the flight (the cell is left in D_5876/D_5878), else 1. Also used by the Fire command (the cannon ball)
// and by ships firing on the overworld.
int COMSUBS_12de_FlyMissile(int fromX, int fromY, int toX, int toY, int missileKind)
{
    byte* ptrYs;
    int fromPixelX_c;
    byte* ptrXs;
    int pathStep_h;
    int heading;
    int toPixelY_b;
    int fromPixelY;
    int toPixelX_b;

    pathStep_h = 0xd;
    heading = 0;
    ULTIMA_0f6e_GRAP_1b_TransferFullscreen(0, 1);

    memset(D_a728_pathXs, 0xff, 0x100);
    memset(D_a872_pathYs, 0xff, 0x100);

    fromPixelX_c = fromX * 0x10 + 0x10;
    fromPixelY = fromY * 0x10 + 0x10;
    toPixelX_b = toX * 0x10 + 0x10;
    toPixelY_b = toY * 0x10 + 0x10;
    COMSUBS_0e26_TracePath(fromPixelX_c, fromPixelY, toPixelX_b, toPixelY_b, D_a728_pathXs, D_a872_pathYs);

    ptrXs = D_a728_pathXs;
    ptrYs = D_a872_pathYs;

    if (D_5893_mapId != 0 && D_5893_mapId < 0x21)
    {
        pathStep_h = 6;
    }
    else
    {
        if (missileKind == 7)
        {
            pathStep_h = 8;
        }
    }

    if (missileKind == 0)
    {
        ptrXs += 4;
        ptrYs += 4;
    }

    while (*ptrXs != 0xff)
    {
        heading = (heading & 3) + 1;
        ULTIMA_3fb4_PixelToViewCell(*ptrXs, *ptrYs);
        if (D_5876_tempX == -1)
        {
            break;
        }

        COMSUBS_0f4a_DrawMissile(*ptrXs, *ptrYs, heading, missileKind, ptrXs, ptrYs);
        ULTIMA_20c8_Delay(1, 0x28);
        ULTIMA_0ace_GRAP_18_TransferArea(1, 0, (*ptrXs & 0xf8) - 8, *ptrYs - 8, (*ptrXs & 0xf8) + 0xf, *ptrYs + 0xf);

        ptrXs += pathStep_h;
        ptrYs += pathStep_h;

#if defined(MATCHING_BUILD)
        if (D_a872_pathYs < ptrXs) // the end of D_a728_pathXs (as D_a728_pathXs + sizeof(D_a728_pathXs) the code is 2 bytes shorter)
        {
            break;
        }
#else
        if (D_a728_pathXs + sizeof(D_a728_pathXs) < ptrXs)
        {
            break;
        }
#endif

        if (*ptrXs == 0xff)
        {
            break;
        }

        if (ULTIMA_3f6e_TestViewTileFlag(D_5876_tempX, D_5878_tempY) == 0 && (fromX != D_5876_tempX || fromY != D_5878_tempY))
        {
            return 0;
        }
    }

    return 1;
}
