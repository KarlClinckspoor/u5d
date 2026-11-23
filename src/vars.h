#ifndef _VARS_H
#define _VARS_H

#ifndef STUB
#define STUB extern
#endif

// ref: u5tech
// Savegame ref: https://wiki.ultimacodex.com/wiki/Ultima_V_internal_formats#SAVED.GAM_and_RAM
// (the +0x... offsets in SAVED.GAM and what the bytes mean are the Codex's where the code agrees or does not say;
// "the Codex" marks a meaning taken from it alone)

#include "structs.h"

// 103c..
// tlk word table

// 1300

STUB u8 shadowlordStartY[8]; // TODO: size?

STUB u16 redColor;
STUB u16 brightWhiteColor;
STUB u16 blueColor;
STUB u16 greenColor;
STUB u16 magentaColor;
STUB u16 brightYellowColor;
STUB u16 lightGrayColor;

STUB EnemyStats D_13bc_enemyStats[0x30]; // 0x30 ~153c
STUB u16 enemyFlags[0x30]; // enemy flags
STUB u8 enemyRange[0x30]; // enemy related
STUB u8 enemyMissile[0x30]; // enemy related

STUB u8 weaponAttack[0x38]; // ~1634) some equip table

STUB u8 equipDefense[0x30]; // ~1664) equip defense table
STUB u8 weaponRange[0x38]; // ~169c) some equip table
STUB u8 weaponMissile[0x38]; // ~16d4) some equip table

STUB u8 enemyCompanion[0x30]; // the other kind of enemy that may come with one (ULTIMA_6bc2)
STUB u8 combatEnemyX[0x10]; // where the enemies start on the combat map
STUB u8 combatEnemyY[0x10]; // ~1714)
STUB u8 combatPartyX[8]; // where the party starts
// gap 2
STUB u8 combatPartyY[8]; // six used
// gap 2
STUB u8 ambushEnemies[8];

STUB u8 monsterTypes[8];
STUB u8 monsterAnimStates[8];

STUB char* equipNames[0x30]; // Equipment names

STUB char* enemyNames[0x30]; // Job names

STUB char* enemyNamesPlural[0x30]; // Foe names

// 1900
STUB char* itemNames[0x26]; // Items
STUB char* equipShortNames[56]; // Armaments
STUB char* D_19d2_reagentNames[8]; // reagent names
STUB char* spellNames[48 + 1]; // spell names

// 1a00

STUB char* classNames[10]; // Class/Jobs (Avatar, Mage, Bard, ...)
STUB s16 classNamePad[9];
STUB char* statusNames[5]; // Health (Good Health, Poisoned, ...)
STUB u16 karmaTextOffsets[5]; // karma string offset
STUB u8 equipSlotKind[0x30]; // equipment type?
STUB u8 equipWeight[0x30];
STUB u8 classTile[9];
// gap 1
STUB u8 equipMarker[0x30];
STUB u8 animSequences[0xb0 + 0x33];
// gap 1
STUB char* mantraWords[26];

STUB char* spellCodes[0x30]; // ~1c90)

STUB u8 spellUsability[0x30];

STUB u8 spellReagents[0x30];
STUB u16 raySlopes[0x15];

STUB u8 cellPattern[0x100];

STUB u8 townMapOffset[0x20]; // map offset table ~1e3a)
STUB char* mapNames[0x28]; // map name table ~1e8a)
STUB u8 placeX[0x28]; // where each map's place is in the world
STUB u8 placeY[0x28]; // ~1ed9) town position y
STUB u8 moonPhases[28 * 2]; // for each day of the month: Trammel's, Felucca's

STUB u8 prisonCellOf[0x30]; // TODO: size, offset

STUB u8 prisonCellX[6];
STUB u8 prisonCellY[6];

STUB char* virtueNames[8];
STUB char* mantras[8]; // mantra
STUB u8 shrineX[8];
STUB u8 shrineY[8];
STUB PairS8 beamCells[0x100]; // ~217e) (x,y) * 0x100 (TODO: Pair)

STUB s16 D_217e_beaconX; // the lighthouse lamp (tile 0x2a in a town, 0x1b outside); -1 none
STUB s16 D_2180_beaconY;
STUB s16 beacon2X;
STUB s16 beacon2Y;
STUB u8 beaconStep; // s8?

STUB s8 missileShapes[0x32]; // ~21ba) comsubs: some line table

STUB u8 sparkOffsets[0x10]; // comsubs: some pixel drawing table

// shop related
STUB char* shopNames[8][16]; // shop names, by shop kind and shop index
STUB char* shopOwners[8][16]; // shop owners, likewise
STUB u8 shopMapIds[8][16]; // the map of each shop: a kind's row is searched for the current map (TalkToMerchant)

