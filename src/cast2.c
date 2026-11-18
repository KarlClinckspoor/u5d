#include "common/common.h"

#include "funcs.h"
#include "vars.h"
#include "macros.h"
#include "tiles.h"

#include "cast2.h"
#include "combat.h"

// The show of a spell going off: a burst of noise, a flash of the map area (inverted twice), two rising and falling
// tones. The level (the circle of the spell, 1-8) sets the pitch; 0 is the light scroll's, 9 and over a single tone.
void CAST2_0000_CastEffect(int level)
{
    if (level < 9)
    {
        ULTIMA_223c_AudioWhiteNoise(800, level * 0x640 + 8000, 700);
        ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
        ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
        ULTIMA_2192_AudioPulse(D_4af6_castFreq[level], 1, level * 4000 + 10000, D_4b08_castParam4Up[level], D_4b2c_castStep[level]);
        ULTIMA_2192_AudioPulse(D_4af6_castFreq[level], 1, level * 4000 + 10000, D_4b1a_castParam4Down[level], -D_4b2c_castStep[level]);
        ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
    }
    else
    {
        ULTIMA_2192_AudioPulse(0x1180, 1, 65000, 300, 1);
    }
}

// Asks "On who:" and prints the party member chosen; returns the member, or -1 for none.
int CAST2_009e_OnWho(void)
{
    int member;

    ULTIMA_1850_PrintString(/*0x94f4*/ "On who: ");

    member = ULTIMA_2e8e_SelectPlayer();
    if (member < 0)
    {
        ULTIMA_1850_PrintString(/*0x94fe*/ "None!");
    }
    else
    {
        ULTIMA_1850_PrintString(D_55a8_party[member].name);
    }

    if (ULTIMA_1f12_GetCurrentTextX() != 0)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    return member;
}

// CHECKED
// Reads a spell name as syllable keys (a letter prints its syllable, J and O are ignored, Backspace takes one back, Esc cancels).
// The letters are sorted and compared with the 48 spell codes (D_1c30_spellCodes). Returns the spell, -1 for no entry, -2 for no such spell.
int CAST2_00de_SelectSpell(void)
{
    int nameLength;
    int matches_n;
    int codePos_g;
    byte letter_d;
    int letterPos;
    int index_l;
    byte letters[4];
    int done_l;
    int count_i;
    byte columns[4];
    int spell;

    count_i = 0; // di
    spell = -1;
    done_l = 0;

    while (done_l == 0)
    {
        letter_d = ULTIMA_2032_ToUpper(ULTIMA_266c_GetChar());
        if (letter_d == 0x4a || letter_d == 0x4f)
        {}
        else if (letter_d >= 0x41 && letter_d <= 0x5a && count_i < 4)
        {
            letters[count_i] = letter_d;

            if (ULTIMA_1f12_GetCurrentTextX() + ULTIMA_216c_strlen(D_1bfc_mantraWords[letter_d - 'A']) > 0xd)
            {
                ULTIMA_16ba_PrintChar('\n');
            }

            columns[count_i] = ULTIMA_1f12_GetCurrentTextX();
            ULTIMA_1850_PrintString(D_1bfc_mantraWords[letter_d - 'A']);
            ULTIMA_16ba_PrintChar(' ');
            count_i++;
        }
        else
        {
            switch (letter_d)
            {
            case U5_KEY_BACKSPACE:
                // 0174
                if (count_i != 0) // di
                {
                    count_i--;
                    if (ULTIMA_1f12_GetCurrentTextX() == 0)
                    {
                        // NOTE: FMT
                        //ULTIMA_16ba_PrintChar(' ');
                        ULTIMA_1bf2_SetTextPosition(columns[count_i], ULTIMA_1cee_GetCurrentTextY() - 1);
                    }
                    else
                    {
                        ULTIMA_1bf2_SetTextPosition(columns[count_i], ULTIMA_1cee_GetCurrentTextY());
                    }

                    nameLength = ULTIMA_216c_strlen(D_1bfc_mantraWords[letters[count_i] - 'A']);

                    for (index_l = 0; index_l < nameLength; index_l++)
                    {
                        ULTIMA_16ba_PrintChar(' ');
                    }

                    ULTIMA_1bf2_SetTextPosition(columns[count_i], ULTIMA_1cee_GetCurrentTextY());
                }
                break;

            case U5_KEY_ESC:
                // 01d8/e3b8
                count_i = 0;
                done_l = 1;
                break;

            case U5_KEY_ENTER:
            case U5_KEY_SPACE:
                // 01da/e3ba
                done_l = 1;
                break;
            }
        }
    }

    // e3c2
    // di--;

    // e3c6
    // bubble sort
    do
    {
        done_l = 1;
        for (index_l = 0; index_l < count_i - 1; index_l++)
        {
            if (letters[index_l + 1] < letters[index_l])
            {
                letter_d = letters[index_l];
                letters[index_l] = letters[index_l + 1];
                letters[index_l + 1] = letter_d;
                done_l = 0;
            }
        }
    } while (done_l == 0);

    // e412
    for (index_l = 0; index_l < 0x30; index_l++)
    {
        // e42e
        matches_n = 1;

        for (letterPos = codePos_g = 0; letterPos < count_i; letterPos++, codePos_g++)
        {
            if (D_1c30_spellCodes[index_l][codePos_g] != letters[letterPos])
            {
                matches_n = 0;
                break;
            }

            while (letterPos < count_i - 1 && letters[letterPos] == letters[letterPos + 1])
            {
                letterPos++;
            }
        }

        // e460
        if (matches_n != 0 && D_1c30_spellCodes[index_l][codePos_g] != '\0')
        {
            matches_n = 0;
        }

        if (matches_n != 0)
        {
            spell = index_l;
            break;
        }
    }

    // e484
    if (count_i == 0)
    {
        spell = -1;
        // -> e4dc
    }
    else
    {
        ULTIMA_16ba_PrintChar('\n');
        if (spell < 0)
        {
            spell = -2;
        }
    }

    // e4dc
    return spell;
}

