#ifndef _VARS_H
#define _VARS_H

#ifndef STUB
#define STUB extern
#endif
// an uninitialised (communal) variable: see DS:a9bc below
#ifndef BSS
#define BSS STUB
#endif

// ref: u5tech
// Savegame ref: https://wiki.ultimacodex.com/wiki/Ultima_V_internal_formats#SAVED.GAM_and_RAM
// (the +0x... offsets in SAVED.GAM and what the bytes mean are the Codex's where the code agrees or does not say;
// "the Codex" marks a meaning taken from it alone)

#include "structs.h"

// 103c..
// tlk word table

// 1300

STUB u8 D_13a6[8]; // TODO: size?

STUB u16 D_13ae_redColor;
STUB u16 D_13b0_brightWhiteColor;
STUB u16 D_13b2_blueColor;
STUB u16 D_13b4_greenColor;
STUB u16 D_13b6_magentaColor;
STUB u16 D_13b8_brightYellowColor;
STUB u16 D_13ba_lightGrayColor;

STUB EnemyStats D_13bc_enemyStats[0x30]; // 0x30 ~153c
STUB u16 D_153c_enemyFlags[0x30]; // enemy flags
STUB u8 D_159c[0x30]; // enemy related
STUB u8 D_15cc[0x30]; // enemy related

STUB u8 D_15fc[0x38]; // ~1634) some equip table

STUB u8 D_1634_equipDefense[0x30]; // ~1664) equip defense table
STUB u8 D_1664[0x38]; // ~169c) some equip table
STUB u8 D_169c[0x38]; // ~16d4) some equip table

STUB u8 D_16d4_enemyCompanion[0x30]; // the other kind of enemy that may come with one (ULTIMA_6bc2)
STUB u8 D_1704_combatEnemyX[0x10]; // where the enemies start on the combat map
STUB u8 D_1714_combatEnemyY[0x10]; // ~1714)
STUB u8 D_1724_combatPartyX[8]; // where the party starts
// gap 2
STUB u8 D_172c_combatPartyY[8]; // six used
// gap 2
STUB u8 D_1734[8];

STUB u8 D_173c[8];
STUB u8 D_1744[8];

STUB char* D_17f6_equipNames[0x30]; // Equipment names

STUB char* D_1856_enemyNames[0x30]; // Job names

STUB char* D_18b6_enemyNamesPlural[0x30]; // Foe names

// 1900
STUB char* D_1916[0x26]; // Items
STUB char* D_1962_equipShortNames[56]; // Armaments
STUB char* D_19d2_reagentNames[8]; // reagent names
STUB char* D_19e2_spellNames[48 + 1]; // spell names

// 1a00

STUB char* D_1a44_classNames[10]; // Class/Jobs (Avatar, Mage, Bard, ...)
STUB s16 D_1a58[9];
STUB char* D_1a6a_statusNames[5]; // Health (Good Health, Poisoned, ...)
STUB u16 D_1a74[5]; // karma string offset
STUB u8 D_1a7e[0x30]; // equipment type?
STUB u8 D_1aae[0x30];
STUB u8 D_1ade[9];
// gap 1
STUB u8 D_1ae8[0x30];
STUB u8 D_1b18_animSequences[0xb0 + 0x33];
// gap 1
STUB char* D_1bfc[26];

STUB char* D_1c30[0x30]; // ~1c90)

STUB u8 D_1c90[0x30];

STUB u8 D_1cc0[0x30];
STUB u16 D_1cf0[0x15];

STUB u8 D_1d1a[0x100];

STUB u8 D_1e1a[0x20]; // map offset table ~1e3a)
STUB char* D_1e3a_mapNames[0x28]; // map name table ~1e8a)
STUB u8 D_1e8a_placeX[0x28]; // where each map's place is in the world
STUB u8 D_1eb2_placeY[0x28]; // ~1ed9) town position y
STUB u8 D_1eda_moonPhases[28 * 2]; // for each day of the month: Trammel's, Felucca's

STUB u8 D_1f12[0x30]; // TODO: size, offset

STUB u8 D_1f42[6];
STUB u8 D_1f48[6];

STUB char* D_1f4e_virtueNames[8];
STUB char* D_1f5e_mantras[8]; // mantra
STUB u8 D_1f6e[8];
STUB u8 D_1f76[8];
STUB PairS8 D_1f7e[0x100]; // ~217e) (x,y) * 0x100 (TODO: Pair)

STUB s16 D_217e_beaconX; // the lighthouse lamp (tile 0x2a in a town, 0x1b outside); -1 none
STUB s16 D_2180_beaconY;
STUB s16 D_2182_beacon2X;
STUB s16 D_2184_beacon2Y;
STUB u8 D_2186_beaconStep; // s8?

STUB s8 D_2188[0x32]; // ~21ba) comsubs: some line table

STUB u8 D_21ba[0x10]; // comsubs: some pixel drawing table

// shop related
STUB char* D_21ca_shopNames[8][16]; // shop names, by shop kind and shop index
STUB char* D_22ca_shopOwners[8][16]; // shop owners, likewise
STUB u8 D_23ca_shopMapIds[8][16]; // the map of each shop: a kind's row is searched for the current map (TALK_00e6_TalkToMerchant)

