#ifndef _ZSTATS_H
#define _ZSTATS_H

void ZSTATS_045e_DrawListFrame(int lastRow);
int ZSTATS_056c_PrevItem(int index, int notused, byte* counts, int member);
int ZSTATS_05a4_NextItem(int index, int count, byte* counts, int member);
void ZSTATS_05e2_PrintItemRow(int index, byte* counts, char** names, uint marker);
void ZSTATS_099a_FillItemCounts(void);
void ZSTATS_0a3a_ZstatsCmd(void);
int ZSTATS_0f2e_SelectItem(int first, int member, int mode);
void ZSTATS_1296_ReadyCmd(void);

#endif
