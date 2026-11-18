#include "common/common.h"
#include "audio/aud_mus.h"
#include "audio/aud_sfx.h"
#include "funcs.h"
#include "macros.h"
#include "vars.h"
#include "tiles.h"

#include "outsubs.h"
#include "town.h"

void MAINOUT_0000_SetUpOverworld(void);
u8 D_6a34_ambientTick = 0;

u16 D_6a36_banjoFreq[9] =
{
    0x0da9, 0x0f56, 0x1136, 0x123c, 0x1478, 0x16fa, 0x1857, 0x19ca, 0x1b53
};

u8 D_6a48_banjoSong[0x36] = // 53 used
{
    0x01, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x05,
    0x00, 0x00, 0x04, 0x09, 0x06, 0x09, 0x07, 0x04,
    0x06, 0x05, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01,
    0x05, 0x00, 0x00, 0x00, 0x04, 0x09, 0x06, 0x05,
    0x06, 0x04, 0x00, 0x00, 0x00, 0x05, 0x08, 0x08,
    0x09, 0x05, 0x06, 0x08, 0x09, 0x05, 0x04, 0x06,
    0x05, 0x04, 0x03, 0x02, 0x01
};

u8 D_6a7e_tileAnimTick = 0;

u8 D_6a80_lightSlope[6] = { 0x02, 0x05, 0x0A, 0x14, 0x22, 0x31 };

// Each living member who wears a ring of regeneration (0x2c) gains a hit point one time in eight.
void ULTIMA_400c_RingRegeneration(void)
{
    int member;
    for (member = 0; member < D_585b_partyCount; member++)
	{
		if (D_55a8_party[member].status != STATUS_DEAD && D_55a8_party[member].equips[4] == 0x2c)
		{
			if (ULTIMA_2092_RandomRange(0, 7) == 7)
			{
				ULTIMA_3f14_IncreaseInt(&D_55a8_party[member].hp, 1, D_55a8_party[member].maxHp);
				D_a9fa_vitalsDirty = 1;
			}
		}
	}
}

// select active player
// The keys 0-6: set the active player (0 none).
int ULTIMA_4080_SetActivePlayer(int key)
{
    int index;
	int result;
    
    result = 1;
    index = key - 0x31;

    ULTIMA_1850_PrintString(/*0xa396*/ "Set Active Plr:\n");
    if (key == 0x30)
    {
        ULTIMA_1850_PrintString(/*0xa3a8*/ "None!\n");
        D_587b_activePlayer = 0xff;

		ULTIMA_2900_UpdateVitalsDisplay();
		result = 0;
    }
    else
    {
        if (D_585b_partyCount > index && D_55a8_party[index].status != STATUS_DEAD && D_55a8_party[index].status != STATUS_SLEEP)
        {
            D_587b_activePlayer = index;

            ULTIMA_1850_PrintString(D_55a8_party[index].name);
            ULTIMA_16ba_PrintChar('\n');

            ULTIMA_2900_UpdateVitalsDisplay();
            result = 0;
        }
		else
		{
            ULTIMA_1850_PrintString(/*0xa3b0*/ "Invalid!\n");
        }
    }

    return result;
}