// dnglook
STUB u8 D_244a[8];
STUB u8 D_2452[6];
STUB u8 D_2458[6];
STUB u8 D_245e[6];
STUB u8 D_2464[6];
STUB u8 D_246a[6];
STUB u8 D_2470[6];
STUB u8 D_2476[0x10];
STUB u8 D_2486[0x10];
STUB u8 D_2496[0x10];
STUB u8 D_24a6[0x10];
STUB u8 D_24b6[0x10];
STUB u8 D_24c6[0x10];

STUB s16 D_24d6_facingDx[4]; // by facing: north, east, south, west
STUB s16 D_24de_facingDy[4]; // direction offset

STUB u8 D_24e6_viewDirty; // the map view must be built again (bit 1: the light map too)
STUB u8 D_24e7; // footstep sfx related flag in dungeon

STUB char* D_24ea_wordTable[0x80]; // the words that bytes of the .TLK scripts (below 0x81) and of SHOPPE.DAT (from 0x80) stand for
STUB char* D_25ea_fileNames[30]; // ~2626) compressed file name table
#define D_25f0 (&D_25ea_fileNames[3])

STUB char* D_2652[4]; // town dat names

STUB u8 D_2742_drunkDirs[4]; // town
STUB u16 D_2746[0xa]; // town: harpsichord audio freqs
STUB u8 D_275a_secretTune[0xd]; // town:harpsichord notes
STUB u8 D_2767_tuneStep; // town

STUB char* D_27dc[3]; // town

STUB u32 D_28c6_npcValueMask[0x20]; // ~2946) town

// 2946~: code strings

// 2900
STUB u8 D_29f6[4]; // mainout: wind-releated
STUB u8 D_29fa[4]; // mainout: wind-releated

// ...code strings...

// mainout
STUB u8 D_2bc0[0xc];
STUB u8 D_2bcc[0x8];
STUB u8 D_2bd4[0x6];
STUB u8 D_2bda[0x2];

STUB u8 D_2bdc[0xc];
STUB u8 D_2be8[0x8];
STUB u8 D_2bf0[0x6];
STUB u8 D_2bf6[0x2];

STUB u16 D_2bf8[4][4];
STUB u8 D_2c18[0x3d];

// 2c58~: Entering room...

// u8 2c54
STUB u8 D_2c55;
// u8 2c56
STUB u8 D_2c57;
// ^ mainout

// dungeon
STUB u8 D_2c76[6]; // TODO: size

STUB u8 D_2de8[8];
STUB u8 D_2df0[8];
STUB u8 D_2df8[12];
STUB u8 D_2e04[12];
STUB char* D_2e10[11];

STUB u8 D_2e26[4];
STUB s16 D_2e2a[4]; // int
STUB s16 D_2e32[2][4]; // int
STUB s16 D_2e42[4]; // int
STUB s16 D_2e4a[4]; // int
STUB s16 D_2e52[4]; // int
STUB s16 D_2e5a[4]; // int
STUB s16 D_2e62[2][4]; // int
STUB s16 D_2e72[4]; // int
STUB s16 D_2e7a[2][4]; // int
STUB u8 D_2e8a[6]; // offset confirmed
STUB u8 D_2e90[10]; // offset confirmed
STUB u8 D_2e9a[10];

STUB Pair D_2ea4[3][8];
STUB Pair D_2ed4[3][5];
STUB Pair D_2ef2[3][4];
STUB Pair D_2f0a[3];
STUB Pair D_2f10[3];

STUB u8 D_2f16[8]; // TODO: size
STUB u8 D_2f1e[8]; // TODO: size

#define D_2f24 D_2f1e[6]
STUB u8 D_2f26;

STUB s8 D_2f28[4]; // direction x
STUB s8 D_2f2c[4]; // direction y
// ^ dungeon

// intro
STUB Pair D_2f98[0x15];
STUB Pair2 D_2fc2[0x15];
STUB u16 D_3016[0x15];
STUB u8 D_3040[0x16]; // 0x15 + gap 1
STUB u8 D_3056[0x16]; // 0x15 + gap 1
STUB u8 D_306c[0x16]; // 0x15 + gap 1
STUB u8 D_3082[0x16]; // 0x15 + gap 1
STUB u8 D_3098[0x16]; // 0x15 + gap 1
STUB u8 D_30ae[0x16]; // 0x15 + gap 1
STUB u8 D_30c4[0x16]; // 0x15 + gap 1
STUB u8 D_30da[0x16]; // 0x15 + gap 1
STUB u8 D_30f0[0x15];
// 0x3105: "WD.BIT"

// 3000

STUB u16 D_3304; // u4 save - is avatar?

STUB u16 D_3656[8];
STUB char* D_3666[8];
// ^intro

// npc
STUB u8 D_367e_npcBlockedTiles[0x20]; // TODO: size ~369e)

// blckthrn
STUB u8 D_369e[0x3c]; // TODO: size
STUB u8 D_36da[0x27];
STUB u8 D_3702[12];
STUB u8 D_370e[8];
STUB u8 D_3716[10];

STUB u16 D_3720[6];
STUB u16 D_372c[6];
STUB u16 D_3738[6];
STUB s16 D_3744[6]; // ~3750)

STUB u8 D_3750[8];
STUB u8 D_3758[8];
STUB u8 D_3760[0xb0]; // ~3810)

STUB char* D_3810[9]; // ~3822)
STUB u8 D_3822[16]; // ~3832)

