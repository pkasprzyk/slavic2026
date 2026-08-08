#ifndef WATER_H__
#define WATER_H__

#include <nds.h>

void water_init(void);
void water_restart(void);
void water_update(void);
void water_spray(int mech_cx, int mech_cy, int target_x, int target_y);
void water_fill_update(void);
void water_show_pump(void);
void water_hide_pump(void);
void water_operate_pump(int x, int y);

#endif
