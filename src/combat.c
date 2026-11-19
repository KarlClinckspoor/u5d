#include "common/common.h"
#include "vars.h"
#include "funcs.h"
#include "macros.h"
#include "tiles.h"

#include <string.h>

#include "cast.h"
#include "cmds.h"
#include "combat.h"
#include "comsubs.h"
#include "sjog.h"
#include "zstats.h"



static void CombatProcessCommand(void);
static int ChooseTarget(int a);
static int MonsterMove(int combatant);
static void TerrainEffect(int combatant);
static void Regurgitate(int combatant);

// CHECKED
// 1 when an actor of that tile may enter the cell (x, y) of the 11x11 combat map: the tile is walkable (and not 0xff), and no actor
// (fields, sleepers, the dead and the invisible do not count) or live combatant stands there. Off the map counts as free.
int IsCellFree(int actorTile, int cellX, int cellY)
{
    int index_f;
    int flags;
    int occupant;

    if (cellX > 0xa || cellY > 0xa || cellX < 0 || cellY < 0)
    {
        // -> a2b0
        return 1;
    }

    // 0026
    if (ULTIMA_2c4c_IsWalkableTile(actorTile, *ULTIMA_4402_GetTileAddr(cellX, cellY)) != 0 &&
        *ULTIMA_4402_GetTileAddr(cellX, cellY) != 0xff)
    {
        // 0043
        for (index_f = 0; index_f < 0x20; index_f++)
        {
            if (D_5c5a_actors[index_f].x == cellX && D_5c5a_actors[index_f].y == cellY) // a317
            {
                occupant = D_5c5a_actors[index_f].tile;
                if (occupant == TILE_ACTOR_EB)
                {
                    return 0; // a33a
                }

                if ((occupant & 0xfc) == TILE_ACTOR_E8 || occupant == TILE_ACTOR_SLEEP || occupant == TILE_ACTOR_DEAD) // a346
                {
                    continue;
                }

                if (occupant == 0 || D_5c5a_actors[index_f].animTile == 0) // a38a
                {
                    continue;
                }
                return 0;
            }
            else if (combatants[index_f].x == cellX && combatants[index_f].y == cellY) // a398
            {
                flags = combatants[index_f].flags; // a3b3
                if ((flags & (COMBAT_FLAGS_DEAD | COMBAT_FLAGS_4)) == 0 && flags != 0)
                {
                    if (D_5c5a_actors[combatants[index_f].actorIdx].animTile == 0)
                    {
                        return 0;
                    }
                    if ((flags & 8) != 0) // sic: both ways return 0
                    {
                        return 0;
                    }
                    return 0;
                }
            }
        }

        // -> a2b0
        return 1;
    }

    // a3d4
    return 0;
}

// A monster's missile or spell at a target: it happens half of the time (a Mimic, 0x1a, always), and not at all for a magic monster when
// magic is negated (status N or 0x1c). warded (the target wears the amulet 0x2d against a magic monster, half of the time) makes it a
// sure miss. Returns 1 when the attack was made.
static int MonsterRanged(int attacker, int target, int warded)
{
    int type_k;
    int struck;
    int onTarget;
    int missileKind_c;

    type_k = combatants[attacker].entityIdx;
    missileKind_c = enemyMissile[type_k];
    if (type_k != 0x1a && ULTIMA_2092_RandomRange(0, 0xff) >= 0x80 || (enemyFlags[type_k] & ENEMY_FLAG_MAGIC) != 0 && (statusIcon == 'N' ||
        statusIcon == 0x1c))
    {
        return 0;
    }
    else
    {
        AudioSweepTone(0x2ee, 400, 5, 0x96);
        if (warded != 0)
        {
            onTarget = 0;
        }
        else
        {
            onTarget = RollToHit(target, attacker, -(int)castCircle, 0);
        }

        struck = ResolveShot(attacker, combatants[target].x, combatants[target].y, onTarget, missileKind_c);
        if (struck >= 0)
        {
            HitEffectOnActor(struck);
            ULTIMA_16ba_PrintChar('\n');
            ResolveHit(struck, attacker);
            ReportHit(struck, attacker);
        }

        return 1;
    }
}

