#include <nds.h>
#include <nf_lib.h>

#include "game.h"
#include "ids.h"
#include "title.h"

#define TITLE_SCREEN  0
#define INSTR_SCREEN  1

#define TITLE_LAYER 1

static int screen;
static u16 saved_palette[256];

static void clear_row(int row) {
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 0, row,
               "                                ");
}

static void clear_all(void) {
  for (int r = 0; r < 24; r++)
    clear_row(r);
}

static void load_menu_bg(void) {
  NF_LoadTiledBg("bg/menu_bg", "menu_bg", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, TITLE_LAYER, "menu_bg");
}

static void load_instr_bg(void) {
  NF_LoadTiledBg("bg/instr_bg", "instr_bg", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, TITLE_LAYER, "instr_bg");
}

static void draw_title(void) {
  clear_all();
  load_menu_bg();
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 10, 2, "CHILLING MECH");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 9, 4, "A Slavic Game");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 5, "Jam 2026");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 9, 9, "[   START   ]");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 8, 13, "[ HOW TO PLAY ]");
  NF_UpdateTextLayers();
}

static void draw_instructions(void) {
  clear_all();
  load_instr_bg();
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 10, 2, "HOW TO PLAY");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 5,
               "D-Pad: Move the mech");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 7,
               "Touch: Spray water on fire");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 9,
               "Blow into mic: Cool reactor");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 11,
               "Rescue animals from the");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 12,
               "burning forest!");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 11, 17, "[   BACK   ]");
  NF_UpdateTextLayers();
}

static bool touch_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
  return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void title_init(void) {
  for (int i = 0; i < 256; i++)
    saved_palette[i] = BG_PALETTE_SUB[i];
  screen = TITLE_SCREEN;
  draw_title();
}

void title_update(void) {
  if (!(keysDown() & KEY_TOUCH))
    return;

  touchPosition touch;
  touchRead(&touch);

  if (screen == TITLE_SCREEN) {
    if (touch_in_rect(touch.px, touch.py, 64, 102, 120, 16)) {
      screen = INSTR_SCREEN;
      draw_instructions();
      return;
    }
    if (touch_in_rect(touch.px, touch.py, 72, 70, 104, 16)) {
      clear_all();
      NF_UpdateTextLayers();
      REG_DISPCNT_SUB &= ~DISPLAY_BG1_ACTIVE;
      for (int i = 0; i < 256; i++)
        BG_PALETTE_SUB[i] = saved_palette[i];
      game_state = GAME_PLAYING;
      game_start_play();
      return;
    }
  } else {
    if (touch_in_rect(touch.px, touch.py, 88, 134, 80, 16)) {
      screen = TITLE_SCREEN;
      draw_title();
      return;
    }
  }
}
