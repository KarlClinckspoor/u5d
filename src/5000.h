#ifndef _ULTIMA_5000_H
#define _ULTIMA_5000_H

// Game cont.

void ULTIMA_5394_PutActorsInView(void);
int ULTIMA_5646_IsEnemy(int a);
void ULTIMA_56ac_DrawMap(void);
void ULTIMA_5910_UpdateFrame(void);

void ULTIMA_5d0a_BuildView(int a, int x, int y, int d);
void ULTIMA_5e4a_BuildLightMap(void);
void ULTIMA_5f86_SpecialMapHandler(int mapFlags, int actorOrGuard, int hours);

#endif