// CHECKED
void ULTIMA_4102_AudioPlayAmbientSfx(void)
{
    int viewY_j;
	int tile;
	int viewX_h;
	int mapY_d;
	int soundKind_k;
	int mapX_e;
	int centerY;
	int centerX;
	int dist2;
	int nearest2;

	soundKind_k = 0;
	nearest2 = 0x33;

	if (D_5893_mapId < 0x80)
	{
		centerX = (uint)D_5896_mapX;
		centerY = (uint)D_5897_mapY;
	}
	else
	{
		centerX = centerY = 5;
	}

    for (mapX_e = centerX - 5, viewX_h = 0; mapX_e < centerX + 6; mapX_e++, viewX_h++)
	{
        for (mapY_d = centerY - 5, viewY_j = 0; mapY_d < centerY + 6; mapY_d++, viewY_j++)
		{
			dist2 = (mapY_d - centerY) * (mapY_d - centerY) + (mapX_e - centerX) * (mapX_e - centerX);
			// a continue, not an if around the rest: with it MSC keeps viewY_j's increment
			// and the signed loop test, instead of a closed form and a jump on equal
			if (dist2 >= nearest2)
				continue;
			{
				tile = *ULTIMA_4402_GetTileAddr(mapX_e, mapY_d);
				if ((tile & 0xfe) == TILE_MAP_CLOCK)
				{
					soundKind_k = 1;
				}
				else if ((tile & 0xfc) == TILE_MAP_WATERFALL)
				{
					soundKind_k = 2;
				}
				else if ((tile & 0xfc) == TILE_MAP_FOUNTAIN)
				{
					soundKind_k = 3;
				}
                else if (GetMapViewport(viewX_h, viewY_j) == 0 && (GetActorMap(viewX_h, viewY_j) & 0xfc) == TILE_ACTOR_BARD)
				{
					// BGM/NOTE: no jingle when bgm is activated
					soundKind_k = 4;
				}
				else
				{
					dist2 = nearest2;
				}

				nearest2 = dist2;
			}
		}
	}

	switch (soundKind_k)
	{
    case 1:
		// clock
        if (D_5884_clockChimes != 0 && (D_6a34_ambientTick == 0 || D_6a34_ambientTick == 4))
        {
            ULTIMA_2192_AudioPulse(0xc2c, 1, 2000, 20000, -10);
        }
        else
        {
            if (D_6a34_ambientTick == 0)
            {
                ULTIMA_22c0_AudioTone(3000, 3);
            }
            else if (D_6a34_ambientTick == 4)
            {
                ULTIMA_22c0_AudioTone(2000, 3);
            }
        }
        break;

    case 2:
		// waterfall
        ULTIMA_223c_AudioWhiteNoise(0x14, 0x3c, 10000);
        break;

    case 3:
		// fountain
        ULTIMA_223c_AudioWhiteNoise(10, 0x1e, 25000);
        break;

    case 4:
        if (D_6a48_banjoSong[D_6a08_songPos] != 0)
        {
            ULTIMA_2192_AudioPulse(D_6a36_banjoFreq[D_6a48_banjoSong[D_6a08_songPos] - 1], 1, 2000, 20000, -10);
        }

        D_6a08_songPos++;
        if (D_6a08_songPos >= 0x35)
        {
            D_6a08_songPos = 0;
        }
        break;
	}

    if (D_5884_clockChimes != 0 && (D_6a34_ambientTick == 0 || D_6a34_ambientTick == 4))
	{
		D_5884_clockChimes--;
	}

	if (++D_6a34_ambientTick > 7)
	{
		D_6a34_ambientTick = 0;
	}
}

void ULTIMA_433e_AudioFootstep(void)
{
	ULTIMA_223c_AudioWhiteNoise(1, 0x19, 1000);
	ULTIMA_20c8_Delay(1, 0x14);
	ULTIMA_223c_AudioWhiteNoise(1, 0x19, 1500);
}

void ULTIMA_4368_AudioVictoryTune(void)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        ULTIMA_2192_AudioPulse(0x11f8, 1, 0x2a30, 300, 6);
    }

    ULTIMA_2192_AudioPulse(0x17d4, 1, 0x5460, 300, 3);
}

void ULTIMA_43ae_AudioSweepTone(int startFreq, int endFreq, int stepTime, int duration)
{
	int elapsed_l;
	int freqStep;
	int stepCount_h;
	int freq;

	stepCount_h = stepTime;
	freqStep = (endFreq - startFreq) * stepCount_h;
    freqStep /= duration;
	freq = startFreq;

	for (elapsed_l = 0; elapsed_l < duration; elapsed_l += stepTime)
	{
		ULTIMA_22e2_PcspkOn(freq);
		ULTIMA_20c8_Delay(1, stepTime);
		freq += freqStep;
	}

	ULTIMA_230e_PcspkOff();
}