// dnglook
STUB u8 centreTiles[8];
STUB u8 roomPartyX[6];
STUB u8 roomPartyY[6];
STUB u8 partyLineA[6];
STUB u8 partyLineB[6];
STUB u8 partyLineC[6];
STUB u8 partyLineD[6];
STUB u8 monsterLineA[0x10];
STUB u8 monsterLineB[0x10];
STUB u8 monsterLineC[0x10];
STUB u8 monsterLineD[0x10];
STUB u8 roomMonsterX[0x10];
STUB u8 roomMonsterY[0x10];

STUB s16 facingDx[4]; // by facing: north, east, south, west
STUB s16 facingDy[4]; // direction offset

STUB u8 viewDirty; // the map view must be built again (bit 1: the light map too)
STUB u8 stepToggle; // footstep sfx related flag in dungeon

STUB char* wordTable[0x80]; // the words that bytes of the .TLK scripts (below 0x81) and of SHOPPE.DAT (from 0x80) stand for
STUB char* fileNames[30]; // ~2626) compressed file name table
#define tilesFileName (&fileNames[3])

STUB char* townFileNames[4]; // town dat names

STUB u8 drunkDirs[4]; // town
STUB u16 harpsichordFreq[0xa]; // town: harpsichord audio freqs
STUB u8 secretTune[0xd]; // town:harpsichord notes
STUB u8 tuneStep; // town

STUB char* shadowlordTraits[3]; // town

STUB u32 npcValueMask[0x20]; // ~2946) town

// 2946~: code strings

// 2900
STUB u8 windDx[4]; // mainout: wind-releated
STUB u8 windDy[4]; // mainout: wind-releated

// ...code strings...

// mainout
STUB u8 landMonsters[0xc];
STUB u8 deepLandMonsters[0x8];
STUB u8 seaMonsters[0x6];
STUB u8 deepSeaMonsters[0x2];

STUB u8 landWeights[0xc];
STUB u8 deepLandWeights[0x8];
STUB u8 seaWeights[0x6];
STUB u8 deepSeaWeights[0x2];

STUB u16 sailDelay[4][4];
STUB u8 chaseRange[0x3d];

// 2c58~: Entering room...

// u8 2c54
STUB u8 D_2c55_quickToggle;
// u8 2c56
STUB u8 D_2c57_mountToggle;
// ^ mainout

// dungeon
STUB u8 facingOfDir[6]; // TODO: size

STUB u8 placeNameBase[8];
STUB u8 placeNameCount[8];
STUB u8 placeNameX[12];
STUB u8 placeNameY[12];
STUB char* dungeonPlaceNames[11];

STUB u8 monsterTurnMirror[4];
STUB s16 monsterX[4]; // int
STUB s16 monsterY[2][4]; // int
STUB s16 fieldTop[4]; // int
STUB s16 fieldBottom[4]; // int
STUB s16 fieldCount[4]; // int
STUB s16 fieldWidth[4]; // int
STUB s16 D_2e62_pieceX[2][4]; // int
STUB s16 itemPieceX[4]; // int
STUB s16 itemPieceY[2][4]; // int
STUB u8 aheadPieceOffset[6]; // offset confirmed
STUB u8 aheadDripY[10]; // offset confirmed
STUB u8 sideDripY[10];

STUB Pair dotsDist0[3][8];
STUB Pair dotsDist1[3][5];
STUB Pair dotsDist2[3][4];
STUB Pair dotsDist3[3];
STUB Pair dotsDist0Extra[3];

STUB u8 mirroredPiece[8]; // TODO: size
STUB u8 plainPiece[8]; // TODO: size

#define plainPiece6 plainPiece[6]
STUB u8 dotFrame;

STUB s8 sideDx[4]; // direction x
STUB s8 sideDy[4]; // direction y
// ^ dungeon

// intro
STUB Pair D_2f98_textLeft[0x15];
STUB Pair2 D_2fc2_textRight[0x15];
STUB u16 storyOffset[0x15];
STUB u8 D_3040_regionTop[0x16]; // 0x15 + gap 1
STUB u8 D_3056_regionBottom[0x16]; // 0x15 + gap 1
STUB u8 pageTextX[0x16]; // 0x15 + gap 1
STUB u8 D_3082_startY[0x16]; // 0x15 + gap 1
STUB u8 D_3098_imageIndex[0x16]; // 0x15 + gap 1
STUB u8 D_30ae_imageFile[0x16]; // 0x15 + gap 1
STUB u8 pageImageX[0x16]; // 0x15 + gap 1
STUB u8 pageImageY[0x16]; // 0x15 + gap 1
STUB u8 pageKind[0x15];
// 0x3105: "WD.BIT"

