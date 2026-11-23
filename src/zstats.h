#ifndef _ZSTATS_H
#define _ZSTATS_H

void DrawListFrame(int lastRow);
int PrevItem(int index, int notused, byte* counts, int member);
int NextItem(int index, int count, byte* counts, int member);
void PrintItemRow(int index, byte* counts, char** names, uint marker);
void FillItemCounts(void);
void ZstatsCmd(void);
int SelectItem(int first, int member, int mode);
void ReadyCmd(void);

#endif
