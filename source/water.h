#ifndef WATER_H__
#define WATER_H__

#include <nds.h>

#define WATER_DROP_INTERVAL 5

void water_init(void);
void water_update(void);
void water_drop_spawn(s16 x, s16 y);

#endif
