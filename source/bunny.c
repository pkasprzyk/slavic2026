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
} bunny_s;

static bunny_s bunnies[BUNNIES_MAX];

s16 bunnies_cnt = 0;
int bunnies_total = 0;
int bunnies_collected = 0;

#define SCREEN_BOT 1
static u16 frame_cnt = 0;

void add_bunny(int x, int y) {
  bunnies[bunnies_cnt].oam_id = BUNNIES_OAM_ID + bunnies_cnt;
  bunnies[bunnies_cnt].x = x;
  bunnies[bunnies_cnt].y = y;
  ++bunnies_cnt;
}

void bunnies_init(void) {
  bunnies_collected = 0;
  add_bunny(8, 8);
  add_bunny(224, 24);
  add_bunny(80, 132);
  add_bunny(16, 200);
  add_bunny(224, 224);
  bunnies_total = bunnies_cnt;
  for (u16 i = 0; i < bunnies_cnt; ++i) {
    NF_CreateSprite(SCREEN_BOT, bunnies[i].oam_id, SPRITE_INFOS[RABBITS].img_id,
                    SPRITE_INFOS[RABBITS].pal_id, bunnies[i].x, bunnies[i].y);
    NF_SpriteLayer(SCREEN_BOT, bunnies[i].oam_id, LAYER_WORLD_BG);
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
  NF_DeleteSprite(SCREEN_BOT, bunnies[bunnyId].oam_id);
  bunnies[bunnyId] = bunnies[bunnies_cnt - 1];
  --bunnies_cnt;
  ++bunnies_collected;
  return true;
}

void bunnies_update(void) {
  ++frame_cnt;
  frame_cnt %= 60 * 2;
  for (s16 i = bunnies_cnt - 1; i >= 0; --i) {
    if (collides_with_mech(i)) {
      collect(i);
      continue;
    }
    s16 phaseSin = sinLerp((frame_cnt - 60) * (32767 / 60)); // 4.12 fixed
    s16 offset = (5 * phaseSin) >> 12;
    s16 x = bunnies[i].x - level_cam_x();
    s16 y = bunnies[i].y - level_cam_y() + offset;
    if (x < -16 || x > 256 || y < -16 || y > 192) // off screen
    {
      NF_ShowSprite(SCREEN_BOT, bunnies[i].oam_id, false);
      continue;
    }
    NF_ShowSprite(SCREEN_BOT, bunnies[i].oam_id, true);
    NF_MoveSprite(SCREEN_BOT, bunnies[i].oam_id, x, y);
    NF_SpriteFrame(SCREEN_BOT, bunnies[i].oam_id, frame_cnt / 30);
  }
}