// 3000

STUB u16 notAvatar; // u4 save - is avatar?

STUB u16 labelRow[8];
STUB char* fieldLabels[8];
// ^intro

// npc
STUB u8 npcBlockedTiles[0x20]; // TODO: size ~369e)

// blckthrn
STUB u8 scriptAfterTest[0x3c]; // TODO: size
STUB u8 scriptThreat[0x27];
STUB u8 scriptGuardsEnter[12];
STUB u8 scriptRelease[8];
STUB u8 scriptBlackthornLeaves[10];

STUB u16 fanfareFreq[6];
STUB u16 fanfareParam3[6];
STUB u16 fanfareParam4[6];
STUB s16 fanfareParam5[6]; // ~3750)

STUB u8 planetStartPos[8];
STUB u8 planetRow[8];
STUB u8 orbitSteps[0xb0]; // ~3810)

STUB char* lookPictures[9]; // ~3822)
STUB u8 shoreLandMask[16]; // ~3832)

STUB u8 edgeBits[8]; // ~383a; [7] + gap 1?

STUB u8 roomsNeverCleared[6];
STUB u8 neverClearedCount;
//STUB u8 D_3841;

STUB u8 objectValueRange[0xd]; // ~384f)? TODO: size, offset
STUB u8 objectValueBase[0xd]; // ~385d)? TODO: size, offset
                     // D_383f, D_384d

STUB u8 randomMonsterTypes[8]; // ~3866)
STUB u8 dungeonChunks[8]; // ~386e)
STUB u8 shrineChunks[8]; // ~3876)
STUB u8 chunkFileIndex[0x100]; // ~3976) TODO: size?

STUB u8 D_3a06_shardX[4];
STUB u8 D_3a0a_shardY[4];
STUB u8 shardValue[3];

STUB u16 apparitionFreq[6];
STUB u8 reagentPrices[5][8];
STUB u8 reagentLots[5][8];

STUB u16 itemBasePrices[0x30]; // TODO: int (FMT)

STUB u8 armsStock[9][8]; // ~3b2a)

STUB u16 greetTexts[8][4]; // int (FMT: 4byte)
STUB u16 farewellTexts[8][4]; // ~3baa)
STUB u16 farewellTexts1[8][4]; // ~3bea)

STUB u16 provisionPrices[4][4];
STUB u16 provisionTexts[3];
STUB u16 reagentTexts[8];    // string offset table. type: int (FMT: 4bytes)

STUB char* D_3c20_reagentNames[8];

STUB u16 horsePrices[4];
STUB s16 stableDx[4]; // 3c38..3c40)
STUB s16 stableDy[4]; // 3c40..3c48)
STUB s16 armsTexts[0x2f]; // 3c40..3ca6)? shop item description TODO: size

STUB char* buyOffers[4];
STUB char* cantPayInsults[4];
STUB char* showPrompts[4];

STUB u16 buybackTexts[8]; // string offset table. type: int (FMT: 4bytes)

STUB char* shopItemNames[0x30];

STUB char* sellPrompts[4];

STUB char* farewells[4];
STUB char* sellMorePrompts[4];
STUB char* buyOrSellGreetings[2];
STUB char* goodReplies[4];
STUB char* stockIntros[4];

STUB char noNeedOfArt[]; // "\n\n\"Thou hast no need of this art!\"\nsays $."

STUB u8 healPrices[8];
STUB u8 curePrices[8];
STUB u16 raisePrices[8];

STUB Pair D_3da6_textLeft[6];
STUB Pair2 D_3db2_textRight[6];
STUB u16 textOffset[6];

STUB u8 D_3dd6_regionTop[6];
STUB u8 D_3ddc_regionBottom[6];
STUB u8 D_3de2_startX[6];
STUB u8 D_3de8_startY[6];
STUB u8 D_3dee_imageIndex[6]; // endgame_0000 image idx
STUB u8 D_3df4_imageFile[6];
STUB u8 imageX[6]; // endgame_0000 image x
STUB u8 imageY[6]; // endgame_0000 image y
STUB u8 hasBanner[6];

STUB char* numberNames[19];
STUB char* tensNames[8];
STUB char* ordinalNames[12];

STUB u8 partyDestX[6];
STUB u8 partyDestY[6];

STUB u8 harvestX[4];
STUB u8 harvestY[4];
STUB u8 D_3e6e_harvestReagent[4];
STUB char* harvestNames[3];

STUB u8 hiddenTile[0x72]; // TODO: size
STUB u8 hiddenValue[0x72]; // hidden item - value TODO: size
STUB u8 hiddenMap[0x72]; // hidden item - map id TODO: size
STUB u8 hiddenLevel[0x72]; // hidden item - map z TODO: size
STUB u8 hiddenX[0x72]; // hidden item - map x TODO: size
STUB u8 hiddenY[0x72]; // hidden item - map y TODO: size

