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

u16 D_6a36[9] =
{
    0x0da9, 0x0f56, 0x1136, 0x123c, 0x1478, 0x16fa, 0x1857, 0x19ca, 0x1b53
};

u8 D_6a48[0x36] = // 53 used
{
    0x01, 0x04, 0x04, 0x00, 0x00, 0x01, 0x05, 0x05,
    0x00, 0x00, 0x04, 0x09, 0x06, 0x09, 0x07, 0x04,
    0x06, 0x05, 0x01, 0x04, 0x00, 0x00, 0x00, 0x01,
    0x05, 0x00, 0x00, 0x00, 0x04, 0x09, 0x06, 0x05,
    0x06, 0x04, 0x00, 0x00, 0x00, 0x05, 0x08, 0x08,
    0x09, 0x05, 0x06, 0x08, 0x09, 0x05, 0x04, 0x06,
    0x05, 0x04, 0x03, 0x02, 0x01
};

u8 D_6a7e = 0;

u8 D_6a80[6] = { 0x02, 0x05, 0x0A, 0x14, 0x22, 0x31 };

// Each living member who wears a ring of regeneration (0x2c) gains a hit point one time in eight.
void ULTIMA_400c_RingRegeneration(void)
{
    int local_4;
    for (local_4 = 0; local_4 < D_585b_partyCount; local_4++)
	{
		if (D_55a8_party[local_4].status != STATUS_DEAD && D_55a8_party[local_4].equips[4] == 0x2c)
		{
			if (ULTIMA_2092_RandomRange(0, 7) == 7)
			{
				ULTIMA_3f14_IncreaseInt(&D_55a8_party[local_4].hp, 1, D_55a8_party[local_4].maxHp);
				D_a9fa_vitalsDirty = 1;
			}
		}
	}
}

// select active player
// The keys 0-6: set the active player (0 none).
int ULTIMA_4080_SetActivePlayer(int param_1)
{
    int local_6_an;
	int local_4;
    
    local_4 = 1;
    local_6_an = param_1 - 0x31;

    ULTIMA_1850_PrintString(_TEXT(0xa396, "Set Active Plr:\n"));
    if (param_1 == 0x30)
    {
        ULTIMA_1850_PrintString(_TEXT(0xa3a8, "None!\n"));
        D_587b_activePlayer = 0xff;

		ULTIMA_2900_UpdateVitalsDisplay();
		local_4 = 0;
    }
    else
    {
        if (D_585b_partyCount > local_6_an && D_55a8_party[local_6_an].status != STATUS_DEAD && D_55a8_party[local_6_an].status != STATUS_SLEEP)
        {
            D_587b_activePlayer = local_6_an;

            ULTIMA_1850_PrintString(D_55a8_party[local_6_an].name);
            ULTIMA_16ba_PrintChar('\n');

            ULTIMA_2900_UpdateVitalsDisplay();
            local_4 = 0;
        }
		else
		{
            ULTIMA_1850_PrintString(_TEXT(0xa3b0, "Invalid!\n"));
        }
    }

    return local_4;
}