// A monster's attack on the target it chose (COMBAT_0d30). A charmed one only strikes a neighbour; otherwise within its range it strikes a
// neighbour (a hit roll; a thief may steal 5 food) or shoots (COMBAT_014e). Returns 1 when it used its turn.
static int MonsterAttack(int monster)
{
    int warded_h;
    int type_m;
    int actorIdx;
    int targetType;
    int target;

    warded_h = 0;
    type_m = combatants[monster].entityIdx;
    actorIdx = combatants[monster].actorIdx;
    target = ChooseTarget(monster);
    if (target < 0)
    {
        // 0256
        return 0;
    }

    // 025c
    targetType = combatants[target].entityIdx;

    if ((combatants[monster].flags & COMBAT_FLAGS_CHARMED) != 0)
    {
        // 0278
        if (CombatantDistance(monster, target) == 1)
        {
            // 0286
            attackWeapon = 0x21;
            MeleeBlow(monster, target, 0x21);
            // -> 035f
        }
        else
        {
            return 0;
        }
    }
    else
    {
        // 029c
        if ((combatants[target].flags & COMBAT_FLAGS_PLAYER) != 0 &&
            D_55a8_party[targetType].equips[5] == 0x2d &&
            (enemyFlags[type_m] & ENEMY_FLAG_MAGIC) != 0)
        {
            warded_h = ULTIMA_2092_RandomRange(0, 0xff) < 0x80;
        }

        // 02df
        if (CombatantDistance(monster, target) <= enemyRange[type_m])
        {
            if (type_m == 0x1a)
            {
                D_5c5a_actors[actorIdx].animState = 0x20;
            }

            if (CombatantDistance(monster, target) == 1)
            {
                if (type_m == 0x2d)
                {
                    D_5c5a_actors[actorIdx].animState = 0x20;
                }

                AudioSweepTone(0x2ee, 400, 5, 0x96);
                lastAttacker[target] = monster;
                if (RollToHit(target, monster, -(uint)castCircle, 0) == 0)
                {
                    return 1;
                }

                if ((enemyFlags[type_m] & ENEMY_FLAG_STEAL) != 0 && ULTIMA_2092_RandomRange(0, 3) != 0 && food != 0)
                {
                    ULTIMA_1850_PrintString(/*0x6d74*/ "\nA ");
                    PrintName(monster);
                    ULTIMA_1850_PrintString(/*0x6d78*/ " stole some food!\n");
                    DecreaseInt(&food, 5);
                    AudioSweepTone(800, 2000, 1, 0x32);
                    ULTIMA_2900_UpdateVitalsDisplay();
                    return 1;
                }

                HitEffectOnActor(target);
                ULTIMA_16ba_PrintChar('\n');
                ResolveHit(target, monster);
                ReportHit(target, monster);
                return 1;
            }

            return MonsterRanged(monster, target, warded_h);
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

// A monster's turn. Nothing happens while time is stopped (status T), to a swallowed monster, or on half the turns under status Q.
// A sleeper may wake; a fleeing one may recover a hit point; otherwise it tries its tricks (COMSUBS_00f4) and its attack
// (COMBAT_0226), then it moves (COMBAT_0ee4). One that moves out of the view escapes (the Shadow Lord's escape: BreakCharm).
static void MonsterTurn(void)
{
    int actorIdx;
    CombatEntity* entity;

    entity = &combatants[curCombatant];
    attackWeapon = 0;
    actorIdx = entity->actorIdx;

    if (statusIcon != 'T' &&
        (statusIcon != 'Q' || ULTIMA_2092_RandomRange(0, 1) != 0) &&
        (entity->flags & COMBAT_FLAGS_4) == 0)
    {
        // 0446
        if ((entity->flags & COMBAT_FLAGS_ASLEEP) != 0)
        {
            if (ULTIMA_2092_RandomRange(0, 0x10) == 0x10)
            {
                WakeCombatant(curCombatant);
            }
        }
        else
        {
            if ((entity->entityIdx) == 0x2d)
            {
                D_5c5a_actors[actorIdx].animState = 0;
            }

            // 047f
            if ((entity->flags & COMBAT_FLAGS_2) != 0)
            {
                if ((entity->flags & COMBAT_FLAGS_PLAYER) == 0 && ULTIMA_2092_RandomRange(0, 3) == 3)
                {
                    entity->hp++;
                }

                WoundLevel(curCombatant);
            }
            else
            {
                if (MonsterSpecial(curCombatant) != 0)
                {
                    return;
                }

                if (MonsterAttack(curCombatant) != 0)
                {
                    return;
                }
            }

            // 04cb
            if (MonsterMove(curCombatant) != 0)
            {
                if ((combatEvents & 0x10) != 0)
                {
                    ULTIMA_16ba_PrintChar('\n');
                    AudioSweepTone(0x4b0, 2000, 1, 0x28);
                    PrintName(curCombatant);
                    ULTIMA_1850_PrintString(/*0x6d8c*/ " escapes!\n");
                    RemoveEntity(-(uint)curCombatant - 1);

                    if ((entity->flags & COMBAT_FLAGS_PLAYER) == 0 && entity->entityIdx == 0x2f)
                    {
                        BreakCharm();
                    }
                }
                ULTIMA_5910_UpdateFrame();
            }
            else
            {
                if ((entity->flags & COMBAT_FLAGS_2) != 0)
                {
                    MonsterAttack(curCombatant);
                }
            }
        }
    }
}

// A command the party shares with the world: prints its word and calls it for a party member (0 Get, 1 Jimmy, 2 Open, 3 Ready, 4 Search,
// 5 Use); a monster says "Can't!" (returns 1).
static int RunCommand(char* verb, int command)
{
    ULTIMA_1850_PrintString(verb);

    if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        switch (command)
        {
        case 0:
            GetCmd();
            break;
        case 1:
            JimmyCmd();
            break;
        case 2:
            OpenCmd();
            break;
        case 3:
            ReadyCmd();
            break;
        case 4:
            SearchCmd();
            break;
        case 5:
            UseCmd();
        }
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x6d98*/ "Can't!\n");
        return 1;
    }

    return 0;
}

// CHECKED
// Adds ", " (after the first) and the name of a readied weapon to the line in scratch; 1 when the weapon exists.
static int AppendWeaponName(int weapon, int listed)
{
    if (weapon != 0xff && weaponAttack[weapon] != 0)
    {
        if (listed != 0)
        {
            // OK
            strcat(scratch, /*0x6da0*/ ", ");
        }

        // 05fb
        strcat(scratch, equipNames[weapon]);

        return 1;
    }
    else
    {
        return 0;
    }
}

// The turn of a party member: the prompt names the member and the weapons in hand, then one command key is read and run (Attack, Cast,
// Get, Jimmy, Klimb, Open, Push, Ready, Search, Use, Yell, Z-stats, Pass, the active player 0-6, the arrows to move, and the
// world's commands, which SJOG_1f26 refuses or runs). A sleeper or a swallowed member cannot act; one holding the weapon 0x23 is
// charmed and acts as a monster. The party display is redrawn first when D_a9fa says so.
static void CombatProcessCommand(void)
{
    int weaponCount_e;
    int member;
    byte key;
    int keyDone;
    int askAgain;

    D_5896_mapX = combatants[curCombatant].x;
    D_5897_mapY = combatants[curCombatant].y;
    member = combatants[curCombatant].entityIdx;

    if (activePlayer != 0xff && ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) == 0 || activePlayer != member))
    {
        TurnUpkeep();
    }
    else
    {
        if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0 && (D_55a8_party[member].equips[2] == 0x23 || D_55a8_party[member].equips[3] == 0x23))
        {
            // a938
            combatants[curCombatant].flags |= COMBAT_FLAGS_CHARMED;
            activePlayer = 0xff;
            D_a9fa_vitalsDirty = 1;
            MonsterTurn();
            // -> ae09
        }
        else
        {
            // a958
            if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                InvertPlayerRow(combatants[curCombatant].entityIdx);
            }

            // a972
            if (D_a9fa_vitalsDirty != 0)
            {
                ULTIMA_2900_UpdateVitalsDisplay();
                D_a9fa_vitalsDirty = 0;
            }

            // a981
            do
            {
                ULTIMA_16ba_PrintChar('\n');
                PrintName(curCombatant);
                scratch[0] = 0;
                if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
                {
                    ULTIMA_1850_PrintString(/*0x6da4*/ ", armed with ");
                    weaponCount_e = AppendWeaponName(D_55a8_party[member].equips[0], 0);
                    weaponCount_e += AppendWeaponName(D_55a8_party[member].equips[2], weaponCount_e);
                    weaponCount_e += AppendWeaponName(D_55a8_party[member].equips[3], weaponCount_e);
                    if (weaponCount_e == 0)
                    {
                        strcat(scratch, /*0x6db2*/ "bare hands");
                    }
                }

                strcat(scratch, /*0x6dbe*/ ":");
                ULTIMA_1850_PrintString(scratch);

                keyDone = askAgain = 0;

                // aa4a
                while (!keyDone)
                {
                    ULTIMA_16ba_PrintChar('\n');
                    ULTIMA_4c2a_PrintLeftCap();
                    if ((combatants[curCombatant].flags & COMBAT_FLAGS_4) != 0)
                    {
                        ULTIMA_1850_PrintString(/*0x6dc0*/ "ARGH!\n");
                        ULTIMA_223c_AudioWhiteNoise(0x28, 3000, 500);
                        Regurgitate(curCombatant);
                        keyDone = 1;
                    }
                    else if ((combatants[curCombatant].flags & COMBAT_FLAGS_ASLEEP) != 0)
                    {
                        if (ULTIMA_2092_RandomRange(0, 0xff) < 0x10)
                        {
                            WakeCombatant(curCombatant);
                        }

                        ULTIMA_1850_PrintString(/*0x6dc8*/ "Zzzzz...\n");
                        keyDone = 1;
                    }
                    else
                    {
                        key = ULTIMA_266c_GetChar();
                        keyDone = 1;
                        askAgain = 0;

                        switch (key)
                        {
                        case U5_KEY_CTRL_B: // ok
                            // Buffer on/off
                            // ab1a
                            ULTIMA_1850_PrintString(/*0x6dd2*/ "Buffer O");
                            keyBufferOff = !keyBufferOff;
                            if (keyBufferOff != 0)
                            {
                                ULTIMA_1850_PrintString(/*0x6ddc*/ "ff\n");
                            }
                            else
                            {
                                ULTIMA_1850_PrintString(/*0x6de0*/ "n\n");
                            }
                            // ab3d
                            keyDone = 0;
                            break;

                        case U5_KEY_CTRL_S: // ok
                            // ab46
                            ULTIMA_1850_PrintString(/*0x6de4*/ "Sound ");
                            if (D_a9ce_soundOn != 0)
                            {
                                ULTIMA_1850_PrintString(/*0x6dec*/ "Off\n");
                            }
                            else
                            {
                                ULTIMA_1850_PrintString(/*0x6df2*/ "On\n");
                            }
                            D_a9ce_soundOn = !D_a9ce_soundOn;
                            // -> ab3d
                            keyDone = 0;
                            break;

                        case 0x41:
                            /* 'A' Attack */
                            // ab70
                            ComsubsAttackCmd(curCombatant, weaponCount_e);
                            break;

                        case 0x43: // ok
                            /* 'C' Cast */
                            // ab80 / 08f0
                            ULTIMA_1850_PrintString(/*0x6df6*/ "Cast...\n");
                            askAgain = 1;
                            if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
                            {
                                if (EnemyInterferes(curCombatant) == 0)
                                {
                                    // abab / 091b
                                    castCircle = D_5890_spellAttack = 1;
                                    askAgain = 0;
                                    if (statusIcon == 'N' || (crown == 0 && D_5894_savedMapId == 0x12))
                                    {
                                        ULTIMA_1850_PrintString(/*0x6e00*/ "Absorbed!\n");
                                        ULTIMA_2192_AudioPulse(0x2648, 1, 28000, 1000, 2);
                                    }
                                    else
                                    {
                                        CastSpellCmd();
                                    }
                                }
                            }
                            else
                            {
                                // abf4
                                ULTIMA_1850_PrintString(/*0x6e0c*/ "Can't!\n");
                            }
                            break;

                        case 0x47:
                            /* 'G' Get */
                            // abfa
                            askAgain = RunCommand(/*0x6e14*/ "Get-", 0);
                            break;

                        case 0x4a: // ok
                            /* 'J' Jimmy */
                            // ac0a
                            askAgain = RunCommand(/*0x6e1a*/ "Jimmy-", 1);
                            break;

                        case 0x4b:
                            /* 'K' Klimb */
                            // ac14
                            if (CombatKlimb() == 0)
                            {
                                // -> ab3d
                                keyDone = 0;
                            }
                            break;

                        case 0x4f:
                            /* 'O' Open */
                            // ac1a
                            askAgain = RunCommand(/*0x6e22*/ "Open-", 2);
                            break;

                        case 0x50:
                            /* 'P' Push */
                            // ac24
                            ULTIMA_1850_PrintString(/*0x6e28*/ "Push-");
                            PushCmd();
                            break;

                        case 0x52:
                            /* 'R' Ready */
                            // ac32
                            askAgain = RunCommand(/*0x6e2e*/ "Ready...\n\n", 3);
                            break;

                        case 0x53: // ok
                            /* 'S' Search */
                            // ac3c
                            askAgain = RunCommand(/*0x6e3a*/ "Search-", 4);
                            break;

                        case 0x55: // ok
                            // ac46
                            askAgain = RunCommand(/*0x6e42*/ "Use item\n\n", 5);
                            break;

                        case 0x59: // ok
                            /* 'Y' Yell */
                            // ac50
                            ULTIMA_1850_PrintString(/*0x6e4e*/ "Yell ");
                            YellCmd();
                            break;

                        case 0x5a: // ok
                            /* 'Z' Z-stats */
                            // ac5e
                            ULTIMA_1850_PrintString(/*0x6e54*/ "Z-stats...\n");
                            ZstatsCmd();
                            break;

                        case U5_KEY_ESC:
                            // ac6c
                            askAgain = Escape();
                            break;

                        case U5_KEY_SPACE: // ok
                            // ac72
                            ULTIMA_1850_PrintString(/*0x6e60*/ "Pass\n");
                            break;
                        case 0x30: // ok
                            /* '0' */
                            // ac7c
                            activePlayer = 0xff;
                            ULTIMA_1850_PrintString(/*0x6e66*/ "Set active plr:\nNone!\n");
                            ULTIMA_2900_UpdateVitalsDisplay();
                            break;

                        case 0x31:
                        case 0x32:
                        case 0x33:
                        case 0x34:
                        case 0x35:
                        case 0x36:
                            /* '1' .. '6' */
                            // ac8e
                            if (CombatSetActivePlayer(key - 0x31) == 0)
                            {
                                // -> acd1
                                askAgain = 1;
                            }
                            break;

                        case U5_KEY_LEFT:
                        case U5_KEY_RIGHT:
                        case U5_KEY_UP:
                        case U5_KEY_DOWN:
                            // aca4
                            if (CombatMovePlayer(curCombatant, key) == 0)
                            {
                                keyDone = 0;
                            }
                            break;

                        case 0x42:
                            /* 'B' Board */
                            // acbc
                            askAgain = CombatMiscCmd(/*0x6e7e*/ "Board", 1);
                            break;

                        case 0x44:
                            /* 'D' What? */
                            // acca
                            ULTIMA_1850_PrintString(/*0x6e84*/ "D-What?\n");
                            askAgain = 1;
                            break;

                        case 0x45: // ok
                            /* 'E' Enter */
                            // acda / 0a4a
                            askAgain = CombatMiscCmd(/*0x6e8e*/ "Enter", 2);
                            break;

                        case 0x46: // ok
                            /* 'F' Fire */
                            // ace4 / 0a54
                            askAgain = CombatMiscCmd(/*0x6e94*/ "Fire", 2);
                            break;

                        case 0x48: // ok
                            /* 'H' Hole up */
                            // acea / 0a5a
                            askAgain = CombatMiscCmd(/*0x6e9a*/ "Hole up", 2);
                            break;

                        case 0x49: // ok
                            /* 'I' Ignite torch */
                            // acf0 / 0a60
                            askAgain = CombatMiscCmd(/*0x6ea2*/ "Ignite torch", 2);
                            break;

                        case 0x4c: // ok
                            /* 'L' Look */
                            // acf6 /  0a66
                            askAgain = CombatMiscCmd(/*0x6eb0*/ "Look", 2);
                            break;

                        case 0x4d: // ok
                            /* 'M' Mix */
                            // acfc / 0a6c
                            askAgain = CombatMiscCmd(/*0x6eb6*/ "Mix", 2);
                            break;

                        case 0x4e: // ok
                            /* 'N' New order */
                            // ad02 / 0a72
                            askAgain = CombatMiscCmd(/*0x6eba*/ "New order", 2);
                            break;

                        case 0x51: // ok
                            /* 'Q' Quit */
                            // ad08 / 0a78
                            askAgain = CombatMiscCmd(/*0x6ec4*/ "Quit", 2);
                            break;

                        case 0x54: // ok
                            /* 'T' Talk */
                            // ad0e
                            askAgain = CombatMiscCmd(/*0x6eca*/ "Talk", 3);
                            break;

                        case 0x56: // ok
                            /* 'V' View */
                            // ad18
                            askAgain = CombatMiscCmd(/*0x6ed0*/ "View", 2);
                            break;

                        case 0x57: // ok
                            /* 'W' What */
                            // ad1e
                            ULTIMA_1850_PrintString(/*0x6ed6*/ "W-What?\n");
                            askAgain = 1;
                            break;

                        case 0x58: // ok
                            /* 'X' X-it */
                            // ad24
                            askAgain = CombatMiscCmd(/*0x6ee0*/ "X-it", 1);
                            break;

                        default:
                            // ad47
                            // -> ab39
                            ULTIMA_1850_PrintString(/*0x6ee6*/ "What?\n");
                            keyDone = 0;
                            break;
                        }
                    }
                }
                // ade6
            } while (askAgain != 0); // -> a981

            // adef
            if ((combatants[curCombatant].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                InvertPlayerRow(combatants[curCombatant].entityIdx);
            }
        }

        // ae09
        if (key < 0x30 || key > 0x36)
        {
            TurnUpkeep();
            ULTIMA_5910_UpdateFrame();
            CheckMirror();
        }
    }
}