// 4100
STUB u8 treasureKind[8];
STUB u8 treasureMinLevel[8];
STUB u8 treasureMaxAmount[8];

STUB u8 gearKind[0x30];
STUB u8 gearMinLevel[0x30]; // ~419c)

STUB char* potionColors[8];
STUB char* scrollRunes[8];
STUB u8 dungeonLootOdds[8];
STUB u8 dungeonLootMax[8];
STUB u8 dungeonLootKind[8];

STUB char* shadowlordNames[3];

STUB char* vileWords[8];
STUB u8 wordTile[8];

STUB u8 fieldMissileKinds[4];
STUB u8 dungeonFieldCells[4];

STUB u8 D_4882_shardX[4]; // map x
STUB u8 D_4886_shardY[4]; // map y
STUB u8 shardMap[4]; // map id
STUB u8 shardLevel[4]; // map level
STUB u8 shardKilledBit[3];

STUB char* talkFileNames[4]; // the .TLK file of each group of eight maps
STUB char* standardWords[0x22+1]; // NAME, JOB, WORK, BYE, THANK, then words that get a rebuke
// 4aec: "ELECTRONIC ARTS"

// talk related
STUB u8 talkPendingCode; // the script code whose argument bytes are being read; 0 none
STUB u8 talkCodeArgCount; // how many of them are in talkCodeArgs
STUB u8 talkCharMask; // 0xff; without the high bit while runes are on (script code 0x8e)
STUB u8 talkWordLength; // the length of the word in talkWord
STUB u8 talkInLabel; // 0xff while a label's question waits for its answer
STUB u8 talkTextX; // the text column after the last character printed
STUB u8 talkLastChar; // the last character processed
STUB u8 talkAfterWord; // 1 after a word of the word table: a space comes before the next character

STUB u16 castFreq[9];
STUB u16 castParam4Up[9]; // cast sound freq table 1
STUB u16 castParam4Down[9]; // cast sound freq table 2
STUB u16 castStep[9]; // cast sound pulse table

STUB char* virtueKeywords[8]; // virtue keywords

STUB u16 questTextPos[8]; // int
STUB u16 codexTextPos[8];

STUB u8 strengthGain[8];
STUB u8 dexterityGain[8];
STUB u8 intelligenceGain[8];

STUB u8 urnFirst[8];
STUB u8 urnX[0x24]; // TODO: size
STUB u8 urnY[0x24]; // TODO: size

// some sound table
STUB u16 shrineFreq[7];
STUB u16 shrineParam3[7];
STUB u16 shrineParam4[7];
STUB u16 shrineParam5[7];

STUB char foodKeys[5];
STUB char drinkKeys[5];
STUB char rationKeys[5];
STUB char gossipKeys[5];

STUB u16 mealPrices[9]; // TODO: size
STUB u16 winePrices[6]; // TODO: size
STUB u16 rationPrices[9]; // TODO: size
STUB u16 rationTexts[7]; // TODO: size

STUB char* pubTopicWords[26];
STUB char* pubPersons[26];
STUB u8  gossipPlaceIdx[26];
STUB char* pubPlaces[13];

STUB u16 gossipPrices[0x1a]; // TODO: size
STUB u16 gossipTexts[4];
STUB u8  pubKindByShop[0xa]; // TODO: size
STUB u16 pubOrderTexts[4];
STUB u16 pubMoreTexts[4];
STUB u16 frigatePrices[4];
STUB u16 skiffPrices[4];
STUB u8  shipX[4];
STUB u8  shipY[4];
STUB u8  innRates[6]; // the inn's rate (per person and night; times 10 per month)

STUB u8 innRooms[6]; // how many companions the inn can keep

STUB u16 innOfferTexts[6]; // the offer, an offset in SHOPPE.DAT
STUB u8 innBedX[6]; // where the party wakes up
STUB u8 innBedY[6];

STUB u8 glyphWidths[0x5b]; // glyph width table, from ' '

#define hyphenWidth glyphWidths[0xd]

STUB u16 textLeftX[2];
STUB u16 textRightX[2];
STUB u16 D_5150_regionTop; // TODO: int?
STUB u16 D_5152_regionBottom; // TODO: int?
STUB u16 spaceWidth;
STUB u16 D_5156_penX;
STUB u16 D_5158_penY; // TODO: int?
STUB u16 toneStep; // = 0

STUB char* sceneCaptions[4];

// 5200