// CHECKED
void ULTIMA_4102_AudioPlayAmbientSfx(void)
{
    int local_16_ak;
	int local_14_am;
	int local_12_a;
	int local_10_aa;
	int local_e_am;
	int local_c;
	int local_a;
	int local_8;
	int local_6;
	int local_4_ab;

	local_e_am = 0;
	local_4_ab = 0x33;

	if (D_5893_mapId < 0x80)
	{
		local_8 = (uint)D_5896_mapX;
		local_a = (uint)D_5897_mapY;
	}
	else
	{
		local_8 = local_a = 5;
	}

    for (local_c = local_8 - 5, local_12_a = 0; local_c < local_8 + 6; local_c++, local_12_a++)
	{
        for (local_10_aa = local_a - 5, local_16_ak = 0; local_10_aa < local_a + 6; local_10_aa++, local_16_ak++)
		{
			local_6 = (local_10_aa - local_a) * (local_10_aa - local_a) + (local_c - local_8) * (local_c - local_8);
			// a continue, not an if around the rest: with it MSC keeps local_16_ak's increment
			// and the signed loop test, instead of a closed form and a jump on equal
			if (local_6 >= local_4_ab)
				continue;
			{
				local_14_am = *ULTIMA_4402_GetTileAddr(local_c, local_10_aa);
				if ((local_14_am & 0xfe) == TILE_MAP_CLOCK)
				{
					local_e_am = 1;
				}
				else if ((local_14_am & 0xfc) == TILE_MAP_WATERFALL)
				{
					local_e_am = 2;
				}
				else if ((local_14_am & 0xfc) == TILE_MAP_FOUNTAIN)
				{
					local_e_am = 3;
				}
                else if (GetMapViewport(local_12_a, local_16_ak) == 0 && (GetActorMap(local_12_a, local_16_ak) & 0xfc) == TILE_ACTOR_BARD)
				{
					// BGM/NOTE: no jingle when bgm is activated
					local_e_am = 4;
				}
				else
				{
					local_6 = local_4_ab;
				}

				local_4_ab = local_6;
			}
		}
	}

	switch (local_e_am)
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
        if (D_6a48[D_6a08] != 0)
        {
            ULTIMA_2192_AudioPulse(D_6a36[D_6a48[D_6a08] - 1], 1, 2000, 20000, -10);
        }

        D_6a08++;
        if (D_6a08 >= 0x35)
        {
            D_6a08 = 0;
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
    int local_4;

    for (local_4 = 0; local_4 < 3; local_4++)
    {
        ULTIMA_2192_AudioPulse(0x11f8, 1, 0x2a30, 300, 6);
    }

    ULTIMA_2192_AudioPulse(0x17d4, 1, 0x5460, 300, 3);
}

void ULTIMA_43ae_AudioSweepTone(int param_1, int param_2, int param_3, int param_4)
{
#if !defined(TARGET_DOS16)
	AUDIO_DispatchSweepTone(param_1, param_2, param_3, param_4);
	return;
#endif

	int local_6;
	int local_4_b;
	int local_a;
	int local_8;

	local_a = param_3;
	local_4_b = (param_2 - param_1) * local_a;
    local_4_b /= param_4;
	local_8 = param_1;

	for (local_6 = 0; local_6 < param_4; local_6 += param_3)
	{
		ULTIMA_22e2_PcspkOn(local_8);
		ULTIMA_20c8_Delay(1, param_3);
		local_8 += local_4_b;
	}

	ULTIMA_230e_PcspkOff();
}

// CHECKED (NOT MATCHING: optimization)
byte* ULTIMA_4402_GetTileAddr(int x, int y)
{
	int local_6_an;
	byte* local_4;

	if (D_5893_mapId > 0x7f) // combat?
	{
		// 440f
        // There are cases where y < 0. original bug
#if !defined(MATCHING_BUILD)
		ASSERT(x >= 0 && x < 0xb && y < 0xb);
		if (y < 0)
			return &D_6a07; // return unused byte
#endif

		local_4 = &GetCombatMap(x, y);
	}
	else if (D_5893_mapId == 0) // 4420; overworld
	{
		x = (x - D_589b_mapOriginX) & 0x1f;
		y = (y - D_589c_mapOriginY) & 0x1f;
		local_6_an = 0;
		if (y > 0xf)
		{
			local_6_an = 2;
			y -= 16;
		}

		if (x > 0xf)
		{
			local_6_an++;
			x -= 16;
		}

		// 4467 (NOT MATCHING: optimization)
		local_4 = &GetMapOverworld(x, y, local_6_an);
	}
	else if (x < 0 || y < 0 || x > 0x1f || y > 0x1f) // 447e
	{
		// 4496
		local_4 = &D_6a07;
	}
	else
	{
		// 449e
		local_4 = &GetMap(x, y);
	}

	return local_4;
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

	if ((D_6a7e & 1) != 0)
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

		if ((D_6a7e & 2) != 0)
		{
            for (i = 0xfa; i < 0xfe; i++)
			{
				D_b11e_tileFrames[i] ^= 1;
			}
		}
	}

	D_6a7e++;
}