// "Direction-": reads an arrow key (Space passes). Leaves the cell beside the party (in combat, beside the active combatant)
// in D_5876_tempX and D_5878_tempY. Returns the key, 0 for Pass.
int CAST2_0306_AskDirection(void)
{
    int key;
    bool chosen;

    if (D_5893_mapId > 0x7f)
    {
        D_5876_tempX = D_ba14_combatants[D_589e_curCombatant].x;
        D_5878_tempY = D_ba14_combatants[D_589e_curCombatant].y;
    }
    else
    {
        D_5876_tempX = D_5896_mapX;
        D_5878_tempY = D_5897_mapY;
    }

    // 033e
    ULTIMA_1850_PrintString(/*0x9504*/ "Direction-");
    chosen = 0;

    do
    {
        key = ULTIMA_266c_GetChar();
        chosen = 1;
        switch (key)
        {
        case U5_KEY_UP:
            ULTIMA_1850_PrintString(/*0x9510*/ "North\n");
            D_5878_tempY--;
            break;
        case U5_KEY_RIGHT:
            ULTIMA_1850_PrintString(/*0x9518*/ "East\n");
            D_5876_tempX++;
            break;
        case U5_KEY_DOWN:
            ULTIMA_1850_PrintString(/*0x951e*/ "South\n");
            D_5878_tempY++;
            break;
        case U5_KEY_LEFT:
            ULTIMA_1850_PrintString(/*0x9526*/ "West\n");
            D_5876_tempX--;
            break;
        case U5_KEY_SPACE:
            ULTIMA_1850_PrintString(/*0x952c*/ "Pass\n");
            key = 0;
            break;
        default:
            chosen = 0;
            break;
        }
    } while (!chosen);

    return key;
}

// Heals a party member by 1 to 30 hit points, up to the maximum. 0 when the member is dead.
int CAST2_03c2_HealMember(int member)
{
    int healed;

    if (D_55a8_party[member].status == STATUS_DEAD)
    {
        healed = 0;
    }
    else
    {
        ULTIMA_3f14_IncreaseInt(&D_55a8_party[member].hp, ULTIMA_3abe_Random1To30(), D_55a8_party[member].maxHp);

        healed = 1;
        D_a9fa_vitalsDirty = 1;
    }

    return healed;
}

// Rel Hur and the wind scroll: sets the wind to a direction (0: none) after the effect.
void CAST2_040a_ChangeWindDirection(int direction, int fromScroll)
{
    if (direction != 0 || fromScroll != 0)
    {
        if (fromScroll != 0)
        {
            CAST2_0000_CastEffect(1);
        }
        else
        {
            CAST2_0000_CastEffect(2);
        }

        switch (direction)
        {
        case DIR_UP:
            // 0448
            ULTIMA_2e96_SetWindDirection(1);
            break;

        case DIR_DOWN:
            // 0452
            ULTIMA_2e96_SetWindDirection(2);
            break;

        case DIR_RIGHT:
            // 0458
            ULTIMA_2e96_SetWindDirection(3);
            break;

        case DIR_LEFT:
            // 045e
            ULTIMA_2e96_SetWindDirection(4);
            break;

        case 0:
            // 0464
            ULTIMA_2e96_SetWindDirection(0);
            break;
        }
    }
}

// Wis An Ylem ("AWY"): shows the map with its walls seen through for 20 frames, the actors still moving (unless time is stopped).
void CAST2_046c_WisAnYlem(void)
{
    int frame;

    ULTIMA_5d0a_BuildView(-1, D_5896_mapX - D_589b_mapOriginX, D_5897_mapY - D_589c_mapOriginY, 0x20);

    for (frame = 0; frame < 0x14; frame++)
    {
        if (D_587a_statusIcon != 'T')
        {
            ULTIMA_4552_AnimateActors();
        }

        ULTIMA_5394_PutActorsInView();
        ULTIMA_56ac_DrawMap();
        ULTIMA_20fa_WaitTicks(1);
    }

    ULTIMA_5910_UpdateFrame();
}

