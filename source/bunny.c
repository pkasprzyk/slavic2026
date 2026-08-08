// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>
#include <stdbool.h>

#include "level.h"
#include "sprites.h"

typedef struct
{
    s16 x;
    s16 y;
    s16 oam_id;
} bunny_s;

#define BUNNIES_MAX 10
bunny_s bunnies [BUNNIES_MAX];

u16 bunnies_cnt = 0;

#define SCREEN_BOT 1

u16 SPRIFE_DEF_ID  = 1;
u16 VRAM_ID  = 1;
u16 PALETTE_ID  = 1;

u16 OAM_ID  = 1;
u16 frame_cnt = 0;

void add_bunny(int x, int y)
{
    bunnies[bunnies_cnt].oam_id = 1 + bunnies_cnt;
    bunnies[bunnies_cnt].x = x;
    bunnies[bunnies_cnt].y = y;
    ++bunnies_cnt;
}

void bunnies_init(void)
{
    add_bunny(8,8);
    add_bunny(224,24);
    add_bunny(80,132);
    add_bunny(16,200);
    add_bunny(224,224);
    for (u16 i = 0; i < bunnies_cnt; ++i)
    {
        NF_CreateSprite(SCREEN_BOT, bunnies[i].oam_id, SPRITE_INFOS[RABBITS].img_id, SPRITE_INFOS[RABBITS].pal_id, bunnies[i].x , bunnies[i].y);
        //NF_SpriteLayer(SCREEN_BOT, 0, 3);
    }
}

void bunnies_update(void)
{
    ++ frame_cnt;
    frame_cnt %= 60*2;
    for (u16 i = 0; i < bunnies_cnt; ++i)
    {
        s16 phaseSin = sinLerp((frame_cnt-60)*(32767/60)); // 4.12 fixed
        s16 offset = (5 * phaseSin) >>12;
        s16 x = bunnies[i].x - level_cam_x();
        s16 y = bunnies[i].y - level_cam_y() + offset;
        if ( x < -16 || x > 256 || y < -16 || y > 192) // off screen
        {
            NF_ShowSprite(SCREEN_BOT, bunnies[i].oam_id, false);
            continue;
        }
        NF_ShowSprite(SCREEN_BOT, bunnies[i].oam_id, true);
        NF_MoveSprite(SCREEN_BOT, bunnies[i].oam_id, x, y);
        NF_SpriteFrame(SCREEN_BOT, bunnies[i].oam_id, frame_cnt / 30);
    }
}
