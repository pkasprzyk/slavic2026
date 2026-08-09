// SPDX-License-Identifier: CC0-1.0

#include <stdlib.h>

#include <nds.h>

#include <nf_lib.h>
#include <stdbool.h>

#include "audio.h"
#include "fire.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "spawn.h"
#include "sprites.h"

static const bool DIE_FAST = false;

typedef struct {
  s16 x;
  s16 y;
  s16 oam_id;
  s16 indicator_oam_id;
  s16 hp;
} bunny_s;

static bunny_s bunnies[BUNNIES_MAX];

s16 bunnies_cnt = 0;
int bunnies_total = 0;
s16 bunnies_collected = 0;
s16 bunnies_died = 0;

typedef struct {
  s16 x;
  s16 y;
} chamberSlot;

static const chamberSlot chamberSlots[BUNNIES_MAX] = {
    {128, 82},  {141, 90},  {161, 112}, {161, 140}, {141, 162},
    {128, 170}, {115, 162}, {95, 140},  {95, 112},  {115, 90},
};

static bool chamber_occupied[BUNNIES_MAX];
static s16 chamber_bunny_slot[BUNNIES_MAX];

static u16 frame_cnt = 0;

void add_bunny(int x, int y, s16 hp) {
  bunnies[bunnies_cnt].oam_id = BUNNIES_OAM_ID + bunnies_cnt;
  bunnies[bunnies_cnt].indicator_oam_id = INDICATOR_OAM_BASE + bunnies_cnt;
  bunnies[bunnies_cnt].x = x;
  bunnies[bunnies_cnt].y = y;
  bunnies[bunnies_cnt].hp = hp;

  if (DIE_FAST) {
    bunnies[bunnies_cnt].hp = 5;
  }
  ++bunnies_cnt;
}

void bunnies_init(void) {
  u8 count = spawn_bunny_count();
  for (u8 i = 0; i < count; i++)
    add_bunny(spawn_bunny_x(i), spawn_bunny_y(i), 2700);
  bunnies_total = bunnies_cnt;
  bunnies_died = 0;
  bunnies_collected = 0;
  for (s16 s = 0; s < BUNNIES_MAX; s++)
    chamber_occupied[s] = false;
  for (u16 i = 0; i < bunnies_cnt; ++i) {
    NF_CreateSprite(SCR_WORLD, bunnies[i].oam_id, SPRITE_INFOS[RABBITS].img_id,
                    SPRITE_INFOS[RABBITS].pal_id, bunnies[i].x, bunnies[i].y);
    NF_SpriteLayer(SCR_WORLD, bunnies[i].oam_id, LAYER_WORLD_BG);

    NF_CreateSprite(SCR_WORLD, bunnies[i].indicator_oam_id,
                    SPRITE_INFOS[INDICATOR_ARROW].img_id,
                    SPRITE_INFOS[INDICATOR_ARROW].pal_id, 0, 0);
    NF_SpriteLayer(SCR_WORLD, bunnies[i].indicator_oam_id, LAYER_WORLD_BG);
    NF_ShowSprite(SCR_WORLD, bunnies[i].indicator_oam_id, false);
  }
}

void bunnies_restart(void) {}

bool inSquare(s16 x, s16 y, s16 w, s16 h) {
  if (x < 0 || x > w || y < 0 || y > h)
    return false;
  return true;
}

bool collides_with_mech(u16 i) {
  s16 xOffset = bunnies[i].x - mech_x;
  s16 yOffset = bunnies[i].y - mech_y;
  return inSquare(xOffset, yOffset, 16, 16);
}