STUB u8 D_3832[8]; // ~383a; [7] + gap 1?

STUB u8 D_383a[6];
STUB u8 D_3840;
//STUB u8 D_3841;

STUB u8 D_3842[0xd]; // ~384f)? TODO: size, offset
STUB u8 D_3850[0xd]; // ~385d)? TODO: size, offset
                     // D_383f, D_384d

STUB u8 D_385e[8]; // ~3866)
STUB u8 D_3866[8]; // ~386e)
STUB u8 D_386e[8]; // ~3876)
STUB u8 D_3876[0x100]; // ~3976) TODO: size?

STUB u8 D_3a06[4];
STUB u8 D_3a0a[4];
STUB u8 D_3a0e[3];

STUB u16 D_3a26[6];
STUB u8 D_3a32_reagentPrices[5][8];
STUB u8 D_3a5a_reagentLots[5][8];

STUB u16 D_3a82_itemBasePrices[0x30]; // TODO: int (FMT)

STUB u8 D_3ae2_armsStock[9][8]; // ~3b2a)

STUB u16 D_3b2a_greetTexts[8][4]; // int (FMT: 4byte)
STUB u16 D_3b6a_farewellTexts[8][4]; // ~3baa)
STUB u16 D_3baa_farewellTexts1[8][4]; // ~3bea)

STUB u16 D_3bea_provisionPrices[4][4];
STUB u16 D_3c0a_provisionTexts[3];
STUB u16 D_3c10_reagentTexts[8];    // string offset table. type: int (FMT: 4bytes)

STUB char* D_3c20[8];

STUB u16 D_3c30_horsePrices[4];
STUB s16 D_3c38_stableDx[4]; // 3c38..3c40)
STUB s16 D_3c40_stableDy[4]; // 3c40..3c48)
STUB s16 D_3c48_armsTexts[0x2f]; // 3c40..3ca6)? shop item description TODO: size

STUB char* D_3ca6[4];
STUB char* D_3cae[4];
STUB char* D_3cb6[4];

STUB u16 D_3cbe_buybackTexts[8]; // string offset table. type: int (FMT: 4bytes)

STUB char* D_3cce[0x30];

STUB char* D_3d2e[4];

STUB char* D_3d36[4];
STUB char* D_3d3e[4];
STUB char* D_3d46[2];
STUB char* D_3d4a[4];
STUB char* D_3d52[4];

STUB char D_3d5a[]; // "\n\n\"Thou hast no need of this art!\"\nsays $."

STUB u8 D_3d86_healPrices[8];
STUB u8 D_3d8e_curePrices[8];
STUB u16 D_3d96_raisePrices[8];

STUB Pair D_3da6[6];
STUB Pair2 D_3db2[6];
STUB u16 D_3dca[6];

STUB u8 D_3dd6[6];
STUB u8 D_3ddc[6];
STUB u8 D_3de2[6];
STUB u8 D_3de8[6];
STUB u8 D_3dee[6]; // endgame_0000 image idx
STUB u8 D_3df4[6];
STUB u8 D_3dfa[6]; // endgame_0000 image x
STUB u8 D_3e00[6]; // endgame_0000 image y
STUB u8 D_3e06[6];

STUB char* D_3e0c[19];
STUB char* D_3e32[8];
STUB char* D_3e42[12];

STUB u8 D_3e5a[6];
STUB u8 D_3e60[6];

STUB u8 D_3e66[4];
STUB u8 D_3e6a[4];
STUB u8 D_3e6e[4];
STUB char* D_3e72[3];

STUB u8 D_3e78[0x72]; // TODO: size
STUB u8 D_3eea[0x72]; // hidden item - value TODO: size
STUB u8 D_3f5c[0x72]; // hidden item - map id TODO: size
STUB u8 D_3fce[0x72]; // hidden item - map z TODO: size
STUB u8 D_4040[0x72]; // hidden item - map x TODO: size
STUB u8 D_40b2[0x72]; // hidden item - map y TODO: size

// 4100
STUB u8 D_4124[8];
STUB u8 D_412c[8];
STUB u8 D_4134[8];

STUB u8 D_413c[0x30];
STUB u8 D_416c[0x30]; // ~419c)

STUB char* D_419c[8];
STUB char* D_41ac[8];
STUB u8 D_41bc[8];
STUB u8 D_41c4[8];
STUB u8 D_41cc[8];

STUB char* D_444a[3];

STUB char* D_4502[8];
STUB u8 D_4512[8];

STUB u8 D_4592_fieldMissileKinds[4];
STUB u8 D_4596_dungeonFieldCells[4];

STUB u8 D_4882_shardX[4]; // map x
STUB u8 D_4886_shardY[4]; // map y
STUB u8 D_488a_shardMap[4]; // map id
STUB u8 D_488e_shardLevel[4]; // map level
STUB u8 D_4892_shardKilledBit[3];

STUB char* D_4aa0_talkFileNames[4]; // the .TLK file of each group of eight maps
STUB char* D_4aa8_standardWords[0x22+1]; // NAME, JOB, WORK, BYE, THANK, then words that get a rebuke
// 4aec: "ELECTRONIC ARTS"

