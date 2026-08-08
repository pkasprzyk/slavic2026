#include "chamber.h"

#include <nf_lib.h>

#include "reactor.h"
#include "ids.h"

void chamber_init(void)
{
    NF_LoadTiledBg("bg/UI_reactor1", "reactor", 256, 256);
    NF_CreateTiledBg(SCR_CHAMBER, LAYER_CHAMBER_REACTOR, "reactor");

    NF_LoadTiledBg("bg/UI_background", "chamber_bg", 256, 256);
    NF_CreateTiledBg(SCR_CHAMBER, LAYER_CHAMBER_BG, "chamber_bg");

    //TEXT in game.c

    NF_LoadTiledBg("bg/UI_water_level", "water_bg", 256, 256);
    NF_CreateTiledBg(SCR_CHAMBER, LAYER_CHAMBER_WATER, "water_bg");

    reactor_init();
}

static s16 water_cnt;

void chamber_update(void) {
    reactor_update();

    ++water_cnt;
    water_cnt %= 95;
}

void chamber_deinit(void) {
    reactor_deinit();
}
