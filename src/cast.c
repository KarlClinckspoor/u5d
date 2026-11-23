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

void SetUpOverworld(void);

u8 fieldMissileKinds[4] = {0x35, 0x33, 0x34, 0x36};

u8 dungeonFieldCells[4] = {0x82, 0x81, 0x80, 0x83};


// 1 for the three who no spell touches: Blackthorn, Lord British and the Shadow Lord.
int IsImmuneToSpells(int combatant)
{
    int type = combatants[combatant].entityIdx;
    if (type == 47 || type == 0xe || type == 0xf)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// A spell that is an attack of its own missile kind (0x30 Grav Por, 0x31 Vas Flam, 0x32 Xen Corp): attacks with it as with a weapon.
static void SpellAttack(int missileKind)
{
    attackWeapon = missileKind;
    AttackWith(curCombatant, missileKind);
}

// The four field spells (0 fire, 1 poison, 2 sleep, 3 energy; In Flam Grav, In Nox Grav, In Zu Grav, In Sanct Grav): in a dungeon the
// cell ahead takes the field if it is empty; in combat a missile of the field's kind is fired (COMSUBS_0c52) and lays it.
static int FieldSpell(int field)
{
    int cell_p;
    int result;
    int cellY;
    int cellX;

    if (mapId < 0x80)
    {
        CastEffect(field == 3 ? 4 : 3);
        cellX = (facingDx[dungeonFacing] + D_5896_mapX) & 7;
        cellY = (facingDy[dungeonFacing] + D_5897_mapY) & 7;

        cell_p = GetDungeonMap(cellX, cellY, D_5895_mapLevel);
        if ((cell_p & 0xf7) != 0)
        {
            result = 0;
        }
        else
        {
            GetDungeonMap(cellX, cellY, D_5895_mapLevel) = (cell_p & 8) | dungeonFieldCells[field];
            result = -1;
        }
    }
    else
    {
        attackWeapon = fieldMissileKinds[field];
        AttackWith(curCombatant, attackWeapon);
        result = -1;
    }

    return result;
}

// An Zu: wakes a sleeping party member (in combat the combatant too). 1 done, 0 not asleep, -1 no member.
static int AnZu(void)
{
    int member;
    int combatant_h;
    int result_b;

    member = OnWho();
    if (member < 0)
    {
        result_b = -1;
    }
    else if (party[member].status == STATUS_SLEEP)
    {
        party[member].status = STATUS_GOOD;
        if (mapId > 0x7f)
        {
            for (combatant_h = 0; combatant_h < 0x20; combatant_h++)
            {
                if ((combatants[combatant_h].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) ==
                    (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_ASLEEP) &&
                    combatants[combatant_h].entityIdx == member)
                {
                    WakeCombatant(combatant_h);
                    break;
                }
            }

            if (combatant_h == 0x20)
            {
                result_b = 0;
            }
        }

        CastEffect(1);
        result_b = -1;
        vitalsDirty = 1;
    }
    else
    {
        result_b = 0;
    }

    return result_b;
}

// An Nox: cures a poisoned member. 1 done, 0 not poisoned, -1 no member.
static int AnNox(void)
{
    int member;
    int result_b;

    member = OnWho();
    if (member < 0)
    {
        result_b = -1;
    }
    else
    {
        if (party[member].status == STATUS_POISONED)
        {
            party[member].status = STATUS_GOOD;
            CastEffect(1);
            vitalsDirty = 1;
            result_b = 1;
        }
        else
        {
            result_b = 0;
        }
    }

    return result_b;
}

// Mani: heals a member by 1-30 hit points (HealMember). -1 for no member.
static int Mani(void)
{
    int member;
    int result_b;

    member = OnWho();
    if (member < 0)
    {
        result_b = -1;
    }
    else
    {
        result_b = HealMember(member);
    }

    if (result_b != 0)
    {
        CastEffect(1);
    }

    return result_b;
}

// An Ylem: in a direction, a piece of furniture (chair, desk, barrel, trunk, mirror..) turns to nothing: "POOF!".
static int AnYlem(void)
{
    int result;
    byte* tilePtr;

    if (AskDirection() == 0)
    {
        return -1;
    }

    CastEffect(1);
    tilePtr = GetTileAddr(tempX, tempY);
    switch (*tilePtr)
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
        *tilePtr = TILE_MAP_44;
        PrintString(/*0x459a*/ "POOF!\n");
        viewDirty |= 2;
        UpdateFrame();
        AudioSweepTone(0x4b0, 2000, 1, 0x28);
        result = -1;
        break;

    default:
        result = 0;
        break;
    }

    return result;
}

// An Sanct: in a dungeon opens (and disarms) the chest underfoot or ahead; elsewhere opens the locked door (B9, BB) or unlocks the
// chest actor in the chosen direction.
static int AnSanct(void)
{
    ActorFmt* actor;
    byte* cellPtr;
    int result_l;
    int actorIdx_n;
    int dx;
    int dy;

    if (mapId > 0x20 && mapId < 0x80)
    {
        CastEffect(2);

        dx = facingDx[dungeonFacing];
        dy = facingDy[dungeonFacing];

        cellPtr = &GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);

        if ((*cellPtr & 0xf0) != 0x40)
        {
            cellPtr =
                &GetDungeonMap((dx + D_5896_mapX) & 7, (dy + D_5897_mapY) & 7, D_5895_mapLevel);
        }

        if ((*cellPtr & 0xf0) == 0x40)
        {
            if ((*cellPtr & 1) != 0)
            {
                PrintString(/*0x45a1*/ "Disarmed!\n");
            }

            *cellPtr = (*cellPtr & 8) | 0x70;

            PrintString(/*0x45ac*/ "Chest opened!\n");

            result_l = -1;
        }
        else
        {
            result_l = 0;
        }
    }
    else
    {
        if (AskDirection() == 0)
        {
            return -1;
        }

        result_l = 0;

        cellPtr = GetTileAddr(tempX, tempY);

        if (*cellPtr == TILE_MAP_DOOR_B9 || *cellPtr == TILE_MAP_DOOR_BB)
        {
            --*cellPtr;

            viewDirty |= 2;

            CastEffect(2);

            result_l = 1;
        }
        else
        {
            for (actorIdx_n = 0; actorIdx_n < 0x20; actorIdx_n++)
            {
                actor = &actors[actorIdx_n];

                if (actor->tile == TILE_ACTOR_CHEST && actor->x == tempX && actor->y == tempY &&
                    (mapId > 0x7f || actor->z == D_5895_mapLevel))
                {
                    actor->value &= 0x7f;

                    CastEffect(2);

                    result_l = 1;
                    break;
                }
            }
        }
    }

    return result_l;
}

