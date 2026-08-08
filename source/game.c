// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <time.h>

#include <nf_lib.h>

#include "audio.h"
#include "bunny.h"
#include "chamber.h"
#include "fire.h"
#include "game.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"

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
  NF_InitSpriteBuffers();
  NF_InitSpriteSys(0);
  NF_InitSpriteSys(1);

  InitSprites();
  level_init();
  fire_init();
  mech_init();
  bunnies_init();
  chamber_init();

  NF_LoadTextFont(PATH_FONT, FONT_NORMAL, 256, 256, 0);
  NF_CreateTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, FONT_NORMAL);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "CHILLING MECH");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 4, "D-Pad moves the mech");
  NF_UpdateTextLayers();

  srand(time(NULL));
  audio_init_SB();
  consoleDemoInit();
  audio_init_wav("nitro:/audio/SGJ2026-Music-22khz-loop.wav");
  // audio_play_sfx(SFX_SFX_FIRE_LOOP, true, 1198);
}

void game_update(void) {
  mech_update();
  fire_update();
  bunnies_update();
  chamber_update();
  NF_UpdateTextLayers();
  audio_update_wav();
  audio_update_loops();
}

void game_deinit(void) { chamber_deinit(); }
