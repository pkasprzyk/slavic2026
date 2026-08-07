// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include <nf_lib.h>

#include "sprites.h"

void InitSprites(void) {
  NF_LoadSpritePal("spr/mech", 0);
  NF_VramSpritePal(1, 0, 0);
  for (int i = 0; i < SPRITE_CNT; i++) {
    NF_LoadSpriteGfx(SPRITE_INFOS[i].path, SPRITE_INFOS[i].id,
                     SPRITE_INFOS[i].width, SPRITE_INFOS[i].height);
    NF_VramSpriteGfx(1, SPRITE_INFOS[i].id, SPRITE_INFOS[i].id, true);
  }
}