// An Xen Corp: every undead monster that does not resist (TargetResists) is left with 1 hit point and flees.
static void AnCorpXen(void)
{
    CombatEntity* target;
    int combatant;

    CastEffect(2);

    for (combatant = 0; combatant < 0x20; combatant++)
    {
        target = &combatants[combatant];
        if (IsImmuneToSpells(combatant) == 0 && (target->flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) == COMBAT_FLAGS_MONSTER &&
            ((byte)enemyFlags[target->entityIdx] & ENEMY_FLAG_UNDEAD) != 0)
        {
            if (TargetResists(curCombatant, combatant, 0) == 0)
            {
                target->hp = 1;
                target->flags |= COMBAT_FLAGS_2;
            }
        }
    }
}

// In Wis: prints where the party is (PrintPosition).
static void InWis(void)
{
    CastEffect(2);
    PrintPosition();
}

// CHECKED
// Kal Xen: gates in a charmed animal (a random one of four kinds) on a free cell, from up to 8 tries. 1 when it came.
static int KalXen(void)
{
    int revealTile;
    u8 animalTile_j;
    int tries_c;
    int result_k;
    int combatant;
    ActorFmt* actor;
    int animal;

    CastEffect(2);

    tries_c = RandomRange(0, 0xf);

    if (tries_c < 6)
    {
        animal = 0x14;
    }
    else if (tries_c < 0xb)
    {
        animal = 0x16;
    }
    else if (tries_c < 0xe)
    {
        animal = 0x15;
    }
    else
    {
        animal = 0x22;
    }

    // 04f9
    result_k = 0;
    tries_c = 0;

    animalTile_j = animal * 4 + 0x40;
    revealTile = animal * 4 + 0x140;

    // 0522
    while (tries_c < 8)
    {
        if (RandomCell() == 0 || IsCellFree(TILE_ACTOR_90, tempX, tempY) == 0)
        {
            tries_c++;
            continue;
        }

        combatant = PlaceCombatant(animal, 0, tempX, tempY, D_5895_mapLevel);
        if (combatant < 0)
        {
            break;
        }

        actor = &actors[combatants[combatant].actorIdx];

        // 056c
        actor->animTile = actor->tile = TILE_ACTOR_CIRCLE;

        // 0588
        ULTIMA_1068_RevealTile(revealTile, tempX, tempY);

        actor->animTile = actor->tile = animalTile_j;
        combatants[combatant].flags |= COMBAT_FLAGS_CHARMED;

        result_k = 1;
        break;
    }
    // 05aa
    return result_k;
}

// In Xen Mani: 1 to 3 food.
static int InManiXen(void)
{
    CastEffect(2);
    IncreaseInt(&food, RandomRange(1, 3), 9999);
    vitalsDirty = 1;
    return 1;
}

// In Por: in combat the caster jumps to a free cell (7 tries, not in the combat that forbids it); outdoors the party jumps along the chosen
// direction to the last grass cell in view. -1 for no direction, 1 or 0.
static int InPor(void)
{
    int stepY_k;
    int stepX_l;
    int limitY_h;
    int limitX_i;
    int result;
    int y;
    int x;
    int attempt_b;
    ActorFmt* actor_h;

    result = 0;
    if (mapId > 0x7f)
    {
        actor_h = &actors[combatants[curCombatant].actorIdx];
        if ((combatFlags & 2) != 0)
        {
            result = 0;
        }
        else
        {
            CastEffect(3);
            for (attempt_b = 0; attempt_b < 7; attempt_b++)
            {
                if (RandomCell() == 0)
                    continue;

                if (IsCellFree(actor_h->tile, tempX, tempY) == 0)
                    continue;

                actor_h->x = combatants[curCombatant].x = tempX;
                actor_h->y = combatants[curCombatant].y = tempY;

                result = 1;
                break;
            }
        }
    }
    else
    {
        // 0680
        if (AskDirection() == 0)
        {
            return -1;
        }
        else
        {
            // 068e
            CastEffect(3);

            stepX_l = tempX - D_5896_mapX;
            stepY_k = tempY - D_5897_mapY;

            limitX_i = mapOriginX + 0x20;
            if (limitX_i > 0x100)
            {
                limitX_i = 0x100;
            }

            limitY_h = mapOriginY + 0x20;
            if (limitY_h > 0x100)
            {
                limitY_h = 0x100;
            }

            // 06d9
            result = 0;
            x = (byte)tempX;
            for (y = (byte)tempY; mapOriginX <= x && x < limitX_i && mapOriginY <= y && y < limitY_h;
                 y += stepY_k)
            {
                if (*GetTileAddr(x, y) == TILE_MAP_GRASS)
                {
                    result = -1;
                    D_5896_mapX = x;
                    D_5897_mapY = y;
                    viewDirty = 1;
                }
                x += stepX_l;
            }

            if (result != 0)
            {
                SetUpOverworld();
            }
        }
    }

    return result;
}

// Wis Quas: every invisible monster shows again.
static void QuasWis(void)
{
    int combatant;
    ActorFmt* actor;

    CastEffect(4);

    for (combatant = 0; combatant < 0x20; combatant++)
    {
        if (combatants[combatant].flags != 0 &&
            (combatants[combatant].flags & COMBAT_FLAGS_PLAYER) == 0 &&
            (combatants[combatant].flags & COMBAT_FLAGS_INVISIBLE) != 0)
        {
            combatants[combatant].flags &= ~COMBAT_FLAGS_INVISIBLE;
            actor = &actors[combatants[combatant].actorIdx];
            actor->animTile = actor->tile;

            PutActorsInView();
            DrawMap();
        }
    }
}