// CHECKED (NOT MATCHING: optimization)
byte* ULTIMA_4402_GetTileAddr(int x, int y)
{
	int quadrant;
	byte* addr_e;

	if (D_5893_mapId > 0x7f) // combat?
	{
		// 440f
        // There are cases where y < 0. original bug

		addr_e = &GetCombatMap(x, y);
	}
	else if (D_5893_mapId == 0) // 4420; overworld
	{
		x = (x - D_589b_mapOriginX) & 0x1f;
		y = (y - D_589c_mapOriginY) & 0x1f;
		quadrant = 0;
		if (y > 0xf)
		{
			quadrant = 2;
			y -= 16;
		}

		if (x > 0xf)
		{
			quadrant++;
			x -= 16;
		}

		// 4467 (NOT MATCHING: optimization)
		addr_e = &GetMapOverworld(x, y, quadrant);
	}
	else if (x < 0 || y < 0 || x > 0x1f || y > 0x1f) // 447e
	{
		// 4496
		addr_e = &D_6a07_dummyCell;
	}
	else
	{
		// 449e
		addr_e = &GetMap(x, y);
	}

	return addr_e;
}

static void ULTIMA_44b8_AnimateTiles(void)
{
	int i;

	for (i = 0xd4; i < 0xd8; i++)
	{
		D_b11e_tileFrames[i]++;
		if (D_b11e_tileFrames[i] == 0xd8)
		{
			D_b11e_tileFrames[i] = 0xd4;
		}
	}

	for (i = 0xd8; i < 0xdc; i++)
	{
		D_b11e_tileFrames[i]++;
		if (D_b11e_tileFrames[i] == 0xdc)
		{
			D_b11e_tileFrames[i] = 0xd8;
		}
	}

	if ((D_6a7e_tileAnimTick & 1) != 0)
	{
        for (i = 0x80; i < 0x84; i++)
		{
			D_b11e_tileFrames[i] ^= 1;
		}

		for (i = 0xec; i < 0xf0; i++)
		{
			D_b11e_tileFrames[i]++;
			if (D_b11e_tileFrames[i] == 0xf0)
			{
				D_b11e_tileFrames[i] = 0xec;
			}
		}

		if ((D_6a7e_tileAnimTick & 2) != 0)
		{
            for (i = 0xfa; i < 0xfe; i++)
			{
				D_b11e_tileFrames[i] ^= 1;
			}
		}
	}

	D_6a7e_tileAnimTick++;
}

// CHECKED
// animate_actors
void ULTIMA_4552_AnimateActors(void)
{
	int step;
	byte* sequence;
	int actorIndex;
	int sequenceIndex;
	int delay;
	bool moved_e;
	ActorFmt* actor_g;
	int op_b;
	int baseTile_g;

	for (actorIndex = 0; actorIndex < 0x20; actorIndex++)
	{
		actor_g = &D_5c5a_actors[actorIndex];
		if (actor_g->tile == 0)
		{
			continue;
		}

        delay = actor_g->animState & 0xf;
        step = actor_g->animState >> 4;
        baseTile_g = actor_g->tile & 0xfc;
        sequenceIndex = D_1b18_animSequences[((baseTile_g - 0x34) >> 2) + 0xb0];
		sequence = &D_1b18_animSequences[sequenceIndex * 0x10];

		if (delay == 0xf)
		{
			continue;
		}

        if (delay != 0)
        {
            delay--;
            actor_g->animState = (actor_g->animState & 0xf0) + delay;
			continue;
        }

		if (actor_g->animTile == 0 || actor_g->animTile == TILE_ACTOR_1D || actor_g->animTile == TILE_ACTOR_SLEEP)
		{
			continue;
		}

		if (baseTile_g < 0x34 || baseTile_g == 0xe8 || baseTile_g == 0xb4)
		{
			continue;
		}

		if (baseTile_g != 0x5c && baseTile_g != 0xa8)
		{
			if (ULTIMA_2092_RandomRange(0, 0xff) < 0x80)
			{
				continue;
			}
		}

        // The gotos are the original's shape: case 0's "goto" stays as a dead jmp after the
        // jump-table dispatch (the table goes straight to L_restart), and cases 1-4 end in
        // case 5's "moved_e = 1; break". Written out, MSC drops case 0's copy and merges
        // case 5's tail into cases 1-4's instead.
        moved_e = 0;
        do
        {
            op_b = sequence[step];
            switch (op_b)
            {
            case 0:
                goto L_restart;

            case 1:
            case 2:
            case 3:
            case 4:
                actor_g->animTile = baseTile_g + op_b - 1;
                step++;
                goto L_done;

            case 5:
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0x40)
                {
                    actor_g->animTile = actor_g->tile;
                    if (baseTile_g == 0x5c)
                    {
                        step++;
                        goto L_done;
                    }

                    delay = 6;
                L_done:
                    moved_e = 1;
                    break;
                }

                step++;
                break;

            case 6:
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0xc0)
                {
                L_restart:
                    step = 0;
                    break;
                }
                step++;
                break;

            case 7:
                step = 2;
                break;

            default:
                delay = op_b - 0x80;
                step++;
                moved_e = 1;
                break;
            }

            actor_g->animState = step * 0x10 + delay;
        } while (!moved_e);
    }

	ULTIMA_44b8_AnimateTiles();
    ULTIMA_6fd6_GRAP_AnimateTiles();
}

