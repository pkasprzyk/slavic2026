#include <nds.h>
#include <nf_lib.h>

#include "game.h"
#include "ids.h"
#include "title.h"

enum MenuState {
  TITLE_SCREEN = 0,
  INSTRUCTIONS_SCREEN = 1,
  CREDITS_SCREEN = 2
};

static int screen;

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
  NF_CreateTiledBg(SCR_WORLD, LAYER_TITLE_TEXT, "menu_bg");
}

static void load_instr_bg(void) {
  NF_LoadTiledBg("bg/instr_bg", "instr_bg", 256, 256);
  NF_CreateTiledBg(SCR_WORLD, LAYER_TITLE_TEXT, "instr_bg");
}

static void draw_title(void) {
  clear_all();
  load_menu_bg();
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 5, 2, "Red Hot Chilling Bunnies");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 10, 4, "A Slavic Game");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 12, 5, "Jam 2026");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 8, 9, "[    START    ]");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 8, 13, "[ HOW TO PLAY ]");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 8, 17, "[   CREDITS   ]");
  NF_UpdateTextLayers();
}

static void draw_instructions(void) {
  clear_all();
  load_instr_bg();
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 10, 2, "HOW TO PLAY");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 5, "D-Pad: Move the mech");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 7, "Touch: Spray water on fire");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 9,
               "Blow into mic: Cool reactor");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 11,
               "Near water: pump up the jam");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 12, "to fill water tank");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 14, "Rescue animals from the");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 15, "burning forest!");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 9, 17, "[   BACK   ]");
  NF_UpdateTextLayers();
}

static void draw_credits(void) {
  clear_all();
  load_instr_bg();
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 3, "Lukasz \"Zephyr\" Sobczyk");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 5,
               "Mateusz \"Grafiszti\" Choinski");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 7, "Szymon Rzosinski");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 9, "Pikej");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 11, "Justyna Kryscio-Rzosinska");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 2, 13,
               "Kamila \"Yreron\" Chmielowiec");
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 9, 17, "[   BACK   ]");
  NF_UpdateTextLayers();
}

static bool touch_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
  return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void title_init(void) {
  screen = TITLE_SCREEN;
  draw_title();
}

void title_restart(void) {}

void title_update(void) {
  if (!(keysDown() & KEY_TOUCH))
    return;

  touchPosition touch;
  touchRead(&touch);

  if (screen == TITLE_SCREEN) {
    if (touch_in_rect(touch.px, touch.py, 64, 102, 120, 16)) {
      screen = INSTRUCTIONS_SCREEN;
      draw_instructions();
      return;
    }
    if (touch_in_rect(touch.px, touch.py, 64, 134, 120, 16)) {
      screen = CREDITS_SCREEN;
      draw_credits();
      return;
    }
    if (touch_in_rect(touch.px, touch.py, 64, 70, 120, 16)) {
      clear_all();
      NF_UpdateTextLayers();
      game_start_play();
      return;
    }
  } else if (screen == INSTRUCTIONS_SCREEN) {
    if (touch_in_rect(touch.px, touch.py, 72, 134, 104, 16)) {
      screen = TITLE_SCREEN;
      draw_title();
      return;
    }
  } else if (screen == CREDITS_SCREEN) {
    if (touch_in_rect(touch.px, touch.py, 72, 134, 104, 16)) {
      screen = TITLE_SCREEN;
      draw_title();
      return;
    }
  }
}