// In Bet Xen: up to four charmed swarms of insects (0x1f) on a free cell found in 8 tries.
static int BetInXen(void)
{
    int attempt;
    int result;
    int swarm;

    CastEffect(5);

    result = 0;

    // 07c7
    for (attempt = 0; attempt < 8; attempt++)
    {
        if (RandomCell() == 0 || IsCellFree(TILE_ACTOR_BC, tempX, tempY) == 0)
            continue;

        result = 1;
        break;
    }

    if (result != 0)
    {
        result = 0;

        for (attempt = 0; attempt < 4; attempt++)
        {
            swarm = PlaceCombatant(0x1f, 0, tempX, tempY, D_5895_mapLevel);
            if (swarm < 0)
            {
                break;
            }

            combatants[swarm].flags |= COMBAT_FLAGS_CHARMED;
            result = 1;
        }
    }

    return result;
}

// An Ex Por: in a direction an open door (B8, B9, BA, BB) is locked by magic (tile 97 or 98). -1 for no direction.
static int AnExPor(void)
{
    int result;
    byte* tilePtr;

    if (AskDirection() == 0)
    {
        return -1;
    }
    else
    {
        CastEffect(5);
        tilePtr = GetTileAddr(tempX, tempY);
        result = 0;
        switch (*tilePtr)
        {
        case TILE_MAP_DOOR_B8:
        case TILE_MAP_DOOR_B9:
            *tilePtr = TILE_MAP_97;
            result = 1;
            viewDirty |= 2;
            break;

        case TILE_MAP_DOOR_BA:
        case TILE_MAP_DOOR_BB:
            *tilePtr = TILE_MAP_98;
            result = 1;
            viewDirty |= 2;
            break;
        }

        return result;
    }
}

// Vas Mani: heals a member to full hit points (not the dead, not in a battle still going). -1 for no member.
static int ManiVas(void)
{
    int member;
    int result_b;

    member = OnWho();
    if (member < 0)
    {
        result_b = -1;
    }
    else if (party[member].status == STATUS_DEAD || (mapId > 0x7f && combatOver == 0))
    {
        result_b = 0;
    }
    else if (mapId > 0x7f && combatOver == 0) // sic
    {
        result_b = 0;
    }
    else
    {
        party[member].hp = party[member].maxHp;
        CastEffect(5);
        vitalsDirty = 1;
        result_b = 1;
    }

    return result_b;
}

// In Vas Por Ylem: the earthquake: every enemy that fails a dexterity roll takes 1 to 20 damage, the caster earns experience for it.
static void InPorVasYlem(int caster)
{
    int combatant;

    CastEffect(6);
    ShakeScreen();
    for (combatant = 0; combatant < 0x20; combatant++)
    {
        if (combatants[combatant].flags != 0)
        {
            if (ULTIMA_5646_IsEnemy(combatant) != 0)
            {
                if (QueryStat(combatant, QUERY_STAT_DEX) <= Random1To30())
                {
                    HitEffectOnActor(combatant);
                    IncreaseInt(&party[caster].exp, ApplyDamage(combatant, RandomRange(1, 0x14)), 9999); // FMT: &local_8 ??
                    ReportHit(combatant, curCombatant);
                }
            }
        }
    }
}

// An Xen Exe: aims at a creature and charms it (or frees it again) unless it resists. -1 when cancelled.
static int AnExXen(void)
{
    int target;

    PrintString(/*0x45bb*/ "Creature: ");
    if (AimCrosshair(curCombatant, 0xf) == 0)
    {
        return -1;
    }

    // 09c6
    CastEffect(6);
    if ((*GetTileAddr(crosshairX, crosshairY) & 0xfe) == TILE_MAP_84)
    {
        return 0;
    }

    target = CombatantAt(crosshairX, crosshairY);
    if (target >= 0)
    {
        if (IsImmuneToSpells(target) == 0 && ULTIMA_5646_IsEnemy(target) != 0 && TargetResists(curCombatant, target, 0) == 0)
        {
            combatants[target].flags ^= COMBAT_FLAGS_CHARMED;
            if ((combatants[target].flags & COMBAT_FLAGS_PLAYER) != 0)
            {
                party[combatants[target].entityIdx].status = STATUS_GOOD;
                UpdateVitalsDisplay();
            }

            PrintName(target);
            PrintString(/*0x45c6*/ " charmed!\n");

            return -1;
        }
    }

    return 0;
}

// Bet Rel Xen: aims at a creature and, unless it resists, changes it into a giant rat (0x14) on the same cell. -1 when cancelled.
static int BetRelXen(void)
{
    int targetY_p;
    int targetX;
    int target;

    PrintString(/*0x45d1*/ "Creature: ");
    if (AimCrosshair(curCombatant, 0xf) == 0)
    {
        return -1;
    }

    // 0a82
    CastEffect(6);
    target = CombatantAt(crosshairX, crosshairY);
    if (target < 0)
    {
        return 0;
    }

    if (IsImmuneToSpells(target) == 0 && TargetResists(curCombatant, target, 0) == 0)
    {
        targetX = combatants[target].x;
        targetY_p = combatants[target].y;
        RemoveEntity(-target - 1);
        PlaceCombatant(0x14, 0, targetX, targetY_p, D_5895_mapLevel);
        return 1;
    }

    return 0;
}

// Sanct Lor: the caster turns invisible.
static int LorSanct(void)
{
    actors[combatants[curCombatant].actorIdx].animTile = TILE_ACTOR_1D;
    combatants[curCombatant].flags |= COMBAT_FLAGS_INVISIBLE;
    CastEffect(7);
    return 1;
}

