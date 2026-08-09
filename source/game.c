// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <stdio.h>
#include <time.h>

#include <nds.h>
#include <nf_lib.h>

#include "audio.h"
#include "bunny.h"
#include "chamber.h"
#include "fire.h"
#include "game.h"
#include "game_over.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "spawn.h"
#include "sprites.h"
#include "title.h"
#include "water.h"

int game_state;
s16 frames_in_end_state;

void game_start_play(void) {
  REG_DISPCNT_SUB &= ~DISPLAY_BG1_ACTIVE;
  game_state = GAME_PLAYING;
  fire_init();
  water_init();
  spawn_load();
  mech_init();
  bunnies_init();
}

void game_init(void) {
  consoleDemoInit();
  if (!nitroFSInit(NULL)) {
    perror("nitroFSInit()");
    while (1)
      swiWaitForVBlank();
  }
  NF_SetRootFolder("NITROFS");

  NF_Set2D(0, 0);
  NF_Set2D(1, 0);

  NF_InitTiledBgBuffers();
  NF_InitTiledBgSys(0);
  NF_InitTiledBgSys(1);

  NF_InitTextSys(0);
  NF_InitTextSys(1);
  NF_InitSpriteBuffers();
  NF_InitSpriteSys(0);
  NF_InitSpriteSys(1);

  InitSprites();
  level_init();
  chamber_init();

  NF_LoadTextFont(PATH_FONT, FONT_NORMAL, 256, 256, 0);
  NF_CreateTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, FONT_NORMAL);
  NF_CreateTextLayer(SCR_WORLD, LAYER_WORLD_TEXT, 0, FONT_NORMAL);

  /* NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "CHILLING MECH");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 4, "D-Pad moves the mech");
  NF_UpdateTextLayers(); */

  srand(time(NULL));
  audio_init_SB();
  audio_init_wav("nitro:/audio/SGJ2026-Music-22khz-loop.wav");
  audio_play_sfx(SFX_SFX_FIRE_LOOP, true, 180, 170);

  game_state = GAME_TITLE;
  title_init();
}

void restart_game() {
  level_restart();
  chamber_restart();
  bunnies_restart();
  mech_restart();
  water_restart();

  NF_ResetSpriteBuffers();
  NF_InitSpriteSys(0);
  NF_InitSpriteSys(1);
  
  game_init();
}

#define PALETTE_COLORS 256
#define FADE_STEPS     16
#define LAYERS         4
static u8 palSnapshot[LAYERS][PALETTE_COLORS][3];

void startFadeOut()
{
    for (int layer = 0; layer < LAYERS; layer++){
      for (int i = 0; i < PALETTE_COLORS; i++)
      {
          NF_BgGetPalColor(SCR_CHAMBER, layer, i,&palSnapshot[layer][i][0], &palSnapshot[layer][i][1], &palSnapshot[layer][i][2]);
      }
    }
}

void fadeOutTo(s16 step)
{
  for (int layer = 0; layer < LAYERS; layer++){
    for (int i = 0; i < PALETTE_COLORS; i++)
    {
        u8 r = (palSnapshot[layer][i][0] * (FADE_STEPS - step)) / FADE_STEPS;
        u8 g = (palSnapshot[layer][i][1] * (FADE_STEPS - step)) / FADE_STEPS;
        u8 b = (palSnapshot[layer][i][2] * (FADE_STEPS - step)) / FADE_STEPS;

        NF_BgEditPalColor(SCR_CHAMBER, layer, i, r, g, b);
    }
    NF_BgUpdatePalette(SCR_CHAMBER, layer);
  }
}

void restoreTextPal()
{
  s16 layer = LAYER_CHAMBER_TEXT;
  s16 step = 0;
  for (int i = 0; i < PALETTE_COLORS; i++)
  {
      u8 r = (palSnapshot[layer][i][0] * (FADE_STEPS - step)) / FADE_STEPS;
      u8 g = (palSnapshot[layer][i][1] * (FADE_STEPS - step)) / FADE_STEPS;
      u8 b = (palSnapshot[layer][i][2] * (FADE_STEPS - step)) / FADE_STEPS;

      NF_BgEditPalColor(SCR_CHAMBER, layer, i, r, g, b);
  }
  NF_BgUpdatePalette(SCR_CHAMBER, layer);
}


void game_update(void) {
  audio_update_wav();
  audio_update_loops();

  if (game_state == GAME_TITLE) {
    title_update();
    return;
  }

  if (game_state == GAME_OVER) {
    game_over_update();
    return;
  }

  mech_update();
  fire_update();
  water_update();
  bunnies_update();
  chamber_update();
  NF_UpdateTextLayers();

  if (bunnies_cnt == 0) {
    if (frames_in_end_state == 0){
      startFadeOut();
    }
    ++frames_in_end_state;
    fadeOutTo(FADE_STEPS * (frames_in_end_state-1) / (60 * 2));
    if (frames_in_end_state > 60 * 2){
      restoreTextPal();
      frames_in_end_state = 0;
      game_state = GAME_OVER;
      game_over_on_enter();
    }
  }
}

void game_deinit(void) { chamber_deinit(); }