// CHECKED (code structure)
// Kal Xen Corp ("CKX") and the scroll: tries up to 8 random cells for one a daemon (0x26) can stand on and gates it in,
// charmed; from the spell the caster's intelligence may fail ("Oops...", -1: the daemon is hostile). Returns 1, 0 or -1.
int CAST2_04c2_KalXenCorp(int fromScroll)
{
    int result_i;
    int tries;
    ActorFmt* daemonActor_g;
    int daemon;

    result_i = 0;

    if (fromScroll != 0)
    {
        CAST2_0000_CastEffect(5);
    }
    else
    {
        CAST2_0000_CastEffect(8);
    }

    // 04e1
    tries = 0;
    while (tries < 8)
    {
        if (COMBAT_120e_RandomCell() == 0 || COMBAT_0000_IsCellFree(TILE_ACTOR_D8, D_5876_tempX, D_5878_tempY) == 0 || *ULTIMA_4402_GetTileAddr(D_5876_tempX, D_5878_tempY) == 0xff)
        {
            tries++;
            continue;
        }

        // 0524
        daemon = ULTIMA_6506_PlaceCombatant(0x26, 0, D_5876_tempX, D_5878_tempY, D_5895_mapLevel);
        if (daemon < 0)
        {
            break;
        }

        // 054c
        ULTIMA_2192_AudioPulse(0xac8, 1, 12000, 500, 5);
        daemonActor_g = &D_5c5a_actors[D_ba14_combatants[daemon].actorIdx];
        daemonActor_g->tile = daemonActor_g->animTile = TILE_ACTOR_CIRCLE;
        ULTIMA_1068_RevealTile(0x1d8, D_5876_tempX, D_5878_tempY);
        daemonActor_g->tile = daemonActor_g->animTile = TILE_ACTOR_D8;

        if (fromScroll == 0 && ULTIMA_3abe_Random1To30() >= COMBAT_13e2_QueryStat(D_589e_curCombatant, QUERY_STAT_INTEL))
        {
            // 05b2
            ULTIMA_1850_PrintString(/*0x9532*/ "Oops...\n");
            result_i = -1;
        }
        else
        {
            // 05c0
            D_ba14_combatants[daemon].flags |= COMBAT_FLAGS_CHARMED;
            result_i = 1;
        }

        // 0518
        break;
    }

    return result_i;
}

// CHECKED
// Resurrects a dead party member (In Mani Corp, the scroll, the healer): good with 1 hit point, magic restored by class,
// experience cut by the karma below 98, level and maximum hit points recomputed. Returns 1, 0 when not dead (told unless
// mode is 0xff), -1 for no member. mode: 0 spell, 1 scroll, 0xff the healer (no show).
int CAST2_05e0_Resurrect(int memberIdx, int mode)
{
    int expLeft;
    short newLevel_g;

    int result_k;
    S_55a8* member;

    if (memberIdx < 0)
    {
        result_k = -1;
    }
    else if (D_55a8_party[memberIdx].status != STATUS_DEAD)
    {
        if (mode != 0)
        {
            ULTIMA_1850_PrintString(/*0x953c*/ "Not dead!\n");
        }

        result_k = 0;
    }
    else
    {
        // 061a
        member = &D_55a8_party[memberIdx];

        member->status = STATUS_GOOD;
        member->hp = 1;

        switch (member->cls)
        {
        case CLASS_AVATAR:
        case CLASS_MAGE:
            member->mag = member->intel;
            break;

        case CLASS_BARD:
            member->mag = member->intel >> 1;
            break;
        }

        if (D_5888_karma < 0x62)
        {
            // NOT MATCHING (pointer)
            member->exp = member->exp * (s32)D_5888_karma / 100;
        }

        newLevel_g = 1;
        for (expLeft = member->exp / 100; 0 < expLeft; expLeft >>= 1)
        {
            newLevel_g++;
        }

        member->level = newLevel_g;
        member->maxHp = newLevel_g * 0x1e;

        if (mode == 1)
        {
            CAST2_0000_CastEffect(6);
        }
        else if (mode == 0)
        {
            CAST2_0000_CastEffect(8);
        }

        result_k = 1;
        D_a9fa_vitalsDirty = 1;
    }

    return result_k;
}

// In Wis ("IW"): prints the position as letters in the runic charset, y then x, each a pair 'A' plus a hex digit.
void CAST2_06ec_PrintPosition(void)
{
    ULTIMA_1c9e_SelectCharset(1);
    ULTIMA_16ba_PrintChar('\n');
    ULTIMA_16ba_PrintChar(((D_5897_mapY & 0xf0) >> 4) + 0x41);
    ULTIMA_16ba_PrintChar('\'');
    ULTIMA_16ba_PrintChar((D_5897_mapY & 0xf) + 0x41);
    ULTIMA_1850_PrintString(/*0x9548*/ "\", ");
    ULTIMA_16ba_PrintChar(((D_5896_mapX & 0xf0) >> 4) + 0x41);
    ULTIMA_16ba_PrintChar('\'');
    ULTIMA_16ba_PrintChar((D_5896_mapX & 0xf) + 0x41);
    ULTIMA_16ba_PrintChar('\"');
    ULTIMA_1c9e_SelectCharset(0);
    ULTIMA_16ba_PrintChar('\n');
}

