// SPDX-License-Identifier: CC0-1.0

#include <math.h>
#include <nds.h>
#include <stdbool.h>

#include <nf_lib.h>

#include "fire.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"

#define MECH_SPEED 2

#define MECH_W 24
#define MECH_H 24
#define MECH_FEET_H 8

#define MECH_MAX_WATER 200

#define SPRITE_ID MECH_OAM_ID

#define WATER_SPRAY_DISTANCE 50
#define FIRE_EXTINGUISH_FRAMES 30

#define WATER_DROP_INTERVAL 5

s16 mech_x;
s16 mech_y;
static s16 last_mech_x;
static s16 last_mech_y;
static u8 mech_water_remaining = 0;
static u8 frame_cnt = 0;
static bool flipped;
static u16 extinguish_frame_cnt = 0;
static bool extinguish_target_active = false;
static s16 extinguish_tx = -1;
static s16 extinguish_ty = -1;

/* Water drop particles */
typedef struct {
  s16 x, y;
  u8 life;
  u8 active;
  u8 frame; /* 0 or 1 for animation */
} water_drop_t;

static water_drop_t water_drops[WATER_DROP_MAX];

static void water_drop_spawn(s16 x, s16 y) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    if (!water_drops[i].active) {
      water_drops[i].x = x;
      water_drops[i].y = y;
      water_drops[i].life = 20 + (rand() % 10); /* 20..29 frames */
      water_drops[i].active = 1;
      return;
    }
  }
}

static void water_drop_update(void) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    if (!water_drops[i].active)
      continue;

    water_drops[i].life--;

    if (water_drops[i].life <= 0) {
      water_drops[i].active = 0;
      NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, -16, -16);
      continue;
    }

    NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i,
                  water_drops[i].x - level_cam_x(),
                  water_drops[i].y - level_cam_y());
  }
}

static void water_drop_init(void) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    water_drops[i].active = 0;
    NF_CreateSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, WATER_DROPS,
                    DEFAULT_SPRITE_PALETTE, -16, -16);
    NF_SpriteLayer(SCR_WORLD, WATER_DROP_OAM_BASE + i, LAYER_WORLD_BG);
  }
}

void mech_init(void) {
  mech_x = 178;
  mech_y = 128;
  mech_water_remaining = MECH_MAX_WATER;

  NF_CreateSprite(SCR_WORLD, SPRITE_ID, MECH, DEFAULT_SPRITE_PALETTE, mech_x,
                  mech_y);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_BG);

  water_drop_init();
}

static int mech_blocked(s32 x, s32 y) {
  s32 x0 = x >> 3;
  // only collide on feet
  s32 y0 = (y + MECH_H - MECH_FEET_H) >> 3;
  s32 x1 = (x + MECH_W - 1) >> 3;
  s32 y1 = (y + MECH_H - 1) >> 3;

  for (s32 ty = y0; ty <= y1; ty++)
    for (s32 tx = x0; tx <= x1; tx++) {
      int t = NF_GetTile(COLMAP_SLOT, tx << 3, ty << 3);
      if (t == TILE_WALL || t == TILE_TREE || t == TILE_DEEP_WATER)
        return 1;
    }
  return 0;
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

  bool touching = touchscreen.px >= 0 && touchscreen.py >= 0;
  bool should_spray = false;

  /* Check if touching a tile with fire */
  if (touching) {
    /* Convert screen touch coords to world tile coords */
    int screen_tx = touchscreen.px + level_cam_x();
    int screen_ty = touchscreen.py + level_cam_y();
    int tile_tx = screen_tx >> 3; /* divide by 8 */
    int tile_ty = screen_ty >> 3;

    if (fire_is_burning(tile_tx, tile_ty)) {
      s32 dist_sq = mech_fire_distance_sq(tile_tx, tile_ty);

      if (dist_sq < (WATER_SPRAY_DISTANCE * WATER_SPRAY_DISTANCE)) {
        should_spray = true;
      }
    }
  }

  if (should_spray) {
    if (!extinguish_target_active) {
      /* Determine which fire tile we're targeting */
      int screen_tx = touchscreen.px + level_cam_x();
      int screen_ty = touchscreen.py + level_cam_y();
      extinguish_tx = screen_tx >> 3;
      extinguish_ty = screen_ty >> 3;
      extinguish_frame_cnt = 0;
      extinguish_target_active = true;
    }

    /* Spawn water drops to form a line from mech to target */
    if (extinguish_frame_cnt % WATER_DROP_INTERVAL == 0) {
      s16 mech_cx = mech_x + MECH_W / 2;
      s16 mech_cy = mech_y + MECH_H / 2;
      s16 target_cx = extinguish_tx * 8 + 4;
      s16 target_cy = extinguish_ty * 8 + 4;

      /* Interpolate position along the line */
      float t = (float)extinguish_frame_cnt / (float)FIRE_EXTINGUISH_FRAMES;
      s16 spawn_x = mech_cx + (target_cx - mech_cx) * t;
      s16 spawn_y = mech_cy + (target_cy - mech_cy) * t;

      water_drop_spawn(spawn_x - 8, spawn_y - 8);
    }

    extinguish_frame_cnt++;
    if (extinguish_frame_cnt >= FIRE_EXTINGUISH_FRAMES) {
      fire_extinguish(extinguish_tx, extinguish_ty);
      extinguish_target_active = false;
    }
  } else {
    if (extinguish_target_active) {
      /* Lost target or no longer spraying — reset counter */
      extinguish_frame_cnt = 0;
      extinguish_target_active = false;
    }
  }
}

void mech_update(void) {
  ++frame_cnt;
  frame_cnt %= 60;

  u16 held = keysHeld();
  last_mech_x = mech_x;
  last_mech_y = mech_y;
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

  for (int s = 0; s < MECH_SPEED; s++) {
    if (dx > 0 && !mech_blocked(mech_x + 1, mech_y))
      mech_x++;
    if (dx < 0 && !mech_blocked(mech_x - 1, mech_y))
      mech_x--;
    if (dy > 0 && !mech_blocked(mech_x, mech_y + 1))
      mech_y++;
    if (dy < 0 && !mech_blocked(mech_x, mech_y - 1))
      mech_y--;
  }

  if (mech_x < 0)
    mech_x = 0;
  if (mech_y < 0)
    mech_y = 0;
  if (mech_x > LEVEL_W - MECH_W)
    mech_x = LEVEL_W - MECH_W;
  if (mech_y > LEVEL_H - MECH_H)
    mech_y = LEVEL_H - MECH_H;

  level_update_camera(mech_x + MECH_W / 2, mech_y + MECH_H / 2);

  NF_SpriteFrame(SCR_WORLD, SPRITE_ID, frame_cnt / 30);
  if (last_mech_x != mech_x) {
    flipped = last_mech_x < mech_x;
    NF_HflipSprite(SCR_WORLD, SPRITE_ID, flipped);
  }
  s16 xOffset = flipped ? -8 : 0;
  NF_MoveSprite(SCR_WORLD, SPRITE_ID, mech_x - level_cam_x() + xOffset,
                mech_y - level_cam_y());

  water_drop_update();
  mech_spray_water();
}
