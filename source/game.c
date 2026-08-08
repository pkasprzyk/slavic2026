// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <stdio.h>
#include <time.h>

#include <nf_lib.h>
#include <nds.h>

#include "audio.h"
#include "bunny.h"
#include "chamber.h"
#include "fire.h"
#include "game.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "sprites.h"
#include "title.h"
#include "water.h"

int game_state;
static bool ending_drawn;


// static void clear_row_world(int row) {
//   NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 0, row,
//                "                                ");
// }

// static void clear_all_world(void) {
//   for (int r = 0; r < 24; r++)
//     clear_row_world(r);
// }

static void draw_bad_ending(void) {
  NF_LoadTiledBg("bg/bg_bad_ending", "bad_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "bad_ending");
  // char buf[38];
  //clear_all_world();
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 5, "BAD ENDING");
  // snprintf(buf, sizeof(buf), "You failed to save all %d bunnies...", bunnies_died);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 1, 10, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  // NF_UpdateTextLayers();
  ending_drawn = 1;
}

static void draw_mid_ending(void) {
  NF_LoadTiledBg("bg/bg_mid_ending", "mid_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "mid_ending");
  // char buf[38];
  //clear_all_world();
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 5, "BAD ENDING");
  // snprintf(buf, sizeof(buf), "You rescued only %d bunnies...", bunnies_collected);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 1, 10, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  // NF_UpdateTextLayers();
  ending_drawn = 1;
}


static void draw_good_ending(void) {
  NF_LoadTiledBg("bg/bg_good_ending", "good_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "good_ending");
  // char buf[38];
  //clear_all_world();
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 10, 5, "GOOD ENDING");
  // snprintf(buf, sizeof(buf), "All %d bunnies rescued!", bunnies_collected);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 4, 10, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  // NF_UpdateTextLayers();
  ending_drawn = 1;
}

// static bool touch_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
//   return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
// }

void game_return_to_title(void) {
  game_state = GAME_TITLE;
  ending_drawn = 0;
  title_init();
}

void game_start_play(void) {
  REG_DISPCNT_SUB &= ~DISPLAY_BG1_ACTIVE;
  game_state = GAME_PLAYING;
  fire_init();
  water_init();
  mech_init();
  bunnies_init();
  ending_drawn = 0;
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

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "CHILLING MECH");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 4, "D-Pad moves the mech");
  NF_UpdateTextLayers();

  srand(time(NULL));
  audio_init_SB();
  audio_init_wav("nitro:/audio/SGJ2026-Music-22khz-loop.wav");
  audio_play_sfx(SFX_SFX_FIRE_LOOP, true, 180, 170);

  game_state = GAME_TITLE;
  title_init();
}

void draw_ending(){
  ending_drawn = true;
  
  NF_DeleteTiledBg(SCR_WORLD, 0);
  NF_DeleteTiledBg(SCR_WORLD, 1);
  NF_DeleteTiledBg(SCR_WORLD, 2);
  NF_DeleteTiledBg(SCR_WORLD, 3);

  swiWaitForVBlank();

  if (game_state == GAME_BAD_ENDING){
    draw_bad_ending();
  } else if (game_state == GAME_GOOD_ENDING){
    draw_good_ending();
  } else {
    draw_mid_ending();
  }
}

void end_state_update(){
  if (!ending_drawn) {
    draw_ending();
  }
  // if (keysDown() & KEY_TOUCH) {
  //   touchPosition touch;
  //   touchRead(&touch);
  //   if (touch_in_rect(touch.px, touch.py, 88, 134, 80, 16))
  //     game_return_to_title();
  // }
}

void game_update(void) {
  audio_update_wav();
  audio_update_loops();

  if (game_state == GAME_TITLE || game_state == GAME_INSTRUCTIONS) {
    title_update();
    return;
  }

  if (game_state == GAME_BAD_ENDING || game_state == GAME_GOOD_ENDING || game_state == GAME_MID_ENDING) {
    end_state_update();
    return;
  }

  mech_update();
  fire_update();
  water_update();
  bunnies_update();
  chamber_update();
  NF_UpdateTextLayers();

  if (bunnies_cnt == 0) {
    if (bunnies_collected == 0)
    {
      game_state = GAME_BAD_ENDING;
      game_state = GAME_MID_ENDING;
    }
    else if (bunnies_collected == bunnies_total)
    {
      game_state = GAME_GOOD_ENDING;
    }
    else
    {
      game_state = GAME_MID_ENDING;
    }
  }
}

void game_deinit(void) { chamber_deinit(); }
