// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <time.h>

#include <nf_lib.h>

#include "audio.h"
#include "game.h"
#include "ids.h"
#include "bunny.h"
#include "level.h"
#include "mech.h"
#include "chamber.h"
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
  NF_InitSpriteSys(1);

  InitSprites();
  level_init();
  mech_init();
  bunnies_init();
  chamber_init();

  NF_LoadTextFont(PATH_FONT, FONT_NORMAL, 256, 256, 0);
  NF_CreateTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, FONT_NORMAL);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "CHILLING MECH");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 4, "D-Pad moves the mech");
  NF_UpdateTextLayers();

  srand(time(NULL));
  // consoleDemoInit();
  audio_init_wav("nitro:/audio/sample3.wav");
}

void game_update(void) {
  mech_update();
  audio_update();
  bunnies_update();
  chamber_update();
  NF_UpdateTextLayers();
}

void game_deinit(void) {
  chamber_deinit();
}