// True when moonstone stoneIndex is buried on this map and level (outdoors: inside the 32x32 map buffer).
static int ULTIMA_4702_IsMoonstoneInView(int stoneIndex)
{
	int viewY;
	int viewX;
	int inView;

	inView = 0;

	if (D_5840_moonstoneMap[stoneIndex] == D_5893_mapId && D_5848_moonstoneZ[stoneIndex] == D_5895_mapLevel)
	{
		if (D_5893_mapId != 0)
		{
			inView = 1;
		}
		else
		{
			viewX = (byte)(D_5830_moonstoneX[stoneIndex] - D_589b_mapOriginX);
			viewY = (byte)(D_5838_moonstoneY[stoneIndex] - D_589c_mapOriginY);

			if (viewX < 0x20 && viewY < 0x20)
			{
				inView = 1;
			}
		}
	}

	return inView;
}

// Raise the moongates at night and lower them by day, and put their tile on the map where a moonstone is buried.
void ULTIMA_475a_UpdateMoongates(void)
{
	int gateTile_l;
	int changed;
	int stone;
	byte* cell;

	gateTile_l = 0xdc;
	if (D_587f_hour >= 0x14 || D_587f_hour < 5)
	{
        ULTIMA_3ef0_IncreaseByte(&D_5887_moongateHeight, 1, 0x10);
    }
	else
	{
        ULTIMA_3f36_DecreaseByte(&D_5887_moongateHeight, 1);
        if (D_5887_moongateHeight == 0)
        {
            gateTile_l = 5;
        }
    }

	for (stone = 0; stone < 8; stone++)
	{
		if (ULTIMA_4702_IsMoonstoneInView(stone) != 0)
		{
			cell = ULTIMA_4402_GetTileAddr(D_5830_moonstoneX[stone], D_5838_moonstoneY[stone]);
			changed = *cell != gateTile_l;
			*cell = gateTile_l;
			D_24e6_viewDirty |= 2;
			if (changed != 0)
			{
				ULTIMA_5e4a_BuildLightMap();
			}
		}
	}
}

// Take the party to where moonstone `index` is buried; 0 when it is not buried.
int ULTIMA_47f4_GoToMoonstone(int stoneIndex)
{
	int oldMapId;

	if (D_5840_moonstoneMap[stoneIndex] == 0xff)
	{
		return 0;
	}

    if (D_5893_mapId == 0)
    {
        ULTIMA_251e_SwitchDisks(1);

        while (ULTIMA_1674_TestOpenFile(/*0xa3ba*/ "BRIT.DAT") == 0) {}

        ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), &D_5c5a_actors[0].tile, 0x100);
    }

    oldMapId = D_5893_mapId;
    D_5893_mapId = D_5840_moonstoneMap[stoneIndex];
    D_5896_mapX = D_5830_moonstoneX[stoneIndex];
    D_5897_mapY = D_5838_moonstoneY[stoneIndex];
    D_5895_mapLevel = D_5848_moonstoneZ[stoneIndex];

    if (D_5893_mapId != 0 && oldMapId != 0 && D_5893_mapId < 0x21 && oldMapId < 0x21)
    {
        TOWN_11f0_Entry(1);
    }
    else
    {
        if (D_5893_mapId == 0 && oldMapId == 0)
        {
            ULTIMA_256e_ReadFileFromDisk(OUTSUBS_0368_GetWorldSavefile(), &D_5c5a_actors[0].tile, 0x100, 0);
            MAINOUT_0000_SetUpOverworld();
        }
    }

    return 1;
}