// talk related
STUB u8 D_4aee_talkPendingCode; // the script code whose argument bytes are being read; 0 none
STUB u8 D_4aef_talkCodeArgCount; // how many of them are in D_bce0_talkCodeArgs
STUB u8 D_4af0_talkCharMask; // 0xff; without the high bit while runes are on (script code 0x8e)
STUB u8 D_4af1_talkWordLength; // the length of the word in D_bce4_talkWord
STUB u8 D_4af2_talkInLabel; // 0xff while a label's question waits for its answer
STUB u8 D_4af3_talkTextX; // the text column after the last character printed
STUB u8 D_4af4_talkLastChar; // the last character processed
STUB u8 D_4af5_talkAfterWord; // 1 after a word of the word table: a space comes before the next character

STUB u16 D_4af6[9];
STUB u16 D_4b08[9]; // cast sound freq table 1
STUB u16 D_4b1a[9]; // cast sound freq table 2
STUB u16 D_4b2c[9]; // cast sound pulse table

STUB char* D_4b3e[8]; // virtue keywords

STUB u16 D_4b5e_questTextPos[8]; // int
STUB u16 D_4b6e_codexTextPos[8];

STUB u8 D_4b7e_strengthGain[8];
STUB u8 D_4b86_dexterityGain[8];
STUB u8 D_4b8e_intelligenceGain[8];

STUB u8 D_4b96_urnFirst[8];
STUB u8 D_4b9e_urnX[0x24]; // TODO: size
STUB u8 D_4bc2_urnY[0x24]; // TODO: size

// some sound table
STUB u16 D_4be6[7];
STUB u16 D_4bf4[7];
STUB u16 D_4c02[7];
STUB u16 D_4c10[7];

STUB char D_4c1e_foodKeys[5];
STUB char D_4c24_drinkKeys[5];
STUB char D_4c2a_rationKeys[5];
STUB char D_4c30_gossipKeys[5];

STUB u16 D_4c36_mealPrices[9]; // TODO: size
STUB u16 D_4c48_winePrices[6]; // TODO: size
STUB u16 D_4c54_rationPrices[9]; // TODO: size
STUB u16 D_4c66_rationTexts[7]; // TODO: size

STUB char* D_4c74[26];
STUB char* D_4ca8[26];
STUB u8  D_4cdc_gossipPlaceIdx[26];
STUB char* D_4cf6[13];

STUB u16 D_4d10_gossipPrices[0x1a]; // TODO: size
STUB u16 D_4d44_gossipTexts[4];
STUB u8  D_4d4c_pubKindByShop[0xa]; // TODO: size
STUB u16 D_4d56_pubOrderTexts[4];
STUB u16 D_4d5e_pubMoreTexts[4];
STUB u16 D_4d66_frigatePrices[4];
STUB u16 D_4d6e_skiffPrices[4];
STUB u8  D_4d76_shipX[4];
STUB u8  D_4d7a_shipY[4];
STUB u8  D_4d7e_innRates[6]; // the inn's rate (per person and night; times 10 per month)

STUB u8 D_4dc4_innRooms[6]; // how many companions the inn can keep

STUB u16 D_4e6e_innOfferTexts[6]; // the offer, an offset in SHOPPE.DAT
STUB u8 D_4e7a_innBedX[6]; // where the party wakes up
STUB u8 D_4e80_innBedY[6];

STUB u8 D_50ea[0x5b]; // glyph width table, from ' '

#define D_50f7 D_50ea[0xd]

STUB u16 D_5146[2];
STUB u16 D_514c[2];
STUB u16 D_5150; // TODO: int?
STUB u16 D_5152; // TODO: int?
STUB u16 D_5154;
STUB u16 D_5156;
STUB u16 D_5158; // TODO: int?
STUB u16 D_515a; // = 0

STUB char* D_515c[4];

// 5200

STUB u8 D_5164[8];
STUB u8 D_516c[8];
STUB u8 D_5174[8];
STUB u16 D_517c[8][8];
STUB u8 D_51fc[8];
STUB u8 D_5204[8];
STUB u16 D_520c; // AFONT.ASM's: where FONT_0e52 saves the video page

// main parameters
// int D_52a2;     // __argc
// char** D_52a4;  // __argv
// char** D_52a6;  // environ

STUB VideoDriverParams D_52ba_vdp;
#if defined(TARGET_DOS16)
// A0878.ASM's own names for two fields: the C modules read them as plain globals, which MSC
// optimizes differently from a struct field (LOOKOBJ_0cf4)
STUB int D_52c4_clipEnabled;
STUB int D_52c8_videoDriver;
#else
#define D_52c4_clipEnabled D_52ba_vdp.clipEnabled
#define D_52c8_videoDriver D_52ba_vdp.videoDriver
#endif

STUB int D_52da_penColor;

STUB int D_52ef_forceEga; // force ega
STUB int D_52f1_forceTandy; // force tandy
STUB int D_52f3_forceHerc; // force herc

STUB int D_52f6; // temp area?
STUB int D_52f8; // temp area?

// ?
STUB int D_52fa; // x1
STUB int D_52fc; // y1
STUB int D_52fe; // x2
STUB int D_5300; // y2

// 5300
STUB int D_5304; // previous video mode

STUB int D_5306[7]; // origin logo frames: widths (signed: centred with sar)
STUB int D_5314[7]; // and heights