// CHECKED
// The battle loop. Every combatant has a timer (36 minus its dexterity); when it runs out, a monster acts (COMBAT_03f4) or the party
// member is asked (COMBAT_063e). Every ten turns the clock advances. VICTORY! when no monster is left, BATTLE IS LOST! when no
// member. Returns 1 (0 when CountCombatants's two counts in D_5876/D_5878 are both zero).
int CombatMainLoop(void)
{
    int finished_c;
    CombatEntity* entity;
    int flags;
    int result;

    result = 1;
    combatExitDir = 0;

    ULTIMA_5910_UpdateFrame();
    ULTIMA_2900_UpdateVitalsDisplay();
    ULTIMA_1b16_ClearKbdBuffer();
    CountCombatants();

    if (D_5876_tempX == 0)
    {
        combatOver = 1;
    }
    else
    {
        combatOver = 0;
    }

    finished_c = 0;
    do
    {
        // ae5a
        for (curCombatant = 0; curCombatant < 0x20; curCombatant++)
        {
            // ae5f
            D_594f_openDoorTile = 0;
            entity = &combatants[curCombatant];
            flags = entity->flags;

            if (((byte)flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) != 0 && ((byte)flags & COMBAT_FLAGS_DEAD) == 0)
            {
                // ae86
                if (((byte)flags & COMBAT_FLAGS_PLAYER) != 0 && D_55a8_party[entity->entityIdx].status == STATUS_DEAD) // monster / non-dead player
                {
                    entity->flags |= COMBAT_FLAGS_DEAD;
                    HitEffectOnActor(curCombatant);
                    ApplyDamage(curCombatant, 99);
                }
                else
                {
                    // aeb6
                    if ((GetCombatMap(entity->x, entity->y) & 0xfe) != 0x84)
                    {
                        // aed3
                        if (--entity->turnTimer == 0) // decrease turn timer
                        {
                            // aedb
                            entity->turnTimer = 36 - entity->dex; // reset timer
                            attackWeapon = D_5890_spellAttack = castCircle = combatEvents = crosshairOn = 0;
                            combatTick++;
                            if (combatTick == 10)
                            {
                                combatTick = 0;
                                ULTIMA_4f7c_AdvanceTime(1);
                            }

                            // af09
                            focusBlink = 1;
                            if (ULTIMA_5646_IsEnemy(curCombatant) != 0)
                            {
                                MonsterTurn();
                            }
                            else
                            {
                                CombatProcessCommand();
                            }

                            // af23
                            lastAttacker[curCombatant] = 0xff;
                            TerrainEffect(curCombatant);
                            CountCombatants();

                            // af39
                            if (D_5878_tempY == 0)
                            {
                                if (D_5876_tempX == 0)
                                {
                                    // af47
                                    result = 0;

                                    // af4c
                                    finished_c = 1;
                                    break; // -> afa6
                                }

                                // af5a
                                if (BreakCharm() == -1)
                                {
                                    ULTIMA_5910_UpdateFrame();
                                    if (combatOver == 0)
                                    {
                                        ULTIMA_1850_PrintString(/*0x6eee*/ "\nBATTLE IS LOST!");
                                        result = 1;
                                    }

                                    // -> af4c
                                    finished_c = 1;
                                    break;
                                }

                                continue;
                            }

                            // af78
                            if (D_5876_tempX == 0 && combatOver == 0)
                            {
                                ULTIMA_1850_PrintString(/*0x6f00*/ "\nVICTORY!\n");
                                combatOver = 1;

                                // BGM/NOTE: no jingle when bgm is activated
                                AudioVictoryTune();
                                ULTIMA_1b16_ClearKbdBuffer();

                            }
                        }
                    }
                }
            }
        } // af98
    } while (finished_c == 0); // afa6

    // afaf
    ULTIMA_1b16_ClearKbdBuffer();
    beaconStep = 0xff;

    return result;
}