// On a moongate: the party sinks into it and comes out at the moonstone of the moon's phase. Returns 1 instead in the first ten minutes after midnight (for the caller to act on), else 0.
int ULTIMA_48a8_EnterMoongate(void)
{
	int midnight;
	int unused; // unused
	int savedTile;

	midnight = 0;
	if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) == TILE_MAP_MOONGATE)
	{
		ULTIMA_3ae6_WaitFrames(1);
		ULTIMA_2192_AudioPulse(0x170c, 1, 30000, 2000, 2);
		savedTile = D_587c_partyTile;
		D_587c_partyTile = TILE_ACTOR_CIRCLE;
		ULTIMA_1068_RevealTile(0xdc, 5, 5);
		D_587c_partyTile = 0;
		ULTIMA_3ae6_WaitFrames(1);

		for (D_5887_moongateHeight = 0xf; D_5887_moongateHeight != 0; D_5887_moongateHeight--)
		{
			ULTIMA_1112_GRAP_60_Moongate(D_5887_moongateHeight, 5, 5);
			ULTIMA_20fa_WaitTicks(2);
		}

		*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) = TILE_MAP_GRASS;

		D_24e6_viewDirty |= 2;

		ULTIMA_10e0_GRAP_51_PutTile(5, 5, 5);
		if (D_587f_hour == 0 && D_5881_minute < 10)
		{
			midnight = 1;
		}
		else
		{
			ULTIMA_47f4_GoToMoonstone((D_587f_hour < 0xc ? D_5885_trammelPhase : D_5886_feluccaPhase) - 0x30);
		}

		D_587c_partyTile = savedTile;
	}

	return midnight;
}

// The party member who acts: the combatant whose turn it is, else the active player, else the only able one, else ask ("Player: "). -1 none.
int ULTIMA_4988_GetActingPlayer(void)
{
    int player;
    int ableCount;
    int member;
    bool chosen;

    player = -1;
    if (D_5893_mapId > 0x80)
    {
        player = D_ba14_combatants[D_589e_curCombatant].entityIdx;
    }
    else if (D_587b_activePlayer != 0xff)
    {
        player = D_587b_activePlayer;
    }
    else
    {
        ableCount = 0;
        for (member = 0; member != D_585b_partyCount; member++)
        {
            if (D_55a8_party[member].status == STATUS_GOOD || D_55a8_party[member].status == STATUS_POISONED)
            {
				player = member;
                ableCount++;
            }
        }

        if (ableCount > 1)
        {
            chosen = 0;
            while (!chosen)
            {
                ULTIMA_1850_PrintString(/*0xa3c4*/ "Player: ");
                player = ULTIMA_2e8e_SelectPlayer();
                if (player < 0)
                {
                    chosen = 1;
                }
                else
                {
                    if (D_55a8_party[player].status == STATUS_GOOD || D_55a8_party[player].status == STATUS_POISONED)
                    {
                        ULTIMA_1850_PrintString(D_55a8_party[player].name);
                        if (ULTIMA_1f12_GetCurrentTextX() != 0)
                        {
                            ULTIMA_16ba_PrintChar('\n');
                        }

                        chosen = 1;
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(/*0xa3ce*/ "Disabled!\n\n");
                    }
                }
            }
        }
    }

    if (player == -1)
    {
        ULTIMA_1850_PrintString(/*0xa3da*/ "None!\n");
    }
    else if (player == -2)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    return player;
}