// 5322..5327: "Tandy"
// 5328..532f: video driver string table
// 5330..5337: "CGA.DRV"
// 5338..533f: "EGA.DRV"
// 5340..5347: "T1K.DRV"
// 5348..534f: "HER.DRV"
STUB void* FAR D_5350_driverEntry; // far pointer into the video driver; the offset is the function number
STUB u16 D_5354_pageSegment; // segment of video page buffer (ega: 0xa800, t1k: allocated memory)
STUB u16 D_5356_timerCalibration; // timer calibration
STUB u16 D_5358_memorySize; // memory size
STUB u16 D_535a; // current timer value
STUB u16 D_535c_dosError; // int21 error code

// 535e..537d
STUB TextWindow D_535e_textWindows[4];

STUB u16 D_5386_textWindowIndex; // charset_id (offset in 539a, data in 53a4..53ab)
STUB u16 D_5388_charsetIndex; // selected charset idx
STUB u16 D_538a; // bool: special keystroke
STUB u16 D_538c_keyBufferOff; // Ctrl-B: "Buffer Off/On"
STUB u16 D_538e; // cursor advance flag
STUB u16 D_5390; // keystroke cursor begin idx
STUB u16 D_5392;
STUB void (FAR *D_5394_diskSwapFn)(void); // disk swap message function pointer
//STUB u16 D_5398; // segment of current charset
STUB u8* FAR D_5398_currentCharset;        // ptr of current charset
STUB TextWindow* D_539a_currentTextWindow; // address of current text window data
//STUB u16 D_539c[4]; // charset segment table
STUB u8* FAR D_539c[4];    // charset table
STUB u16 D_53a4_underline; // underline flag (bit 0 of byte 7)
STUB u16 D_53a6; // ? flag (bit 1 of byte 7)
STUB u16 D_53a8_inverse; // inverse flag (bit 1 of byte 7)
STUB u8  D_53aa_textFgColor; // text fg color (lo nibble of byte 6)
STUB u8  D_53ab_textBgColor; // text bg color (hi nibble of byte 6)

STUB u16 D_53b4; // loaded file size (lsb)
STUB u16 D_53b6; // loaded file size (msb)

STUB u16 D_53b8; // file handle
STUB u16 D_53ba; // first allocated segment
STUB u16 D_53bc; // first allocated segment (for return)
STUB u16 D_53be; // second allocated segment (for lzw)

// u16 53c4: read pointer
// u16 53cc: 0x102
// u16 53d0: 0x200
// u16 53d2: read advance amount? (9)

// 53d6..53e8: "Out of Memory...\n$"

STUB u8 D_53ea[24]; // ~5402) character data from charset file (herc=24, others=8)
STUB s16 D_5402[5]; // 5402..540c) 1, 10, 100, 1000, 10000
STUB u16 D_540c; // cursor anim idx // TODO: int
STUB u8 D_540e[8]; // extended ascii code for arrow keys
STUB u8 D_5416[8]; // special key table
STUB u8 D_541e; // drive number
STUB u16 D_5420_randomSeed; // random seed?
STUB u16 D_5422;
STUB u16 D_5424;
STUB u16 D_5426[2]; // timer-related?

STUB u16 D_5448; // current timer value
STUB u16 D_544a; // timer tick count to wait
// far D_544c; // far addr of original int 1c handler

STUB u8 D_545e;

// ...54d2: strings

// 5500
STUB u8 D_54d4[32]; // normal walkable tile flag table
STUB u8 D_54f4[64]; // actor tile -> move type table
STUB u8 D_5534[16]; // skiff walkable flag table
STUB u8 D_5544[4];

STUB u8 D_559e[8]; // ~55a6)

//
// Savegame / User state
// 

// [55a6..6606): savegame (0x1060)
STUB u16 D_55a6_saveStart; // +0x0: two bytes of unknown use; the block is read and written from here

STUB S_55a8 D_55a8_party[0x10]; // ~57a8)

// 5700
STUB s16 D_57a8_food;        // +0x0202 Food // int (TALK_0682_ReceiveItem, MOVSX)
STUB s16 D_57aa_gold;        // +0x0204 Gold // int (MAINOUT_1b3e, MOVSX)
STUB u8  D_57ac_keys; // +0x206
STUB u8  D_57ad_gems; // +0x207
STUB u8  D_57ae_torches; // +0x208
STUB u8  D_57af_grapple; // +0x209
STUB u8  D_57b0_carpets; // +0x20A magic carpets
STUB u8  D_57b1_skullKeys; // +0x20B
STUB u8  D_57b2_skullKeysDay; // +0x20C the day Minoc's skull keys were last taken
STUB u8  D_57b3_amulet; // +0x20D Lord British's
STUB u8  D_57b4_crown; // +0x20E
STUB u8  D_57b5_sceptre; // +0x20F
STUB u8  D_57b6_shards[4]; // +0x210
STUB u8  D_57ba_spyglasses; // +0x214
STUB u8  D_57bb_hmsCapePlans; // +0x215
STUB u8  D_57bc_sextants; // +0x216
STUB u8  D_57bd_pocketWatch; // +0x217
STUB u8  D_57be_blackBadge; // +0x218
STUB u8  D_57bf_sandalwoodBox; // +0x219
// FMT pad 2byte (?)
STUB u8  D_57c0_equipment[0x30]; // +0x21A how many of each piece of equipment the party owns
#define D_57db_arrows D_57c0_equipment[0x1b] /* +0x0235 Arrows */
#define D_57dd_quarrels D_57c0_equipment[0x1d] /* +0x0237 Quarrels */
STUB u8  D_57f0_mixedSpells[0x30]; // +0x24A