STUB u8 intBonus[8];
STUB u8 dexBonus[8];
STUB u8 strBonus[8];
STUB u16 questionOffset[8][8];
STUB u8 cardX[8];
STUB u8 cardY[8];
STUB u16 savedVideoPage; // AFONT.ASM's: where SaveVideoPage saves the video page

// main parameters
// int D_52a2;     // __argc
// char** D_52a4;  // __argv
// char** D_52a6;  // environ

STUB VideoDriverParams vdp;
// A0878.ASM's own names for two fields: the C modules read them as plain globals, which MSC
// optimizes differently from a struct field (DrawShore)
STUB int D_52c4_clipEnabled;
STUB int D_52c8_videoDriver;

STUB int D_52da_penColor;

STUB int forceEga; // force ega
STUB int forceTandy; // force tandy
STUB int forceHerc; // force herc

STUB int clipTempX; // temp area?
STUB int clipTempY; // temp area?

// ?
STUB int D_52fa_clipX1; // x1
STUB int D_52fc_clipY1; // y1
STUB int D_52fe_clipX2; // x2
STUB int D_5300_clipY2; // y2

// 5300
STUB int prevVideoMode; // previous video mode

STUB int logoFrameWidth[7]; // origin logo frames: widths (signed: centred with sar)
STUB int logoFrameHeight[7]; // and heights

// 5322..5327: "Tandy"
// 5328..532f: video driver string table
// 5330..5337: "CGA.DRV"
// 5338..533f: "EGA.DRV"
// 5340..5347: "T1K.DRV"
// 5348..534f: "HER.DRV"
STUB void* far driverEntry; // far pointer into the video driver; the offset is the function number
STUB u16 pageSegment; // segment of video page buffer (ega: 0xa800, t1k: allocated memory)
STUB u16 timerCalibration; // timer calibration
STUB u16 memorySize; // memory size
STUB u16 D_535a_timerValue; // current timer value
STUB u16 dosError; // int21 error code

// 535e..537d
STUB TextWindow textWindows[4];

STUB u16 textWindowIndex; // charset_id (offset in 539a, data in 53a4..53ab)
STUB u16 charsetIndex; // selected charset idx
STUB u16 numpadKeys; // bool: special keystroke
STUB u16 keyBufferOff; // Ctrl-B: "Buffer Off/On"
STUB u16 cursorAdvance; // cursor advance flag
STUB u16 cursorFirstChar; // keystroke cursor begin idx
STUB u16 cursorFrameCount;
STUB void (far *diskSwapFn)(void); // disk swap message function pointer
//STUB u16 D_5398; // segment of current charset
STUB u8* far currentCharset;        // ptr of current charset
STUB TextWindow* currentTextWindow; // address of current text window data
//STUB u16 charsetTable[4]; // charset segment table
STUB u8* far charsetTable[4];    // charset table
STUB u16 underline; // underline flag (bit 0 of byte 7)
STUB u16 centerText; // ? flag (bit 1 of byte 7)
STUB u16 inverse; // inverse flag (bit 1 of byte 7)
STUB u8  textFgColor; // text fg color (lo nibble of byte 6)
STUB u8  textBgColor; // text bg color (hi nibble of byte 6)

STUB u16 fileSizeLo; // loaded file size (lsb)
STUB u16 fileSizeHi; // loaded file size (msb)

STUB u16 fileHandle; // file handle
STUB u16 firstSegment; // first allocated segment
STUB u16 returnSegment; // first allocated segment (for return)
STUB u16 lzwSegment; // second allocated segment (for lzw)

// u16 53c4: read pointer
// u16 53cc: 0x102
// u16 53d0: 0x200
// u16 53d2: read advance amount? (9)

// 53d6..53e8: "Out of Memory...\n$"

STUB u8 glyphBuffer[24]; // ~5402) character data from charset file (herc=24, others=8)
STUB s16 D_5402_powersOfTen[5]; // 5402..540c) 1, 10, 100, 1000, 10000
STUB u16 cursorFrame; // cursor anim idx // TODO: int
STUB u8 arrowKeyCodes[8]; // extended ascii code for arrow keys
STUB u8 specialKeyTable[8]; // special key table
STUB u8 driveNumber; // drive number
STUB u16 randomSeed; // random seed?
STUB u16 delayInner;
STUB u16 delayOuter;
STUB u16 delayShift[2]; // timer-related?

STUB u16 D_5448_timerValue; // current timer value
STUB u16 ticksToWait; // timer tick count to wait
// far D_544c; // far addr of original int 1c handler

// The disk last selected. A plain char in the DOS build: with it, 1988's ULTIMA_2322 compares
// it with the disk wanted as "mov al,bl; cmp lastDisk,al" (u8: "mov al,lastDisk; cmp bl,al").
// The other modules only store 0xff into it, the same code for both types.
STUB char lastDisk;