// draw moon phase
// Draw the sun and the two moons in the top border for the hour and the day (D_1eda_moonPhases); blank underground.
void ULTIMA_4a84_DrawSunAndMoons(void)
{
	u16 savedWindow;
	int i;
	byte skyRow[12 + 1];
	int pos_n;

	if (D_5893_mapId < 0x21)
	{
		// 4a96
		for (i = 0; i < 12; i++)
		{
			skyRow[i] = 0x20;
		}

		skyRow[12] = 0;
		savedWindow = D_5386_textWindowIndex;
		ULTIMA_1b94_SelectTextWindow(0);
		ULTIMA_1c9e_SelectCharset(1);

		// 4ac3
		pos_n = -((int)D_587f_hour - 17);
		if (pos_n < 12 && pos_n >= 0)
		{
			skyRow[pos_n] = 0x2a;
		}

		// 4adf
		D_5885_trammelPhase = D_1eda_moonPhases[(D_587e_day - 1) * 2];
		pos_n = -(D_587f_hour - 8);
		if (pos_n < -12)
		{
			pos_n += 24;
		}

		// 4b04
		if (pos_n < 12 && pos_n >= 0)
		{
			skyRow[pos_n] = D_5885_trammelPhase;
		}

		// 4b19
		D_5886_feluccaPhase = D_1eda_moonPhases[(D_587e_day - 1) * 2 + 1];
		pos_n = -(D_587f_hour - 2);
		if (pos_n < -12)
		{
			pos_n += 24;
		}

		// 4b3e
		if (pos_n < 12 && pos_n >= 0)
		{
			skyRow[pos_n] = D_5886_feluccaPhase;
		}

		// 4b53
		ULTIMA_1bf2_SetTextPosition(6, 0);
		if (D_5893_mapId != 0x19 && D_5895_mapLevel < 0x80)
		{
			for (i = 0; skyRow[i] != 0;)
			{
				if (skyRow[i] == 0x2a)
				{
					// -> 4b74
					ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
				}
				else
				{
					// 4b70
					ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
				}

				ULTIMA_16ba_PrintChar(skyRow[i++]);
			}

			ULTIMA_1cca_SetTextForegroundColor(D_13b0_brightWhiteColor);
		}
		else
		{
			// 4ba2
			ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
			ULTIMA_0aa6_GRAP_3f_FillRect(0x28, 0, 0x98, 6);
			ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
			ULTIMA_0b10_GRAP_Line(0x28, 7, 0x98, 7);
		}

		// 4bd5
		ULTIMA_1b94_SelectTextWindow(savedWindow);
		ULTIMA_1c9e_SelectCharset(0);
	}
}

// print map name?
// Blank the sky in the top border.
void ULTIMA_4be8_ClearSky(void)
{
	int savedWindow;

	savedWindow = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);

	if (D_5893_mapId != 0x19 && D_5895_mapLevel < 0x80)
	{
		ULTIMA_1bf2_SetTextPosition(5, 0);
		ULTIMA_4c2a_PrintLeftCap();
		ULTIMA_1850_PrintString(/*0xa3e2*/ "            ");
		ULTIMA_4cce_PrintRightCap();
	}

	ULTIMA_1b94_SelectTextWindow(savedWindow);
}

// print >
// Print the left end cap of a title in a border.
void ULTIMA_4c2a_PrintLeftCap(void)
{
	int pixelY_c;
	int pixelX_d;
	int savedColors_g;
	TextWindow* win;

	win = &D_535e_textWindows[D_5386_textWindowIndex];

	pixelX_d = ((uint)win->left + ULTIMA_1f12_GetCurrentTextX()) * 8;
	pixelY_c = ((uint)win->top + ULTIMA_1cee_GetCurrentTextY()) * 8;

	savedColors_g = (uint)win->colors;
	ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
	ULTIMA_1f26_SetTextBackgroundColor(0);
	ULTIMA_16ba_PrintChar(2);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0b10_GRAP_Line(pixelX_d, pixelY_c, pixelX_d + 5, pixelY_c + 3);
	ULTIMA_0b10_GRAP_Line(pixelX_d + 5, pixelY_c + 4, pixelX_d, pixelY_c + 7);
	ULTIMA_1cca_SetTextForegroundColor(savedColors_g & 0xf);
	ULTIMA_1f26_SetTextBackgroundColor(savedColors_g >> 4);
}

// print <
// Print the right end cap of a title in a border.
void ULTIMA_4cce_PrintRightCap(void)
{
	int pixelY_c;
	int pixelX_d;
	int savedColors_g;
	TextWindow* win;

	win = &D_535e_textWindows[D_5386_textWindowIndex];

	pixelX_d = ((uint)win->left + ULTIMA_1f12_GetCurrentTextX()) * 8;
	pixelY_c = ((uint)win->top + ULTIMA_1cee_GetCurrentTextY()) * 8;

	savedColors_g = (uint)win->colors;
	ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
	ULTIMA_1f26_SetTextBackgroundColor(0);
	ULTIMA_16ba_PrintChar(1);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0b10_GRAP_Line(pixelX_d + 7, pixelY_c, pixelX_d + 2, pixelY_c + 3);
	ULTIMA_0b10_GRAP_Line(pixelX_d + 2, pixelY_c + 4, pixelX_d + 7, pixelY_c + 7);
	ULTIMA_1cca_SetTextForegroundColor(savedColors_g & 0xf);
	ULTIMA_1f26_SetTextBackgroundColor(savedColors_g >> 4);
}

