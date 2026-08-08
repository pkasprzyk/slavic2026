// SPDX-License-Identifier: CC0-1.0

#include <math.h>
#include <nds.h>
#include <stdbool.h>
#include <stdio.h>

#include <nf_lib.h>

#include "audio.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "soundbank.h"
#include "sprites.h"
#include "water.h"

#define MECH_SPEED 8

#define MECH_W 24
#define MECH_H 24
#define MECH_FEET_H 8

#define SPRITE_ID MECH_OAM_ID

enum MECH_ANIM {
  MECH_ANIM_IDLE = 0,
  MECH_ANIM_WALK_DOWN = 2,
  MECH_ANIM_WALK_UP = 4,
  MECH_ANIM_WALK_LEFT = 6,
  MECH_ANIM_WALK_RIGHT = 8,
};

s16 mech_x;
s16 mech_y;
s32 mech_precise_x;
s32 mech_precise_y;
static s32 last_mech_x;
static s32 last_mech_y;
static u8 frame_cnt = 0;
static bool was_in_water = false;
static u8 footstep_sfx = 0;
static s16 footstep_timer = 0;

void mech_init(void) {
  mech_x = 178;
  mech_y = 128;
  mech_precise_x = mech_x << 3;
  mech_precise_y = mech_y << 3;

  NF_CreateSprite(SCR_WORLD, SPRITE_ID, MECH, DEFAULT_SPRITE_PALETTE, mech_x,
                  mech_y);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_BG);
}

static int mech_blocked(s32 x, s32 y) {
  s32 x0 = x >> 6;
  // only collide on feet
  s32 y0 = ((y >> 3) + MECH_H - MECH_FEET_H) >> 3;
  s32 x1 = ((x >> 3) + MECH_W - 1) >> 3;
  s32 y1 = ((y >> 3) + MECH_H - 1) >> 3;

  for (s32 ty = y0; ty <= y1; ty++)
    for (s32 tx = x0; tx <= x1; tx++) {
      int t = NF_GetTile(COLMAP_SLOT, tx << 3, ty << 3);
      if (t == TILE_WALL || t == TILE_TREE || t == TILE_DEEP_WATER)
        return 1;
    }
  return 0;
}

static bool is_in_water(s32 x, s32 y) {
  s32 x0 = x >> 6;
  // only check on feet
  s32 y0 = ((y >> 3) + MECH_H - MECH_FEET_H) >> 3;
  s32 x1 = ((x >> 3) + MECH_W - 1) >> 3;
  s32 y1 = ((y >> 3) + MECH_H - 1) >> 3;

  for (s32 ty = y0; ty <= y1; ty++)
    for (s32 tx = x0; tx <= x1; tx++) {
      int t = NF_GetTile(COLMAP_SLOT, tx << 3, ty << 3);
      if (t == TILE_SHALLOW_WATER)
        return true;
    }
  return false;
}

s32 mech_fire_distance_sq(s16 fire_tx, s16 fire_ty) {
  /* Calculate distance from mech center to fire tile center */
  s16 mech_cx = mech_x + MECH_W / 2;
  s16 mech_cy = mech_y + MECH_H / 2;
  s16 fire_cx = fire_tx * 8 + 4;
  s16 fire_cy = fire_ty * 8 + 4;
  s16 dx = mech_cx - fire_cx;
  s16 dy = mech_cy - fire_cy;
  return (s32)dx * dx + (s32)dy * dy;
}

void mech_spray_water(void) {
  touchPosition touchscreen;
  touchRead(&touchscreen);

  bool touching = touchscreen.px > 0 && touchscreen.py > 0;
  u16 pump_start_x = 192;
  u16 pump_start_y = 60;
  u16 pump_end_x = 256;
  u16 pump_end_y = 132;

  bool touching_pump = false;
  if (was_in_water) {
    touching_pump =
        touchscreen.px >= pump_start_x && touchscreen.px <= pump_end_x &&
        touchscreen.py >= pump_start_y && touchscreen.py <= pump_end_y;
  }

  if (touching && !touching_pump) {
    /* Convert screen touch coords to world tile coords */
    s16 mech_cx = mech_x + MECH_W / 2;
    s16 mech_cy = mech_y + MECH_H / 2;
    water_spray(mech_cx, mech_cy, touchscreen.px + level_cam_x(),
                touchscreen.py + level_cam_y());
  } else if (touching_pump) {
    water_operate_pump(touchscreen.px, touchscreen.py);
  }
}

