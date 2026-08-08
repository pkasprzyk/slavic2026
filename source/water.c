
#include <nds.h>
#include <nf_lib.h>

#include "level.h"
#include "sprites.h"
#include "water.h"

/* Water drop particles */
typedef struct {
  s16 x, y;
  u8 life;
  u8 active;
  u8 frame; /* 0 or 1 for animation */
} water_drop_t;

static water_drop_t water_drops[WATER_DROP_MAX];

void water_drop_init(void) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    water_drops[i].active = 0;
    NF_CreateSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, WATER_DROPS,
                    DEFAULT_SPRITE_PALETTE, -16, -16);
    NF_SpriteLayer(SCR_WORLD, WATER_DROP_OAM_BASE + i, LAYER_WORLD_BG);
  }
}

void water_drop_update(void) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    if (!water_drops[i].active)
      continue;

    water_drops[i].life--;

    if (water_drops[i].life <= 0) {
      water_drops[i].active = 0;
      NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, -16, -16);
      continue;
    }

    NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i,
                  water_drops[i].x - level_cam_x(),
                  water_drops[i].y - level_cam_y());
  }
}

void water_init(void) { water_drop_init(); }

void water_update(void) { water_drop_update(); }

void water_drop_spawn(s16 x, s16 y) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    if (!water_drops[i].active) {
      water_drops[i].x = x;
      water_drops[i].y = y;
      water_drops[i].life = 20 + (rand() % 10); /* 20..29 frames */
      water_drops[i].active = 1;
      return;
    }
  }
}
