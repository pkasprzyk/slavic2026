// SPDX-License-Identifier: CC0-1.0

#ifndef IDS_H__
#define IDS_H__

#include <stdint.h>

enum {
  SCR_CHAMBER = 0,
  SCR_WORLD = 1,
};

enum {
  LAYER_CHAMBER_TEXT = 2,
  LAYER_WORLD_BG = 3,
};

enum {
  COLMAP_SLOT = 0,
  TILE_SOLID = 1,
};

#define PATH_BG_FOREST "bg/forest"
#define PATH_COLMAP "maps/colmap"
#define PATH_FONT "fnt/default"

#define BG_FOREST "forest"
#define FONT_NORMAL "normal"

#endif
