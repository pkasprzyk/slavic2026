
#include <nds.h>
#include <nf_lib.h>
#include <stdio.h>

#include "fire.h"
#include "level.h"
#include "sprites.h"
#include "water.h"

#define WATER_STREAM_DEBUG 1

#define WATER_SPRAY_DISTANCE 50
#define WATER_SPRAY_INTERVAL 10
#define WATER_DROP_LIFETIME 20

#define WATER_DROP_SIZE 16

int water_spray_cooldown = 0;
int water_drop_cooldown = 0;

#define MAX_WATER 1000
#define WATER_DROP_AMOUNT 15
#define WATER_FILL_AMOUNT 3

static s16 water_remaining = 500;
static u8 water_fill_timer = 0;

/* Water drop particles */
typedef struct {
  s16 x, y;
  u8 life;
  u8 active;
  u8 frame;
} water_drop_t;

static water_drop_t water_drops[WATER_DROP_MAX];

void show_water_remaining(void) {
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "WATER: %04d", water_remaining);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 3, 15, buffer);

  const s16 MAX_SCROLL = 95;
  NF_ScrollBg(SCR_CHAMBER, LAYER_CHAMBER_WATER, 0, MAX_SCROLL * water_remaining/MAX_WATER);

}

void water_drop_init(void) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    water_drops[i].active = 0;
    water_drops[i].frame = 0;
    NF_CreateSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, WATER_DROPS,
                    DEFAULT_SPRITE_PALETTE, -WATER_DROP_SIZE, -WATER_DROP_SIZE);
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
      NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i, -WATER_DROP_SIZE,
                    -WATER_DROP_SIZE);
      NF_SpriteFrame(SCR_WORLD, WATER_DROP_OAM_BASE + i, water_drops[i].frame);
      continue;
    }

    NF_MoveSprite(SCR_WORLD, WATER_DROP_OAM_BASE + i,
                  water_drops[i].x - level_cam_x(),
                  water_drops[i].y - level_cam_y());
    NF_SpriteFrame(SCR_WORLD, WATER_DROP_OAM_BASE + i, water_drops[i].frame);
  }
}

void water_init(void) { water_drop_init(); }

void water_update(void) {
  water_drop_update();
  show_water_remaining();
  if (water_drop_cooldown > 0)
    water_drop_cooldown--;
  if (water_spray_cooldown > 0)
    water_spray_cooldown--;
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

void water_drop_spawn(int x, int y, int frame_number) {
  for (int i = 0; i < WATER_DROP_MAX; i++) {
    if (!water_drops[i].active) {
      water_drops[i].x = x - WATER_DROP_SIZE / 2;
      water_drops[i].y = y - WATER_DROP_SIZE / 2;
      water_drops[i].life = WATER_DROP_LIFETIME;
      water_drops[i].active = 1;
      water_drops[i].frame = frame_number;
      return;
    }
  }
}

void water_spray(int mech_cx, int mech_cy, int target_x, int target_y) {
  // control damage done
  if (water_spray_cooldown > 0) {
    return;
  }

  if (water_remaining >= WATER_DROP_AMOUNT) {
    water_remaining -= WATER_DROP_AMOUNT;
    if (water_remaining < 0)
      water_remaining = 0;
  } else {
    return;
  }

  water_spray_cooldown = WATER_SPRAY_INTERVAL;

  // calculate actual stream hit point based on range
  float dx = target_x - mech_cx;
  float dy = target_y - mech_cy;
  s32 distance = hw_sqrtf(dx * dx + dy * dy);
  if (distance > WATER_SPRAY_DISTANCE) {
    float scale = (float)WATER_SPRAY_DISTANCE / distance;
    distance = WATER_SPRAY_DISTANCE;
    dx = (float)(dx * scale);
    dy = (float)(dy * scale);
    target_x = mech_cx + dx;
    target_y = mech_cy + dy;
  }

  // resolve tile coordinates of the hit point
  int tile_tx = target_x >> 3;
  int tile_ty = target_y >> 3;
  if (fire_is_burning(tile_tx, tile_ty)) {
    fire_partial_extinguish(tile_tx, tile_ty, 1);
  }

  // control animation speed
  if (water_drop_cooldown > 0) {
    return;
  }

  water_drop_cooldown = WATER_DROP_LIFETIME;

  // spawn water particles on the way
  int steps = (distance / WATER_DROP_SIZE) + 1;
  dx /= steps;
  dy /= steps;
  int spawn_x = mech_cx + (int)dx;
  int spawn_y = mech_cy + (int)dy;
  for (int i = 0; i < steps - 1; i++) {
    water_drop_spawn(spawn_x, spawn_y, 0);
    spawn_x += (int)dx;
    spawn_y += (int)dy;
  }
  water_drop_spawn(spawn_x, spawn_y, 1);
}