// CHECKED (07b2 block)
// In Ex Por ("EIP") and the Skull Key: in the chosen direction a door locked by magic (tile 97 or 98, An Ex Por) opens; -1 when
// no direction was given, else 1 or 0.
int CAST2_0768_InExPor(void)
{
    int result;
    byte* tilePtr_k;

    if (CAST2_0306_AskDirection() == 0)
    {
        return -1;
    }

    tilePtr_k = ULTIMA_4402_GetTileAddr(D_5876_tempX, D_5878_tempY);
    result = 0;

    switch (*tilePtr_k)
    {
    case TILE_MAP_97:
        // 07a0
        *tilePtr_k = TILE_MAP_DOOR_B8;
        D_24e6_viewDirty |= 2;
        result = 1;
        break;

    case TILE_MAP_98:
        // 07b2
        *tilePtr_k = TILE_MAP_DOOR_BA;
        D_24e6_viewDirty |= 2;
        result = 1;
        break;
    }

    return result;
}

// CHECKED (085d nop)
// An Grav ("AG"): destroys a field. In a dungeon the field underfoot or the one ahead; elsewhere the field actor (E8-EB) in the
// chosen direction. Returns -1 or 1 when it did, else 0. withEffect plays the cast effect first.
int CAST2_07bc_AnGrav(int withEffect)
{
    int actorIdx_h;
    byte* cellPtr;
    int result_i;
    int dx;
    int dy;

    if (D_5893_mapId < 0x80)
    {
        if (withEffect != 0)
        {
            CAST2_0000_CastEffect(4);
        }

        dx = D_24d6_facingDx[D_6603_dungeonFacing];
        dy = D_24de_facingDy[D_6603_dungeonFacing];

        cellPtr = &GetDungeonMap(D_5896_mapX, D_5897_mapY, D_5895_mapLevel);
        if ((*cellPtr & 0xf0) != 0x80)
        {
            cellPtr = &GetDungeonMap((D_5896_mapX + dx) & 7, (D_5897_mapY + dy) & 7, D_5895_mapLevel);
        }

        if ((*cellPtr & 0xf0) == 0x80)
        {
            *cellPtr &= 8;
            ULTIMA_1850_PrintString(/*0x954c*/ "Field destroyed!\n");
            result_i = -1;
            // NOT MATCHING: 085d: nop (not aligned?)
        }
        else
        {
            result_i = 0;
        }
    }
    else
    {
        // 0866
        if (CAST2_0306_AskDirection() == 0)
        {
            return -1;
        }
        
        if (withEffect != 0)
        {
            CAST2_0000_CastEffect(4);
        }

        result_i = 0;
        for (actorIdx_h = 0; actorIdx_h < 0x20; actorIdx_h++)
        {
            if ((D_5c5a_actors[actorIdx_h].tile & 0xfc) == TILE_ACTOR_E8 && D_5876_tempX == D_5c5a_actors[actorIdx_h].x &&
                D_5878_tempY == D_5c5a_actors[actorIdx_h].y)
            {
                result_i = 1;
                ULTIMA_3a74_SetActor(0, 0, 0, 0, 0, 0, actorIdx_h);
                break;
            }
        }
    }
    
    return result_i;
}

// Sets the turns of light left (D_58a6).
void CAST2_08ea_SetLightTurns(int param_1) { D_58a6_lightSpellTurns = param_1; }

// Sets the party's spell status (icon letter and turns left: P protection, Q quickness, C confusion?, N negate magic,
// T negate time; 0x0e, 0x1c and 0x1d for the amulet, crown and badge), plays the effect and
// redraws the party display.
void CAST2_08f8_SetSpellStatus(int icon, int turns, int level)
{
    D_587a_statusIcon = icon;
    D_588e_protectTurns = turns;

    CAST2_0000_CastEffect(level);

    ULTIMA_2900_UpdateVitalsDisplay();
}

//
// Region: Shrine
//

// Puts count markers (potion tiles) on the Codex map for the party members who are gone (probably their urns), at the
// positions in the tables D_4b9e and D_4bc2 from the index D_4b96 gives.
static void CAST2_0914_PlaceUrns(int count)
{
    int index;
    int position;

    position = D_4b96_urnFirst[count - 1];
    if (count > 0)
    {
        for (index = 0; index != count; index++)
        {
            ULTIMA_3a74_SetActor(TILE_ACTOR_POTION, TILE_ACTOR_POTION, D_4b9e_urnX[position], D_4bc2_urnY[position], D_5895_mapLevel, 0, index + 1);
            position++;
        }
    }
}

u16 D_4af6_castFreq[9] = {0x226a, 0x1e96, 0x1b94, 0x1996, 0x173e, 0x15c2, 0x143c, 0x12d4, 0x1180};

u16 D_4b08_castParam4Up[9] = {0x0a8c, 0x0bb8, 0x03e8, 0x0064, 0x1388, 0x0fa0, 0x09c4, 0x03e8, 0x0001};

u16 D_4b1a_castParam4Down[9] = {0x7fbc, 0x7918, 0x9088, 0xafc8, 0x7918, 0x84d0, 0x8e94, 0x9858, 0xa410};