// CHECKED
// animate_actors
void ULTIMA_4552_AnimateActors(void)
{
	int local_14_am;
	byte* local_12;
	int local_10_aa;
	int local_e_am;
	int local_c;
	bool local_a;
	ActorFmt* local_8;
	int local_6;
	int local_4_ab;

	for (local_10_aa = 0; local_10_aa < 0x20; local_10_aa++)
	{
		local_8 = &D_5c5a_actors[local_10_aa];
		if (local_8->tile == 0)
		{
			continue;
		}

        local_c = local_8->animState & 0xf;
        local_14_am = local_8->animState >> 4;
        local_4_ab = local_8->tile & 0xfc;
        local_e_am = D_1b18_animSequences[((local_4_ab - 0x34) >> 2) + 0xb0];
		local_12 = &D_1b18_animSequences[local_e_am * 0x10];

		if (local_c == 0xf)
		{
			continue;
		}

        if (local_c != 0)
        {
            local_c--;
            local_8->animState = (local_8->animState & 0xf0) + local_c;
			continue;
        }

		if (local_8->animTile == 0 || local_8->animTile == TILE_ACTOR_1D || local_8->animTile == TILE_ACTOR_SLEEP)
		{
			continue;
		}

		if (local_4_ab < 0x34 || local_4_ab == 0xe8 || local_4_ab == 0xb4)
		{
			continue;
		}

		if (local_4_ab != 0x5c && local_4_ab != 0xa8)
		{
			if (ULTIMA_2092_RandomRange(0, 0xff) < 0x80)
			{
				continue;
			}
		}

        // The gotos are the original's shape: case 0's "goto" stays as a dead jmp after the
        // jump-table dispatch (the table goes straight to L_restart), and cases 1-4 end in
        // case 5's "local_a = 1; break". Written out, MSC drops case 0's copy and merges
        // case 5's tail into cases 1-4's instead.
        local_a = 0;
        do
        {
            local_6 = local_12[local_14_am];
            switch (local_6)
            {
            case 0:
                goto L_restart;

            case 1:
            case 2:
            case 3:
            case 4:
                local_8->animTile = local_4_ab + local_6 - 1;
                local_14_am++;
                goto L_done;

            case 5:
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0x40)
                {
                    local_8->animTile = local_8->tile;
                    if (local_4_ab == 0x5c)
                    {
                        local_14_am++;
                        goto L_done;
                    }

                    local_c = 6;
                L_done:
                    local_a = 1;
                    break;
                }

                local_14_am++;
                break;

            case 6:
                if (ULTIMA_2092_RandomRange(0, 0xff) < 0xc0)
                {
                L_restart:
                    local_14_am = 0;
                    break;
                }
                local_14_am++;
                break;

            case 7:
                local_14_am = 2;
                break;

            default:
                local_c = local_6 - 0x80;
                local_14_am++;
                local_a = 1;
                break;
            }

            local_8->animState = local_14_am * 0x10 + local_c;
        } while (!local_a);
    }

	ULTIMA_44b8_AnimateTiles();
    ULTIMA_6fd6_GRAP_AnimateTiles();
}

static int ULTIMA_4702(int param_1)
{
	int local_8;
	int local_6;
	int local_4_ab;

	local_4_ab = 0;

	if (D_5840_moonstoneMap[param_1] == D_5893_mapId && D_5848_moonstoneZ[param_1] == D_5895_mapLevel)
	{
		if (D_5893_mapId != 0)
		{
			local_4_ab = 1;
		}
		else
		{
			local_6 = (byte)(D_5830_moonstoneX[param_1] - D_589b_mapOriginX);
			local_8 = (byte)(D_5838_moonstoneY[param_1] - D_589c_mapOriginY);

			if (local_6 < 0x20 && local_8 < 0x20)
			{
				local_4_ab = 1;
			}
		}
	}

	return local_4_ab;
}

