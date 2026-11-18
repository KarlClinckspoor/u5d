#include "common/common.h"
#include "vars.h"

#include <stdio.h>
#include <string.h>

#include "common/file.h"

#if !defined(TARGET_DOS16)

#define READ_16(TARGET) /*debug("%s: %d", #TARGET, (int)ftell(stream)); */FILE_ReadU16LE(stream, &(TARGET))
#define READ_8(TARGET) /*debug("%s: %d", #TARGET, (int)ftell(stream)); */FILE_ReadU8(stream, &(TARGET))

#define WRITE_16(TARGET) /*debug("%s: %d", #TARGET, (int)ftell(stream)); */ FILE_WriteU16LE(stream, TARGET)
#define WRITE_8(TARGET) /*debug("%s: %d", #TARGET, (int)ftell(stream)); */ FILE_WriteU8(stream, TARGET)

// based on FMT
int FILE_ReadSavegameFile(char* fileName)
{
    FILE* stream;

    char buf[256];
    if (!strcmp(fileName, "BRIT.OOL") || !strcmp(fileName, "UNDER.OOL") || !strcmp(fileName, "SAVED.OOL") ||
        !strcmp(fileName, "SAVED.GAM"))
    {
        sprintf(buf, "SAVEGAME\\%s", fileName);
        fileName = buf;
    }

    stream = fopen(fileName, "rb");
    if (!stream)
        return -1;

    ASSERT(sizeof(S_55a8) == 0x20);

    READ_16(D_55a6_saveStart);
    fread(D_55a8_party, 0x20, 0x10, stream);
    READ_16(D_57a8_food); // 3dfb4
    READ_16(D_57aa_gold); // 3dfb6
    READ_8(D_57ac_keys); // 3dfb8
    READ_8(D_57ad_gems); // 3dfb9_gem_count
    READ_8(D_57ae_torches); // 3dfba
    READ_8(D_57af_grapple); // 3dfbb
    READ_8(D_57b0_carpets); // 3dfbc
    READ_8(D_57b1_skullKeys); // 3dfbd
    READ_8(D_57b2_skullKeysDay); // 3dfbe
    READ_8(D_57b3_amulet); // 3dfbf
    READ_8(D_57b4_crown); // 3dfc0
    READ_8(D_57b5_sceptre); // 3dfc1
    fread(D_57b6_shards, 4, 1, stream); // 3dfc4
    READ_8(D_57ba_spyglasses); // 3dfc8
    READ_8(D_57bb_hmsCapePlans); // 3dfc9
    READ_8(D_57bc_sextants); // 3dfca
    READ_8(D_57bd_pocketWatch); // 3dfcb
    READ_8(D_57be_blackBadge); // 3dfcc
    READ_8(D_57bf_sandalwoodBox); // 3dfcd
    // FMT pad 2bytes
    fread(D_57c0_equipment, 0x30, 1, stream); // 3dfd0
    fread(D_57f0_mixedSpells, 0x30, 1, stream); // 3e000
    fread(D_5820_scrolls, 8, 1, stream); // 3e030
    fread(D_5828_potions, 8, 1, stream); // 3e038
    fread(D_5830_moonstoneX, 8, 1, stream); // 3e040
    fread(D_5838_moonstoneY, 8, 1, stream); // 3e048
    fread(D_5840_moonstoneMap, 8, 1, stream); // 3e050
    fread(D_5848_moonstoneZ, 8, 1, stream); // 3e058
    fread(D_5850_reagents, 8, 1, stream); // 3e060
    fread(D_5858_harvestDay, 3, 1, stream); // 3e068
    READ_8(D_585b_partyCount); // 3e06b
    fread(D_585c_takenItemFlags, 0x18, 1, stream); // 3e06c
    READ_16(D_5874_year); // 3e084
    READ_16(D_5876_tempX); // 3e086
    READ_16(D_5878_tempY); // 3e088
    READ_8(D_587a_statusIcon); // 3e08a
    READ_8(D_587b_activePlayer); // 3e08b
    READ_8(D_587c_partyTile); // 3e08c
    READ_8(D_587d_month); // 3e08d
    READ_8(D_587e_day); // 3e08e
    READ_8(D_587f_hour); // 3e08f
    READ_8(D_5880_lastHour); // 3e090
    READ_8(D_5881_minute); // 3e091
    READ_8(D_5882_combatTick); // 3e092
    READ_8(D_5883_sailCounter); // 3e093
    READ_8(D_5884_clockChimes); // 3e094
    READ_8(D_5885_trammelPhase); // 3e095
    READ_8(D_5886_feluccaPhase); // 3e096
    READ_8(D_5887_moongateHeight); // 3e097
    READ_8(D_5888_karma); // 3e098
    READ_8(D_5889_savedUnused); // 3e099
    READ_8(D_588a_savedUnused); // 3e09a
    READ_8(D_588b_turnCount); // 3e09b
    READ_8(D_588c_campHealWait); // 3e09c
    READ_8(D_588d_apparitionMonth); // 3e09d
    READ_8(D_588e_protectTurns); // 3e09e
    READ_8(D_588f_castCircle); // 3e09f
    READ_8(D_5890_spellAttack); // 3e0a0
    READ_8(D_5891_animate); // 3e0a1
    READ_8(D_5892_windDir); // 0003e0a2
    READ_8(D_5893_mapId); // 3e0a3_5893_map_id
    READ_8(D_5894_savedMapId); // 3e0a4
    READ_8(D_5895_mapLevel); // 3e0a5_5895_map_level
    READ_8(D_5896_mapX); // 3e0a6_5896_map_x
    READ_8(D_5897_mapY); // 3e0a7_5897_map_y
    READ_8(D_5898_crosshairOn); // 3e0a8
    READ_8(D_5899_crosshairX); // 3e0a9
    READ_8(D_589a_crosshairY); // 3e0aa
    READ_8(D_589b_mapOriginX); // 3e0ab
    READ_8(D_589c_mapOriginY); // 3e0ac
    READ_8(D_589d_attackWeapon); // 3e0ad
    READ_8(D_589e_curCombatant); // 3e0ae
    READ_8(D_589f_focusBlink); // 3e0af
    READ_8(D_58a0_combatExitDir); // 3e0b0
    READ_8(D_58a1_combatFlags); // 3e0b1
    READ_8(D_58a2_combatEvents); // 3e0b2
    READ_8(D_58a3_combatOver); // 3e0b3_58a3_battle_finish_flg
    READ_8(D_58a4_viewOn); // 3e0b4
    READ_8(D_58a5_lightLevel); // 3e0b5
    READ_8(D_58a6_lightSpellTurns); // 3e0b6
    READ_8(D_58a7_torchTurns); // 3e0b7
    fread(&D_58a8_lastAttacker, 0x20, 1, stream); // 3e0b8
    fread(&D_58c8_shadowlordTowns, 3, 1, stream); // 3e0d8
    READ_8(D_58cb_summonedLord); // 3e0db
    READ_16(D_58cc_questsOrdained); // 3e0dc
    READ_16(D_58ce_questsDone); // 3e0de
    fread(D_58d0_dungeonOpen, 8, 1, stream); // 3e0e0
    fread(D_58d8_shrineDestroyed, 8, 1, stream); // 3e0e8
    fread(D_58e0_roomsCleared, 0xe, 1, stream); // 3e0f0
    // FMT pad 1byte
    fread(D_58ee_annotX, 0x20, 1, stream); // 3e100
    fread(D_590e_annotY, 0x20, 1, stream); // 3e120
    fread(D_592e_annotTile, 0x20, 1, stream); // 3e140
    READ_8(D_594e_annotCount); // 3e160
    READ_8(D_594f_openDoorTile); // 3e161
    READ_8(D_5950_openDoorX); // 3e162
    READ_8(D_5951_openDoorY); // 3e163
    READ_8(D_5952_openDoorTurns); // 3e164
    READ_8(D_5953_boughtShipX); // 3e165
    READ_8(D_5954_boughtShipY); // 3e166
    READ_8(D_5955_sailDir); // 3e167
    READ_8(D_5956_newPrompt); // 3e168
    READ_8(D_5957_drunkTurns); // 3e169
    READ_8(D_5958_townShadowlord); // 3e16a
    READ_8(D_5959_fewerEnemies); // 3e16b
    fread(D_595a_dungeonMap, 0x200, 1, stream); // 3e16c; map data
    fread(D_5b5a_npcKilled, 0x80, 1, stream); // 3e36c
    fread(D_5bda_npcMet, 0x80, 1, stream); // 3e3ec
    fread(D_5c5a_actors, 0x100, 1, stream); // 3e46c
    READ_16(D_5d5a_unused); // 3e56c
    READ_16(D_5d5c_unused); // 3e56e
    fread(D_5d5e_npcSchedules, 0x200, 1, stream); // 3e570
    fread(D_5f5e_npcs, 0x200, 1, stream); // 3e770
    fread(D_615e_moveLists, 0x400, 1, stream); // 3e970
    fread(D_655e_moveListPos, 0x40, 1, stream); // 3ed70
    fread(D_659e_npcTypes, 0x20, 1, stream); // 3edb0
    // fread(D_659e_npcTypes, 0x20, 1, stream); // 3edb0 ; FMT sic
    READ_8(D_65be_npcApproach); // 3edd0
    READ_8(D_65bf_npcApproaching); // 3edd1
    READ_16(D_65c0_unused); // 3edd2
    fread(D_65c2_npcStuckCount, 0x40, 1, stream); // 3edd4
    READ_8(D_6602_dungeonMoveDir);// 3ee14
    READ_8(D_6603_dungeonFacing);// 3ee15
    READ_8(D_6604_dungeonStyle);// 3ee16
    READ_8(D_6605_boughtShip);// 3ee17
    //READ_16(D_6606_saveEnd);// 3ee18

    int pos = (int)ftell(stream);
    debug("pos: %d", pos);

    fclose(stream);

    //ASSERT(pos == 0x1060);

    return 0;
}