// CHECKED
// Chooses the opponent of a combatant: the nearest live and visible one of the other side (under status C a monster may mistake its side).
// D_5876/D_5878 get the step toward it (-1, 0 or 1; reversed when fleeing). With nobody left the monsters all flee. Returns its index, or -1.
static int ChooseTarget(int combatant)
{
    int distance;
    int best_k;
    int index;
    int bestY;
    int playersSeen_k;
    int bestX;
    CombatEntity* other;
    int otherIsEnemy_c;
    int selfIsEnemy_k;
    CombatEntity* self_f;
    int bestDistance_f;

    playersSeen_k = -1;
    best_k = -1;
    bestDistance_f = 99;
    self_f = &combatants[combatant];

    selfIsEnemy_k = ULTIMA_5646_IsEnemy(combatant);
    if (statusIcon == 67)
    {
        distance = QueryStat(combatant, QUERY_STAT_INTEL);
        if (distance < Random1To30())
        {
            selfIsEnemy_k = 0;
        }
    }

    for (index = 0x1f; index > -1; index--)
    {
        other = &combatants[index];
        if (index != combatant && other->flags != 0 && (other->flags & COMBAT_FLAGS_DEAD) == 0)
        {
            otherIsEnemy_c = ULTIMA_5646_IsEnemy(index);
            if (selfIsEnemy_k != otherIsEnemy_c)
            {
                if (((D_5894_savedMapId == 40 || self_f->entityIdx == 47 || (other->flags & COMBAT_FLAGS_INVISIBLE) == 0) && (other->flags & COMBAT_FLAGS_4) == 0))
                {
                    if (index < 5)
                    {
                        playersSeen_k = playersSeen_k + 1;
                    }

                    // b06b
                    distance = ComsubsDistance(self_f->x, self_f->y, other->x, other->y);
                    if (distance < bestDistance_f)
                    {
                        bestDistance_f = distance;
                        best_k = index;
                        bestX = other->x;
                        bestY = other->y;
                    }
                }
            }
        }
    }

    if (playersSeen_k == -1 && best_k == -1)
    {
        best_k = BreakCharm();
    }

    if (best_k == -1)
    {
        for (index = 0x1f; index > 5; index--)
        {
            if ((combatants[index].flags & COMBAT_FLAGS_MONSTER) != 0)
            {
                combatants[index].hp = 1;
                combatants[index].flags |= COMBAT_FLAGS_2;
            }
        }

        bestX = 5;
        bestY = 5;
    }

    D_5876_tempX = D_5878_tempY = 0;

    if (self_f->x > bestX)
    {
        D_5876_tempX = -1;
    }

    if (self_f->x < bestX)
    {
        D_5876_tempX = 1;
    }

    if (self_f->y > bestY)
    {
        D_5878_tempY = -1;
    }

    if (self_f->y < bestY)
    {
        D_5878_tempY = 1;
    }

    if ((self_f->flags & COMBAT_FLAGS_2) != 0)
    {
        D_5876_tempX *= -1;
        D_5878_tempY *= -1;
    }

    if (best_k == combatant)
    {
        best_k = -1;
    }

    return best_k;
}