s16 distance_sq(s16 x1, s16 y1, s16 x2, s16 y2) {
  return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

bool bunny_in_fire(u16 i) {
  s16 tx = bunnies[i].x >> 3;
  s16 ty = bunnies[i].y >> 3;

  const s16 radius = 4;
  for (s16 x = tx - radius; x < tx + radius + 1; ++x)
    for (s16 y = ty - radius; y < ty + radius + 1; ++y)
      if (fire_is_burning(x, y))
        if (distance_sq((x << 3) + 4, (y << 3) + 4, bunnies[i].x + 8,
                        bunnies[i].y + 8) <= radius * radius * 8 * 8)
          return true;

  return DIE_FAST;
}

static void collect(u16 bunnyId) {
  audio_play_sfx(SFX_SFX_BUNNY_PICK_UP, false, IGNORED_LEN, 190);
  NF_DeleteSprite(SCR_WORLD, bunnies[bunnyId].oam_id);
  NF_DeleteSprite(SCR_WORLD, bunnies[bunnyId].indicator_oam_id);
  bunnies[bunnyId] = bunnies[bunnies_cnt - 1];
  --bunnies_cnt;

  s16 free_slots[BUNNIES_MAX];
  s16 free_cnt = 0;
  for (s16 s = 0; s < BUNNIES_MAX; s++)
    if (!chamber_occupied[s])
      free_slots[free_cnt++] = s;
  s16 slot = free_slots[rand() % free_cnt];
  chamber_occupied[slot] = true;
  chamber_bunny_slot[bunnies_collected] = slot;

  NF_CreateSprite(SCR_CHAMBER, bunnies_collected, SPRITE_INFOS[RABBITS].img_id,
                  SPRITE_INFOS[RABBITS].pal_id, chamberSlots[slot].x,
                  chamberSlots[slot].y);
  NF_EnableSpriteRotScale(SCR_CHAMBER, bunnies_collected, bunnies_collected,
                          false);
  ++bunnies_collected;
}

void collect_all_bunnies()
{
  for (s16 i = 0; i < bunnies_cnt; ++i){
    collect(0);
  }
}

static void kill_bunny(u16 bunnyId, bool silent) {
  if (!silent)
    audio_play_sfx(SFX_SFX_BUNNY_DEATH, false, IGNORED_LEN, 190);
  NF_ShowSprite(SCR_WORLD, bunnies[bunnyId].indicator_oam_id, false);
  NF_DeleteSprite(SCR_WORLD, bunnies[bunnyId].oam_id);
  bunnies[bunnyId] = bunnies[bunnies_cnt - 1];
  --bunnies_cnt;
  ++bunnies_died;
}

void kill_all_bunnies(void) {
  for (s16 i = bunnies_cnt - 1; i >= 0; i--) {
    kill_bunny(i, true);
  }
}

static void compute_edge_indicator(s16 bunny_wx, s16 bunny_wy, s16 *out_x,
                                   s16 *out_y, int *out_frame, bool *out_hflip,
                                   bool *out_vflip) {
  s16 cx = mech_x - level_cam_x();
  s16 cy = mech_y - level_cam_y();
  s16 dx = bunny_wx - level_cam_x() - cx;
  s16 dy = bunny_wy - level_cam_y() - cy;

  if (dx == 0 && dy == 0) {
    *out_x = cx;
    *out_y = cy;
    *out_frame = 2;
    *out_hflip = false;
    *out_vflip = false;
    return;
  }

  s16 best_t = 0x7FFF;

  if (dx < 0) {
    s16 t = ((-cx) << 8) / dx;
    s16 y = cy + ((t * dy) >> 8);
    if (y >= 0 && y < SCREEN_H && t >= 0 && t < best_t)
      best_t = t;
  } else if (dx > 0) {
    s16 t = ((SCREEN_W - 1 - cx) << 8) / dx;
    s16 y = cy + ((t * dy) >> 8);
    if (y >= 0 && y < SCREEN_H && t >= 0 && t < best_t)
      best_t = t;
  }

  if (dy < 0) {
    s16 t = ((-cy) << 8) / dy;
    s16 x = cx + ((t * dx) >> 8);
    if (x >= 0 && x < SCREEN_W && t >= 0 && t < best_t)
      best_t = t;
  } else if (dy > 0) {
    s16 t = ((SCREEN_H - 1 - cy) << 8) / dy;
    s16 x = cx + ((t * dx) >> 8);
    if (x >= 0 && x < SCREEN_W && t >= 0 && t < best_t)
      best_t = t;
  }

  *out_x = cx + ((best_t * dx) >> 8);
  *out_y = cy + ((best_t * dy) >> 8);

  if (*out_x < 0)
    *out_x = 0;
  if (*out_x >= SCREEN_W - 8)
    *out_x = SCREEN_W - 9;
  if (*out_y < 0)
    *out_y = 0;
  if (*out_y >= SCREEN_H - 8)
    *out_y = SCREEN_H - 9;

  s16 adx = dx < 0 ? -dx : dx;
  s16 ady = dy < 0 ? -dy : dy;

  if (ady * 5 < adx * 2) {
    *out_frame = 0;
    *out_hflip = (dx < 0);
    *out_vflip = false;
  } else if (adx * 5 < ady * 2) {
    *out_frame = 1;
    *out_hflip = false;
    *out_vflip = (dy < 0);
  } else {
    *out_frame = 2;
    *out_hflip = (dx < 0);
    *out_vflip = (dy < 0);
  }
}

static void updateBunny(int i) {
  bool isHurt = false;
  if (collides_with_mech(i)) {
    collect(i);
    return;
  }
  if (bunny_in_fire(i)) {
    bunnies[i].hp -= 1;
    isHurt = true;
    if (bunnies[i].hp <= 0) {
      bunnies[i].hp = 0;
      kill_bunny(i, false);
      return;
    }
  }
  s16 phaseSin = sinLerp((frame_cnt - 60) * (32767 / 60)); // 4.12 fixed
  s16 offset = (3 * phaseSin) >> 12;
  if (isHurt) {
    offset = 0;
  }
  s16 x = bunnies[i].x - level_cam_x();
  s16 y = bunnies[i].y - level_cam_y() + offset;
  if (x < -16 || x > 256 || y < -16 || y > 192) { // off screen
    NF_ShowSprite(SCR_WORLD, bunnies[i].oam_id, false);

    s16 ex, ey;
    int frame;
    bool hflip, vflip;
    compute_edge_indicator(bunnies[i].x, bunnies[i].y, &ex, &ey, &frame, &hflip,
                           &vflip);
    NF_ShowSprite(SCR_WORLD, bunnies[i].indicator_oam_id, true);
    NF_MoveSprite(SCR_WORLD, bunnies[i].indicator_oam_id, ex, ey);
    NF_SpriteFrame(SCR_WORLD, bunnies[i].indicator_oam_id, frame);
    NF_HflipSprite(SCR_WORLD, bunnies[i].indicator_oam_id, hflip);
    NF_VflipSprite(SCR_WORLD, bunnies[i].indicator_oam_id, vflip);

    return;
  }
  NF_ShowSprite(SCR_WORLD, bunnies[i].indicator_oam_id, false);

  NF_ShowSprite(SCR_WORLD, bunnies[i].oam_id, true);
  NF_MoveSprite(SCR_WORLD, bunnies[i].oam_id, x, y);
  NF_SpriteFrame(SCR_WORLD, bunnies[i].oam_id, frame_cnt / 30);
  NF_VflipSprite(SCR_WORLD, bunnies[i].oam_id, isHurt);
}

void updateChamberBunnies() {
  for (s16 i = 0; i < bunnies_collected; ++i) {
    s16 phaseSin = sinLerp((frame_cnt - 60) * (32767 / 60));

    s16 si = chamber_bunny_slot[i];
    s16 baseY = chamberSlots[si].y;
    s16 bob_scale = 4 + (baseY - 82) * 6 / 88;
    s16 bob = (bob_scale * phaseSin) >> 12;
    s16 x = chamberSlots[si].x;
    s16 y = baseY + bob;

    s32 scale = 256 + ((y - 126) * 64) / 96;

    NF_ShowSprite(SCR_CHAMBER, i, true);
    NF_MoveSprite(SCR_CHAMBER, i, x, y);
    NF_SpriteFrame(SCR_CHAMBER, i, frame_cnt / 30);
    NF_SpriteRotScale(SCR_CHAMBER, i, 0, scale, scale);
  }
}

void bunnies_update(void) {
  ++frame_cnt;
  frame_cnt %= 60 * 2;
  for (s16 i = bunnies_cnt - 1; i >= 0; --i) {
    updateBunny(i);
  }
  updateChamberBunnies();
}

void bunnies_end_screen_update() {
  ++frame_cnt;
  frame_cnt %= 60 * 2;
  updateChamberBunnies();
}