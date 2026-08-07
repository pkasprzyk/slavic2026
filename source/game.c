// SPDX-License-Identifier: CC0-1.0

#include <time.h>

#include <nf_lib.h>

#include "audio.h"
#include "game.h"
#include "level.h"
#include "mech.h"

void game_init(void)
{
    NF_InitTiledBgBuffers();
    NF_InitTiledBgSys(0);
    NF_InitTiledBgSys(1);

    NF_InitTextSys(0);
    NF_InitSpriteBuffers();
    NF_InitSpriteSys(1);

    level_init();
    mech_init();

    NF_LoadTextFont("fnt/default", "normal", 256, 256, 0);
    NF_CreateTextLayer(0, 2, 0, "normal");
    NF_WriteText(0, 2, 2, 2, "CHILLING MECH");
    NF_WriteText(0, 2, 2, 4, "D-Pad moves the mech");
    NF_UpdateTextLayers();

    srand(time(NULL));

    audio_init();
}

void game_update(void)
{
    mech_update();
}