// In Quas Xen: aims at a creature and makes a copy of it (a free combatant and actor) placed on a free cell. -1 when cancelled.
static int InQuasXen(void)
{
    int freeActor;
    int index;
    int result_i;
    int target;
    int freeCombatant_f;

    PrintString(/*0x45dc*/ "Creature: ");
    if (AimCrosshair(curCombatant, 0xf) == 0)
    {
        return -1;
    }

    CastEffect(7);
    target = CombatantAt(crosshairX, crosshairY);
    if (target < 0)
    {
        result_i = 0;
    }
    else
    {
        // 0b72
        freeCombatant_f = freeActor = 0xff;
        for (index = 0; index < 0x20; index++)
        {
            if (actors[index].tile == 0)
            {
                freeActor = index;
                break;
            }
        }

        if (freeActor != 0xff)
        {
            for (index = 0; index < 0x20; index++)
            {
                if (combatants[index].flags == 0)
                {
                    freeCombatant_f = index;
                    break;
                }
            }

            if (freeCombatant_f != 0xff)
            {
                // 0bbc
                combatants[freeCombatant_f] = combatants[target];
                
                combatants[freeCombatant_f].actorIdx = freeActor;

                actors[freeActor] = actors[combatants[target].actorIdx];

                do
                {
                    if (RandomCell() == 0 || IsCellFree(TILE_ACTOR_AVATAR, tempX, tempY) == 0)
                    {
                        result_i = 0;
                    }
                    else
                    {
                        actors[freeActor].x = combatants[freeCombatant_f].x = tempX;
                        actors[freeActor].y = combatants[freeCombatant_f].y = tempY;
                        result_i = 1;
                    }
                } while (result_i == 0);
            }
        }
    }

    return result_i;
}

// In Quas Corp: every monster that does not resist is left with 1 hit point and flees (fear).
static void CorpInQuas(void)
{
    int combatant;
    CombatEntity* target;

    CastEffect(7);

    for (combatant = 0; combatant < 0x20; combatant++)
    {
        target = &combatants[combatant];
        if ((target->flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER)) == COMBAT_FLAGS_MONSTER)
        {
            if (IsImmuneToSpells(combatant) == 0)
            {
                if (TargetResists(curCombatant, combatant, 0) == 0)
                {
                    target->hp = 1;
                    target->flags |= COMBAT_FLAGS_2;
                }
            }
        }
    }
}

// Vas Rel Por: asks "To phase:" a moonstone (1-8) and moves the party to its buried place; not aboard a ship.
static int PorRelVas(void)
{
    byte key;

    if ((partyTile & 0xf0) != TILE_ACTOR_FRIGATE_20)
    {
        PrintString(/*0x45e7*/ "To phase: ");
        key = GetChar();
        if (key >= 0x20)
        {
            PrintChar(key);
        }

        PrintChar('\n');

        if (key >= 0x31 && key <= 0x38)
        {
            key -= 0x31;
            CastEffect(8);
            if (GoToMoonstone(key) != 0)
            {
                return -1;
            }
        }
    }

    return 0;
}

// An Tym: negates time for 10 turns (status T); a Shadow Lord on the map absorbs the magic.
static int AnTym(void)
{
    int result;
    int actorIdx;

    CastEffect(8);
    result = -1;

    for (actorIdx = 0; actorIdx < 0x20; actorIdx++)
    {
        if (actors[actorIdx].tile == TILE_ACTOR_SHADOWLORD)
        {
            PrintString(/*0x45f2*/ "Magic absorbed!\n");
            AudioPulse(0x2648, 1, 28000, 1000, 2);
            result = 0;
            break;
        }
    }

    if (result != 0)
    {
        statusIcon = 0x54;
        protectTurns = 10;
        UpdateVitalsDisplay();
    }

    return result;
}

// defined in castb.c, the part of CAST.OVL built with MSC 5.0
void ConeSpell(int caster, int kind, int color);

static char S_0x466a[] = "Scroll\n\n";

