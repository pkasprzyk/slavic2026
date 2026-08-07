// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include <nf_lib.h>

#include "sprites.h"
#include "ids.h"

void LoadPalette(const char* file, u16 palette_id){
  NF_LoadSpritePal(file, palette_id );
  NF_VramSpritePal(SCR_WORLD,palette_id, palette_id);
}

void LoadGraphicImage(SpriteInfo info){
  u16 imgId = info.img_id;
  NF_LoadSpriteGfx(info.path, imgId, info.width, info.height);
  NF_VramSpriteGfx(SCR_WORLD, imgId, imgId,  true);
}

void InitSprites(void) {
  LoadPalette(SPRITE_INFOS[MECH].path, SPRITE_INFOS[MECH].pal_id);
  LoadPalette(SPRITE_INFOS[RABBITS].path, SPRITE_INFOS[RABBITS].pal_id);

  for (int i = 0; i < SPRITE_CNT; i++) {
    LoadGraphicImage(SPRITE_INFOS[i]);
  }
}