// ...54d2: strings

// 5500
STUB u8 blockedTileBits[32]; // normal walkable tile flag table
STUB u8 moveType[64]; // actor tile -> move type table
STUB u8 skiffWaterBits[16]; // skiff walkable flag table
STUB u8 skiffTileBits[4];

STUB u8 trapKinds[8]; // ~55a6)

//
// Savegame / User state
// 

// [55a6..6606): savegame (0x1060)
STUB u16 saveStart; // +0x0: two bytes of unknown use; the block is read and written from here

STUB S_55a8 party[0x10]; // ~57a8)

// 5700
STUB s16 food;        // +0x0202 Food // int (ReceiveItem, MOVSX)
STUB s16 gold;        // +0x0204 Gold // int (TrollToll, MOVSX)
STUB u8  keys; // +0x206
STUB u8  gems; // +0x207
STUB u8  torches; // +0x208
STUB u8  grapple; // +0x209
STUB u8  carpets; // +0x20A magic carpets
STUB u8  skullKeys; // +0x20B
STUB u8  skullKeysDay; // +0x20C the day Minoc's skull keys were last taken
STUB u8  amulet; // +0x20D Lord British's
STUB u8  crown; // +0x20E
STUB u8  sceptre; // +0x20F
STUB u8  shards[4]; // +0x210
STUB u8  spyglasses; // +0x214
STUB u8  hmsCapePlans; // +0x215
STUB u8  sextants; // +0x216
STUB u8  pocketWatch; // +0x217
STUB u8  blackBadge; // +0x218
STUB u8  sandalwoodBox; // +0x219
// FMT pad 2byte (?)
STUB u8  equipment[0x30]; // +0x21A how many of each piece of equipment the party owns
#define D_57db_arrows equipment[0x1b] /* +0x0235 Arrows */
#define quarrels equipment[0x1d] /* +0x0237 Quarrels */
STUB u8  mixedSpells[0x30]; // +0x24A

// 5800
STUB u8  scrolls[8]; // +0x27A
STUB u8  potions[8]; // +0x282
STUB u8  moonstoneX[8]; // +0x28A
STUB u8  moonstoneY[8]; // +0x292
STUB u8  moonstoneMap[8]; // +0x29A the map it is buried in (0 Britannia); 0xff: carried
STUB u8  moonstoneZ[8]; // +0x2A2
STUB u8  D_5850_reagents[8]; // +0x2AA
STUB u8  harvestDay[3]; // +0x2B2
STUB u8  partyCount;        // +0x02B5 party member count
STUB u8  takenItemFlags[0x18]; // +0x2B6 the Codex's "non-regenerating object flags" (SJOG sets a bit per find)
STUB u16 year; // +0x2CE
STUB s16 tempX; // +0x2D0 scratch: a direction chosen, a position, the actor found
STUB s16 tempY; // +0x2D2
STUB u8  statusIcon; // +0x2D4 the letter between the party and the food line: 'T', 'Q', 'P', 'N'...
STUB u8  activePlayer;        // +0x02D5 Active character
STUB u8  partyTile; // +0x02D6 party tile
STUB u8  D_587d_month; // +0x2D7
STUB u8  D_587e_day; // +0x2D8
STUB u8  D_587f_hour;        // +0x02D9 Current hour
STUB u8  lastHour; // +0x2DA
STUB u8  minute; // +0x2DB
STUB u8  combatTick;
STUB u8  sailCounter;        // ?
STUB u8  clockChimes; // +0x2DE strokes the clock has still to chime
STUB u8  trammelPhase; // +0x2DF
STUB u8  feluccaPhase; // +0x2E0
STUB u8  moongateHeight; // +0x2E1
STUB u8  karma;        // +0x02E2 Karma
STUB u8  D_5889_savedUnused;
STUB u8  D_588a_savedUnused;
STUB u8  turnCount; // +0x2E5
STUB u8  campHealWait; // +0x2E6 hours until camping heals again (the Codex)
STUB u8  apparitionMonth;
STUB u8  protectTurns; // +0x2E8
STUB u8  castCircle;
STUB u8  D_5890_spellAttack;
STUB u8  animate; // +0x2EB 0, 1 or 0xff
STUB u8  windDir; // +0x2EC
STUB u8  mapId; // +0x2ED
STUB u8  D_5894_savedMapId; // +0x2EE the map while mapId is a combat map
STUB u8  D_5895_mapLevel; // +0x2EF
STUB u8  D_5896_mapX; // +0x2F0
STUB u8  D_5897_mapY; // +0x2F1
STUB u8  crosshairOn; // +0x2F2
STUB u8  crosshairX; // +0x2F3
STUB u8  crosshairY; // +0x2F4
STUB u8  mapOriginX; // +0x2F5 the map buffer's upper left corner in the world
STUB u8  mapOriginY; // +0x2F6
STUB u8  attackWeapon; // +0x2F7
STUB u8  curCombatant; // +0x2F8 the entry of combatants whose turn it is; 0xff none
STUB u8  focusBlink; // +0x2F9 toggled each frame: the frame around the combatant
STUB u8  combatExitDir; // +0x2FA
STUB u8  combatFlags; // +0x2FB
STUB u8  combatEvents;
STUB u8  combatOver; // +0x2FD the Codex's "no more enemies"
STUB u8  viewOn; // +0x2FE the map view is updated and animated
STUB u8  D_58a5_lightLevel; // +0x2FF
STUB u8  lightSpellTurns; // +0x300
STUB u8  torchTurns; // +0x301
STUB u8  lastAttacker[0x20];  // [58a8..58c8)
STUB u8  shadowlordTowns[3]; // +0x322
STUB u8  summonedLord;
STUB u16 questsOrdained; // +0x326
STUB u16 questsDone; // +0x328
STUB u8  dungeonOpen[8]; // +0x32A 0 sealed, 0x80 open
STUB u8  shrineDestroyed[8]; // +0x332 bit 7
STUB u8  roomsCleared[0xe]; // +0x33A
STUB u8  annotX[0x20]; // +0x348 the Codex's "annotations" (TOWN lists the tiles 0x48/0x49 of the map here)
// 5900
STUB u8  annotY[0x20]; // +0x368
STUB u8  annotTile[0x20]; // +0x388
STUB u8  annotCount; // +0x3A8
STUB u8  openDoorTile; // +0x3A9
STUB u8  openDoorX; // +0x3AA
STUB u8  openDoorY; // +0x3AB
STUB u8  openDoorTurns; // +0x3AC
STUB u8  boughtShipX; // +0x3AD
STUB u8  boughtShipY; // +0x3AE
STUB u8  sailDir; // +0x3AF
STUB u8  newPrompt; // +0x3B0
STUB u8  drunkTurns; // +0x3B1
STUB u8  townShadowlord; // which of the three shadowlords (shadowlordTowns) is in this town, 0xff none (SetupShadowlord)
STUB u8  fewerEnemies;
STUB u8 dungeonMap[0x200]; // +0x3B4
// 5b00
STUB u8 npcKilled[0x80]; // +0x5B4
STUB u8 npcMet[0x80]; // +0x634

