// SPDX-License-Identifier: CC0-1.0

#ifndef SPRITES_H__
#define SPRITES_H__

#include <stdbool.h>

#include <nds.h>

#include "ids.h"

bool sprite_create(int screen, const SpriteDef *def, s32 x, s32 y);
void sprite_move(int screen, const SpriteDef *def, s32 x, s32 y);

#endif
