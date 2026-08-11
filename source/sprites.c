// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include <nf_lib.h>

#include "ids.h"
#include "sprites.h"

void LoadPalette(const char *file, u16 palette_id) {
  NF_LoadSpritePal(file, palette_id);
  NF_VramSpritePal(SCR_WORLD, palette_id, palette_id);
}

void LoadGraphicImage(SpriteInfo info) {
  u16 imgId = info.img_id;
  NF_LoadSpriteGfx(info.path, imgId, info.width, info.height);
  if (info.screens & TOP) {
    NF_VramSpriteGfx(SCR_CHAMBER, imgId, imgId, false);
  } 
  if (info.screens & BOTTOM) {
    NF_VramSpriteGfx(SCR_WORLD, imgId, imgId, false);
  }
}

void InitSprites(void) {
  LoadPalette(DEFAULT_PALETTE_PATH, DEFAULT_SPRITE_PALETTE);
  NF_VramSpritePal(SCR_CHAMBER, DEFAULT_SPRITE_PALETTE, DEFAULT_SPRITE_PALETTE);

  // LoadPalette(SPRITE_INFOS[MECH].path, SPRITE_INFOS[MECH].pal_id);

  for (int i = 0; i < SPRITE_CNT; i++) {
    LoadGraphicImage(SPRITE_INFOS[i]);
  }
}
