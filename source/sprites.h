// SPDX-License-Identifier: CC0-1.0

#ifndef SPRITES_H__
#define SPRITES_H__

#include <stdbool.h>

#include <nds.h>

#include "ids.h"

typedef struct {
  const char *path;
  uint16_t width;
  uint16_t height;
  uint16_t id;
  uint16_t pal_id;
} SpriteInfo;

enum { MECH = 0, RABBITS = 1, FIRE = 2 };

#define SPRITE_CNT 1

static const SpriteInfo SPRITE_INFOS[SPRITE_CNT] = {
    [MECH] = {"spr/mech", 32, 32, 0, 0},
    //[RABBITS] = {"spr/rabbits", 32, 32, 1, 0},
    //[FIRE] = {"spr/fire", 32, 32, 2, 0}
};

void InitSprites(void);

#endif
