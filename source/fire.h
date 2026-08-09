#ifndef FIRE_H__
#define FIRE_H__

#define MAX_GRID_X 64
#define MAX_GRID_Y 64
#define FIRE_CELLS_MAX 2048
#define FIRE_BURN_DURATION (60 * 60)
#define FIRE_SPREAD_MIN (4 * 60)
#define FIRE_SPREAD_MAX (8 * 60)
#define FIRE_FRAME_TOGGLE 15

void fire_init(void);
void fire_restart(void);
void fire_update(void);
int fire_is_burning(int tx, int ty);
void fire_extinguish(int tx, int ty);
void fire_partial_extinguish(int tx, int ty, int value);
void extinguish_all_flames();

#endif
