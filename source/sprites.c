// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include <nf_lib.h>

#include "sprites.h"

bool sprite_create(int screen, const SpriteDef *def, s32 x, s32 y)
{
    if (def->gfx >= SLOTS_RAM_GFX || def->pal >= SLOTS_RAM_PAL ||
        def->vgfx >= SLOTS_VRAM_GFX || def->vpal >= SLOTS_VRAM_PAL ||
        def->sprite >= SLOTS_SPRITE) {
        printf("sprite slot out of range: %u/%u/%u/%u/%u\n", def->gfx, def->pal,
               def->vgfx, def->vpal, def->sprite);
        return false;
    }

    if (!NF_SPR256GFX[def->gfx].available) {
        printf("gfx slot %u already in use\n", def->gfx);
        return false;
    }
    if (!NF_SPR256PAL[def->pal].available) {
        printf("pal slot %u already in use\n", def->pal);
        return false;
    }
    if (NF_SPR256VRAM[screen][def->vgfx].inuse) {
        printf("vram gfx slot %u already in use\n", def->vgfx);
        return false;
    }
    if (NF_SPRPALSLOT[screen][def->vpal].inuse) {
        printf("vram pal slot %u already in use\n", def->vpal);
        return false;
    }
    if (NF_SPRITEOAM[screen][def->sprite].created) {
        printf("sprite id %u already in use\n", def->sprite);
        return false;
    }

    NF_LoadSpriteGfx(def->path, def->gfx, def->width, def->height);
    NF_LoadSpritePal(def->path, def->pal);
    NF_VramSpriteGfx(screen, def->gfx, def->vgfx, true);
    NF_VramSpritePal(screen, def->pal, def->vpal);
    NF_CreateSprite(screen, def->sprite, def->vgfx, def->vpal, x, y);
    NF_SpriteLayer(screen, def->sprite, def->layer);

    return true;
}

void sprite_move(int screen, const SpriteDef *def, s32 x, s32 y)
{
    NF_MoveSprite(screen, def->sprite, x, y);
}

void sprite_hflip(int screen, const SpriteDef *def, bool flip)
{
    NF_HflipSprite(screen, def->sprite, flip);
}
