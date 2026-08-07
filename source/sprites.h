// SPDX-License-Identifier: CC0-1.0

#ifndef SPRITES_H__
#define SPRITES_H__

#include <stdbool.h>

#include <nds.h>

#include "ids.h"

typedef struct
{
    uint16_t vgfx;
    uint16_t vpal;
    uint16_t layer;
} SpriteGfx;

bool sprite_load(int screen, const SpriteDef *def, SpriteGfx *gfx);
int sprite_create(int screen, const SpriteGfx *gfx, s32 x, s32 y);
void sprite_move(int screen, int sprite, s32 x, s32 y);
void sprite_hflip(int screen, int sprite, bool flip);

#endif