// The Cast command. Asks the acting player, reads the spell (SelectSpell), checks that it can be cast here (spellUsability: bit 8 on
// the overworld, 4 in a town, 2 in a dungeon, 1 in combat; Absorbed! in Lord British's castle without the crown and at Stonegate),
// that it is mixed, and that the caster has the magic points and the level for its circle, then calls the spell by its index.
// Prints Success! or Failed!.
int CastSpellCmd(void)
{
    int allowedHere;
    int result_d;
    int circle;
    int passTurn;
    int caster_o;
    int spell;

    result_d = allowedHere = D_5890_spellAttack = castCircle = passTurn = 1;

    caster_o = GetActingPlayer();
    if (caster_o < 0)
    {
        return caster_o;
    }

    PrintString(/*0x4603*/ "Spell name:\n:");
    spell = SelectSpell();
    if (spell == -1)
    {
        PrintString(/*0x4611*/ "None!\n");
        return; // sic
    }

    if (spell == -2)
    {
        PrintString(/*0x4618*/ "No effect!\n");
        return; // sic
    }

    castCircle = circle = spell / 6 + 1;

    if (mapId == 0)
    {
        if ((spellUsability[spell] & 8) == 0)
        {
            allowedHere = 0;
        }
    }
    else if (mapId > 0x7f)
    {
        if ((spellUsability[spell] & 1) == 0)
        {
            allowedHere = 0;
        }
    }
    else
    {
        if (((mapId == 0x12) && (crown == 0)) || (mapId == 0x1d)) // LB crown / at STONEGATE
        {
            PrintString(/*0x4624*/ "Absorbed!\n");
            AudioPulse(0x2648, 1, 28000, 1000, 2);
            return; // sic
        }

        if (mapId < 0x21)
        {
            if ((spellUsability[spell] & 4) == 0)
            {
                allowedHere = 0;
            }
        }
        else
        {
            if ((spellUsability[spell] & 2) == 0)
            {
                allowedHere = 0;
            }
        }
    }

    if (allowedHere == 0)
    {
        PrintString(/*0x462f*/ "Not here!\n");
        AudioSweepTone(800, 2000, 1, 0x32);
        return; // sic
    }

    if (mixedSpells[spell] == 0)
    {
        PrintString(/*0x463a*/ "None mixed!\n");
        return; // sic
    }

    mixedSpells[spell]--;
    if (party[caster_o].mag < circle)
    {
        PrintString(/*0x4647*/ "M.P. too low!\n");
        result_d = 0;
    }
    else
    {
        party[caster_o].mag -= circle;
        result_d = -1;
        if (party[caster_o].level < circle)
        {
            result_d = 0;
        }
        else
        {
            // spellCodes idx
            switch (spell)
            {
            case 0: // IL
                SetLightTurns(100);
                CastEffect(1);
                break;
            case 1: // GP
                SpellAttack(0x30);
                break;
            case 2: // AZ
                result_d = AnZu();
                break;
            case 3: // AN
                result_d = AnNox();
                break;
            case 4: // M
                result_d = Mani();
                break;
            case 5: // AY
                result_d = AnYlem();
                break;
            case 6: // AS
                result_d = AnSanct();
                break;
            case 7: // ACX
                AnCorpXen();
                break;
            case 8: // HR
                ChangeWindDirection(AskDirection(), 0);
                break;
            case 9: // IW
                InWis();
                break;
            case 10: // KX
                result_d = KalXen();
                break;
            case 0xb: // IMX
                result_d = InManiXen();
                break;
            case 0xc: // LV
                SetLightTurns(0xff);
                CastEffect(3);
                break;
            case 0xd: // FV
                SpellAttack(0x31);
                break;
            case 0xe: // FGI
                result_d = FieldSpell(0);
                break;
            case 0xf: // GIN
                result_d = FieldSpell(1);
                break;
            case 0x10: // GIZ
                result_d = FieldSpell(2);
                break;
            case 0x11: // IP
                result_d = InPor();
                break;
            case 0x12: // AG
                result_d = AnGrav(1);
                break;
            case 0x13: // IS
                SetSpellStatus(0x50, 0x14, 4);
                break;
            case 0x14: // GIS
                result_d = FieldSpell(3);
                break;
            case 0x15: // PU
                if (mapId == 40)
                {
                    result_d = 0;
                    break;
                }

                CastEffect(4);
                if (Klimb(-1, 1) != 0)
                {
                    DUNGEON_1d08_Exit();
                }
                break;
            case 0x16: // DP
                if (mapId == 40)
                {
                    result_d = 0;
                    break;
                }

                CastEffect(4);
                if (Klimb(1, 1) != 0)
                {
                    DUNGEON_1d08_Exit();
                }
                break;
            case 0x17: // QW
                QuasWis();
                break;
            case 0x18: // BIX
                result_d = BetInXen();
                break;
            case 0x19: // AEP
                result_d = AnExPor();
                break;
            case 0x1a: // EIP
                result_d = InExPor();
                if (result_d == -1)
                    break;
                CastEffect(5);
                break;
            case 0x1b: // MV
                result_d = ManiVas();
                break;
            case 0x1c: // IZ
                ConeSpell(curCombatant, 1, magentaColor);
                break;
            case 0x1d: // RT
                SetSpellStatus(0x51, 0x1e, 5);
                break;
            case 0x1e: // IPVY
                InPorVasYlem(caster_o);
                break;
            case 0x1f: // AQW
                SetSpellStatus(0x43, 0x14, 6);
                break;
            case 0x20: // AI
                SetSpellStatus(0x4e, 10, 6);
                break;
            case 0x21: // AWY
                CastEffect(6);
                WisAnYlem();
                break;
            case 0x22: // AEX
                result_d = AnExXen();
                break;
            case 0x23: // BRX
                result_d = BetRelXen();
                break;
            case 0x24: // LS
                result_d = LorSanct();
                break;
            case 0x25: // CX
                SpellAttack(0x32);
                break;
            case 0x26: // IQX
                result_d = InQuasXen();
                break;
            case 0x27: // IQW view?
                CastEffect(7);
                if (mapId < 0x21)
                {
                    LookobjViewCmd(D_5896_mapX, D_5897_mapY);
                }
                else
                {
                    DnglookViewCmd();
                }
                break;
            case 0x28: // HIN
                ConeSpell(curCombatant, 2, greenColor);
                break;
            case 0x29: // CIQ
                CorpInQuas();
                break;
            case 0x2a: // CIM
                result_d = Resurrect(OnWho(), 0);
                UpdateVitalsDisplay();
                break;
            case 0x2b: // CKX
                result_d = KalXenCorp(0);
                break;
            case 0x2c: // CGIV
                ConeSpell(curCombatant, 4, blueColor);
                break;
            case 0x2d: // FHI
                ConeSpell(curCombatant, 3, redColor);
                break;
            case 0x2e: // PRV
                result_d = PorRelVas();
                if (result_d != 0)
                {
                    passTurn = 0;
                }
                break;
            case 0x2f: // AT
                result_d = AnTym();
                break;
            }
        }
    }

    if (result_d == 1)
    {
        PrintString(/*0x4656*/ "Success!\n");
    }
    else if (result_d == 0)
    {
        PrintString(/*0x4660*/ "Failed!\n");
        AudioSweepTone(800, 2000, 1, 0x32);
    }

    return passTurn;
}

