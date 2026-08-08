// SPDX-License-Identifier: CC0-1.0

#ifndef IDS_H__
#define IDS_H__

#include <stdint.h>

enum {
  SCR_CHAMBER = 0,
  SCR_WORLD = 1,
};

enum {
  LAYER_CHAMBER_REACTOR = 0,
  LAYER_CHAMBER_TEXT = 1,
  LAYER_CHAMBER_BG = 2,
  LAYER_CHAMBER_WATER = 3,
  
  LAYER_WORLD_TEXT = 0,
  LAYER_WORLD_BG = 3,
  LAYER_WORLD_FIRE = 2,
};

enum {
    COLMAP_SLOT = 0,
    TILE_WALL = 1,
    TILE_TREE = 2,
    TILE_BUSH = 3,
    TILE_SHALLOW_WATER = 4,
    TILE_DEEP_WATER = 5,
    TILE_FIRE = 6,
};

#define REACTOR_BG_NAME "reactor"

extern const char* REACTOR_LEVEL_IMG_FILE [4];

#define PATH_BG_FOREST "bg/forest"
#define PATH_COLMAP "maps/colmap"
#define PATH_FONT "fnt/default"

#define BG_FOREST "forest"
#define FONT_NORMAL "normal"

#define BG_CHAMBER "chamber"


#endif