u16 D_4b2c_castStep[9] = {0x0003, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001};

char* D_4b3e_virtueKeywords[8] =
{
    /*0x955e*/ "hone",
    /*0x9564*/ "comp",
    /*0x956a*/ "valo",
    /*0x9570*/ "just",
    /*0x9576*/ "sacr",
    /*0x957c*/ "hono",
    /*0x9582*/ "spir",
    /*0x9588*/ "humi"
};

// DS:4b4e, referenced by no code
u16 D_4b4e_unused[8] = {0x0003, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0002, 0x0003};

u16 D_4b5e_questTextPos[8] = {0x0000, 0x001b, 0x0039, 0x005f, 0x007b, 0x009f, 0x00b9, 0x00d6};

u16 D_4b6e_codexTextPos[8] = {0x0100, 0x014e, 0x0189, 0x01d4, 0x022b, 0x027f, 0x02c0, 0x0310};

u8 D_4b7e_strengthGain[8] = {0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00};

u8 D_4b86_dexterityGain[8] = {0x00, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00};

u8 D_4b8e_intelligenceGain[8] = {0x01, 0x00, 0x00, 0x01, 0x00, 0x01, 0x01, 0x00};

u8 D_4b96_urnFirst[8] = {0x00, 0x01, 0x03, 0x06, 0x0a, 0x0f, 0x15, 0x1c};

u8 D_4b9e_urnX[0x24] =
{
    0x05, 0x04, 0x06, 0x04, 0x05, 0x06, 0x03, 0x04,
    0x06, 0x07, 0x03, 0x04, 0x05, 0x06, 0x07, 0x01,
    0x01, 0x01, 0x09, 0x09, 0x09, 0x01, 0x01, 0x01,
    0x09, 0x09, 0x09, 0x05, 0x01, 0x01, 0x01, 0x01,
    0x09, 0x09, 0x09, 0x09
};

u8 D_4bc2_urnY[0x24] =
{
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
    0x03, 0x04, 0x02, 0x03, 0x04, 0x02, 0x03, 0x04,
    0x02, 0x03, 0x04, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x02, 0x03, 0x04, 0x05
};

u16 D_4be6_shrineFreq[7] = {0x0ce4, 0x0f55, 0x0f55, 0x0f55, 0x0f55, 0x0e74, 0x0f55};

u16 D_4bf4_shrineParam3[7] = {0x1b58, 0x1770, 0x0bb8, 0x0bb8, 0x0bb8, 0x0bb8, 0x1f40};

u16 D_4c02_shrineParam4[7] = {0x03e8, 0x03e8, 0x03e8, 0x03e8, 0x03e8, 0x03e8, 0x01f4};

u16 D_4c10_shrineParam5[7] = {0x0009, 0x000a, 0x0015, 0x0015, 0x0015, 0x0015, 0x0008};