// The Use command on a scroll (0 Light, 1 Wind change, 2 Protection, 3 Negate magic, 4 View, 5 Summon Daemon, 6 Resurrection, 7 Negate time).
static int UseScroll(int scroll)
{
    int direction;
    int result_b = 1;

    scrolls[scroll]--;
    PrintString(S_0x466a);
    switch (scroll)
    {
    case 0:
        SetLightTurns(0xf0);
        PrintString(/*0x4673*/ "Light!\n");
        CastEffect(0);
        break;

    case 1:
        PrintString(/*0x467b*/ "Wind change!\n");
        direction = AskDirection();
        if (mapId < 0x21)
        {
            ChangeWindDirection(direction, 1);
            break;
        }
        result_b = 0;
        break;

    case 2:
        PrintString(/*0x4689*/ "Protection!\n");
        SetSpellStatus(0x50, 100, 2);
        break;

    case 3:
        PrintString(/*0x4696*/ "Negate magic!\n");
        SetSpellStatus(0x4e, 0x14, 3);
        break;

    case 4:
        // 1278
        PrintString(/*0x46a5*/ "View!\n");
        if (mapId > 0x7f)
        {
            PrintString(/*0x46ac*/ "Not here!\n");
            break;
        }
        else
        {
            // 1290
            CastEffect(4);
            if (mapId < 0x21)
            {
                LookobjViewCmd(D_5896_mapX, D_5897_mapY);
                break;
            }
            DnglookViewCmd();
            break;
        }

    case 5:
        // 12b4
        PrintString(/*0x46b7*/ "Summon Daemon!\n");
        if (mapId > 0x7f)
        {
            result_b = KalXenCorp(1);
            break;
        }
        else
        {
            PrintString(/*0x46c7*/ "Not here!\n");
            break;
        }

    case 6:
        // 12d2
        PrintString(/*0x46d2*/ "Resurrection!\n");
        if (mapId < 0x80)
        {
            result_b = Resurrect(OnWho(), 1);
            UpdateVitalsDisplay();
            break;
        }
        PrintString(/*0x46e1*/ "Not here!\n");
        break;

    case 7:
        if ((mapId == 0x1d) || (mapId == 0x28))
        {
            PrintString(/*0x46ec*/ "No effect!\n");
            AudioSweepTone(800, 2000, 1, 0x32);
            break;
        }
        PrintString(/*0x46f8*/ "Negate time!\n");
        SetSpellStatus(0x54, 0x14, 7);
        break;
    }

    return result_b;
}

static char S_0x474c[] = "\nNo noticeable effect now!\n";

// CHECKED
// The Use command on a potion: asks on whom (in combat the user); 1 in 16 it is a sleeping draught instead and 1 in 16 a random other.
// The kinds: 0 awakens, 1 heals, 2 cures poison, 3 poisons, 4 puts to sleep, 5 poof (vanish, in combat), 6 invisible (in combat), 7 X-ray (outdoors).
static int UsePotion(int potion)
{
    ActorFmt* actor;
    int roll;
    int result_l;
    int member_d;

    result_l = 1;
    potions[potion]--;
    PrintString(/*0x4706*/ "Potion\n");
    if (mapId > 0x7f)
    {
        member_d = combatants[curCombatant].entityIdx;
    }
    else
    {
        member_d = OnWho();
    }

    if (member_d < 0)
    {
        return member_d;
    }

    CastEffect(potion);
    roll = RandomRange(0, 0xf);
    if (roll == 0)
    {
        potion = 4;
    }
    else if (roll == 1)
    {
        potion = RandomRange(0, 7);
    }

    switch (potion)
    {
    case 0:
        if (party[member_d].status == STATUS_SLEEP)
        {
            party[member_d].status = STATUS_GOOD;

            if (mapId > 0x7f)
            {
                if (member_d == combatants[curCombatant].entityIdx)
                {
                    if ((combatants[curCombatant].flags & (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_MONSTER | COMBAT_FLAGS_DEAD | COMBAT_FLAGS_ASLEEP)) ==
                        (COMBAT_FLAGS_PLAYER | COMBAT_FLAGS_ASLEEP))
                    {
                        WakeCombatant(curCombatant);
                    }
                    else
                    {
                        result_l = 0;
                    }
                }
                else
                {
                    result_l = 0;
                }
            }
            else
            {
                UpdateVitalsDisplay();
            }
        }
        else
        {
            result_l = 0;
        }
        break;

    case 1:
        result_l = HealMember(member_d);
        if (result_l != 0)
        {
            PrintString(/*0x470e*/ "Healed!\n");
            UpdateVitalsDisplay();
        }
        break;

    case 2:
        if (party[member_d].status == STATUS_POISONED)
        {
            party[member_d].status = STATUS_GOOD;

            PrintString(/*0x4717*/ "Poison cured!\n");
            UpdateVitalsDisplay();
        }
        else
        {
            result_l = 0;
        }
        break;

    case 3:
        if (party[member_d].status == STATUS_GOOD)
        {
            party[member_d].status = STATUS_POISONED;

            PrintString(/*0x4726*/ "POISONED!\n");
            UpdateVitalsDisplay();
        }
        else
        {
            result_l = 0;
        }
        break;

    case 4:
        if (party[member_d].status == STATUS_GOOD)
        {
            if (mapId < 0x80)
            {
                party[member_d].status = STATUS_SLEEP;
            }
            else
            {
                PutToSleep(member_d);
            }

            PrintString(/*0x4731*/ "Slept!\n");
            UpdateVitalsDisplay();
        }
        else
        {
            result_l = 0;
        }
        break;

    case 5:
        if (mapId > 0x7f)
        {
            PrintString(/*0x4739*/ "Poof!\n");
            actor = &actors[combatants[curCombatant].actorIdx];
            actor->tile = actor->animTile = TILE_ACTOR_90;
        }
        else
        {
            PrintString(S_0x474c);
        }
        break;

    case 6:
        if (mapId > 0x7f)
        {
            combatants[curCombatant].flags |= COMBAT_FLAGS_INVISIBLE;
            actor = &actors[combatants[curCombatant].actorIdx];
            PrintString(/*0x4740*/ "Invisible!\n");
            actor->tile = actor->animTile = TILE_ACTOR_1D;
        }
        else
        {
            PrintString(S_0x474c);
        }
        break;

    case 7:
        if (mapId < 0x21)
        {
            WisAnYlem();
        }
        else
        {
            PrintString(S_0x474c);
        }
        break;
    }

    return result_l;
}

// The Use command on a moonstone: buries it at the party's place when the ground is grass, crops or the like.
static void UseMoonstone(int stone)
{
    int tile;

    tile = *GetTileAddr(D_5896_mapX, D_5897_mapY);
    PrintString(/*0x4768*/ "Moonstone ");
    if (mapId < 0x21 && (tile == TILE_MAP_CROPS_PICKED || tile == TILE_MAP_CROPS || (tile > TILE_MAP_WATER_3 && tile < TILE_MAP_B)))
    {
        PrintString(/*0x4773*/ "buried!\n");
        moonstoneX[stone] = D_5896_mapX;
        moonstoneY[stone] = D_5897_mapY;
        moonstoneMap[stone] = mapId;
        moonstoneZ[stone] = D_5895_mapLevel;
    }
    else
    {
        PrintString(/*0x477c*/ "cannot be buried here!\n");
    }
}

