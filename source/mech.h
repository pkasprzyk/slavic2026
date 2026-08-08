#ifndef MECH_H__
#define MECH_H__

#include <nds.h>

void mech_init(void);
void mech_update(void);

extern s16 mech_x;
extern s16 mech_y;

s32 mech_fire_distance_sq(s16 fire_tx, s16 fire_ty);

#endif