// CHECKED
// A monster's move. A teleporter (ENEMY_FLAG_TELEPORT) tries to jump to a free random cell: always when boxed in (IsSurrounded), else 3 in 4.
// The Mimic and the Reaper (0x1a, 0x1b) never move. Otherwise it steps toward its target when it can, else a random side step; leaving
// the view sets combatEvents to 0x10. Returns 1 when it moved.
static int MonsterMove(int combatant)
{
    int target;
    int moved_h;
    int moveCode;
    CombatEntity* self_f;

    moved_h = 0;
    moveCode = 0;
    self_f = &combatants[combatant];

    if (self_f->entityIdx != 0x1b && self_f->entityIdx != 0x1a)
    {
        // 0f17
        if ((self_f->flags & COMBAT_FLAGS_PLAYER) == 0 && (enemyFlags[self_f->entityIdx] & ENEMY_FLAG_TELEPORT) != 0 && statusIcon != 0x4e && statusIcon != 0x1c &&
            (IsSurrounded(combatant) != 0 || Random(3) != 3) && RandomCell() != 0)
        {
            if (IsCellFree(D_5c5a_actors[self_f->actorIdx].tile, D_5876_tempX, D_5878_tempY) != 0)
            {
                D_5c5a_actors[self_f->actorIdx].x = self_f->x = D_5876_tempX;
                D_5c5a_actors[self_f->actorIdx].y = self_f->y = D_5878_tempY;

                ULTIMA_1850_PrintString(enemyNames[self_f->entityIdx]);
                ULTIMA_1850_PrintString(/*0x6f0c*/ " teleports!\n");
                moved_h++;
            }
        }

        // 0fab
        if (moved_h == 0 && IsSurrounded(combatant) == 0)
        {
            // 0fc1
            target = ChooseTarget(combatant);
            if (Random(0xff) > 0x7f && IsCellBlocked(self_f->x + D_5876_tempX, self_f->y, combatant) == 0)
            {
                D_5878_tempY = 0;
                moveCode = 999;
            }
            else
            {
                if (IsCellBlocked(self_f->x, self_f->y + D_5878_tempY, combatant) == 0)
                {
                    D_5876_tempX = 0;
                    moveCode = 998;
                }
            }

            if (moveCode < 990)
            {
                // 1030
                moveCode = 0;
                for (moved_h = 4; moved_h > 0; moved_h--)
                {
                    moveCode = Random(3);
                    switch (moveCode)
                    {
                    case 0:
                        D_5878_tempY = 1;
                        D_5876_tempX = 0;
                        break;
                    case 1:
                        D_5876_tempX = 1;
                        D_5878_tempY = 0;
                        break;
                    case 2:
                        D_5878_tempY = -1;
                        D_5876_tempX = 0;
                        break;
                    case 3:
                        D_5876_tempX = -1;
                        D_5878_tempY = 0;
                        break;
                    }

                    // 1070
                    if (IsCellBlocked(self_f->x + D_5876_tempX, self_f->y + D_5878_tempY, combatant) == 0)
                    {
                        moveCode = 991;
                    }

                    if (moveCode > 990)
                        break;

                    
                    D_5876_tempX = D_5878_tempY = 0;
                }
            }

            // 10c7
            if (moveCode != 0)
            {
                D_5c5a_actors[self_f->actorIdx].x = self_f->x += D_5876_tempX;
                D_5c5a_actors[self_f->actorIdx].y = self_f->y += D_5878_tempY;

                moved_h = 1;

                if (IsInView(self_f->x, self_f->y) == 0)
                {
                    combatEvents = 0x10;
                }
            }
        }
    }

    return moved_h;
}

// The combat map's triggers (D_58a1 bit 0x80 or 2): a trigger at the cell (x, y) writes its new tile at its two places, once. 1 when one went off.
int FireMapTrigger(int cellX, int cellY)
{
    byte* triggerXs_l;
    byte* newTiles;
    int fired;
    int index;
    byte* newTile2Ys;
    byte* newTile1Ys;
    byte* newTile2Xs;
    byte* newTile1Xs;
    byte* triggerYs;

    fired = 0;

    if ((combatFlags & 0x80) != 0 || (combatFlags & 2) != 0)
    {
        triggerXs_l = &GetCombatMapTriggerX(0);
        triggerYs = &GetCombatMapTriggerY(0);
        newTiles = &GetCombatMapNewTileId(0);
        newTile1Xs = &GetCombatMapNewTile1X(0);
        newTile1Ys = &GetCombatMapNewTile1Y(0);
        newTile2Xs = &GetCombatMapNewTile2X(0);
        newTile2Ys = &GetCombatMapNewTile2Y(0);

        for (index = 0; index < 8; index++)
        {
            if (*triggerXs_l == cellX && *triggerYs == cellY)
            {
                *triggerXs_l = *triggerYs = 0xff;

                if (*newTile1Xs < 0xb && *newTile1Ys < 0xb)
                {
                    GetCombatMap((uint)*newTile1Xs, (uint)*newTile1Ys) = *newTiles;
                }

                if (*newTile2Xs < 0xb && *newTile2Ys < 0xb)
                {
                    GetCombatMap((uint)*newTile2Xs, (uint)*newTile2Ys) = *newTiles;
                }
                fired = 1;
            }

            triggerXs_l++;
            triggerYs++;
            newTiles++;
            newTile1Xs++;
            newTile1Ys++;
            newTile2Xs++;
            newTile2Ys++;
        }

        if (fired != 0)
        {
            ULTIMA_5910_UpdateFrame();
        }
    }

    return fired;
}

