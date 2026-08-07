// SPDX-License-Identifier: CC0-1.0

#ifndef IDS_H__
#define IDS_H__

#include <stdint.h>

enum
{
    SCR_CHAMBER = 0,
    SCR_WORLD   = 1,
};

enum
{
    LAYER_CHAMBER_TEXT = 2,
    LAYER_WORLD_BG     = 3,
};

enum
{
    COLMAP_SLOT = 0,
    TILE_SOLID  = 1,
};

#define SLOTS_RAM_GFX   256
#define SLOTS_RAM_PAL   64
#define SLOTS_VRAM_GFX  128
#define SLOTS_VRAM_PAL  16
#define SLOTS_SPRITE    128

typedef struct
{
    const char *path;
    uint16_t width;
    uint16_t height;
    uint16_t gfx;
    uint16_t pal;
    uint16_t vgfx;
    uint16_t vpal;
    uint16_t sprite;
    uint16_t layer;
} SpriteDef;

enum
{
    SPR_MECH = 0,
    SPR_COUNT,
};

static const SpriteDef SPRITES[SPR_COUNT] = {
    [SPR_MECH] = { "spr/mech", 32, 32, 0, 0, 0, 0, 0, LAYER_WORLD_BG },
};

_Static_assert(SPR_COUNT <= SLOTS_SPRITE, "sprite map exceeds NFLib slot limit");

#define PATH_BG_FOREST "bg/forest"
#define PATH_COLMAP    "maps/colmap"
#define PATH_FONT      "fnt/default"

#define BG_FOREST   "forest"
#define FONT_NORMAL "normal"

#endif
