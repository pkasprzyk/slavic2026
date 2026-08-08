// SPDX-License-Identifier: CC0-1.0

#include <nds.h>
#include <stdbool.h>
#include <math.h>

#include <nf_lib.h>

#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"
#include "reactor.h"
#include "fire.h"

#define MECH_SPEED 2

#define MECH_W 24
#define MECH_H 24

#define MECH_MAX_WATER 200

#define SPRITE_ID 0

#define WATER_SPRAY_DISTANCE 50
#define FIRE_EXTINGUISH_FRAMES 30

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

void mech_init(void) {
  mech_x = 178;
  mech_y = 128;
  mech_water_remaining = MECH_MAX_WATER;

  NF_CreateSprite(SCR_WORLD, SPRITE_ID, MECH, DEFAULT_SPRITE_PALETTE, mech_x, mech_y);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_BG);
}

static int mech_blocked(s32 x, s32 y) {
    s32 x0 = x >> 3;
    s32 y0 = y >> 3;
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
        int tile_tx = screen_tx >> 3;   /* divide by 8 */
        int tile_ty = screen_ty >> 3;

        if (fire_is_burning(tile_tx, tile_ty)) {
            /* Calculate distance from mech center to fire tile center */
            s16 mech_cx = mech_x + MECH_W / 2;
            s16 mech_cy = mech_y + MECH_H / 2;
            s16 fire_cx = tile_tx * 8 + 4;
            s16 fire_cy = tile_ty * 8 + 4;
            s16 dx = mech_cx - fire_cx;
            s16 dy = mech_cy - fire_cy;
            s32 dist_sq = (s32)dx * dx + (s32)dy * dy;

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

        // FIXME: animation + water usage

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
        reactor_increase_temp(MOVE_INCREASE_AMOUNT);
    }
    if (held & KEY_RIGHT) {
        dx += MECH_SPEED;
        reactor_increase_temp(MOVE_INCREASE_AMOUNT);
    }
    if (held & KEY_UP)
        dy -= MECH_SPEED;
    if (held & KEY_DOWN)
        dy += MECH_SPEED;

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
  if (last_mech_x != mech_x){
    flipped = last_mech_x < mech_x;
    NF_HflipSprite(SCR_WORLD, SPRITE_ID, flipped);
  }
  s16 xOffset = flipped ? -8 : 0;
  NF_MoveSprite(SCR_WORLD, SPRITE_ID, mech_x - level_cam_x() + xOffset,
                mech_y - level_cam_y());

    mech_spray_water();
}
