// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>

#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"

#define MECH_SPEED 2

#define MECH_W 32
#define MECH_H 32

#define MECH_MAX_WATER 200

#define SPRITE_ID 0

static s16 mech_x;
static s16 mech_y;
static s16 last_mech_x;
static s16 last_mech_y;
static s8 mech_water_remaining;
static u16 frame_cnt = 0;

void mech_init(void) {
  mech_x = 128;
  mech_y = 128;
  mech_water_remaining = MECH_MAX_WATER;

  NF_CreateSprite(SCR_WORLD, SPRITE_ID, MECH, 0, mech_x, mech_y);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_BG);
}

static int tile_solid(s32 x, s32 y) {
  return NF_GetTile(COLMAP_SLOT, x, y) == TILE_SOLID;
}

static int mech_blocked(s32 x, s32 y) {
  if (tile_solid(x, y))
    return 1;
  if (tile_solid(x + MECH_W - 1, y))
    return 1;
  if (tile_solid(x, y + MECH_H - 1))
    return 1;
  if (tile_solid(x + MECH_W - 1, y + MECH_H - 1))
    return 1;
  return 0;
}

void mech_update(void) {
  ++frame_cnt;
  frame_cnt %= 60;

  u16 held = keysHeld();
  last_mech_x = mech_x;
  last_mech_y = mech_y;
  s16 nx = mech_x;
  s16 ny = mech_y;

  if (held & KEY_LEFT)
    nx -= MECH_SPEED;
  if (held & KEY_RIGHT)
    nx += MECH_SPEED;
  if (held & KEY_UP)
    ny -= MECH_SPEED;
  if (held & KEY_DOWN)
    ny += MECH_SPEED;

  if (nx != mech_x && !mech_blocked(nx, mech_y))
    mech_x = nx;
  if (ny != mech_y && !mech_blocked(mech_x, ny))
    mech_y = ny;

  if (mech_x < 0)
    mech_x = 0;
  if (mech_y < 0)
    mech_y = 0;
  if (mech_x > LEVEL_W - MECH_W)
    mech_x = LEVEL_W - MECH_W;
  if (mech_y > LEVEL_H - MECH_H)
    mech_y = LEVEL_H - MECH_H;

  level_update_camera(mech_x + MECH_W / 2, mech_y + MECH_H / 2);

  NF_MoveSprite(SCR_WORLD, SPRITE_ID, mech_x - level_cam_x(),
                mech_y - level_cam_y());
  NF_SpriteFrame(SCR_WORLD, SPRITE_ID, frame_cnt / 30);
  if (last_mech_x != mech_x)
    NF_HflipSprite(SCR_WORLD, SPRITE_ID, last_mech_x > mech_x);
}
