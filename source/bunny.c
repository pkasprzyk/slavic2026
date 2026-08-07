// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>

#include "level.h"

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
    NF_LoadSpriteGfx("spr/sprite_hare_idle", SPRIFE_DEF_ID, 16, 16);
    NF_LoadSpritePal("spr/sprite_hare_idle", SPRIFE_DEF_ID);
    NF_VramSpriteGfx(SCREEN_BOT, SPRIFE_DEF_ID, VRAM_ID, true);
    NF_VramSpritePal(SCREEN_BOT, SPRIFE_DEF_ID, PALETTE_ID);

    for (u16 i = 0; i < BUNNIES_MAX; ++i){
        add_bunny(i*16, i*16);
    }
    for (u16 i = 0; i < bunnies_cnt; ++i)
    {
        NF_CreateSprite(SCREEN_BOT, bunnies[i].oam_id, VRAM_ID, PALETTE_ID, bunnies[i].x , bunnies[i].y);
        //NF_SpriteLayer(SCREEN_BOT, 0, 3);
    }
}

void bunnies_update(void)
{
    ++ frame_cnt;
    frame_cnt %= 60*2;
    for (u16 i = 0; i < bunnies_cnt; ++i)
    {
        NF_MoveSprite(SCREEN_BOT, bunnies[i].oam_id, bunnies[i].x - level_cam_x(), bunnies[i].y - level_cam_y());
        NF_SpriteFrame(SCREEN_BOT, bunnies[i].oam_id, frame_cnt / 30);
    }
}
