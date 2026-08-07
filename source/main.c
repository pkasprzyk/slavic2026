// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <nds.h>
#include <nf_lib.h>


#include "game.h"

int main(void)
{

    game_init();

    while (1) {
        scanKeys();
        game_update();
        NF_SpriteOamSet(1);
        swiWaitForVBlank();
        oamUpdate(&oamSub);
    }
}