// Raise the moongates at night and lower them by day, and put their tile on the map where a moonstone is buried.
void ULTIMA_475a_UpdateMoongates(void)
{
	int local_a;
	int local_8;
	int local_6;
	byte* local_4_ab;

	local_a = 0xdc;
	if (D_587f_hour >= 0x14 || D_587f_hour < 5)
	{
        ULTIMA_3ef0_IncreaseByte(&D_5887_moongateHeight, 1, 0x10);
    }
	else
	{
        ULTIMA_3f36_DecreaseByte(&D_5887_moongateHeight, 1);
        if (D_5887_moongateHeight == 0)
        {
            local_a = 5;
        }
    }

	for (local_6 = 0; local_6 < 8; local_6++)
	{
		if (ULTIMA_4702(local_6) != 0)
		{
			local_4_ab = ULTIMA_4402_GetTileAddr(D_5830_moonstoneX[local_6], D_5838_moonstoneY[local_6]);
			local_8 = *local_4_ab != local_a;
			*local_4_ab = local_a;
			D_24e6_viewDirty |= 2;
			if (local_8 != 0)
			{
				ULTIMA_5e4a_BuildLightMap();
			}
		}
	}
}

// Take the party to where moonstone `index` is buried; 0 when it is not buried.
int ULTIMA_47f4_GoToMoonstone(int param_1)
{
	int local_4;

	if (D_5840_moonstoneMap[param_1] == 0xff)
	{
		return 0;
	}

    if (D_5893_mapId == 0)
    {
        ULTIMA_251e_SwitchDisks(1);

        while (ULTIMA_1674_TestOpenFile(_TEXT(0xa3ba, "BRIT.DAT")) == 0) {}

        ULTIMA_25d8_WriteFileToDisk(OUTSUBS_0368_GetWorldSavefile(), &D_5c5a_actors[0].tile, 0x100);
    }

    local_4 = D_5893_mapId;
    D_5893_mapId = D_5840_moonstoneMap[param_1];
    D_5896_mapX = D_5830_moonstoneX[param_1];
    D_5897_mapY = D_5838_moonstoneY[param_1];
    D_5895_mapLevel = D_5848_moonstoneZ[param_1];

    if (D_5893_mapId != 0 && local_4 != 0 && D_5893_mapId < 0x21 && local_4 < 0x21)
    {
        TOWN_11f0_Entry(1);
    }
    else
    {
        if (D_5893_mapId == 0 && local_4 == 0)
        {
#if defined(ENABLE_BGM)
			AUDIO_StopBgm();
#endif

            ULTIMA_256e_ReadFileFromDisk(OUTSUBS_0368_GetWorldSavefile(), &D_5c5a_actors[0].tile, 0x100, 0);
            MAINOUT_0000_SetUpOverworld();

#if defined(ENABLE_BGM)
			AUDIO_PlayBgmPerMap();
#endif
        }
    }

    return 1;
}

// On a moongate: the party sinks into it and comes out at the moonstone of the moon's phase. Returns 1 instead in the first ten minutes after midnight (for the caller to act on), else 0.
int ULTIMA_48a8_EnterMoongate(void)
{
	int local_8;
	int local_6; // unused
	int local_4_ab;

	local_8 = 0;
	if (*ULTIMA_4402_GetTileAddr(D_5896_mapX, D_5897_mapY) == TILE_MAP_MOONGATE)
	{
		ULTIMA_3ae6_WaitFrames(1);
		ULTIMA_2192_AudioPulse(0x170c, 1, 30000, 2000, 2);
		local_4_ab = D_587c_partyTile;
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
			local_8 = 1;
		}
		else
		{
			ULTIMA_47f4_GoToMoonstone((D_587f_hour < 0xc ? D_5885_trammelPhase : D_5886_feluccaPhase) - 0x30);
		}

		D_587c_partyTile = local_4_ab;
	}

	return local_8;
}

