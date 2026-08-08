// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>

#include "ids.h"
#include "level.h"

#define CAM_EASE 4
#define CAM_FP 8

static s32 cam_x;
static s32 cam_y;

void level_init(void) {
  NF_LoadTiledBg(PATH_BG_FOREST, BG_FOREST, BG_W, BG_H);
  NF_CreateTiledBg(SCR_WORLD, LAYER_WORLD_BG, BG_FOREST);

  NF_LoadTilesForBg("bg/fire_tileset",  // Reads tileset.img and tileset.pal
                    "dynamic_fire_map", // Internal background name
                    BG_W,               // Map width in pixels
                    BG_H,               // Map height in pixels
                    0,                  // First tile
                    3                   // Last tile
  );
  NF_CreateTiledBg(SCR_WORLD, LAYER_WORLD_FIRE, "dynamic_fire_map");

  NF_InitCmapBuffers();
  NF_LoadCollisionMap(PATH_COLMAP, COLMAP_SLOT, LEVEL_W, LEVEL_H);

  cam_x = 0;
  cam_y = 0;
  NF_ScrollBg(SCR_WORLD, LAYER_WORLD_BG, 0, 0);
}

void level_update_camera(s16 target_x, s16 target_y) {
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

  NF_ScrollBg(SCR_WORLD, LAYER_WORLD_BG, cam_x >> CAM_FP, cam_y >> CAM_FP);
}

s16 level_cam_x(void) { return cam_x >> CAM_FP; }

s16 level_cam_y(void) { return cam_y >> CAM_FP; }
