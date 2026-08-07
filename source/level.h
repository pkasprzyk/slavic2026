#ifndef LEVEL_H__
#define LEVEL_H__

#include <nds.h>

#define LEVEL_W 512
#define LEVEL_H 512
#define SCREEN_W 256
#define SCREEN_H 192

void level_init(void);
void level_update_camera(s16 target_x, s16 target_y);
s16 level_cam_x(void);
s16 level_cam_y(void);

#endif
