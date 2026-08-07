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

#define MECH_MAX_WATER 1000

static s16 mech_x;
static s16 mech_y;
static s16 last_mech_x;
static s16 last_mech_y;
static s16 mech_water_remaining;

void mech_init(void)
{
    mech_x = 128;
    mech_y = 128;
    mech_water_remaining = MECH_MAX_WATER;

    if (!sprite_create(SCR_WORLD, &SPRITES[SPR_MECH], mech_x, mech_y))
        while (1)
            swiWaitForVBlank();
}

static int tile_blocked(s32 x, s32 y)
{
    int t = NF_GetTile(COLMAP_SLOT, x, y);
    return t == TILE_SOLID || t == TILE_WATER;
}

static int mech_blocked(s32 x, s32 y)
{
    if (tile_blocked(x, y))
        return 1;
    if (tile_blocked(x + MECH_W - 1, y))
        return 1;
    if (tile_blocked(x, y + MECH_H - 1))
        return 1;
    if (tile_blocked(x + MECH_W - 1, y + MECH_H - 1))
        return 1;
    return 0;
}

void mech_update(void)
{
    u16 held = keysHeld();
    last_mech_x = mech_x;
    last_mech_y = mech_y;
    int dx = 0, dy = 0;

    if (held & KEY_LEFT)
        dx -= MECH_SPEED;
    if (held & KEY_RIGHT)
        dx += MECH_SPEED;
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

    sprite_move(SCR_WORLD, &SPRITES[SPR_MECH], mech_x - level_cam_x(),
                mech_y - level_cam_y());
    if (last_mech_x != mech_x)
        sprite_hflip(SCR_WORLD, &SPRITES[SPR_MECH], last_mech_x > mech_x);
}