// CHECKED (code structure)
// The shrine of a virtue: the party kneels, the altar asks for the virtue (D_4b3e_virtueKeywords) and three mantras (D_1f5e_mantras).
// Wrong answers: "Thine thoughts are unfocused". Right ones: with no quest yet a quest is ordained (to be answered at the
// Codex); with the quest done at the Codex, the reward (karma, a point of strength, dexterity or intelligence by the
// virtue); after that the altar takes an offer of gold in hundredweights for karma.
static void CAST2_0966_ShrineOfVirtue(void)
{
    bool paid_b;
    int step_e;
    int hundreds_b;
    int virtue_c;
    bool allRight;

    allRight = 1;
    paid_b = 0;
    for (virtue_c = 0; virtue_c < 8; virtue_c++) // 0a2e -> 0a37 -> 0982, 0994
    {
        // 0982
        if (D_5896_mapX == D_1f6e_shrineX[virtue_c] && D_5897_mapY == D_1f76_shrineY[virtue_c])
            break; // -> 0a2e ??
    }

    // 0994
    if (virtue_c == 8)
    {
        virtue_c = 6;
    }

    // 09a1
    D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_BEGGAR;
    ULTIMA_5910_UpdateFrame();
    // "...and thou dost kneel before the Altar.\n\n"
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x36d]);
    ULTIMA_3ae6_WaitFrames(10);
    // "Upon what virtue dost thou meditate?\n\n:"
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x398]);

    ULTIMA_3b1c_GetString(D_bd08_answer, 0xc);

    if (D_bd08_answer[0] == '\0')
    {
        // -> 0d1d
        return;
    }

    // 09d6
    if (ULTIMA_6f1e_CompareWord(D_4b3e_virtueKeywords[virtue_c], D_bd08_answer) == -1)
    {
        allRight = 0;
    }

    // 09ee
    ULTIMA_3ae6_WaitFrames(6);
    ULTIMA_16ba_PrintChar('\n');

    for (step_e = 0; step_e < 3; step_e++)
    {
        // 0a0c
        ULTIMA_1850_PrintString(/*0x958e*/ "\nMantra:");
        ULTIMA_3b1c_GetString(D_bd08_answer, 0xc);
        if (D_bd08_answer[0] == '\0')
        {
            return;
        }

        // 0a3a
        if (ULTIMA_6f1e_CompareWord(D_1f5e_mantras[virtue_c], D_bd08_answer) == -1)
        {
            allRight = 0;
        }

        // 0a4b
        ULTIMA_3ae6_WaitFrames(0xc);
    }

    // NOTE: FMT addition
    //ULTIMA_16ba_PrintChar('\n');

    // 0a5e
    if (!allRight)
    {
        // "\n\nThine thoughts are unfocused.\n"
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x3c0]);
    }
    else
    {
        // 0a6c
        if (((byte)D_58ce_questsDone & (1 << virtue_c)) == 0)
        {
            // TODO: 58cc, 58ce: u8?
            *((byte*)&D_58cc_questsOrdained) |= (1 << virtue_c);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x3e1]);

            D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_AVATAR;

            ULTIMA_266c_GetChar();
            // "\n\nThe Altar speaks and a Quest is ordained! "
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x40e]);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[D_4b5e_questTextPos[virtue_c]]);
            ULTIMA_1850_PrintString(/*0x9598*/ "\"\n");
            ULTIMA_266c_GetChar();
            // "\n\"Return again when thy Quest is done!\"\n"
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x44b]);

            // 0ac6: a do-while, so the loop ends jb top; jmp end
            step_e = 0;
            do
            {
                ULTIMA_2192_AudioPulse(D_4be6_shrineFreq[step_e], 1, D_4bf4_shrineParam3[step_e], D_4c02_shrineParam4[step_e], D_4c10_shrineParam5[step_e]);
                step_e++;
            } while (step_e < 7);
        }
        // 0b08
        else if (((byte)D_58cc_questsOrdained & (1 << virtue_c)) == 0)
        {
            // 0b1d -> 0b5f

            // the inner wait is an empty while (its exit jumps to 0b26, which MSC then
            // places before the test at 0b5f); paid_b lives in DI
            while (!paid_b) // 0b5f
            {
                // 0b63
                // "\n\nOffer how many hundredweights gold? "
                ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x474]);

                // 0b6a
                while ((unsigned)(hundreds_b = ULTIMA_266c_GetChar()) < 0x30 || (unsigned)hundreds_b > 0x39)
                    ;

                // 0b26 (OK)
                ULTIMA_16ba_PrintChar(hundreds_b);
                hundreds_b -= 0x30;
                if (hundreds_b == 0)
                {
                    // 0b2f
                    ULTIMA_1850_PrintString(/*0x959c*/ " gp\n");
                    return; // -> 0d1d
                }

                // 0b40 (OK)
                ULTIMA_1850_PrintString(/*0x95a2*/ "00 gp\n\n");
                if ((int)D_57aa_gold < hundreds_b * 100)
                {
                    // "Thou hast not that much gold!"
                    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x49b]);
                }
                else
                {
                    // 0b5c (OK)
                    paid_b = 1;
                }

                // 0b5f
            }

            // 0b82 (OK)
            D_57aa_gold -= hundreds_b * 100;
            ULTIMA_2900_UpdateVitalsDisplay();
            D_5888_karma += hundreds_b;
            if (D_5888_karma > 99)
            {
                D_5888_karma = 99;
            }

            // 0ba1 (OK)
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString(/*0x95aa*/ "ALAKAZAM");
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_1850_PrintString(/*0x95b4*/ "!\n");
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);

            // 0bd3 (OK)
            for (step_e = 2000; step_e < 25000; step_e += 0x32)
            {
                ULTIMA_2192_AudioPulse(0xa8c, 1, 200, step_e, 0);
            }

            // 0bf5 (OK)
            for (step_e = 25000; step_e > 2000; step_e -= 0x32)
            {
                ULTIMA_2192_AudioPulse(0xa8c, 1, 200, step_e, 0);
            }
        }
        else
        {
            // 0c18 (OK)
            *((byte*)&D_58cc_questsOrdained) &= (1 << virtue_c & (byte)D_58cc_questsOrdained) ^ 0xff;

            // "\n\nA thunderous voice booms:\n\n\"WELL DONE!\"\n\n"
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x4b9]);
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);

            // 0c47 (OK)
            for (step_e = 2000; step_e < 25000; step_e += 0x32)
            {
                ULTIMA_2192_AudioPulse(0xc1c, 1, 0x96, step_e, 0);
            }

            // 0c69 (OK)
            for (step_e = 25000; step_e > 2000; step_e -= 0x32)
            {
                ULTIMA_2192_AudioPulse(0xc1c, 1, 0x96, step_e, 0);
            }

            ULTIMA_3072_ShakeScreen();
            D_5888_karma += 3;
            if (D_5888_karma > 99)
            {
                D_5888_karma = 99;
            }

            // 0c9c (OK)
            if (D_4b7e_strengthGain[virtue_c] != 0)
            {
                D_55a8_party[0].str++;
                if (D_55a8_party[0].str > 0x1e)
                {
                    D_55a8_party[0].str = 0x1e;
                }

                ULTIMA_1850_PrintString(/*0x95b8*/ "Strength +1\n");
            }

            if (D_4b86_dexterityGain[virtue_c] != 0)
            {
                D_55a8_party[0].dex++;
                if (D_55a8_party[0].dex > 0x1e)
                {
                    D_55a8_party[0].dex = 0x1e;
                }

                ULTIMA_1850_PrintString(/*0x95c6*/ "Dexterity +1\n");
            }

            if (D_4b8e_intelligenceGain[virtue_c] != 0)
            {
                D_55a8_party[0].intel++;
                if (D_55a8_party[0].intel > 0x1e)
                {
                    D_55a8_party[0].intel = 0x1e;
                }

                ULTIMA_1850_PrintString(/*0x95d4*/ "Intelligence +1\n");
            }

            if (virtue_c == 7)
            {
                D_5888_karma += 3;
                if (D_5888_karma > 99)
                {
                    D_5888_karma = 99;
                }
            }
        }

        // 0d16
        ULTIMA_3ae6_WaitFrames(10);
    }
}

