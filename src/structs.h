#ifndef _STRUCTS_H
#define _STRUCTS_H

#pragma pack(push, 1)

// S_13bc
// size: 8
// monster/foe stats
// ref: Prima's Official Guide
typedef struct EnemyStats
{
	u8 str;			// 0 / 13bc / 3f050
	u8 dex;			// 1 / 13bd
	u8 intel;		// 2 / 13be / 3f052
	u8 def;			// 3 / 13bf; def / arm
	u8 atk;			// 4 / 13c0; atk / dmg
	u8 maxHp;		// 5 / 13c1 / 3f055
	u8 maxEncounterCount;	// 6 / 13c2; n
	u8 treas;		// 7 / 13c3
} EnemyStats;

// size: 32; 55a8..55c7 * n
// "Party"
typedef struct S_55a8
{
	char name[9];	// 0 / 55a8; name
	u8 gender;		// 9 / 55b1; gender (0b: male, 0c: female)
	u8 cls;			// a / 55b2; class ('A', 'F', 'B', 'M', ...)
	u8 status;		// b / 55b3; status ('G': good, 'D': dead, ...)
	u8 str;			// c / 55b4; strength
	u8 dex;			// d / 55b5; dexerity
	u8 intel;		// e / 55b6; intelligence
	u8 mag;			// f / 55b7; magic
	s16 hp;			// 10 / 55b8; hp
	s16 maxHp;		// 12 / 55ba; max_hp (HM)
	s16 exp;		// 14 / 55bc; exp
	u8 level;		// 16 / 55be; level
	u8 monthsAtInn;	// 17 / 55bf; counted up each month, to 25 (ULTIMA_4f7c_AdvanceTime); the inn bills for it (SHOPPES3)
	u8 defense;			// 18 / 55c0; def? (read once, in COMBAT)
	u8 equips[6];	// 19 / 55c1 ; equips ([0]: helmet, [1]: armour, [2], [3]: weapon and shield (the hands), [4]: magic_ring, [5]: amulet); 0xff none
	u8 innMapId;		// 1f / 55c7; 0: in the party; 0x7f: gone (probably); else the map of the inn where the member waits
} S_55a8;

// x/y pair (u8) "coord"
typedef struct Pair
{
	u8 x;
	u8 y;
} Pair;

// x/y pair (s8) "coord"
typedef struct PairS8
{
	s8 x;
	s8 y;
} PairS8;

// x/y pair (u16) "coord"
typedef struct Pair2
{
	u16 x;
	u16 y;
} Pair2;

// 52ba
typedef struct VideoDriverParams
{
	int forceCga;         // 52ba 0
	int tileXOffset;      // 52bc 2
	int tileYOffset;      // 52be 4
	int tileWidth;                  // 52c0
	int tileHeight;                  // 52c2
	int clipEnabled;                  // 52c4
	int spare52c6;                  // 52c6
	int videoDriver; // 52c8: video driver selection (0=cga, 1=ega, 2=tandy, 3=herc, ffff=none)
	int machineType;      // 52ca: machine type (0=pcjr, 2=ibm, 3=tandy, 6=other)
	int penX;             // 52cc
	int penY;             // 52ce
	int clipX1;     // 52d0
	int clipX2;     // 52d2
	int clipY1;     // 52d4
	int clipY2;     // 52d6
	int page;             // 52d8 1e
} VideoDriverParams;            // size: 0x20?

// 535e size: 8
typedef struct TextWindow
{
	u8 left;   // 0
	u8 top;    // 1
	u8 right;  // 2
	u8 bottom; // 3
	u8 currentX; // 4
	u8 currentY; // 5
	u8 colors;  // 6; hi: bg, lo: fg
	u8 flags; // 7; flags: bits 0..2
} TextWindow;

