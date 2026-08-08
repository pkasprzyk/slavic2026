
#include <nds.h>
#include <nf_lib.h>
#include <stdio.h>

#include "level.h"
#include "sprites.h"
#include "water.h"

#define MAX_WATER 1000
#define WATER_DROP_AMOUNT 20
#define WATER_FILL_AMOUNT 1

static s16 water_remaining = 500;
static u8 water_fill_timer = 0;

/* Water drop particles */
typedef struct {
  s16 x, y;
  u8 life;
  u8 active;
  u8 frame; /* 0 or 1 for animation */
} water_drop_t;

static water_drop_t water_drops[WATER_DROP_MAX];

void show_water_remaining(void) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "WATER: %04d", water_remaining);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 3, 15, buffer);
}

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

void water_update(void) {
  water_drop_update();
  show_water_remaining();
}

void water_fill_update(void) {
  water_fill_timer++;
  water_fill_timer %= 2;
  if (water_fill_timer == 1)
    return;
  water_remaining += WATER_FILL_AMOUNT;
  if (water_remaining > MAX_WATER)
    water_remaining = MAX_WATER;
}

void water_drop_spawn(s16 x, s16 y) {
  if (water_remaining > 0) {
    water_remaining -= WATER_DROP_AMOUNT;
    if (water_remaining < 0)
      water_remaining = 0;
  }

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