// A random cell of 16x16 into D_5876_tempX and D_5878_tempY: 1 when it is on the 11x11 map.
int RandomCell(void)
{
    D_5876_tempX = Random(0xf);
    D_5878_tempY = Random(0xf);

    if (D_5876_tempX > 0xa || D_5878_tempY > 0xa)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// Removes an actor from the map: the actor index + 1, or, negative, the combatant -(index + 1) and with it its actor.
void RemoveEntity(int which)
{
    int actorIdx;
    CombatEntity* entity;

    if (which < 0)
    {
        which *= -1;
        which--;
        entity = &combatants[which]; // [-which - 1]
        actorIdx = entity->actorIdx;

        entity->hp = entity->dex = entity->actorIdx = 0;
        entity->flags = entity->turnTimer = entity->x = entity->y = 0;
    }
    else
    {
        actorIdx = which - 1;
    }

    D_5c5a_actors[actorIdx].y =
        D_5c5a_actors[actorIdx].z =
        D_5c5a_actors[actorIdx].value =
        D_5c5a_actors[actorIdx].tile =
        D_5c5a_actors[actorIdx].animTile =
        D_5c5a_actors[actorIdx].x = 0;
}

// Damage of a hit: a monster's attack value, or the weapon's (a random 1 to its damage; 99 kills at once; bare hands 1; weapon 0x27 shatters
// and does 99; 0x28 none) less a random 1 to the victim's defense. Negative is a miss.
static int RollDamage(int attacker, int victim)
{
    int attack;
    int damage;
    int defense_e;

    if ((combatants[attacker].flags & COMBAT_FLAGS_MONSTER) != 0)
    {
        attack = D_13bc_enemyStats[combatants[attacker].entityIdx].atk;
    }
    else
    {
        attack = attackWeapon;
        switch (attack)
        {
        case 0x27:
            ULTIMA_1850_PrintString(/*0x6f1a*/ "Thy sword hath shattered!\n");
            Unequip(attacker, attack);
            attack = 99;
            break;

        case 0xff:
            attack = 1;
            break;

        case 0x28:
            attack = 0;
            break;

        default:
            attack = weaponAttack[attack];
            if (attack > 1 && attack != 99)
            {
                attack = ULTIMA_2092_RandomRange(1, attack);
            }
            break;
        }
    }

    if (attack == 99)
    {
        damage = 99;
    }
    else
    {
        if ((combatants[victim].flags & COMBAT_FLAGS_MONSTER) != 0)
        {
            defense_e = D_13bc_enemyStats[combatants[victim].entityIdx].def;
        }
        else
        {
            defense_e = D_55a8_party[combatants[victim].entityIdx].defense;
        }

        damage = attack;
        if (defense_e != 0)
        {
            damage -= ULTIMA_2092_RandomRange(1, defense_e);
        }
    }

    return damage;
}

// The dexterity used for turns and hits: 1 for a monster while time is stopped, for the Mimic (0x1a) and for the asleep; else the stored one.
static int EffectiveDex(int combatant)
{
    if (statusIcon == 'T' && (combatants[combatant].flags & COMBAT_FLAGS_MONSTER) != 0)
    {
        return 1;
    }

    if (combatants[combatant].entityIdx == 0x1a)
    {
        return 1;
    }

    if ((combatants[combatant].flags & COMBAT_FLAGS_ASLEEP) != 0)
    {
        return 1;
    }

    return combatants[combatant].dex;
}

// A stat of a combatant: -1 intelligence, -2 dexterity (COMBAT_139a), -3 strength, -4 defense. A weapon number (> 0) picks strength or
// dexterity by its kind (weaponMissile == 8: strength), and 0 does so for a monster by its flags.
int QueryStat(int combatant, int stat)
{
    int value;
    CombatEntity* entity = &combatants[combatant];

    if ((entity->flags & COMBAT_FLAGS_MONSTER) != 0)
    {
        if (stat == 0)
        {
            if ((enemyFlags[entity->entityIdx] & ENEMY_FLAG_80) != 0)
            {
                // 1411
                stat = QUERY_STAT_STR;
            }
            else
            {
                // -> 1428
                stat = QUERY_STAT_DEX;
            }
        }
    }
    else
    {
        // 1418
        if (stat > 0)
        {
            if (weaponMissile[stat] == 8) // TODO: offset? (+1?)
            {
                // -> 1411
                stat = QUERY_STAT_STR;
            }
            else
            {
                // 1428
                stat = QUERY_STAT_DEX;
            }
        }
    }

    // 142d
    switch (stat)
    {
    case QUERY_STAT_INTEL:
        // 1444
        if ((entity->flags & COMBAT_FLAGS_MONSTER) != 0)
        {
            value = D_13bc_enemyStats[entity->entityIdx].intel;
        }
        else
        {
            // 145e
            value = D_55a8_party[entity->entityIdx].intel;
        }
        break;

    case QUERY_STAT_DEX:
        // 1478
        value = EffectiveDex(combatant);
        break;

    case QUERY_STAT_STR:
        // 1480
        if ((entity->flags & COMBAT_FLAGS_MONSTER) != 0)
        {
            value = D_13bc_enemyStats[entity->entityIdx].str;
        }
        else
        {
            value = D_55a8_party[entity->entityIdx].str;
        }
        break;

    case QUERY_STAT_DEF:
        // 14aa
        if ((entity->flags & COMBAT_FLAGS_MONSTER) != 0)
        {
            value = D_13bc_enemyStats[entity->entityIdx].def;
        }
        else
        {
            value = PlayerDefense(entity->entityIdx);
        }
        break;
    }

    return value; // sic: uninitialised for other stats
}

// The roll to hit: (the target's stat - the attacker's + 30) / 2 is not above a roll of 1 to 30. Weapons use the target's dexterity and the
// weapon's stat; spells both intelligence. Always a hit: the weapons 0x23, 0x27 and 0x28, and the spells 0x2a-0x31 and 0x33.
int RollToHit(int target, int attacker, int unused, int weapon)
{
    int attackStat;
    int defendStat;
    int hit_n;
    int chance;

    attackStat = QUERY_STAT_INTEL;
    hit_n = 0;

    if (castCircle != 0)
    {
        if (weapon > 0x29 && weapon < 0x32 || weapon == 0x33)
        {
            hit_n = 1;
        }
        else
        {
            defendStat = attackStat = QUERY_STAT_INTEL;
        }
    }
    else
    {
        // 1512
        if (weapon == 0x27 || weapon == 0x23 || weapon == 0x28)
        {
            hit_n = 1;
        }
        else
        {
            defendStat = QUERY_STAT_DEX;
            attackStat = weapon;
        }
    }

    // 152f
    if (hit_n == 0)
    {
        defendStat = QueryStat(target, defendStat);
        attackStat = QueryStat(attacker, attackStat);
        chance = ((defendStat - attackStat) + 0x1e) / 2;
        if (chance <= Random1To30())
        {
            hit_n = 1;
        }
    }

    return hit_n;
}

// Damage to a combatant. A member at 0 hit points (or 99 damage) dies. A monster is halved if undead (unless by magic) and unharmed if
// flagged so; at 0 it dies and may leave a chest or a corpse, vanish, or become a swarm (the Gazer); the experience and the treasure level
// are from its record. A monster that divides gets a twin instead. Returns the experience.
int ApplyDamage(int target, int damage)
{
    int cellY_f;
    int cellX_g;
    int groundTile;
    int attempt_n;
    int experience;
    int treasure_f;
    int clone;

    CombatEntity* entity;

    experience = 0;
    treasure_f = 0;
    clone = -1;
    entity = &combatants[target];

    if (damage < 1)
    {
        combatEvents = 0x20;
        damage = 0;
    }

    if ((entity->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        D_55a8_party[entity->entityIdx].hp -= damage;
        if (D_55a8_party[entity->entityIdx].hp < 1 || damage == 99)
        {
            D_55a8_party[entity->entityIdx].hp = 0;
            entity->flags |= COMBAT_FLAGS_DEAD;
            D_55a8_party[entity->entityIdx].status = STATUS_DEAD;

            D_5c5a_actors[entity->actorIdx].tile = D_5c5a_actors[entity->actorIdx].animTile = TILE_ACTOR_SLEEP;
            if (entity->entityIdx == activePlayer)
            {
                activePlayer = 0xff;
            }
        }

        ULTIMA_2900_UpdateVitalsDisplay();
    }
    else
    {
        if ((((byte)enemyFlags[entity->entityIdx]) & ENEMY_FLAG_UNDEAD) != 0 && D_5890_spellAttack == 0)
        {
            damage /= 2;
        }

        if ((((byte)enemyFlags[entity->entityIdx]) & ENEMY_FLAG_NO_DAMAGE) != 0)
        {
            damage = 0;
        }

        if (entity->hp < damage)
        {
            entity->hp = 0;
        }
        else
        {
            entity->hp -= damage;
        }

        if (entity->hp == 0 || damage == 99)
        {
            experience = (D_13bc_enemyStats[entity->entityIdx].maxHp >> 2) + 1;
            treasure_f = D_13bc_enemyStats[entity->entityIdx].treas;

            entity->flags = COMBAT_FLAGS_DEAD;

            entity->hp = 0;
            cellX_g = entity->x;
            cellY_f = entity->y;
            groundTile = GetCombatMap(cellX_g, cellY_f);

            if ((enemyFlags[entity->entityIdx] & (ENEMY_FLAG_VANISH | ENEMY_FLAG_NO_CORPSE)) == 0)
            {
                if (entity->entityIdx == 0x1c)
                {
                    D_5c5a_actors[entity->actorIdx].tile = D_5c5a_actors[entity->actorIdx].animTile = TILE_ACTOR_DEAD;
                    PlaceCombatant(0x1f, 0, cellX_g, cellY_f, D_5895_mapLevel);
                    ULTIMA_5910_UpdateFrame();
                }
                else if (entity->entityIdx == 0x1e)
                {
                    *ULTIMA_4402_GetTileAddr(cellX_g, cellY_f) = TILE_MAP_4C;
                    RemoveEntity(-target - 1);
                }
                else if (groundTile == 0x87 || groundTile < 4)
                {
                    RemoveEntity(-target - 1);
                }
                else if (treasure_f >= Random1To30())
                {
                    D_5c5a_actors[entity->actorIdx].tile = D_5c5a_actors[entity->actorIdx].animTile = TILE_ACTOR_CHEST;
                    D_5c5a_actors[entity->actorIdx].value = treasure_f;

                    if (treasure_f > Random1To30())
                    {
                        D_5c5a_actors[entity->actorIdx].value |= 0x80;
                    }
                }
                else
                {
                    D_5c5a_actors[entity->actorIdx].tile = D_5c5a_actors[entity->actorIdx].animTile = TILE_ACTOR_DEAD;
                }
            }
            else if ((enemyFlags[entity->entityIdx] & ENEMY_FLAG_VANISH) != 0)
            {
                ULTIMA_1850_PrintString(enemyNames[entity->entityIdx]);
                ULTIMA_1850_PrintString(/*0x6f36*/ " vanishes!");
                combatEvents = 2;
                D_5c5a_actors[entity->actorIdx].tile = D_5c5a_actors[entity->actorIdx].animTile = TILE_ACTOR_CIRCLE;
                ULTIMA_1068_RevealTile(GetCombatMap(entity->x, entity->y), entity->x, entity->y);
                RemoveEntity(-(target + 1));
                BreakCharm();
            }
            else
            {
                RemoveEntity(-(target + 1));
            }

        }
        else if ((enemyFlags[entity->entityIdx] & ENEMY_FLAG_DIVIDE) != 0)
        {
            for (attempt_n = 0; attempt_n < 8; attempt_n++)
            {
                PickNearbyCell(entity->x, entity->y);

                cellX_g = D_5876_tempX;
                cellY_f = D_5878_tempY;
                if (IsCellFree(D_5c5a_actors[entity->actorIdx].tile, cellX_g, cellY_f) != 0)
                {
                    clone = PlaceCombatant((D_5c5a_actors[entity->actorIdx].tile - TILE_ACTOR_WIZARD) >> 2, 0, cellX_g, cellY_f, D_5895_mapLevel);
                    if (clone > -1)
                    {
                        combatants[clone].hp = entity->hp;
                        ULTIMA_1850_PrintString(enemyNames[entity->entityIdx]);
                        ULTIMA_1850_PrintString(/*0x6f42*/ " divides!\n");
                        break;
                    }
                }
            }
        }
    }

    return experience;
}

// Poison: a good party member becomes poisoned; anything else takes up to 20 damage (the attacking member earns the experience).
void CombatPoison(int target, int attacker)
{
    int damage;

    if ((combatants[target].flags & COMBAT_FLAGS_PLAYER) != 0 && D_55a8_party[combatants[target].entityIdx].status == STATUS_GOOD)
    {
        D_55a8_party[combatants[target].entityIdx].status = STATUS_POISONED;
        ULTIMA_1850_PrintString(D_55a8_party[combatants[target].entityIdx].name);
        ULTIMA_1850_PrintString(/*0x6f4e*/ " is poisoned!\n");
        combatEvents = 8;
        D_a9fa_vitalsDirty = 1;
    }
    else
    {
        damage = ApplyDamage(target, Random(0x14));
        if (attacker > -1 && (combatants[attacker].flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            IncreaseInt(&D_55a8_party[combatants[attacker].entityIdx].exp, damage, 9999);
        }
    }
}

// The effect of a hit: a poisonous monster poisons (3 in 4), the Gazer puts to sleep, a spell of sleep or poison acts as a field would,
// else the damage is rolled (COMBAT_12b0) and applied; the attacking member earns experience. A miss on a member sets combatEvents to 0x20.
void ResolveHit(int target, int attacker)
{
    int damage;
    int byMonster_l;
    int slept_h;

    byMonster_l = 0;
    if ((combatants[attacker].flags & COMBAT_FLAGS_PLAYER) == 0)
    {
        byMonster_l = 1;
    }

    // 1969
    if (byMonster_l && (enemyFlags[combatants[attacker].entityIdx] & (ENEMY_FLAG_POISON_200 | ENEMY_FLAG_POISON_4)) != 0 && ULTIMA_2092_RandomRange(0, 3) != 0)
    {
        CombatPoison(target, attacker);
    }
    else
    {
        // 19a0
        slept_h = 0;
        if (byMonster_l && combatants[attacker].entityIdx == 0x1c && (combatants[target].flags & COMBAT_FLAGS_ASLEEP) == 0)
        {
            PutToSleep(target);
            slept_h = 1;
        }

        // 19d0
        if (slept_h)
        {
            // -> 1a54
            return;
        }

        if (D_5890_spellAttack != 0 && attackWeapon == 52)
        {
            PutToSleep(target);
            // -> 1a54
            return;
        }

        // 19ee
        if (D_5890_spellAttack == 0 || attackWeapon != 51)
        {
            // 19fc
            damage = RollDamage(attacker, target);
            if (damage < 0 && (combatants[target].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                combatEvents = 0x20;
                return; // -> 1a54
            }

            // 1a22
            damage = ApplyDamage(target, damage);
            if ((combatants[attacker].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                // 1a3c
                IncreaseInt(&D_55a8_party[combatants[attacker].entityIdx].exp, damage, 9999);
            }
        }
        else
        {
            CombatPoison(target, attacker);
            return;
        }
    }
}

// CHECKED
// How hurt a monster is: 4 above three quarters of its hit points, 3 above half, 2 above a quarter, 1 below. A monster below half has about
// 1 in 64 to start to flee (flag 2), one below a quarter always does. Party members return nothing.
int WoundLevel(int combatant)
{
    int quarter;
    int flees;
    int level_h;
    CombatEntity* entity_c;

    flees = 0;
    level_h = 0;
    entity_c = &combatants[combatant];

    if ((entity_c->flags & COMBAT_FLAGS_PLAYER) != 0)
    {
        // 1a81
        return; // sic (IBM: returns &combatants[combatant], FMT: returns combatants[combatant]._2)
    }
    // 1a84
    quarter = D_13bc_enemyStats[entity_c->entityIdx].maxHp / 4; // max hp / 4 (25%)
    if (entity_c->hp < quarter)
    {
        level_h = 1;
        // 1aa9
        flees = 1;
    }
    else
    {
        // 1ab0
        quarter *= 2;
        if (entity_c->hp < quarter) // max hp / 2 (50%)
        {
            level_h = 2;
            quarter = Random(0x100);
            if (quarter > 0xfb)
            {
                flees = 1;
            }
        }
        else
        {
            // 1ad2
            quarter /= 2;
            quarter *= 3; // max hp / 4 * 3 (75%)
            if (entity_c->hp < quarter)
            {
                level_h = 3;
            }
            else
            {
                level_h = 4;
            }
        }
    }

    if (flees)
    {
        entity_c->flags |= COMBAT_FLAGS_2;
    }
    else
    {
        entity_c->flags &= ~COMBAT_FLAGS_2;
    }

    return level_h;
}

// The ground under a combatant at the end of its turn: lava (0x8f, 0xbc) and fire (EA) burn, poison (E8, or the tile 0x04) poisons, sleep (E9) sleeps.
static void TerrainEffect(int combatant)
{
    CombatEntity* entity_c;
    int tile_b;
    int index;
    int hazard;

    hazard = 0;
    entity_c = &combatants[combatant];
    tile_b = GetCombatMap(entity_c->x, entity_c->y);
    if (tile_b == 0x8f || tile_b == 0xbc)
    {
        hazard = 100;
    }

    if (tile_b == 4)
    {
        hazard = 50;
    }

    // 1b6f
    if (hazard == 0)
    {
        for (index = 0; index < 0x20; index++)
        {
            if (index != entity_c->actorIdx &&
                entity_c->x == D_5c5a_actors[index].x &&
                entity_c->y == D_5c5a_actors[index].y)
            {
                if (D_5c5a_actors[index].tile == TILE_ACTOR_EA)
                {
                    hazard = 100;
                }

                if (D_5c5a_actors[index].tile == TILE_ACTOR_E8)
                {
                    hazard = 50;
                }

                if (D_5c5a_actors[index].tile == TILE_ACTOR_E9)
                {
                    hazard = 150;
                }

                if (hazard != 0)
                    break;
            }
        }
    }

    // 1bdd
    switch (hazard)
    {
    case 150:
        // 1c0c
        PutToSleep(combatant);
        break;

    case 100:
        // 1c14
        HitEffectOnActor(combatant);
        ApplyDamage(combatant, Random(10));
        ReportHit(combatant, 0xff);
        D_a9fa_vitalsDirty = 1;
        break;

    case 50:
        // 1c3a
        if (D_5c5a_actors[entity_c->actorIdx].tile < TILE_ACTOR_80)
        {
            CombatPoison(combatant, -1);
            HitEffectOnActor(combatant);
        }
        break;
    }
}

// A swallowed combatant (flag 4) is brought up again when a roll of 1 to 30 is below its dexterity: "regurgitated!".
static void Regurgitate(int combatant)
{
    if (Random1To30() < combatants[combatant].dex)
    {
        if ((combatants[combatant].flags & COMBAT_FLAGS_PLAYER) != 0)
        {
            ULTIMA_1850_PrintString(D_55a8_party[combatants[combatant].entityIdx].name);
        }
        else
        {
            ULTIMA_1850_PrintString(enemyNames[combatants[combatant].entityIdx]);
        }

        ULTIMA_1850_PrintString(/*0x6f5e*/ " regurgitated!\n");
        ULTIMA_223c_AudioWhiteNoise(1, 7000, 600);

        combatants[combatant].flags &= ~COMBAT_FLAGS_4;

        D_5c5a_actors[combatants[combatant].actorIdx].animTile = D_5c5a_actors[combatants[combatant].actorIdx].tile;
    }
}
