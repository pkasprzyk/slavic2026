// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>
#include <time.h>

#include <nds.h>
#include <filesystem.h>

#include <nf_lib.h>

#include "level.h"

int main(void)
{
    NF_Set2D(0, 0);
    NF_Set2D(1, 0);
    consoleDemoInit();
    printf("\n Chilling Mech\n\n NitroFS init...\n");
    swiWaitForVBlank();

    if (!nitroFSInit(NULL))
    {
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

    level_init();

    NF_LoadTextFont("fnt/default", "normal", 256, 256, 0);
    NF_CreateTextLayer(0, 2, 0, "normal");
    NF_WriteText(0, 2, 2, 2, "CHILLING MECH");
    NF_WriteText(0, 2, 2, 4, "T1.1 skeleton");
    NF_UpdateTextLayers();

    srand(time(NULL));

    while (1)
    {
        scanKeys();

        swiWaitForVBlank();
    }

    return 0;
}
