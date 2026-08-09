// SPDX-License-Identifier: CC0-1.0

#include <math.h>
#include <nds.h>
#include <stdbool.h>
#include <stdio.h>

#include <nf_lib.h>

#include "audio.h"
#include "bunny.h"
#include "fire.h"
#include "game.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "nds/input.h"
#include "reactor.h"
#include "soundbank.h"
#include "spawn.h"
#include "sprites.h"
#include "water.h"

#define MECH_SPEED 32

#define MECH_W 24
#define MECH_H 24

#define MECH_COLLISION_W 12
#define MECH_COLLISION_OFFSET 6
#define MECH_FEET_H 8

#define PRECISION_BITS 5

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
static s16 timer_to_end = -1;

void mech_init(void) {
  mech_x = spawn_player_x();
  mech_y = spawn_player_y();
  mech_precise_x = mech_x << PRECISION_BITS;
  mech_precise_y = mech_y << PRECISION_BITS;

  NF_CreateSprite(SCR_WORLD, SPRITE_ID, MECH, DEFAULT_SPRITE_PALETTE, mech_x,
                  mech_y);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_FIRE);
}

void mech_restart(void) {}

static int mech_blocked(s32 x, s32 y) {
  s32 x0 = ((x >> PRECISION_BITS) + MECH_COLLISION_OFFSET) >> 3;
  // only collide on feet
  s32 y0 = ((y >> PRECISION_BITS) + MECH_H - MECH_FEET_H) >> 3;
  s32 x1 = ((x >> PRECISION_BITS) + MECH_COLLISION_OFFSET + MECH_COLLISION_W - 1) >> 3;
  s32 y1 = ((y >> PRECISION_BITS) + MECH_H - 1) >> 3;

  for (s32 ty = y0; ty <= y1; ty++)
    for (s32 tx = x0; tx <= x1; tx++) {
      if (fire_is_burning(tx, ty))
        return 1;
      int t = NF_GetTile(COLMAP_SLOT, tx << 3, ty << 3);
      if (t == TILE_WALL || t == TILE_TREE || t == TILE_DEEP_WATER)
        return 1;
    }
  return 0;
}

static bool is_in_water(s32 x, s32 y) {
  s32 x0 = ((x >> PRECISION_BITS) + MECH_COLLISION_OFFSET) >> 3;
  // only check on feet
  s32 y0 = ((y >> PRECISION_BITS) + MECH_H - MECH_FEET_H) >> 3;
  s32 x1 = ((x >> PRECISION_BITS) + MECH_COLLISION_OFFSET + MECH_COLLISION_W - 1) >> 3;
  s32 y1 = ((y >> PRECISION_BITS) + MECH_H - 1) >> 3;

  for (s32 ty = y0; ty <= y1; ty++)
    for (s32 tx = x0; tx <= x1; tx++) {
      int t = NF_GetTile(COLMAP_SLOT, tx << 3, ty << 3);
      if (t == TILE_SHALLOW_WATER)
        return true;
    }
  return false;
}

bool mech_occupies_tile(int tx, int ty) {
  int tile_x = tx * 8;
  int tile_y = ty * 8;
  return mech_x < tile_x + 8 && mech_x + MECH_W > tile_x &&
         mech_y < tile_y + 8 && mech_y + MECH_H > tile_y;
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
  bool touching_pump = false;

  if (was_in_water) {
    if (left_handed_mode)
      touching_pump = touchscreen.px < 64;
    else
      touching_pump = touchscreen.px >= 192;
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

void mech_update(void) {
  if (timer_to_end > 0) {
    timer_to_end--;
    if (timer_to_end <= 0) {
      // LID easter egg
      timer_to_end = 0;
      kill_all_bunnies();
      extinguish_all_flames();
      audio_play_sfx(SFX_SFX_BUNNY_DEATH, false, IGNORED_LEN, 190);
    }
  }

  ++frame_cnt;
  frame_cnt %= 60;

  if (footstep_timer > 0) {
    footstep_timer--;
    if (footstep_timer <= 0) {
      footstep_timer = -2;
    }
  }

  enum MECH_ANIM anim = MECH_ANIM_IDLE;

  u16 held = keysHeld();
  last_mech_x = mech_precise_x;
  last_mech_y = mech_precise_y;
  int dx = 0, dy = 0;

  int speed = MECH_SPEED - heat_speed_penalty;

  /* char buffer[128];
  snprintf(buffer, sizeof(buffer), "SPEEED: %03d", speed);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, 8, buffer); */

  if (left_handed_mode) {
    if (held & KEY_Y)
      dx -= speed;
    if (held & KEY_A)
      dx += speed;
    if (held & KEY_X)
      dy -= speed;
    if (held & KEY_B)
      dy += speed;
  } else {
    if (held & KEY_LEFT)
      dx -= speed;
    if (held & KEY_RIGHT)
      dx += speed;
    if (held & KEY_UP)
      dy -= speed;
    if (held & KEY_DOWN)
      dy += speed;
  }
  if (held & KEY_LID) {
    if (timer_to_end >= -1)
      timer_to_end = 20;
  }

  if ((held & KEY_L) && (held & KEY_R) && (held & KEY_Y))
  {
    collect_all_bunnies();
  }

  for (int s = 0; s < speed; s++) {
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
    if (water_cooling_cooldown <= 0) {
      reactor_cool_from_water();
    }
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
  if (mech_precise_x > (LEVEL_W - MECH_W) << PRECISION_BITS)
    mech_precise_x = (LEVEL_W - MECH_W) << PRECISION_BITS;
  if (mech_precise_y > (LEVEL_H - MECH_H) << PRECISION_BITS)
    mech_precise_y = (LEVEL_H - MECH_H) << PRECISION_BITS;

  if (last_mech_x < mech_precise_x) {
    anim = MECH_ANIM_WALK_RIGHT;
  } else if (last_mech_x > mech_precise_x) {
    anim = MECH_ANIM_WALK_LEFT;
  } else if (last_mech_y < mech_precise_y) {
    anim = MECH_ANIM_WALK_DOWN;
  } else if (last_mech_y > mech_precise_y) {
    anim = MECH_ANIM_WALK_UP;
  }

  mech_x = mech_precise_x >> PRECISION_BITS;
  mech_y = mech_precise_y >> PRECISION_BITS;

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