bool holding_start = false;
void mech_update(void) {
  ++frame_cnt;
  frame_cnt %= 60;

  if (footstep_timer > 0) {
    footstep_timer--;
    if (footstep_timer <= 0) {
      footstep_timer = 0;
    }
  }

  enum MECH_ANIM anim = MECH_ANIM_IDLE;

  u16 held = keysHeld();
  last_mech_x = mech_precise_x;
  last_mech_y = mech_precise_y;
  int dx = 0, dy = 0;

  if (held & KEY_LEFT) {
    dx -= MECH_SPEED;
  }
  if (held & KEY_RIGHT) {
    dx += MECH_SPEED;
  }
  if (held & KEY_UP) {
    dy -= MECH_SPEED;
  }
  if (held & KEY_DOWN) {
    dy += MECH_SPEED;
  }
  if (held & KEY_START) {
    holding_start = true;
  } else if (holding_start) {
    holding_start = false;
    audio_close_wav();
    audio_init_wav("nitro:/audio/SGJ2026-Music-22khz-loop.wav");
  }

  for (int s = 0; s < MECH_SPEED; s++) {
    if (dx > 0 && !mech_blocked(mech_precise_x + 1, mech_precise_y))
      mech_precise_x++;
    if (dx < 0 && !mech_blocked(mech_precise_x - 1, mech_precise_y))
      mech_precise_x--;
    if (dy > 0 && !mech_blocked(mech_precise_x, mech_precise_y + 1))
      mech_precise_y++;
    if (dy < 0 && !mech_blocked(mech_precise_x, mech_precise_y - 1))
      mech_precise_y--;
  }

  if (is_in_water(mech_precise_x, mech_precise_y)) {
    water_show_pump();
    if (!was_in_water) {
      audio_set_looped_volume(SFX_SFX_FIRE_LOOP, 80);
      was_in_water = true;
    }
  } else if (was_in_water) {
    water_hide_pump();
    was_in_water = false;
    audio_set_looped_volume(SFX_SFX_FIRE_LOOP, 170);
  }

  if (mech_precise_x < 0)
    mech_precise_x = 0;
  if (mech_precise_y < 0)
    mech_precise_y = 0;
  if (mech_precise_x > (LEVEL_W - MECH_W) << 3)
    mech_precise_x = (LEVEL_W - MECH_W) << 3;
  if (mech_precise_y > (LEVEL_H - MECH_H) << 3)
    mech_precise_y = (LEVEL_H - MECH_H) << 3;

  if (last_mech_x < mech_precise_x) {
    anim = MECH_ANIM_WALK_RIGHT;
  } else if (last_mech_x > mech_precise_x) {
    anim = MECH_ANIM_WALK_LEFT;
  } else if (last_mech_y < mech_precise_y) {
    anim = MECH_ANIM_WALK_DOWN;
  } else if (last_mech_y > mech_precise_y) {
    anim = MECH_ANIM_WALK_UP;
  }

  mech_x = mech_precise_x >> 3;
  mech_y = mech_precise_y >> 3;

  if (mech_precise_x != last_mech_x || mech_precise_y != last_mech_y) {
    if (footstep_timer <= 0) {
      footstep_sfx = (footstep_sfx + 1) % 2;
      audio_play_sfx(footstep_sfx == 0 ? SFX_SFX_FOOTSTEP_1
                                       : SFX_SFX_FOOTSTEP_2,
                     false, IGNORED_LEN, 210);
      footstep_timer = 30;
    }
  }
  level_update_camera(mech_x + MECH_W / 2, mech_y + MECH_H / 2);

  NF_SpriteFrame(SCR_WORLD, SPRITE_ID, anim + frame_cnt / 30);
  NF_MoveSprite(SCR_WORLD, SPRITE_ID, mech_x - level_cam_x(),
                mech_y - level_cam_y());

  mech_spray_water();
}
