// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024

// There are two main ways to animate a sprite: we can either load all frames
// to VRAM from the start (high VRAM usage, low CPU usage), or we can load one
// frame and replace it by new ones whenever we want to change it (low VRAM
// usage, high CPU usage). This example shows how to animate a sprite with both
// systems, as they are both important depending on the situation.

#include <nds.h>

#include "advnt.h" // advnt.png 
#include "forest_town.h" // forest_town.png


int bg;
int bgsub;
u16 *gfxOneFrame;
u16 *gfxAllFrames[6];

void loadBG();
void loadSprites();

// Copy the requested frame of the advnt sprite to the provided address.
void copy_sprite_frame(void *dst, int frame)
{
    uint32_t frame_size = 32 * 64;
    uint32_t offset = frame_size * frame;
    uint8_t *base = (uint8_t *)advntTiles;

    memcpy(dst, base + offset, frame_size);
}

int main(int argc, char *argv[])
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_MAIN_BG,
                        VRAM_B_MAIN_SPRITE,
                        VRAM_C_SUB_BG,
                        VRAM_D_SUB_SPRITE);

    oamInit(&oamMain, SpriteMapping_1D_32, false);
    oamInit(&oamSub, SpriteMapping_1D_32, false);

    loadSprites();
    loadBG();

    int frame = 0;
    int delay = 0;
    int x = 0, y = 0;

    while (1)
    {
        swiWaitForVBlank();

        bgSetScroll(bg, x, y);
        bgSetScroll(bgsub, x, y);

        bgUpdate();

        oamUpdate(&oamMain);
        oamUpdate(&oamSub);

        delay++;
        if (delay > 20)
        {
            delay = 0;

            frame++;
            if (frame > 5)
                frame = 0;

            // Copy a new frame for the pointer in the main screen
            copy_sprite_frame(gfxOneFrame, frame);

            // Point the sprite in the sub screen to a new pre-loaded frame
            oamSetGfx(&oamSub, 0, SpriteSize_32x64, SpriteColorFormat_256Color,
                      gfxAllFrames[frame]);
        }

        scanKeys();

        u16 keys_held = keysHeld();
        if (keys_held & KEY_UP)
            y--;
        else if (keys_held & KEY_DOWN)
            y++;

        if (keys_held & KEY_LEFT)
            x--;
        else if (keys_held & KEY_RIGHT)
            x++;
       
    }

    oamFreeGfx(&oamMain, gfxOneFrame);
    for (int i = 0; i < 6; i++)
        oamFreeGfx(&oamSub, gfxAllFrames[i]);

    return 0;
}

void loadBG()
{
    bg = bgInitHidden(0, BgType_Text8bpp, BgSize_T_256x256, 0,1);

    memcpy(bgGetGfxPtr(bg), forest_townTiles, forest_townTilesLen);
    memcpy(bgGetMapPtr(bg), forest_townMap, forest_townMapLen);
    memcpy(BG_PALETTE, forest_townPal, forest_townPalLen);

    bgsub = bgInitHiddenSub(0, BgType_Text8bpp, BgSize_T_256x256, 0,1);

    memcpy(bgGetGfxPtr(bgsub), forest_townTiles, forest_townTilesLen);
    memcpy(bgGetMapPtr(bgsub), forest_townMap, forest_townMapLen);
    memcpy(BG_PALETTE_SUB, forest_townPal, forest_townPalLen);

    bgShow(bg);
    bgShow(bgsub);

}

void loadSprites()
{
    // Load palette
    memcpy(SPRITE_PALETTE, advntPal, advntPalLen);
    memcpy(SPRITE_PALETTE_SUB, advntPal, advntPalLen);

    // The sprite on the main screen will only have one frame in VRAM
    gfxOneFrame = oamAllocateGfx(&oamMain, SpriteSize_32x64,
                                      SpriteColorFormat_256Color);
    copy_sprite_frame(gfxOneFrame, 0);

    // The sprite on the sub screen will keep all frames in VRAM
    for (int i = 0; i < 6; i++)
    {
        gfxAllFrames[i] = oamAllocateGfx(&oamSub, SpriteSize_32x64,
                                         SpriteColorFormat_256Color);
        copy_sprite_frame(gfxAllFrames[i], i);
    }

    oamSet(&oamMain, 0,
           100, 50, // X, Y
           0, // Priority
           0, // Palette index
           SpriteSize_32x64, SpriteColorFormat_256Color, // Size, format
           gfxOneFrame,  // Graphics offset
           -1, // Affine index
           false, // Double size
           false, // Hide
           false, false, // H flip, V flip
           false); // Mosaic

    oamSet(&oamSub, 0,
           150, 70, // X, Y
           0, // Priority
           0, // Palette index
           SpriteSize_32x64, SpriteColorFormat_256Color, // Size, format
           gfxAllFrames[0],  // Graphics offset
           -1, // Affine index
           false, // Double size
           false, // Hide
           false, false, // H flip, V flip
           false); // Mosaic
}