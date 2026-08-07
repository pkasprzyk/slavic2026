// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>

#include "level.h"

#define CAM_EASE 4
#define CAM_FP 8

static s32 cam_x;
static s32 cam_y;

void level_init(void)
{
    NF_LoadTiledBg("bg/forest", "forest", LEVEL_W, LEVEL_H);
    NF_CreateTiledBg(1, 3, "forest");

    cam_x = 0;
    cam_y = 0;
    NF_ScrollBg(1, 3, 0, 0);
}

void level_update_camera(s16 target_x, s16 target_y)
{
    s32 tx = target_x - SCREEN_W / 2;
    s32 ty = target_y - SCREEN_H / 2;

    if (tx < 0)
        tx = 0;
    if (ty < 0)
        ty = 0;
    if (tx > LEVEL_W - SCREEN_W)
        tx = LEVEL_W - SCREEN_W;
    if (ty > LEVEL_H - SCREEN_H)
        ty = LEVEL_H - SCREEN_H;

    cam_x += ((tx << CAM_FP) - cam_x) / CAM_EASE;
    cam_y += ((ty << CAM_FP) - cam_y) / CAM_EASE;

    NF_ScrollBg(1, 3, cam_x >> CAM_FP, cam_y >> CAM_FP);
}

s16 level_cam_x(void)
{
    return cam_x >> CAM_FP;
}

s16 level_cam_y(void)
{
    return cam_y >> CAM_FP;
}