// 5800
STUB u8  D_5820_scrolls[8]; // +0x27A
STUB u8  D_5828_potions[8]; // +0x282
STUB u8  D_5830_moonstoneX[8]; // +0x28A
STUB u8  D_5838_moonstoneY[8]; // +0x292
STUB u8  D_5840_moonstoneMap[8]; // +0x29A the map it is buried in (0 Britannia); 0xff: carried
STUB u8  D_5848_moonstoneZ[8]; // +0x2A2
STUB u8  D_5850_reagents[8]; // +0x2AA
STUB u8  D_5858_harvestDay[3]; // +0x2B2
STUB u8  D_585b_partyCount;        // +0x02B5 party member count
STUB u8  D_585c_takenItemFlags[0x18]; // +0x2B6 the Codex's "non-regenerating object flags" (SJOG sets a bit per find)
STUB u16 D_5874_year; // +0x2CE
STUB s16 D_5876_tempX; // +0x2D0 scratch: a direction chosen, a position, the actor found
STUB s16 D_5878_tempY; // +0x2D2
STUB u8  D_587a_statusIcon; // +0x2D4 the letter between the party and the food line: 'T', 'Q', 'P', 'N'...
STUB u8  D_587b_activePlayer;        // +0x02D5 Active character
STUB u8  D_587c_partyTile; // +0x02D6 party tile
STUB u8  D_587d_month; // +0x2D7
STUB u8  D_587e_day; // +0x2D8
STUB u8  D_587f_hour;        // +0x02D9 Current hour
STUB u8  D_5880_lastHour; // +0x2DA
STUB u8  D_5881_minute; // +0x2DB
STUB u8  D_5882;
STUB u8  D_5883;        // ?
STUB u8  D_5884_clockChimes; // +0x2DE strokes the clock has still to chime
STUB u8  D_5885_trammelPhase; // +0x2DF
STUB u8  D_5886_feluccaPhase; // +0x2E0
STUB u8  D_5887_moongateHeight; // +0x2E1
STUB u8  D_5888_karma;        // +0x02E2 Karma
STUB u8  D_5889;
STUB u8  D_588a;
STUB u8  D_588b_turnCount; // +0x2E5
STUB u8  D_588c_campHealWait; // +0x2E6 hours until camping heals again (the Codex)
STUB u8  D_588d;
STUB u8  D_588e_protectTurns; // +0x2E8
STUB u8  D_588f;
STUB u8  D_5890;
STUB u8  D_5891_animate; // +0x2EB 0, 1 or 0xff
STUB u8  D_5892_windDir; // +0x2EC
STUB u8  D_5893_mapId; // +0x2ED
STUB u8  D_5894_savedMapId; // +0x2EE the map while D_5893_mapId is a combat map
STUB u8  D_5895_mapLevel; // +0x2EF
STUB u8  D_5896_mapX; // +0x2F0
STUB u8  D_5897_mapY; // +0x2F1
STUB u8  D_5898_crosshairOn; // +0x2F2
STUB u8  D_5899_crosshairX; // +0x2F3
STUB u8  D_589a_crosshairY; // +0x2F4
STUB u8  D_589b_mapOriginX; // +0x2F5 the map buffer's upper left corner in the world
STUB u8  D_589c_mapOriginY; // +0x2F6
STUB u8  D_589d_attackWeapon; // +0x2F7
STUB u8  D_589e_curCombatant; // +0x2F8 the entry of D_ba14_combatants whose turn it is; 0xff none
STUB u8  D_589f_focusBlink; // +0x2F9 toggled each frame: the frame around the combatant
STUB u8  D_58a0_combatExitDir; // +0x2FA
STUB u8  D_58a1_combatFlags; // +0x2FB
STUB u8  D_58a2;
STUB u8  D_58a3_combatOver; // +0x2FD the Codex's "no more enemies"
STUB u8  D_58a4_viewOn; // +0x2FE the map view is updated and animated
STUB u8  D_58a5_lightLevel; // +0x2FF
STUB u8  D_58a6_lightSpellTurns; // +0x300
STUB u8  D_58a7_torchTurns; // +0x301
STUB u8  D_58a8[0x20];  // [58a8..58c8)
STUB u8  D_58c8_shadowlordTowns[3]; // +0x322
STUB u8  D_58cb;
STUB u16 D_58cc_questsOrdained; // +0x326
STUB u16 D_58ce_questsDone; // +0x328
STUB u8  D_58d0_dungeonOpen[8]; // +0x32A 0 sealed, 0x80 open
STUB u8  D_58d8_shrineDestroyed[8]; // +0x332 bit 7
STUB u8  D_58e0_roomsCleared[0xe]; // +0x33A
STUB u8  D_58ee_annotX[0x20]; // +0x348 the Codex's "annotations" (TOWN lists the tiles 0x48/0x49 of the map here)
// 5900
STUB u8  D_590e_annotY[0x20]; // +0x368
STUB u8  D_592e_annotTile[0x20]; // +0x388
STUB u8  D_594e_annotCount; // +0x3A8
STUB u8  D_594f_openDoorTile; // +0x3A9
STUB u8  D_5950_openDoorX; // +0x3AA
STUB u8  D_5951_openDoorY; // +0x3AB
STUB u8  D_5952_openDoorTurns; // +0x3AC
STUB u8  D_5953_boughtShipX; // +0x3AD
STUB u8  D_5954_boughtShipY; // +0x3AE
STUB u8  D_5955_sailDir; // +0x3AF
STUB u8  D_5956_newPrompt; // +0x3B0
STUB u8  D_5957_drunkTurns; // +0x3B1
STUB u8  D_5958_townShadowlord; // which of the three shadowlords (D_58c8_shadowlordTowns) is in this town, 0xff none (TOWN_02ae_SetupShadowlord)
STUB u8  D_5959;
STUB u8 D_595a_dungeonMap[0x200]; // +0x3B4
// 5b00
STUB u8 D_5b5a_npcKilled[0x80]; // +0x5B4
STUB u8 D_5bda_npcMet[0x80]; // +0x634