STUB ActorFmt actors[32]; // +0x6B4

// 5d00
STUB u16 D_5d5a_unused; // unused?
STUB u16 D_5d5c_unused; // unused?

STUB NpcScheduleFmt npcSchedules[32]; // +0x7B8

// 5f00
STUB NpcFmt npcs[32]; // +0x9B8

// 6100
STUB MovementFmt moveLists[0x20]; // +0xBB8
//STUB u8 moveLists[0x400];

STUB s16 moveListPos[0x20]; // +0xFB8
STUB u8 npcTypes[0x20]; // +0xFF8
STUB u8 npcApproach;
STUB u8 npcApproaching;
STUB u16 D_65c0_unused; // unused?
STUB s16 npcStuckCount[0x20];

// 6600
STUB u8 dungeonMoveDir;
STUB u8 dungeonFacing; // +0x105D 0 north, 1 east, 2 south, 3 west
STUB u8 dungeonStyle; // +0x105E 1 cave, 2 mine, 3 dungeon
STUB u8 boughtShip; // +0x105F 0x40 a skiff; 0x80 + n a frigate with n skiffs
STUB u16 saveEnd; // +0x1060: the end of the block

STUB S_6608_Map D_6608_map; // ~6a08) map data: layer4 * y16 * x16 / y32 * x32
#define D_6608 D_6608_map.raw
#define dummyCell (D_6608[1023])

// in the original, DS:6708 is a place inside D_6608_map, not a variable of its own
#define D_6708_map (*(S_6608_Map*)(D_6608_map.raw + 0x100))
#define D_6708 D_6708_map.raw

// The original reaches the overworld map through a name of its own for DS:6608: in
// ULTIMA_4402 MSC merges the town and overworld paths only up to the store, which it
// does when their final `add ax,OFFSET ...` name different symbols. Which object defined
// it is not known; the link defines it as D_6608_map's place (tools/u5.py, LINK_DEFINES).
STUB S_6608_Map D_6608_overworld;

STUB u8 songPos; // some audio related

STUB u16 D_6a0a_powersOfTen[5];

STUB u8 viewTileFlags[0x20];

STUB u8 ambientTick;
STUB u16 banjoFreq[9]; // banjo freq table

