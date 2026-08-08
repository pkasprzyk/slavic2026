// SPDX-License-Identifier: CC0-1.0

#include <nds.h>

#include <nf_lib.h>
#include <stdbool.h>

#include "audio.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"


typedef struct {
  s16 x;
  s16 y;
  s16 oam_id;
  s16 indicator_oam_id;
} bunny_s;

static bunny_s bunnies[BUNNIES_MAX];

s16 bunnies_cnt = 0;
int bunnies_total = 0;
s16 bunnies_collected = 0;

typedef struct {
  s16 x;
  s16 y;
} savedBunnySlot;

static savedBunnySlot savedSlots [BUNNIES_MAX] = {
  {28,156},
  {68,159},
  {160,160},
  {205,145},
  {113,4},

  {50,100},
  {60,100},
  {70,100},
  {80,100},
  {90,100},
};

static u16 frame_cnt = 0;

void add_bunny(int x, int y) {
  bunnies[bunnies_cnt].oam_id = BUNNIES_OAM_ID + bunnies_cnt;
  bunnies[bunnies_cnt].indicator_oam_id = INDICATOR_OAM_BASE + bunnies_cnt;
  bunnies[bunnies_cnt].x = x;
  bunnies[bunnies_cnt].y = y;
  ++bunnies_cnt;
}

void bunnies_init(void) {
  add_bunny(8, 8);
  add_bunny(224, 24);
  add_bunny(80, 132);
  add_bunny(16, 200);
  add_bunny(224, 224);
  bunnies_total = bunnies_cnt;
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

bool collect(u16 bunnyId) {
  audio_play_sfx(SFX_SFX_BUNNY_PICK_UP, false, IGNORED_LEN, DEFAULT_VOLUME);
  NF_DeleteSprite(SCR_WORLD, bunnies[bunnyId].oam_id);
  NF_DeleteSprite(SCR_WORLD, bunnies[bunnyId].indicator_oam_id);
  bunnies[bunnyId] = bunnies[bunnies_cnt - 1];
  --bunnies_cnt;

  NF_CreateSprite(SCR_CHAMBER, bunnies_collected, SPRITE_INFOS[RABBITS].img_id,
                SPRITE_INFOS[RABBITS].pal_id, savedSlots[bunnies_collected].x, savedSlots[bunnies_collected].y);
  ++bunnies_collected;

  return true;
}

static void compute_edge_indicator(s16 bunny_wx, s16 bunny_wy,
                                     s16 *out_x, s16 *out_y,
                                     int *out_frame, bool *out_hflip, bool *out_vflip) {
  s16 cx = mech_x - level_cam_x();
  s16 cy = mech_y - level_cam_y();
  s16 dx = bunny_wx - level_cam_x() - cx;
  s16 dy = bunny_wy - level_cam_y() - cy;

  if (dx == 0 && dy == 0) {
    *out_x = cx; *out_y = cy;
    *out_frame = 2; *out_hflip = false; *out_vflip = false;
    return;
  }

  s16 best_t = 0x7FFF;

  if (dx < 0) {
    s16 t = ((-cx) << 8) / dx;
    s16 y = cy + ((t * dy) >> 8);
    if (y >= 0 && y < SCREEN_H && t >= 0 && t < best_t) best_t = t;
  } else if (dx > 0) {
    s16 t = ((SCREEN_W - 1 - cx) << 8) / dx;
    s16 y = cy + ((t * dy) >> 8);
    if (y >= 0 && y < SCREEN_H && t >= 0 && t < best_t) best_t = t;
  }

  if (dy < 0) {
    s16 t = ((-cy) << 8) / dy;
    s16 x = cx + ((t * dx) >> 8);
    if (x >= 0 && x < SCREEN_W && t >= 0 && t < best_t) best_t = t;
  } else if (dy > 0) {
    s16 t = ((SCREEN_H - 1 - cy) << 8) / dy;
    s16 x = cx + ((t * dx) >> 8);
    if (x >= 0 && x < SCREEN_W && t >= 0 && t < best_t) best_t = t;
  }

  *out_x = cx + ((best_t * dx) >> 8);
  *out_y = cy + ((best_t * dy) >> 8);

  if (*out_x < 0) *out_x = 0;
  if (*out_x >= SCREEN_W) *out_x = SCREEN_W - 1;
  if (*out_y < 0) *out_y = 0;
  if (*out_y >= SCREEN_H) *out_y = SCREEN_H - 1;

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

void bunnies_update(void) {
  ++frame_cnt;
  frame_cnt %= 60 * 2;
  for (s16 i = bunnies_cnt - 1; i >= 0; --i) {
    if (collides_with_mech(i)) {
      collect(i);
      continue;
    }
    s16 phaseSin = sinLerp((frame_cnt - 60) * (32767 / 60));
    s16 offset = (3 * phaseSin) >> 12;
    s16 x = bunnies[i].x - level_cam_x();
    s16 y = bunnies[i].y - level_cam_y() + offset;
    if (x < -16 || x > 256 || y < -16 || y > 192) {
      NF_ShowSprite(SCR_WORLD, bunnies[i].oam_id, false);

      s16 ex, ey;
      int frame;
      bool hflip, vflip;
      compute_edge_indicator(bunnies[i].x, bunnies[i].y,
                             &ex, &ey, &frame, &hflip, &vflip);
      NF_ShowSprite(SCR_WORLD, bunnies[i].indicator_oam_id, true);
      NF_MoveSprite(SCR_WORLD, bunnies[i].indicator_oam_id, ex, ey);
      NF_SpriteFrame(SCR_WORLD, bunnies[i].indicator_oam_id, frame);
      NF_HflipSprite(SCR_WORLD, bunnies[i].indicator_oam_id, hflip);
      NF_VflipSprite(SCR_WORLD, bunnies[i].indicator_oam_id, vflip);
      continue;
    }
    NF_ShowSprite(SCR_WORLD, bunnies[i].indicator_oam_id, false);
    NF_ShowSprite(SCR_WORLD, bunnies[i].oam_id, true);
    NF_MoveSprite(SCR_WORLD, bunnies[i].oam_id, x, y);
    NF_SpriteFrame(SCR_WORLD, bunnies[i].oam_id, frame_cnt / 30);
  }
  for (s16 i = 0 ; i < bunnies_collected; ++i) {
    s16 phaseSin = sinLerp((frame_cnt - 60) * (32767 / 60));
    s16 offset = (10 * phaseSin) >> 12;
    s16 x = savedSlots[i].x;
    s16 y = savedSlots[i].y + offset;
    NF_ShowSprite(SCR_CHAMBER, i, true);
    NF_MoveSprite(SCR_CHAMBER, i, x, y);
    NF_SpriteFrame(SCR_CHAMBER, i, frame_cnt / 30);
  }
}