// CHECKED
// find index
int ULTIMA_4d76_FindIndex(char* str, int ch)
{
	int index;

	index = 0;
	while (str[index] != 0)
	{
		if ((byte)str[index] == ch) break;
		index++;
	}

	return index;
}

// Remove the scroll arrow from the border under the stats window.
void ULTIMA_4daa_HideScrollArrow(void)
{
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
	ULTIMA_0aa6_GRAP_3f_FillRect(0xf0, 0x51, 0x107, 0x56);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0c9c_GRAP_39_HorizLine(0xf0, 0x50, 0x107);
	ULTIMA_0c9c_GRAP_39_HorizLine(0xf0, 0x57, 0x107);
}

// Show a scroll arrow (character 0x18, 0x19 or 0x12) in the border under the stats window.
void ULTIMA_4dea_ShowScrollArrow(int arrowChar)
{
	int savedWindow;

	savedWindow = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);
	ULTIMA_1bf2_SetTextPosition(0x1e, 10);
	ULTIMA_4c2a_PrintLeftCap();
	ULTIMA_16ba_PrintChar(arrowChar);
	ULTIMA_4cce_PrintRightCap();
	ULTIMA_1b94_SelectTextWindow(savedWindow);
}

// Remove the title from the top border of the stats window.
void ULTIMA_4e20_ClearStatsTitle(void)
{
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
	ULTIMA_0aa6_GRAP_3f_FillRect(0xc0, 0, 0x137, 0x6);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0c9c_GRAP_39_HorizLine(0xc0, 0x7, 0x137);
}

// Show a title centred in the top border of the stats window.
void ULTIMA_4e50_ShowStatsTitle(char* title)
{
	int startCol;
	int savedWindow;
	int endCol;

	startCol = -(ULTIMA_216c_strlen(title) / 2 - 30);
	endCol = ULTIMA_216c_strlen(title) + startCol + 2;
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
	ULTIMA_0aa6_GRAP_3f_FillRect(192, 0, startCol * 8, 6);
	ULTIMA_0aa6_GRAP_3f_FillRect(endCol * 8, 0, 311, 6);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0c9c_GRAP_39_HorizLine(192, 7, startCol * 8);
	ULTIMA_0c9c_GRAP_39_HorizLine(endCol * 8, 7, 311);
	savedWindow = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);
	ULTIMA_1bf2_SetTextPosition(startCol, 0);
	ULTIMA_4c2a_PrintLeftCap();
	ULTIMA_1850_PrintString(title);
	ULTIMA_4cce_PrintRightCap();
	ULTIMA_1b94_SelectTextWindow(savedWindow);
}

// Open the divider between the party and the food line, so that a list can use the whole stats window.
void ULTIMA_4efc_OpenStatsDivider(void)
{
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0cf2_GRAP_3c_VertLine(0xbf, 0x38, 0x3f);
	ULTIMA_0cf2_GRAP_3c_VertLine(0x138, 0x38, 0x3f);
	ULTIMA_0a70_GRAP_2d_SetPenColor(0);
	ULTIMA_0aa6_GRAP_3f_FillRect(0xc0, 0x38, 0x137, 0x3f);
}

// Draw the divider between the party and the food line.
void ULTIMA_4f3c_DrawStatsDivider(void)
{
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
	ULTIMA_0aa6_GRAP_3f_FillRect(0xbf, 0x39, 0x138, 0x3e);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0c9c_GRAP_39_HorizLine(0xc0, 0x38, 0x137);
	ULTIMA_0c9c_GRAP_39_HorizLine(0xc0, 0x3f, 0x137);
}