// based on FMT
int FILE_WriteSavegameFile(char* fileName)
{
    FILE* stream;

    char buf[256];
    if (!strcmp(fileName, "BRIT.OOL") || !strcmp(fileName, "UNDER.OOL") || !strcmp(fileName, "SAVED.OOL") ||
        !strcmp(fileName, "SAVED.GAM"))
    {
        sprintf(buf, "SAVEGAME\\%s", fileName);
        fileName = buf;
    }

    stream = fopen(fileName, "wb");
    if (!stream)
        return -1;

    ASSERT(sizeof(S_55a8) == 0x20);

    WRITE_16(D_55a6_saveStart);
    fwrite(D_55a8_party, 0x20, 0x10, stream);
    WRITE_16(D_57a8_food);             // 3dfb4
    WRITE_16(D_57aa_gold);             // 3dfb6
    WRITE_8(D_57ac_keys);              // 3dfb8
    WRITE_8(D_57ad_gems);              // 3dfb9_gem_count
    WRITE_8(D_57ae_torches);              // 3dfba
    WRITE_8(D_57af_grapple);              // 3dfbb
    WRITE_8(D_57b0_carpets);              // 3dfbc
    WRITE_8(D_57b1_skullKeys);              // 3dfbd
    WRITE_8(D_57b2_skullKeysDay);              // 3dfbe
    WRITE_8(D_57b3_amulet);              // 3dfbf
    WRITE_8(D_57b4_crown);              // 3dfc0
    WRITE_8(D_57b5_sceptre);              // 3dfc1
    fwrite(D_57b6_shards, 4, 1, stream); // 3dfc4
    WRITE_8(D_57ba_spyglasses);              // 3dfc8
    WRITE_8(D_57bb_hmsCapePlans);              // 3dfc9
    WRITE_8(D_57bc_sextants);              // 3dfca
    WRITE_8(D_57bd_pocketWatch);              // 3dfcb
    WRITE_8(D_57be_blackBadge);              // 3dfcc
    WRITE_8(D_57bf_sandalwoodBox);              // 3dfcd
    // FMT pad 2bytes
    fwrite(D_57c0_equipment, 0x30, 1, stream);  // 3dfd0
    fwrite(D_57f0_mixedSpells, 0x30, 1, stream);  // 3e000
    fwrite(D_5820_scrolls, 8, 1, stream);     // 3e030
    fwrite(D_5828_potions, 8, 1, stream);     // 3e038
    fwrite(D_5830_moonstoneX, 8, 1, stream);     // 3e040
    fwrite(D_5838_moonstoneY, 8, 1, stream);     // 3e048
    fwrite(D_5840_moonstoneMap, 8, 1, stream);     // 3e050
    fwrite(D_5848_moonstoneZ, 8, 1, stream);     // 3e058
    fwrite(D_5850_reagents, 8, 1, stream);     // 3e060
    fwrite(D_5858_harvestDay, 3, 1, stream);     // 3e068
    WRITE_8(D_585b_partyCount);                  // 3e06b
    fwrite(D_585c_takenItemFlags, 0x18, 1, stream);  // 3e06c
    WRITE_16(D_5874_year);                 // 3e084
    WRITE_16(D_5876_tempX);                 // 3e086
    WRITE_16(D_5878_tempY);                 // 3e088
    WRITE_8(D_587a_statusIcon);                  // 3e08a
    WRITE_8(D_587b_activePlayer);                  // 3e08b
    WRITE_8(D_587c_partyTile);                  // 3e08c
    WRITE_8(D_587d_month);                  // 3e08d
    WRITE_8(D_587e_day);                  // 3e08e
    WRITE_8(D_587f_hour);                  // 3e08f
    WRITE_8(D_5880_lastHour);                  // 3e090
    WRITE_8(D_5881_minute);                  // 3e091
    WRITE_8(D_5882_combatTick);                  // 3e092
    WRITE_8(D_5883_sailCounter);                  // 3e093
    WRITE_8(D_5884_clockChimes);                  // 3e094
    WRITE_8(D_5885_trammelPhase);                  // 3e095
    WRITE_8(D_5886_feluccaPhase);                  // 3e096
    WRITE_8(D_5887_moongateHeight);                  // 3e097
    WRITE_8(D_5888_karma);                  // 3e098
    WRITE_8(D_5889_savedUnused);                  // 3e099
    WRITE_8(D_588a_savedUnused);                  // 3e09a
    WRITE_8(D_588b_turnCount);                  // 3e09b
    WRITE_8(D_588c_campHealWait);                  // 3e09c
    WRITE_8(D_588d_apparitionMonth);                  // 3e09d
    WRITE_8(D_588e_protectTurns);                  // 3e09e
    WRITE_8(D_588f_castCircle);                  // 3e09f
    WRITE_8(D_5890_spellAttack);                  // 3e0a0
    WRITE_8(D_5891_animate);                  // 3e0a1
    WRITE_8(D_5892_windDir);         // 0003e0a2
    WRITE_8(D_5893_mapId);           // 3e0a3_5893_map_id
    WRITE_8(D_5894_savedMapId);                  // 3e0a4
    WRITE_8(D_5895_mapLevel);        // 3e0a5_5895_map_level
    WRITE_8(D_5896_mapX);            // 3e0a6_5896_map_x
    WRITE_8(D_5897_mapY);            // 3e0a7_5897_map_y
    WRITE_8(D_5898_crosshairOn);                  // 3e0a8
    WRITE_8(D_5899_crosshairX);                  // 3e0a9
    WRITE_8(D_589a_crosshairY);                  // 3e0aa
    WRITE_8(D_589b_mapOriginX);                  // 3e0ab
    WRITE_8(D_589c_mapOriginY);                  // 3e0ac
    WRITE_8(D_589d_attackWeapon);                  // 3e0ad
    WRITE_8(D_589e_curCombatant);                  // 3e0ae
    WRITE_8(D_589f_focusBlink);                  // 3e0af
    WRITE_8(D_58a0_combatExitDir);                  // 3e0b0
    WRITE_8(D_58a1_combatFlags);                  // 3e0b1
    WRITE_8(D_58a2_combatEvents);                  // 3e0b2
    WRITE_8(D_58a3_combatOver);                  // 3e0b3_58a3_battle_finish_flg
    WRITE_8(D_58a4_viewOn);                  // 3e0b4
    WRITE_8(D_58a5_lightLevel);                  // 3e0b5
    WRITE_8(D_58a6_lightSpellTurns);                  // 3e0b6
    WRITE_8(D_58a7_torchTurns);                  // 3e0b7
    fwrite(&D_58a8_lastAttacker, 0x20, 1, stream); // 3e0b8
    fwrite(&D_58c8_shadowlordTowns, 3, 1, stream);    // 3e0d8
    WRITE_8(D_58cb_summonedLord);                  // 3e0db
    WRITE_16(D_58cc_questsOrdained);                 // 3e0dc
    WRITE_16(D_58ce_questsDone);                 // 3e0de
    fwrite(D_58d0_dungeonOpen, 8, 1, stream);    // 3e0e0
    fwrite(D_58d8_shrineDestroyed, 8, 1, stream);    // 3e0e8
    fwrite(D_58e0_roomsCleared, 0xe, 1, stream);  // 3e0f0
    // FMT pad 1byte
    fwrite(D_58ee_annotX, 0x20, 1, stream); // 3e100
    fwrite(D_590e_annotY, 0x20, 1, stream); // 3e120
    fwrite(D_592e_annotTile, 0x20, 1, stream); // 3e140
    WRITE_8(D_594e_annotCount);                  // 3e160
    WRITE_8(D_594f_openDoorTile);                  // 3e161
    WRITE_8(D_5950_openDoorX);                  // 3e162
    WRITE_8(D_5951_openDoorY);                  // 3e163
    WRITE_8(D_5952_openDoorTurns);                  // 3e164
    WRITE_8(D_5953_boughtShipX);                  // 3e165
    WRITE_8(D_5954_boughtShipY);                  // 3e166
    WRITE_8(D_5955_sailDir);                  // 3e167
    WRITE_8(D_5956_newPrompt);                  // 3e168
    WRITE_8(D_5957_drunkTurns);                  // 3e169
    WRITE_8(D_5958_townShadowlord);                  // 3e16a
    WRITE_8(D_5959_fewerEnemies);                  // 3e16b
    fwrite(D_595a_dungeonMap, 0x200, 1, stream); // 3e16c; map data
    fwrite(D_5b5a_npcKilled, 0x80, 1, stream);  // 3e36c
    fwrite(D_5bda_npcMet, 0x80, 1, stream);  // 3e3ec
    fwrite(D_5c5a_actors, 0x100, 1, stream); // 3e46c
    WRITE_16(D_5d5a_unused);                 // 3e56c
    WRITE_16(D_5d5c_unused);                 // 3e56e
    fwrite(D_5d5e_npcSchedules, 0x200, 1, stream); // 3e570
    fwrite(D_5f5e_npcs, 0x200, 1, stream); // 3e770
    fwrite(D_615e_moveLists, 0x400, 1, stream); // 3e970
    fwrite(D_655e_moveListPos, 0x40, 1, stream);  // 3ed70
    fwrite(D_659e_npcTypes, 0x20, 1, stream);  // 3edb0
    // fwrite(D_659e_npcTypes, 0x20, 1, stream); // 3edb0 ; FMT sic
    WRITE_8(D_65be_npcApproach);                 // 3edd0
    WRITE_8(D_65bf_npcApproaching);                 // 3edd1
    WRITE_16(D_65c0_unused);                // 3edd2
    fwrite(D_65c2_npcStuckCount, 0x40, 1, stream); // 3edd4
    WRITE_8(D_6602_dungeonMoveDir);                 // 3ee14
    WRITE_8(D_6603_dungeonFacing);                 // 3ee15
    WRITE_8(D_6604_dungeonStyle);                 // 3ee16
    WRITE_8(D_6605_boughtShip);                 // 3ee17
    // WRITE_16(D_6606_saveEnd);// 3ee18

    int pos = (int)ftell(stream);
    debug("pos: %d", pos);

    fclose(stream);

    ASSERT(pos == 0x1060);

    return 0;
}

#endif