// 5c5a: Actor. size: 8 "Obj": the 32 objects and monsters on the map; [0] is the party.
// (the Codex's "monster/object table", SAVED.GAM +0x6B4)
typedef struct ActorFmt // D_5c5a_actors..
{
	u8 tile; // base tile (shape)
	u8 animTile; // animated tile
	u8 x;
	u8 y;
	u8 z; // level
	u8 value; // by kind: hull of a frigate, hit points of a monster in combat, item or count of an object
	u8 animState; // low nibble: frames to wait (0xf: never animate); high nibble: place in the tile sequence (AnimateActors)
	u8 extra; // by kind: skiffs aboard a frigate; 0xff none
} ActorFmt;

// 5f5e: Character (npc) / monster. size: 16
typedef struct NpcFmt
{
	u16 mode;   // 0 / 5f5e; what the NPC is doing, 0-7 (0: not there; NPC.OVL sets and tests it)
	s16 x; // 2 / 5f60 x
	s16 y; // 4 / 5f62 y
	u16 z; // 6 / 5f64 z
	u16 npcType;   // 8 / 5f66
	u16 dialogNumber;   // a / 5f68
	u16 actorIdx;   // c / 5f6a; the NPC's slot in D_5c5a_actors   // TODO: s16?
	u16 scheduleIdx;   // e / 5f6c; which entry of the NPC's schedule is in force
} NpcFmt;

// size: 16
typedef struct NpcScheduleFmt
{
	u8 type[3]; // 0 / type
	u8 x[3];    // 3 / x
	u8 y[3];    // 6 / y
	u8 z[3];    // 9 / z (s8?)
	u8 time[4]; // c / time
} NpcScheduleFmt;

// size: 32
typedef struct MovementFmt
{
	u8 data[0x20];
} MovementFmt;

// size: 1024 (0x400)
typedef union S_6608_Map
{
	u8 overworld[4][16][16]; // overworld
	u8 town[32][32]; // town / castle / ...
	u8 raw[1024];
} S_6608_Map;

// size: 8
// combat entity (ba14)
typedef struct CombatEntity
{
	u8 hp;			// 0 / ba14; current hp
	u8 dex;			// 1 / ba15; dex
	u8 flags;		// 2 / ba16; flags (0x80: player, 0x40: monster, ...)
	u8 entityIdx;	// 3 / ba17; entity idx (player, monster, ...) (->13bc, 153c, 15cc, 1856, 55a8, )
	u8 actorIdx;	// 4 / ba18; actor idx (->5c5a)
	u8 turnTimer;	// 5 / ba19; turn timer?
	u8 x;			// 6 / ba1a; x coord
	u8 y;			// 7 / ba1b; y coord
} CombatEntity;

// size: 182
// U4 party related (party.sav:0x140)
typedef struct S_bb1c
{
	u8 fill0[6]; // bb1c; fill

	u16 honesty; // bb22 (+6)
	u16 compassion; // bb24 (+8)
	u16 valor; // bb26 (+a)
	u16 justice; // bb28 (+c)
	u16 sacrifice; // bb2a (+e)
	u16 honor; // bb2c (+10)
	u16 spirituality; // bb2e (+12)
	u16 humility; // bb30 (+14)

	u8 fill16[0xa0]; // fill
} S_bb1c;

// size: 40
// u4 character stat (party.sav:0x8)
typedef struct S_bc88_U4Party
{
	u16 hp;		// bc88; current hp
	u16 maxHp;	// bc8a; max hp
	u16 exp;	// bc8c; exp
	u16 str;	// bc8e; str
	u16 dex;	// bc90; dex
	u16 intel;	// bc92; int
	u16 mag;	// bc94; mag
	u16 unused0e;	// bc96; not accessed
	u16 weapon;	// bc98; weapon; not accessed
	u16 armor;	// bc9a; armor; not accessed
	u8 name[16]; // bc9c; name
	u8 gender;	// bcac; gender
	u8 cls;		// bcad; class
	u8 unused26;	// bcae; not accessed
	u8 unused27;	// bcaf
} S_bc88_U4Party;

#pragma pack(pop)

#endif
