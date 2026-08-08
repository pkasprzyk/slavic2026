#ifndef BUNNY_H__
#define BUNNY_H__

extern int bunnies_cnt;
extern int bunnies_total;
extern int bunnies_collected;
extern int bunnies_died;

void bunnies_init(void);
void bunnies_update(void);
void kill_all_bunnies(void);

#endif