u8 D_4882_shardX[4] = {0x0f, 0x0f, 0x0f};

u8 D_4886_shardY[4] = {0x09, 0x03, 0x10};

u8 shardMap[4] = {0x1e, 0x1f, 0x20};

u8 shardLevel[4] = {0x02, 0x01, 0xff};

u8 shardKilledBit[3] = {0x02, 0x04, 0x08};

// CHECKED
// The Use command on an evil gem shard (0 Falsehood, 1 Hatred, 2 Cowardice): cast into the right flame (D_4882..) in front of its Shadow Lord
// (Faulinei, Astaroth, Nosfentor), it destroys him.
static void UseGemShard(int shard)
{
    int index;

    PrintString(/*0x4794*/ "Gem Shard\n\nThou dost hold above thee the evil Shard of ");
    switch (shard)
    {
    case 0:
        PrintString(/*0x47cc*/ "Falsehood...");
        break;
    case 1:
        PrintString(/*0x47d9*/ "Hatred...");
        break;
    case 2:
        PrintString(/*0x47e3*/ "Cowardice...");
        break;
    }

    for (index = 2000; index < 25000; index += 0x32)
    {
        AudioPulse(0xa50, 1, 200, index, 0);
    }

    for (index = 25000; index > 2000; index -= 0x32)
    {
        AudioPulse(0xa50, 1, 200, index, 0);
    }

    if (D_4882_shardX[shard] != D_5896_mapX || D_4886_shardY[shard] != D_5897_mapY || shardMap[shard] != mapId ||
        shardLevel[shard] != D_5895_mapLevel)
    {
        PrintString(/*0x47f0*/ "\n\nNo effect!\n");
        AudioSweepTone(800, 2000, 1, 0x32);
        return;
    }

    WaitFrames(7);
    PrintString(/*0x47fe*/ "\n\n...and cast it into the Flame of ");
    switch (shard)
    {
    case 0:
        PrintString(/*0x4822*/ "Truth!\n");
        break;
    case 1:
        PrintString(/*0x482a*/ "Love!\n");
        break;
    case 2:
        PrintString(/*0x4831*/ "Courage!\n");
        break;
    }

    ShakeScreen();
    ShakeScreen();
    ShakeScreen();
    WaitFrames(3);

    if (FindActorTileAtPos(D_5896_mapX, D_5897_mapY - 1, D_5895_mapLevel) == TILE_ACTOR_SHADOWLORD &&
        shard == summonedLord)
    {
        DespawnNpc(FindNpcByActor(tempX));

        for (index = 0; index < 7; index++)
        {
            HitEffectAt(D_5896_mapX, D_5897_mapY - 1);
        }

        shadowlordTowns[shard] = 0xff;
        shards[shard] = 0;

        *(u32*)&npcKilled[0x70] |= shardKilledBit[shard];

        PrintString(/*0x483b*/ "\nThe doom of the Shadowlord ");
        switch (shard)
        {
        case 0:
            PrintString(/*0x4858*/ "Faulinei");
            break;
        case 1:
            PrintString(/*0x4861*/ "Astaroth");
            break;
        case 2:
            PrintString(/*0x486a*/ "Nosfentor");
            break;
        }

        PrintString(/*0x4874*/ " is wrought!\n");
        AudioVictoryTune();
    }
}

// Wearing an item that sets the status icon: 0 ("Removed!") when the item is already worn and is taken off, 1 otherwise.
static bool RemoveWornItem(int icon)
{
    if (icon == statusIcon)
    {
        PrintString(/*0x4895*/ "Removed!\n");

        statusIcon = protectTurns = 0;
        vitalsDirty = 1;

        return 0;
    }

    return 1;
}

static char S_0x4a84[] = " of Lord British...\n";

