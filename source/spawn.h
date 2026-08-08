#ifndef SPAWN_H__
#define SPAWN_H__

#include <nds.h>

void spawn_load(void);
u16 spawn_player_x(void);
u16 spawn_player_y(void);
u8  spawn_bunny_count(void);
u16 spawn_bunny_x(u8 index);
u16 spawn_bunny_y(u8 index);

#endif