STUB ActorFmt D_5c5a_actors[32]; // +0x6B4

// 5d00
STUB u16 D_5d5a; // unused?
STUB u16 D_5d5c; // unused?

STUB NpcScheduleFmt D_5d5e_npcSchedules[32]; // +0x7B8

// 5f00
STUB NpcFmt D_5f5e_npcs[32]; // +0x9B8

// 6100
STUB MovementFmt D_615e_moveLists[0x20]; // +0xBB8
//STUB u8 D_615e_moveLists[0x400];

STUB s16 D_655e_moveListPos[0x20]; // +0xFB8
STUB u8 D_659e_npcTypes[0x20]; // +0xFF8
STUB u8 D_65be;
STUB u8 D_65bf;
STUB u16 D_65c0; // unused?
STUB s16 D_65c2[0x20];

// 6600
STUB u8 D_6602;
STUB u8 D_6603_dungeonFacing; // +0x105D 0 north, 1 east, 2 south, 3 west
STUB u8 D_6604_dungeonStyle; // +0x105E 1 cave, 2 mine, 3 dungeon
STUB u8 D_6605_boughtShip; // +0x105F 0x40 a skiff; 0x80 + n a frigate with n skiffs
STUB u16 D_6606_saveEnd; // +0x1060: the end of the block

STUB S_6608_Map D_6608_map; // ~6a08) map data: layer4 * y16 * x16 / y32 * x32
#define D_6608 D_6608_map.raw
#define D_6a07 (D_6608[1023])

#if defined(TARGET_DOS16)
// in the original, DS:6708 is a place inside D_6608_map, not a variable of its own
#define D_6708_map (*(S_6608_Map*)(D_6608_map.raw + 0x100))
#else
STUB S_6608_Map D_6708_map; // copy of 6608
#endif
#define D_6708 D_6708_map.raw

#if defined(TARGET_DOS16)
// The original reaches the overworld map through a name of its own for DS:6608: in
// ULTIMA_4402 MSC merges the town and overworld paths only up to the store, which it
// does when their final `add ax,OFFSET ...` name different symbols. Which object defined
// it is not known; the link defines it as D_6608_map's place (tools/u5.py, LINK_DEFINES).
STUB S_6608_Map D_6608_overworld;
#else
#define D_6608_overworld D_6608_map
#endif

STUB u8 D_6a08; // some audio related

STUB u16 D_6a0a_powersOfTen[5];

STUB u8 D_6a14[0x20];

STUB u8 D_6a34_ambientTick;
STUB u16 D_6a36[9]; // banjo freq table

STUB u8 D_6a48[0x36]; // iolo's banjo song note table

STUB u8 D_6a7e;

STUB u8 D_6a80[6]; // light intensity slope?
STUB u8 D_6a86[0x13]; // ~6a99)

STUB u8 D_6a9a[10];

STUB u16 D_6aa4;
STUB u8 D_6aa8[0x24];

// a300


//////// (static) data: ..a522]

// a500

// DS:a524-a9bc is the modules' _BSS, their static variables, in link order: D_a524 (mainout.c),
// D_a526 and D_a527 (mainoutb.c), D_a528 and D_a628 (dnglook.c), D_a728_pathXs and D_a872_pathYs (comsubs.c).

// a900

// From DS:a9bc (where the runtime's empty XOB/XO/XOE segments are) to _end: communal
// variables, declared with BSS. vars.c defines them; the link allocates them in PLINK86's
// order, by their sizes (tools/u5.py, COMMUNALS). The few that no code refers to (D_bbd2,
// D_bcb0, D_bd14, D_bd3a, D_bd3b) are declared so that the others land where they are.

BSS u8  D_a9bc_townJustEntered;
BSS u8 D_a9bd_curDisk[1]; // the disk wanted (ULTIMA_251e_SwitchDisks)
BSS u8 D_a9be; // two bytes on an odd address would have been put on an even one
BSS u8* D_a9c0; // loaded image pointer
BSS u8 D_a9c2;
BSS u8* D_a9c4; // loaded image pointer
BSS u8* D_a9c6; // loaded image pointer

// disk drive table
BSS u8 D_a9c8_diskDrives[6]; // the drive of each disk; 0xff: not set

#define D_a9c9 D_a9c8_diskDrives[1]
#define D_a9ca D_a9c8_diskDrives[2]
#define D_a9cb D_a9c8_diskDrives[3]
#define D_a9cc D_a9c8_diskDrives[4]
#define D_a9cd D_a9c8_diskDrives[5]