// The party member who acts: the combatant whose turn it is, else the active player, else the only able one, else ask ("Player: "). -1 none.
int ULTIMA_4988_GetActingPlayer(void)
{
    int local_a;
    int local_8;
    int local_6;
    bool local_4_ab;

    local_a = -1;
    if (D_5893_mapId > 0x80)
    {
        local_a = D_ba14_combatants[D_589e_curCombatant].entityIdx;
    }
    else if (D_587b_activePlayer != 0xff)
    {
        local_a = D_587b_activePlayer;
    }
    else
    {
        local_8 = 0;
        for (local_6 = 0; local_6 != D_585b_partyCount; local_6++)
        {
            if (D_55a8_party[local_6].status == STATUS_GOOD || D_55a8_party[local_6].status == STATUS_POISONED)
            {
				local_a = local_6;
                local_8++;
            }
        }

        if (local_8 > 1)
        {
            local_4_ab = 0;
            while (!local_4_ab)
            {
                ULTIMA_1850_PrintString(_TEXT(0xa3c4, "Player: "));
                local_a = ULTIMA_2e8e_SelectPlayer();
                if (local_a < 0)
                {
                    local_4_ab = 1;
                }
                else
                {
                    if (D_55a8_party[local_a].status == STATUS_GOOD || D_55a8_party[local_a].status == STATUS_POISONED)
                    {
                        ULTIMA_1850_PrintString(D_55a8_party[local_a].name);
                        if (ULTIMA_1f12_GetCurrentTextX() != 0)
                        {
                            ULTIMA_16ba_PrintChar('\n');
                        }

                        local_4_ab = 1;
                    }
                    else
                    {
                        ULTIMA_1850_PrintString(_TEXT(0xa3ce, "Disabled!\n\n"));
                    }
                }
            }
        }
    }

    if (local_a == -1)
    {
        ULTIMA_1850_PrintString(_TEXT(0xa3da, "None!\n"));
    }
    else if (local_a == -2)
    {
        ULTIMA_16ba_PrintChar('\n');
    }

    return local_a;
}

