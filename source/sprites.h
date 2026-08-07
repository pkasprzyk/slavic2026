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
  uint16_t img_id;
  uint16_t pal_id;
} SpriteInfo;

enum { MECH = 0, RABBITS = 1, FIRE = 2 };

#define SPRITE_CNT 2

#define DEFAULT_SPRITE_PALETTE 0

static const SpriteInfo SPRITE_INFOS[SPRITE_CNT] = {
    [MECH] = {"spr/sprite_robot_idle", 32, 32, MECH, DEFAULT_SPRITE_PALETTE},
    [RABBITS] = {"spr/sprite_hare_idle", 16, 16, RABBITS, DEFAULT_SPRITE_PALETTE},
    //[FIRE] = {"spr/fire", 32, 32, 2, 0}
};

void InitSprites(void);

#endif
