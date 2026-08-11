// SPDX-License-Identifier: CC0-1.0

#ifndef SPRITES_H__
#define SPRITES_H__

#include <stdbool.h>

#include <nds.h>

#include "ids.h"

enum SCREENS_TO_LOAD { TOP = 1, BOTTOM = 2, BOTH = TOP | BOTTOM };

typedef struct {
  const char *path;
  uint16_t width;
  uint16_t height;
  uint16_t img_id;
  uint16_t pal_id;
  u16 screens;
} SpriteInfo;

enum {
  MECH = 0,
  RABBITS = 1,
  FIRE = 2,
  WATER_DROPS = 3,
  INDICATOR_ARROW = 4,
  PUMP = 5,
  PUMP_HANDLE = 6,
  SPRITE_CNT
};

#define DEFAULT_PALETTE_PATH "spr/default_sprite"
#define DEFAULT_SPRITE_PALETTE 0
#define PLAYER_PALETTE_CPY SPRITE_CNT

static const SpriteInfo SPRITE_INFOS[SPRITE_CNT] = {
    [MECH] = {"spr/sprite_robot_all", 32, 32, MECH, DEFAULT_SPRITE_PALETTE,
              BOTTOM},
    [RABBITS] = {"spr/sprite_hare_idle", 16, 16, RABBITS,
                 DEFAULT_SPRITE_PALETTE, BOTH},
    [FIRE] = {"spr/sprite_fire_small", 8, 8, FIRE, DEFAULT_SPRITE_PALETTE,
              BOTTOM},
    [WATER_DROPS] = {"spr/sprite_water_drops", 16, 16, WATER_DROPS,
                     DEFAULT_SPRITE_PALETTE, BOTTOM},
    [INDICATOR_ARROW] = {"spr/indicator_arrow", 8, 8, INDICATOR_ARROW,
                         DEFAULT_SPRITE_PALETTE, BOTTOM},
    [PUMP] = {"spr/pump_body", 64, 64, PUMP, DEFAULT_SPRITE_PALETTE, BOTTOM},
    [PUMP_HANDLE] = {"spr/pump_handle", 64, 32, PUMP_HANDLE,
                     DEFAULT_SPRITE_PALETTE, BOTTOM},
};

void InitSprites(void);

#define MECH_OAM_ID 0

#define BUNNIES_OAM_ID 1
#define BUNNIES_MAX 10

#define WATER_DROP_OAM_BASE 11
#define WATER_DROP_MAX 5

#define PUMP_UP_OAM_ID 101
#define PUMP_DOWN_OAM_ID 102
#define PUMP_HANDLE_OAM_ID 100

#define INDICATOR_OAM_BASE 16
#define INDICATOR_MAX BUNNIES_MAX

#endif