// draw moon phase
// Draw the sun and the two moons in the top border for the hour and the day (D_1eda_moonPhases); blank underground.
void ULTIMA_4a84_DrawSunAndMoons(void)
{
	u16 local_16_an;
	int local_14;
	byte local_12_aa[12 + 1];
	int local_4_aa;

	if (D_5893_mapId < 0x21)
	{
		// 4a96
		for (local_14 = 0; local_14 < 12; local_14++)
		{
			local_12_aa[local_14] = 0x20;
		}

		local_12_aa[12] = 0;
		local_16_an = D_5386_textWindowIndex;
		ULTIMA_1b94_SelectTextWindow(0);
		ULTIMA_1c9e_SelectCharset(1);

		// 4ac3
		local_4_aa = -((int)D_587f_hour - 17);
		if (local_4_aa < 12 && local_4_aa >= 0)
		{
			local_12_aa[local_4_aa] = 0x2a;
		}

		// 4adf
		D_5885_trammelPhase = D_1eda_moonPhases[(D_587e_day - 1) * 2];
		local_4_aa = -(D_587f_hour - 8);
		if (local_4_aa < -12)
		{
			local_4_aa += 24;
		}

		// 4b04
		if (local_4_aa < 12 && local_4_aa >= 0)
		{
			local_12_aa[local_4_aa] = D_5885_trammelPhase;
		}

		// 4b19
		D_5886_feluccaPhase = D_1eda_moonPhases[(D_587e_day - 1) * 2 + 1];
		local_4_aa = -(D_587f_hour - 2);
		if (local_4_aa < -12)
		{
			local_4_aa += 24;
		}

		// 4b3e
		if (local_4_aa < 12 && local_4_aa >= 0)
		{
			local_12_aa[local_4_aa] = D_5886_feluccaPhase;
		}

		// 4b53
		ULTIMA_1bf2_SetTextPosition(6, 0);
		if (D_5893_mapId != 0x19 && D_5895_mapLevel < 0x80)
		{
			for (local_14 = 0; local_12_aa[local_14] != 0;)
			{
				if (local_12_aa[local_14] == 0x2a)
				{
					// -> 4b74
					ULTIMA_1cca_SetTextForegroundColor(D_13b8_brightYellowColor);
				}
				else
				{
					// 4b70
					ULTIMA_1cca_SetTextForegroundColor(D_13ba_lightGrayColor);
				}

				ULTIMA_16ba_PrintChar(local_12_aa[local_14++]);
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
		ULTIMA_1b94_SelectTextWindow(local_16_an);
		ULTIMA_1c9e_SelectCharset(0);
	}
}

// print map name?
// Blank the sky in the top border.
void ULTIMA_4be8_ClearSky(void)
{
	int local_4;

	local_4 = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);

	if (D_5893_mapId != 0x19 && D_5895_mapLevel < 0x80)
	{
		ULTIMA_1bf2_SetTextPosition(5, 0);
		ULTIMA_4c2a_PrintLeftCap();
		ULTIMA_1850_PrintString(_TEXT(0xa3e2, "            "));
		ULTIMA_4cce_PrintRightCap();
	}

	ULTIMA_1b94_SelectTextWindow(local_4);
}

// print >
// Print the left end cap of a title in a border.
void ULTIMA_4c2a_PrintLeftCap(void)
{
	int local_8_al;
	int local_6_an;
	int local_4;
	TextWindow* win;

	win = &D_535e_textWindows[D_5386_textWindowIndex];

	local_6_an = ((uint)win->left + ULTIMA_1f12_GetCurrentTextX()) * 8;
	local_8_al = ((uint)win->top + ULTIMA_1cee_GetCurrentTextY()) * 8;

	local_4 = (uint)win->colors;
	ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
	ULTIMA_1f26_SetTextBackgroundColor(0);
	ULTIMA_16ba_PrintChar(2);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0b10_GRAP_Line(local_6_an, local_8_al, local_6_an + 5, local_8_al + 3);
	ULTIMA_0b10_GRAP_Line(local_6_an + 5, local_8_al + 4, local_6_an, local_8_al + 7);
	ULTIMA_1cca_SetTextForegroundColor(local_4 & 0xf);
	ULTIMA_1f26_SetTextBackgroundColor(local_4 >> 4);
}

// print <
// Print the right end cap of a title in a border.
void ULTIMA_4cce_PrintRightCap(void)
{
	int local_8_al;
	int local_6_an;
	int local_4;
	TextWindow* win;

	win = &D_535e_textWindows[D_5386_textWindowIndex];

	local_6_an = ((uint)win->left + ULTIMA_1f12_GetCurrentTextX()) * 8;
	local_8_al = ((uint)win->top + ULTIMA_1cee_GetCurrentTextY()) * 8;

	local_4 = (uint)win->colors;
	ULTIMA_1cca_SetTextForegroundColor(D_13b2_blueColor);
	ULTIMA_1f26_SetTextBackgroundColor(0);
	ULTIMA_16ba_PrintChar(1);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0b10_GRAP_Line(local_6_an + 7, local_8_al, local_6_an + 2, local_8_al + 3);
	ULTIMA_0b10_GRAP_Line(local_6_an + 2, local_8_al + 4, local_6_an + 7, local_8_al + 7);
	ULTIMA_1cca_SetTextForegroundColor(local_4 & 0xf);
	ULTIMA_1f26_SetTextBackgroundColor(local_4 >> 4);
}

// CHECKED
// find index
int ULTIMA_4d76_FindIndex(char* param_2, int param_1)
{
	int local_4;

	local_4 = 0;
	while (param_2[local_4] != 0)
	{
		if ((byte)param_2[local_4] == param_1) break;
		local_4++;
	}

	return local_4;
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
void ULTIMA_4dea_ShowScrollArrow(int param_1)
{
	int local_4;

	local_4 = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);
	ULTIMA_1bf2_SetTextPosition(0x1e, 10);
	ULTIMA_4c2a_PrintLeftCap();
	ULTIMA_16ba_PrintChar(param_1);
	ULTIMA_4cce_PrintRightCap();
	ULTIMA_1b94_SelectTextWindow(local_4);
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
void ULTIMA_4e50_ShowStatsTitle(char* param_1)
{
	int local_6_an;
	int local_4;
	int local_2;

	local_6_an = -(ULTIMA_216c_strlen(param_1) / 2 - 30);
	local_2 = ULTIMA_216c_strlen(param_1) + local_6_an + 2;
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b2_blueColor);
	ULTIMA_0aa6_GRAP_3f_FillRect(192, 0, local_6_an * 8, 6);
	ULTIMA_0aa6_GRAP_3f_FillRect(local_2 * 8, 0, 311, 6);
	ULTIMA_0a70_GRAP_2d_SetPenColor(D_13b0_brightWhiteColor);
	ULTIMA_0c9c_GRAP_39_HorizLine(192, 7, local_6_an * 8);
	ULTIMA_0c9c_GRAP_39_HorizLine(local_2 * 8, 7, 311);
	local_4 = D_5386_textWindowIndex;
	ULTIMA_1b94_SelectTextWindow(0);
	ULTIMA_1bf2_SetTextPosition(local_6_an, 0);
	ULTIMA_4c2a_PrintLeftCap();
	ULTIMA_1850_PrintString(param_1);
	ULTIMA_4cce_PrintRightCap();
	ULTIMA_1b94_SelectTextWindow(local_4);
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
void ULTIMA_4f7c_AdvanceTime(int param_1)
{
	int local_a;
	int local_8;
	int local_6;
	uint local_4_ab;

	if (param_1 != 0)
	{
		// 4f8d
		if ((D_587a_statusIcon == 'Q') && (param_1 = param_1 >> 1, param_1 == 0))
		{
			param_1++;
		}

		// 4fa0
		D_5880_lastHour = D_587f_hour;
		if (D_587a_statusIcon != 'T')
		{
			D_5881_minute += param_1;
			ULTIMA_3f36_DecreaseByte(&D_58a7_torchTurns, param_1);
			ULTIMA_3f36_DecreaseByte(&D_58a6_lightSpellTurns, param_1);
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
				for (local_6 = 0; local_6 < 3; local_6++)
				{
					// 4ffa
					if (D_58c8_shadowlordTowns[local_6] < 0x80)
					{
						do
						{
							// 5004
							local_4_ab = ULTIMA_2092_RandomRange(1, 8);
							if (D_5893_mapId == local_4_ab)
							{
								local_4_ab = 0;
							}
							// 501c
							for (local_8 = 0; local_8 < 3; local_8++)
							{
								// 5020
								if (D_58c8_shadowlordTowns[local_8] == local_4_ab)
								{
									local_4_ab = 0;
								}
							}
						} while (local_4_ab == 0);

						D_58c8_shadowlordTowns[local_6] = (u8)local_4_ab;
					}
				}

				// 5048
				D_587e_day++;

				if (D_587e_day > 28)
				{
					D_5858_harvestDay[0] = D_5858_harvestDay[1] = D_5858_harvestDay[2] = 0;
					D_57b2_skullKeysDay = 0;
					D_587e_day = 1;
					D_5959 = 0;

					// 5077
					// TODO: loop counter
					for (local_6 = 0; local_6 < 0x10; local_6++)
					{
						// 507a
						if (D_55a8_party[local_6].monthsAtInn < 0x19)
						{
							D_55a8_party[local_6].monthsAtInn++;
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
		local_a = D_58a5_lightLevel;
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
				D_58a5_lightLevel = D_6a80[D_5881_minute / 10];
			}
			// 50f4
			else if (D_587f_hour == 19)
			{
				D_58a5_lightLevel = D_6a80[(-(D_5881_minute - 59)) / 10];
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
		if (local_a != D_58a5_lightLevel)
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
