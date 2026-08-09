#include <stdbool.h>
#include <nf_lib.h>

#include "ids.h"
#include "bunny.h"
#include "audio.h"
#include "game.h"
#include "game_over.h"

static bool ending_drawn;

enum Ending {
  GOOD_ENDING,
  MID_ENDING,
  BAD_ENDING,
};
static u16 ending;

static s16 frames_on_end_screen = 0;


void write_press_to_reset(){
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-7, 22, "TOUCH TO RESET");
  NF_UpdateTextLayers();
}


void game_over_on_enter()
{
  frames_on_end_screen = 0;
  ending_drawn = false;
  if (bunnies_collected == 0) {
    ending =  BAD_ENDING;
  } else if (bunnies_collected == bunnies_total) {
    ending = GOOD_ENDING;
  } else {
    ending = MID_ENDING;
  }
}

void game_over_update(){
  ++frames_on_end_screen;
  if (!ending_drawn){
    draw_ending();
  }
  bunnies_end_screen_update();

  if (frames_on_end_screen == 90)
  {
    write_press_to_reset();
  }
  if (frames_on_end_screen > 90 && keysDown() & KEY_TOUCH) {
    restart_game();
  }

}

void game_over_cleanup(){

}

static void draw_bad_ending(void) {
  NF_LoadTiledBg("bg/bg_bad_ending", "bad_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "bad_ending");

  char buf[80];
  snprintf(buf, sizeof(buf), "You failed to save\n\n    any of the %d bunnies...",  bunnies_died);
  NF_ClearTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT);

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-5, 8, "SAD ROBOT");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-9, 12, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  NF_UpdateTextLayers();

  ending_drawn = 1;
}

static void draw_mid_ending(void) {
  NF_LoadTiledBg("bg/bg_mid_ending", "mid_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "mid_ending");

  char buf[50];
  if (bunnies_collected == 1){
    snprintf(buf, sizeof(buf), "  You rescued 1 bunny...");
  } else {
    snprintf(buf, sizeof(buf), "You rescued %d bunnies...", bunnies_collected);
  }
  char buf2[50];
  if (bunnies_died == 1){
    snprintf(buf2, sizeof(buf2), "  But 1 bunny didn't make it :(");
  } else {
    snprintf(buf2, sizeof(buf2), "But %d bunnies didn't make it :(", bunnies_died);
  }
  NF_ClearTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT);

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-4, 2, "THE END");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-12, 4, buf);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-15, 6, buf2);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  NF_UpdateTextLayers();

  ending_drawn = 1;
}

static void draw_good_ending(void) {
  NF_LoadTiledBg("bg/bg_good_ending", "good_ending", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_ENDING_IMAGE, "good_ending");

  char buf[50];
  snprintf(buf, sizeof(buf), "All %d bunnies rescued!", bunnies_collected);
  NF_ClearTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT);

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-8, 3, "CONGRATULATIONS !!!");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-12, 7, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  NF_UpdateTextLayers();

  ending_drawn = 1;
}

void draw_ending() {
  audio_close_wav();
  ending_drawn = true;

  NF_InitSpriteSys(SCR_WORLD);
  NF_InitTiledBgSys(SCR_CHAMBER);
  NF_InitTiledBgSys(SCR_WORLD);
  NF_LoadTextFont(PATH_FONT, FONT_NORMAL, 256, 256, 0);
  NF_CreateTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, FONT_NORMAL);

  swiWaitForVBlank();
  audio_init_wav("nitro:/audio/SGJ2026-Music-Win-Chill-22khz-loop.wav");
  if (ending == BAD_ENDING) {
    draw_bad_ending();
    audio_set_looped_volume(SFX_SFX_FIRE_LOOP, 150);
  } else if (ending == GOOD_ENDING) {
    draw_good_ending();
    audio_stop_looped_sfx(SFX_SFX_FIRE_LOOP);
  } else {
    draw_mid_ending();
    audio_stop_looped_sfx(SFX_SFX_FIRE_LOOP);
  }
}