// The Codex: the voice asks for a virtue's answer; with a quest ordained the Codex gives it and marks it done. With all
// eight done the final scene is read.
static void CAST2_0d24_Codex(void)
{
    int virtue;

    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x4e5]);
    ULTIMA_266c_GetChar();
    ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x515]);
    ULTIMA_266c_GetChar();

    for (virtue = 0; virtue < 8; virtue++)
    {
        if (((byte)D_58cc_questsOrdained & (1 << virtue)) != 0)
            break;
    }

    if (virtue == 8)
    {
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x53e]);
    }
    else
    {
        *((byte*)&D_58ce_questsDone) |= 1 << virtue;

        ULTIMA_16ba_PrintChar('\"');
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[D_4b6e_codexTextPos[virtue]]);
        ULTIMA_1850_PrintString(/*0x95e6*/ "\"\n\n");
        ULTIMA_266c_GetChar();

        if ((byte)D_58ce_questsDone == 0xff)
        {
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_3072_ShakeScreen();
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_3072_ShakeScreen();
            ULTIMA_0a70_GRAP_2d_SetPenColor(D_13ae_redColor);
            ULTIMA_0b86_GRAP_XorFillRect(8, 8, 0xb7, 0xb7);
            ULTIMA_3072_ShakeScreen();
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x555]);
            ULTIMA_266c_GetChar();
            ULTIMA_1850_PrintString(/*0x95ea*/ "Thou dost read:\n\n");
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x57f]);
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_266c_GetChar();
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x5d1]);
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_266c_GetChar();
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x60c]);
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_266c_GetChar();
            ULTIMA_1c9e_SelectCharset(1);
            ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x680]);
            ULTIMA_1c9e_SelectCharset(0);
            ULTIMA_266c_GetChar();
        }
    }
}

// A step of the walk in the shrine: a pause, the footstep sound, a pause.
static void CAST2_0e64_Footstep(void)
{
    ULTIMA_3ae6_WaitFrames(1);
    ULTIMA_433e_AudioFootstep();
    ULTIMA_3ae6_WaitFrames(4);
}

