// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>

#include "level.h"
#include "mech.h"

#define MECH_SPEED 2

#define MECH_W 32
#define MECH_H 32

static s16 mech_x;
static s16 mech_y;

void mech_init(void)
{
    NF_LoadSpriteGfx("spr/mech", 0, 32, 32);
    NF_LoadSpritePal("spr/mech", 0);
    NF_VramSpriteGfx(1, 0, 0, true);
    NF_VramSpritePal(1, 0, 0);

    mech_x = (LEVEL_W - MECH_W) / 2;
    mech_y = (LEVEL_H - MECH_H) / 2;

    NF_CreateSprite(1, 0, 0, 0, mech_x, mech_y);
    NF_SpriteLayer(1, 0, 3);
}

void mech_update(void)
{
    u16 held = keysHeld();

    if (held & KEY_UP)
        mech_y -= MECH_SPEED;
    if (held & KEY_DOWN)
        mech_y += MECH_SPEED;
    if (held & KEY_LEFT)
        mech_x -= MECH_SPEED;
    if (held & KEY_RIGHT)
        mech_x += MECH_SPEED;

    if (mech_x < 0)
        mech_x = 0;
    if (mech_y < 0)
        mech_y = 0;
    if (mech_x > LEVEL_W - MECH_W)
        mech_x = LEVEL_W - MECH_W;
    if (mech_y > LEVEL_H - MECH_H)
        mech_y = LEVEL_H - MECH_H;

    level_update_camera(mech_x + MECH_W / 2, mech_y + MECH_H / 2);

    NF_MoveSprite(1, 0, mech_x - level_cam_x(), mech_y - level_cam_y());
}