STUB u8 banjoSong[0x36]; // iolo's banjo song note table

STUB u8 tileAnimTick;

STUB u8 lightSlope[6]; // light intensity slope?
STUB u8 opaqueTiles[0x13]; // ~6a99)

STUB u8 lightTiles[10];

STUB u16 D_6aa4_unused;
STUB u8 viewDist2[0x24];

// a300


//////// (static) data: ..a522]

// a500

// DS:a524-a9bc is the modules' _BSS, their static variables, in link order: capeToggle (mainout.c),
// lastX and lastY (mainoutb.c), viewQueueX and viewQueueY (dnglook.c), D_a728_pathXs and D_a872_pathYs (comsubs.c).

// a900

// From DS:a9bc (where the runtime's empty XOB/XO/XOE segments are) to _end: communal
// (uninitialised) variables. These, used by several modules, are defined (without extern)
// at the top of ultima.c; the others by the one overlay module that uses them, at its top
// (intro0.c, intro2.c, endgame.c, talk.c, cast2.c, shoppes2.c, shoppe2b.c, font.c). PLINK86
// allocates them in COMDEF record order, object by object in link order (ULTIMA first), and
// MSC writes an object's records in its symbol table's order (research/RULES.md, section 2):
// declared consecutively, each group comes out in declaration order, which is address order.
// The five no code refers to (D_bbd2, D_bcb0 in intro2.c, D_bd14 in cast2.c, D_bd3a, D_bd3b in
// font.c) are declared so that the others land where they are.

STUB u8  townJustEntered;
STUB u8 curDisk[1]; // the disk wanted (SwitchDisks)
STUB u8 promptMode; // two bytes on an odd address would have been put on an even one
STUB u8* wallImage; // loaded image pointer
STUB u8 floppyCount;
STUB u8* itemsImage; // loaded image pointer
STUB u8* monsterImage; // loaded image pointer

// disk drive table
STUB u8 diskDrives[6]; // the drive of each disk; 0xff: not set

#define D_a9c9 diskDrives[1]
#define D_a9ca diskDrives[2]
#define D_a9cb diskDrives[3]
#define D_a9cc diskDrives[4]
#define D_a9cd diskDrives[5]

// used in intro
STUB u8 soundOn; // use sound effect?

STUB u16 rayBalance[0x15];

STUB u8 vitalsDirty; // dirty flag for vital
STUB u8 stepVolume; // footstep sfx duration in dungeon

// combat?
STUB ActorFmt savedActors[0x20]; // actors during combat

STUB char* shopName; // current shop name
STUB char* shopOwner; // current shop owner

STUB char* itemName;
STUB u8 viewTiles[0x160]; // the 11x11 view, 0x20 bytes a row: 0 an actor (in viewActors), 0xff hidden
STUB char* placeName;

STUB u8 viewActors[0xb0]; // actor map / misc map [0x10 * b] (from 51b8)

STUB u8 combatMap[32 * 32]; // the combat map; outside combat the 32x32 light map

// b100

STUB s16 shopIndex; // s16 (FMT MOVSX); the shop's column in shopNames..., 0-15
STUB s16 shopKind; // s16 (FMT MOVSX); 0-7, the merchant's dialog number - 0x81
STUB s16 shopPrice; // s16 (FMT MOVSX); the % of a shop text
STUB u16 shopQuantity;

STUB u8* searchGrid;
STUB u8 tileFrames[0x100]; // the tile drawn for each map tile (animation)

#define SCRATCH_SIZE 2000

STUB u8 scratch[SCRATCH_SIZE]; // scratch buffer ~b9ee)

#define D_b31e (scratch + 0x100)
#define shopTextOut (scratch + 1500)

// b900

STUB u8  itemCounts[0x26]; // owned item counts
// [8]
#define potionCounts (itemCounts + 8)
#define carpetCount itemCounts[0x10]
#define skullKeyCount itemCounts[0x11]
#define amuletOwned itemCounts[0x12]
#define crownOwned itemCounts[0x13]
#define sceptreOwned itemCounts[0x14]
// [8]
#define moonstoneFlags (itemCounts + 0x15)
// [3]
#define shardFlags (itemCounts + 0x1d)
#define spyglassCount itemCounts[0x20]
#define capePlansOwned itemCounts[0x21]
#define sextantCount itemCounts[0x22]
#define pocketWatchOwned itemCounts[0x23]
#define blackBadgeOwned itemCounts[0x24]
#define sandalwoodBoxOwned itemCounts[0x25]

STUB CombatEntity combatants[32]; // combat entities

STUB u8  wallTile; // pos?
STUB u8  D_bb15_floorTile; // pos?
STUB u8  altTileset;
STUB u8  loadFlags;











// ..bd3f]

#endif
