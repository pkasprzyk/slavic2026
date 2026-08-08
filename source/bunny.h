#ifndef BUNNY_H__
#define BUNNY_H__

#include <nds.h>

extern s16 bunnies_cnt;
extern s16 bunnies_total;
extern s16 bunnies_collected;
extern s16 bunnies_died;

void bunnies_init(void);
void bunnies_update(void);
void bunnies_end_screen_update();
void kill_all_bunnies(void);

#endif