// The Use command: lists the usable items (ZSTATS) and uses the one chosen: scrolls, potions, moonstones, the carpet, skull key,
// amulet, crown, sceptre, gem shards, spyglass, plans, sextant, watch, badge.
void UseCmd(void)
{
    int item;
    int dy;
    int hour12_l;
    int dx;
    int result_e;
    int fieldResult;
    byte* tilePtr_m;
    int firstItem_j;
    int cleared;

    result_e = 1;
    FillItemCounts();

    firstItem_j = NextItem(-1, 0x26, itemCounts, 0xff);
    if (firstItem_j == -1)
    {
        PrintString(/*0x489f*/ "No usable items!\n");
        return;
    }

    PrintString(/*0x48b1*/ "Item: ");
    OpenStatsDivider();
    SelectTextWindow(1);
    ShowStatsTitle(/*0x48b8*/ "Items:");
    DrawListFrame(8);

    item = SelectItem(firstItem_j, 0xff, 0x55);
    HideScrollArrow();
    ClearStatsTitle();
    DrawStatsDivider();
    UpdateVitalsDisplay();
    if (item < 0)
    {
        return;
    }

    if (item < 8)
    {
        result_e = UseScroll(item);
    }
    else if (item < 0x10)
    {
        result_e = UsePotion(item - 8);
    }
    else if (item > 0x14 && item < 0x1d)
    {
        UseMoonstone(item - 0x15);
    }
    else
    {
        switch (item)
        {
        case 0x10:
            PrintString(/*0x48bf*/ "Carpet\n\n");
            if (mapId < 0x21 && *GetTileAddr(D_5896_mapX, D_5897_mapY) != TILE_MAP_C)
            {
                if (partyTile == TILE_ACTOR_AVATAR)
                {
                    PrintString(/*0x48c8*/ "Boarded!\n");
                    partyTile = RandomRange(0, 1) + TILE_ACTOR_FLYING_CARPET;
                    carpets--;
                }
                else if ((partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
                {
                    PrintString(/*0x48d2*/ "X-it ship first!\n");
                }
                else
                {
                    PrintString(/*0x48e4*/ "Only on foot!\n");
                }
            }
            else
            {
                PrintString(/*0x48f3*/ "Not here!\n");
            }
            break;

        case 0x11:
            skullKeys--;
            PrintString(/*0x48fe*/ "Skull Key\n");
            if (mapId < 0x21 || 0x7f < mapId)
            {
                result_e = InExPor();
                if (result_e != 0 && mapId < 0x80)
                {
                    HitEffectAt(tempX, tempY);
                }
                break;
            }

            PrintString(/*0x4909*/ "Not here!\n");
            break;

        case 0x12:
            PrintString(/*0x4914*/ "Amulet\n\n");
            if (RemoveWornItem(0xe) == 0)
                break;

            PrintString(/*0x491d*/ "Wearing the Amulet");
            PrintString(S_0x4a84);
            SetSpellStatus(0xe, 0xff, 9);
            break;

        case 0x13:
            PrintString(/*0x4930*/ "Crown\n\n");
            if (RemoveWornItem(0x1c) == 0)
                break;

            PrintString(/*0x4938*/ "Thou dost don the Crown");
            PrintString(S_0x4a84);
            SetSpellStatus(0x1c, 0xff, 9);
            break;

        case 0x14:
            PrintString(/*0x4950*/ "Sceptre\n\n");
            PrintString(/*0x495a*/ "Wielding the Sceptre");
            PrintString(S_0x4a84);
            AudioPulse(0x1450, 1, 50000, 5000, 1);

            cleared = 0;
            if (mapId < 0x21 || mapId > 0x28)
            {
                for (dx = -1; dx <= 1; dx++)
                {
                    for (dy = -1; dy <= 1; dy++)
                    {
                        tilePtr_m = GetTileAddr(dx + D_5896_mapX, dy + D_5897_mapY);
                        if ((*tilePtr_m & 0xf0) == TILE_MAP_70)
                        {
                            *tilePtr_m = TILE_MAP_GRASS;
                            UpdateFrame();
                            AudioWhiteNoise(10, 3000, 2000);
                            cleared++;
                        }
                    }
                }
            }

            if (cleared != 0)
                break;

            fieldResult = AnGrav(0);
            if (fieldResult == 1)
            {
                PrintString(/*0x496f*/ "Field dissolved!\n");
            }
            else if (fieldResult == 0)
            {
                PrintString(/*0x4981*/ "No effect!\n");
            }
            break;

        case 0x1d:
        case 0x1e:
        case 0x1f:
            UseGemShard(item - 0x1d);
            break;

        case 0x20:
            PrintString(/*0x498d*/ "Spyglass\n\n");
            if (mapId < 0x21 && D_5895_mapLevel < 0x80)
            {
                if (D_587f_hour < 6 || 0x12 < D_587f_hour)
                {
                    PrintString(/*0x4998*/ "Looking...\n");
                    LookSky();
                    break;
                }

                PrintString(/*0x49a4*/ "No stars!\n");
            }
            else
            {
                PrintString(/*0x49af*/ "Not here!\n");
            }
            break;

        case 0x21:
            PrintString(/*0x49ba*/ "Plans\n\n");
            if ((partyTile & 0xf8) == TILE_ACTOR_FRIGATE_20)
            {
                hmsCapePlans |= 0x80;
                PrintString(/*0x49c2*/ "Ship rigged for double speed!\n");
            }
            else
            {
                PrintString(/*0x49e1*/ "Only usable on shipboard!\n");
            }
            break;

        case 0x22:
            PrintString(/*0x49fc*/ "Sextant\n\n");
            if (D_5895_mapLevel > 0x7f || mapId != 0)
            {
                PrintString(/*0x4a06*/ "Only outdoors!\n");
            }
            else
            {
                if (D_587f_hour > 5 && D_587f_hour < 0x13)
                {
                    PrintString(/*0x4a16*/ "Only at night!\n");
                }
                else
                {
                    PrintString(/*0x4a26*/ "Position:");
                    PrintPosition();
                }
            }
            break;

        case 0x23:
            PrintString(/*0x4a30*/ "Watch\n\nThe pocket watch reads ");
            hour12_l = (byte)(D_587f_hour % 0xc);
            if (hour12_l == 0)
            {
                hour12_l = 0xc;
            }

            PrintNumber(hour12_l, 1, 0x20);
            PrintChar(':');
            PrintNumber(minute, 2, 0x30);
            if (D_587f_hour > 0xb)
            {
                PrintString(/*0x4a4f*/ " PM.\n");
            }
            else
            {
                PrintString(/*0x4a55*/ " AM.\n");
            }
            break;

        case 0x24:
            PrintString(/*0x4a5b*/ "Badge\n\n");
            if (RemoveWornItem(0x1d) != 0)
            {
                PrintString(/*0x4a63*/ "Badge worn!\n");
                statusIcon = 0x1d;
                protectTurns = 0xff;
                vitalsDirty = 1;
            }
            break;

        case 0x25:
            PrintString(/*0x4a70*/ "Box\n\nHow?\n");
            break;
        }
    }

    if (result_e == 0)
    {
        PrintString(/*0x4a7b*/ "Failed!\n");
        AudioSweepTone(800, 2000, 1, 0x32);
    }
}

// Plots a pixel of a spray at (x, y) in a color (and a click): -1 in both D_5876 and D_5878 when off the view; 0 when the cell it is in stops
// it, else 1.
int PlotSprayPixel(int x, int y, int penColor)
{
    if (x < 8 || x > 0xb6 || y < 8 || y > 0xb6)
    {
        tempX = tempY = -1;
        return 0;
    }
    else
    {
        GRAP_2d_SetPenColor(penColor);
        GRAP_30_Pset(x, y);
        GRAP_30_Pset(x + 1, y);
        PcspkOn(RandomRange(100, 10000));

        if ((y & 1) != 0)
        {
            PixelToViewCell(x, y);
            if (tempX >= 0 && tempY >= 0)
            {
                return TestViewTileFlag(tempX, tempY);
            }

            return 0;
        }
        else
        {
            return 1;
        }
    }
}
