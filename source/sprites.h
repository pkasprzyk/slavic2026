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
  bool is_top_screen;
} SpriteInfo;

enum { MECH = 0, RABBITS = 1, REACTOR_CORE = 2, FIRE = 3, };

#define SPRITE_CNT 4

#define DEFAULT_SPRITE_PALETTE 0

static const SpriteInfo SPRITE_INFOS[SPRITE_CNT] = {
    [MECH] = {"spr/sprite_robot_idle", 32, 32, MECH, DEFAULT_SPRITE_PALETTE, false},
    [RABBITS] = {"spr/sprite_hare_idle", 16, 16, RABBITS, DEFAULT_SPRITE_PALETTE, false},
    [REACTOR_CORE] = {"spr/reactor_core", 32, 32, REACTOR_CORE, DEFAULT_SPRITE_PALETTE, true},
    [FIRE] = {"spr/sprite_fire_small", 8, 8, FIRE, DEFAULT_SPRITE_PALETTE, false},
};

void InitSprites(void);

#endif
