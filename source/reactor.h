#ifndef REACTOR_H__
#define REACTOR_H__

#include <nds.h>

extern int fire_heat_cooldown;

void reactor_init(void);
void reactor_update(void);
void reactor_deinit(void);

void reactor_increase_temp(int amount);
void reactor_decrease_temp(int amount);

void reactor_heat_from_fire(s32 distance_sq);

#endif