// used in intro
BSS u8 D_a9ce_soundOn; // use sound effect?

BSS u16 D_a9d0_rayBalance[0x15];

BSS u8 D_a9fa_vitalsDirty; // dirty flag for vital
BSS u8 D_a9fb; // footstep sfx duration in dungeon

// combat?
BSS ActorFmt D_a9fc_savedActors[0x20]; // D_5c5a_actors during combat

BSS char* D_aafc_shopName; // current shop name
BSS char* D_aafe_shopOwner; // current shop owner

BSS char* D_ab00_itemName;
BSS u8 D_ab02_viewTiles[0x160]; // the 11x11 view, 0x20 bytes a row: 0 an actor (in D_ac64_viewActors), 0xff hidden
BSS char* D_ac62_placeName;

BSS u8 D_ac64_viewActors[0xb0]; // actor map / misc map [0x10 * b] (from 51b8)

BSS u8 D_ad14_combatMap[32 * 32]; // the combat map; outside combat the 32x32 light map

// b100

BSS s16 D_b114_shopIndex; // s16 (FMT MOVSX); the shop's column in D_21ca_shopNames..., 0-15
BSS s16 D_b116_shopKind; // s16 (FMT MOVSX); 0-7, the merchant's dialog number - 0x81
BSS s16 D_b118_shopPrice; // s16 (FMT MOVSX); the % of a shop text
BSS u16 D_b11a_shopQuantity;

BSS u8* D_b11c;
BSS u8 D_b11e_tileFrames[0x100]; // the tile drawn for each map tile (animation)

#if !defined(MATCHING_BUILD)
#define SCRATCH_SIZE 3000
#else
#define SCRATCH_SIZE 2000
#endif

BSS u8 D_b21e_scratch[SCRATCH_SIZE]; // scratch buffer ~b9ee)

#define D_b31e (D_b21e_scratch + 0x100)
#define D_b7fa_shopTextOut (D_b21e_scratch + 1500)

// b900

BSS u8  D_b9ee[0x26]; // owned item counts
// [8]
#define D_b9f6_potionCounts (D_b9ee + 8)
#define D_b9fe_carpetCount D_b9ee[0x10]
#define D_b9ff_skullKeyCount D_b9ee[0x11]
#define D_ba00_amuletOwned D_b9ee[0x12]
#define D_ba01_crownOwned D_b9ee[0x13]
#define D_ba02_sceptreOwned D_b9ee[0x14]
// [8]
#define D_ba03_moonstoneFlags (D_b9ee + 0x15)
// [3]
#define D_ba0b_shardFlags (D_b9ee + 0x1d)
#define D_ba0e_spyglassCount D_b9ee[0x20]
#define D_ba0f_capePlansOwned D_b9ee[0x21]
#define D_ba10_sextantCount D_b9ee[0x22]
#define D_ba11_pocketWatchOwned D_b9ee[0x23]
#define D_ba12_blackBadgeOwned D_b9ee[0x24]
#define D_ba13_sandalwoodBoxOwned D_b9ee[0x25]

BSS CombatEntity D_ba14_combatants[32]; // combat entities

BSS u8  D_bb14; // pos?
BSS u8  D_bb15; // pos?
BSS u8  D_bb16;
BSS u8  D_bb17;
BSS u16 D_bb18; // "lord british" index
BSS void* D_bb1a; // pointer

BSS S_bb1c D_bb1c;
BSS u8 D_bbd2[0xb6]; // no code refers to it; as large as D_bb1c

BSS S_bc88_U4Party D_bc88;
BSS u16 D_bcb0; // no code refers to it

BSS char D_bcb2[0x27 + 1];

BSS u8 D_bd2a[8];
BSS u8 D_bd32[8];

BSS u8 D_bcda;

BSS u16 D_bcdc_talkNpcIndex; // the slot of the NPC talked to
BSS u8* D_bcde_talkPtr; // the place in the NPC's script
BSS u8 D_bce0_talkCodeArgs[4]; // the bytes after a script code (TALK_0dbe_ProcessCodeArgument)
BSS char D_bce4_talkWord[0x10]; // the word being collected, printed at its end (word wrap)
BSS u8 D_bcf4_talkLabel; // the label being asked, 0x91-0x9f
BSS u16 D_bcf6_talkKeywordIndex; // the NPC's keyword that matched
BSS char D_bcf8_inputText[16]; // what the player typed

BSS char D_bd08_answer[12];
BSS u8 D_bd14; // no code refers to it

BSS u8 D_bd15_savedMapId;
BSS u8 D_bd16_pubKind;
BSS s16 D_bd18_boughtSomething; // int?
BSS int D_bd1a_dinersFed; // int
BSS int D_bd1c_dinerCount; // int
BSS int D_bd1e_tableTile; // int
BSS s16 D_bd20_drinksBought; // int?
BSS s16 D_bd22_thrownOut;
BSS s16 D_bd24_shipSold;

BSS u8 D_bd26;
BSS u8 D_bd27;
BSS u8 D_bd28;
BSS u8 D_bd29;

BSS u8 D_bd3a; // no code refers to it
BSS u8 D_bd3b; // no code refers to it
BSS u8 D_bd3c;
BSS u8 D_bd3d;
BSS u8 D_bd3e;

// ..bd3f]

#endif