// CHECKED (stack, loop)
// time related
// Let `minutes` pass: the clock and the calendar, the light and torch turns, the shadowlords' move at midnight, the light level for the hour.
void ULTIMA_4f7c_AdvanceTime(int minutes)
{
	int oldLight;
	int other;
	int slot;
	uint town_h;

	if (minutes != 0)
	{
		// 4f8d
		if ((D_587a_statusIcon == 'Q') && (minutes = minutes >> 1, minutes == 0))
		{
			minutes++;
		}

		// 4fa0
		D_5880_lastHour = D_587f_hour;
		if (D_587a_statusIcon != 'T')
		{
			D_5881_minute += minutes;
			ULTIMA_3f36_DecreaseByte(&D_58a7_torchTurns, minutes);
			ULTIMA_3f36_DecreaseByte(&D_58a6_lightSpellTurns, minutes);
		}
		// 4fc8
		if (D_5881_minute > 59)
		{
			// 4fd2
			D_5881_minute -= 60;
			ULTIMA_3f36_DecreaseByte(&D_588c_campHealWait, 1);
			D_587f_hour++;
			if (D_587f_hour > 23)
			{
				// 4ff0
				D_587f_hour = 0;
				for (slot = 0; slot < 3; slot++)
				{
					// 4ffa
					if (D_58c8_shadowlordTowns[slot] < 0x80)
					{
						do
						{
							// 5004
							town_h = ULTIMA_2092_RandomRange(1, 8);
							if (D_5893_mapId == town_h)
							{
								town_h = 0;
							}
							// 501c
							for (other = 0; other < 3; other++)
							{
								// 5020
								if (D_58c8_shadowlordTowns[other] == town_h)
								{
									town_h = 0;
								}
							}
						} while (town_h == 0);

						D_58c8_shadowlordTowns[slot] = (u8)town_h;
					}
				}

				// 5048
				D_587e_day++;

				if (D_587e_day > 28)
				{
					D_5858_harvestDay[0] = D_5858_harvestDay[1] = D_5858_harvestDay[2] = 0;
					D_57b2_skullKeysDay = 0;
					D_587e_day = 1;
					D_5959_fewerEnemies = 0;

					// 5077
					// TODO: loop counter
					for (slot = 0; slot < 0x10; slot++)
					{
						// 507a
						if (D_55a8_party[slot].monthsAtInn < 0x19)
						{
							D_55a8_party[slot].monthsAtInn++;
						}
					}

					D_587d_month++;
					if (0xd < D_587d_month)
					{
						D_587d_month = 1;
						D_5874_year++;
					}
				}

				// 509e
				ULTIMA_2900_UpdateVitalsDisplay();
			}
		}
	}

	// 50a1
	// setup light intensity
	if (D_58a5_lightLevel < 0x33)
	{
		// 50ab
		oldLight = D_58a5_lightLevel;
		if (D_5893_mapId == 0x19 || D_5895_mapLevel > 0x7f || D_587f_hour < 5 || D_587f_hour > 0x13)
		{
			// 50cf
			D_58a5_lightLevel = 2;
		}
		else
		{
			// 50d6
			if (D_587f_hour == 5)
			{
				D_58a5_lightLevel = D_6a80_lightSlope[D_5881_minute / 10];
			}
			// 50f4
			else if (D_587f_hour == 19)
			{
				D_58a5_lightLevel = D_6a80_lightSlope[(-(D_5881_minute - 59)) / 10];
			}
			else
			{
				D_58a5_lightLevel = 0x32;
			}
		}

		// 5115
		if (D_58a6_lightSpellTurns != 0 && D_58a5_lightLevel < 0x12)
		{
			D_58a5_lightLevel = 0x12;
		}
		// 5128
		if (D_58a7_torchTurns != 0 && D_58a5_lightLevel < 10)
		{
			D_58a5_lightLevel = 10;
		}
		// 513b
		if (oldLight != D_58a5_lightLevel)
		{
			D_24e6_viewDirty = 1;
		}
	}

	// 514a
	if (D_587f_hour != D_5880_lastHour)
	{
		if (D_5893_mapId < 0x21 && D_5895_mapLevel < 0x80)
		{
			ULTIMA_4a84_DrawSunAndMoons();
		}

		// 5164
		if (D_587f_hour == 0)
		{
			D_5884_clockChimes = 0xc;
		}
		else
		{
			// 5172
			if (D_587f_hour > 0xc)
			{
				D_5884_clockChimes = D_587f_hour - 0xc;
			}
			else
			{
				D_5884_clockChimes = D_587f_hour;
			}
		}
	}

	if (D_5893_mapId != 0 && D_5893_mapId < 0x21)
	{
		ULTIMA_71aa_UpdateClockTiles();
	}
}
