// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include <filesystem.h>
#include <nds.h>

#include <nf_lib.h>

#include "game.h"

int main(void)
{
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    consoleDemoInit();
    printf("\n Chilling Mech\n\n NitroFS init...\n");
    swiWaitForVBlank();

    if (!nitroFSInit(NULL)) {
        perror("nitroFSInit()");
        while (1)
            swiWaitForVBlank();
    }
    NF_SetRootFolder("NITROFS");

    NF_Set2D(0, 0);
    NF_Set2D(1, 0);

    game_init();

    while (1) {
        scanKeys();
        game_update();
        NF_SpriteOamSet(1);
        swiWaitForVBlank();
        oamUpdate(&oamSub);
    }
}
