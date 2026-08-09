#include <stdbool.h>
#include <nf_lib.h>

#include "ids.h"
#include "bunny.h"
#include "audio.h"

static bool ending_drawn;

enum Ending {
  GOOD_ENDING,
  MID_ENDING,
  BAD_ENDING,
};
static u16 ending;


void game_over_on_enter()
{
  ending_drawn = false;
  if (bunnies_collected == 0) {
    ending =  BAD_ENDING;
  } else if (bunnies_collected == bunnies_total) {
    ending = GOOD_ENDING;
  } else {
    ending = MID_ENDING;
  }
}

void draw_ending(); // fwd

void game_over_update(){
  if (!ending_drawn){
    draw_ending();
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

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-5, 5, "SO SAD");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-9, 10, buf);
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
    snprintf(buf2, sizeof(buf2), "But %d bunnies didnt make it :(", bunnies_died);
  } else {
    snprintf(buf2, sizeof(buf2), "  But 1 bunny didnt make it :(");
  }
  NF_ClearTextLayer(SCR_CHAMBER, LAYER_CHAMBER_TEXT);

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-4, 5, "THE END");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-12, 10, buf);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-15, 12, buf2);
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

  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-5, 5, "GOOD ENDING");
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 16-12, 10, buf);
  // NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[  EXIT  ]");
  NF_UpdateTextLayers();

  ending_drawn = 1;
}

void draw_ending() {
  audio_close_wav();
  ending_drawn = true;

  //NF_DeleteTiledBg(SCR_CHAMBER, 0); keep text
  NF_DeleteTiledBg(SCR_CHAMBER, 1);
  NF_DeleteTiledBg(SCR_CHAMBER, 2);
  NF_DeleteTiledBg(SCR_CHAMBER, 3);


  NF_DeleteTiledBg(SCR_WORLD, 0);
  NF_DeleteTiledBg(SCR_WORLD, 1);
  NF_DeleteTiledBg(SCR_WORLD, 2);
  NF_DeleteTiledBg(SCR_WORLD, 3);

  swiWaitForVBlank();
  audio_init_wav("nitro:/audio/SGJ2026-Music-Win-Chill-22khz-loop.wav");
  if (ending == BAD_ENDING) {
    draw_bad_ending();
    audio_set_looped_volume(SFX_SFX_FIRE_LOOP, 150);
    audio_play_sfx(SFX_SFX_BUNNY_DEATH, false, IGNORED_LEN, 190);
  } else if (ending == GOOD_ENDING) {
    draw_good_ending();
    audio_stop_looped_sfx(SFX_SFX_FIRE_LOOP);
  } else {
    draw_mid_ending();
    audio_stop_looped_sfx(SFX_SFX_FIRE_LOOP);
  }
}

static bool touch_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
  return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void end_state_update() {
  if (!ending_drawn) {
    draw_ending();
  }
  bunnies_end_screen_update();
  if (keysDown() & KEY_TOUCH) {
    touchPosition touch;
    touchRead(&touch);
    if (touch_in_rect(touch.px, touch.py, 88, 134, 80, 16)){
      // TODO: uncomment when restart is fixed
      //restart_game();
    }
  }
}