// CHECKED (loop)
// Enters a shrine or the Codex chamber: the world's actors are saved aside, the room (MISCMAPS.DAT) and its texts
// (MISCMSG.DAT) are read, the party walks to the altar (the avatar alone), the scene plays, then everything is restored.
void CAST2_0e76_Shrine(void)
{
    int column;
    int row;
    int urnCount_f;
    int placeTile;
    int stepsIn_i;

    urnCount_f = 0;

    placeTile = *ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY);
    D_24e6_viewDirty = 1;
    D_bd15_savedMapId = D_5893_mapId;
    D_5893_mapId = 0xff;
    D_589e_curCombatant = 0xff;

    // 0eb3: NOT MATCHING
    for (row = 0; row < 0x20; row++)
    {
        D_a9fc_savedActors[row] = D_5c5a_actors[row];
        D_5c5a_actors[row].tile = 0;
    }

    if (placeTile != TILE_MAP_CODEX)
    {
        ULTIMA_256e_ReadFileFromDisk(/*0x95fc*/ "MISCMAPS.DAT", D_ac64_viewActors, 0xb0, 0xb0);
    }
    else
    {
        ULTIMA_256e_ReadFileFromDisk(/*0x960a*/ "MISCMAPS.DAT", D_ac64_viewActors, 0xb0, 0x160);
    }

    ULTIMA_256e_ReadFileFromDisk(/*0x9618*/ "MISCMSG.DAT", D_b21e_scratch, 2000, 0x3ab);

    // 0f0c
    for (row = 0; row < 0xb; row++)
    {
        for (column = 0; column < 0xb; column++)
        {
            GetCombatMap(column, row) = D_ac64_viewActors[row * 0x10 + column];
        }
    }

    ULTIMA_5910_UpdateFrame();

    // 0f44 (NOT MATCHING)
    for (row = 1; row < 0x10; row++)
    {
        if (D_55a8_party[row].innMapId == 0x7f)
        {
            urnCount_f++;
        }
    }

    if (urnCount_f > 0)
    {
        CAST2_0914_PlaceUrns(urnCount_f);
    }

    // 0f69
    if (placeTile != TILE_MAP_CODEX)
    {
        // "\nThou dost approach the tranquil Shrine...\n\n"
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x6ae]);
    }
    else
    {
        // "\nThe Codex of Ultimate Wisdom lies before thee..."
        ULTIMA_1850_PrintString((char*)&D_b21e_scratch[0x6db]);
    }

    // 0f7b (OK)
    for (row = 0; row < 4; row++)
    {
        CAST2_0e64_Footstep();
    }

    D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_AVATAR;
    D_5c5a_actors[0].x = 5;
    D_5c5a_actors[0].y = 10;
    D_5c5a_actors[0].z = D_5895_mapLevel;

    CAST2_0e64_Footstep();
    if (placeTile == TILE_MAP_CODEX && urnCount_f > 0)
    {
        ULTIMA_1850_PrintString(/*0x9624*/ "\n\nThou dost see\n");
        if (urnCount_f == 1)
        {
            ULTIMA_1850_PrintString(/*0x9636*/ "an urn marked:\n\n");
        }
        else
        {
            ULTIMA_1850_PrintString(/*0x9648*/ "urns marked:\n\n");
        }

        // 0fcf (OK)
        for (row = 1; row < 0x10; row++)
        {
            if (D_55a8_party[row].innMapId == 0x7f)
            {
                column = 0;
                while (D_55a8_party[row].name[column] != '\0')
                {
                    ULTIMA_16ba_PrintChar(ULTIMA_2032_ToUpper((byte)D_55a8_party[row].name[column++]));
                }

                // 101f
                ULTIMA_16ba_PrintChar('\n');
            }
        }
    }

    // 103c
    stepsIn_i = placeTile != TILE_MAP_CODEX ? 4 : 7;

    for (row = 0; row < stepsIn_i; row++)
    {
        // 105c
        D_5c5a_actors[0].y--;
        CAST2_0e64_Footstep();
    }

    // 1066
    if (placeTile != TILE_MAP_CODEX)
    {
        CAST2_0966_ShrineOfVirtue();
    }
    else
    {
        CAST2_0d24_Codex();
    }

    // 1075
    D_5c5a_actors[0].tile = D_5c5a_actors[0].animTile = TILE_ACTOR_AVATAR;
    ULTIMA_3ae6_WaitFrames(1);
    for (row = D_5c5a_actors[0].y; row < 10; row++)
    {
        D_5c5a_actors[0].y++;
        CAST2_0e64_Footstep();
    }

    D_5c5a_actors[0].tile = D_5c5a_actors[0].x = D_5c5a_actors[0].y = 0;

    for (row = 0; row < 4; row++)
    {
        CAST2_0e64_Footstep();
    }

    ULTIMA_3ae6_WaitFrames(10);

    // 10c6 (NOT MATCHING)
    for (row = 0; row < 0x20; row++)
    {
        D_5c5a_actors[row] = D_a9fc_savedActors[row];
    }

    // 10e7
    D_5893_mapId = D_bd15_savedMapId;
    ULTIMA_4f7c_AdvanceTime(0x10);
    ULTIMA_5e4a_BuildLightMap();
}

//
// Region: Savegame
//

// NOTE: IBM version (the original); not compatible with modern platforms
// The Save game question: with Y the world is written to SAVED.GAM and SAVED.OOL, and UNDER.OOL on the saved disk.
void CAST2_10fe_SaveGame(void)
{
    byte local_4;
    byte local_6_an;

    ULTIMA_1850_PrintString(/*0x9658*/ "\nSave game? ");

    while ((local_6_an = ULTIMA_266c_GetChar()) != 'Y')
    {
        if (local_6_an == 'N')
            break;
    }

    if (local_6_an == 'N')
    {
        ULTIMA_1850_PrintString(/*0x9666*/ "No\n");
    }
    else
    {
        ULTIMA_1850_PrintString(/*0x966a*/ "Yes\nSaving...\n");
        local_4 = D_a9bd_curDisk[0];
        if (D_a9bd_curDisk[0] != 5)
        {
            ULTIMA_251e_SwitchDisks(1);
        }

        ULTIMA_256e_ReadFileFromDisk(/*0x967a*/ "UNDER.OOL", D_b21e_scratch + 0x100, 0x100, 0);
        ULTIMA_251e_SwitchDisks(1);
        ULTIMA_256e_ReadFileFromDisk(/*0x9684*/ "BRIT.OOL", D_b21e_scratch, 0x100, 0);
        if (local_4 != 1)
        {
            ULTIMA_25d8_WriteFileToDisk(/*0x968e*/ "UNDER.OOL", D_b21e_scratch + 0x100, 0x100);
        }

        ULTIMA_251e_SwitchDisks(3);
        ULTIMA_25d8_WriteFileToDisk(/*0x9698*/ "SAVED.GAM", &D_55a6_saveStart, ((int)&D_6606_saveEnd - (int)&D_55a6_saveStart) /*0x1060*/);
        ULTIMA_25d8_WriteFileToDisk(/*0x96a2*/ "SAVED.OOL", D_b21e_scratch, 0x200);
        ULTIMA_251e_SwitchDisks(local_4);
        ULTIMA_1850_PrintString(/*0x96ac*/ "Done.\n");
    }
}

