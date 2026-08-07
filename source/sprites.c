// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>
#include <string.h>

#include <nf_lib.h>

#include "sprites.h"

typedef struct
{
    char path[32];
    char palpath[32];
    int screen;
    uint16_t vgfx;
    uint16_t vpal;
    uint16_t layer;
} LoadedGfx;

typedef struct
{
    char path[32];
} LoadedPal;

static LoadedGfx g_loaded[SPR_COUNT * 2];
static int g_loaded_count;
static LoadedPal g_pals[2][SLOTS_VRAM_PAL];

static bool ram_gfx_alloc(uint16_t *slot)
{
    for (uint16_t i = 0; i < SLOTS_RAM_GFX; i++) {
        if (NF_SPR256GFX[i].available) {
            *slot = i;
            return true;
        }
    }
    return false;
}

static bool ram_pal_alloc(uint16_t *slot)
{
    for (uint16_t i = 0; i < SLOTS_RAM_PAL; i++) {
        if (NF_SPR256PAL[i].available) {
            *slot = i;
            return true;
        }
    }
    return false;
}

static bool vram_gfx_alloc(int screen, uint16_t *slot)
{
    for (uint16_t i = 0; i < SLOTS_VRAM_GFX; i++) {
        if (!NF_SPR256VRAM[screen][i].inuse) {
            *slot = i;
            return true;
        }
    }
    return false;
}

static bool vram_pal_alloc(int screen, uint16_t *slot)
{
    for (uint16_t i = 0; i < SLOTS_VRAM_PAL; i++) {
        if (!NF_SPRPALSLOT[screen][i].inuse) {
            *slot = i;
            return true;
        }
    }
    return false;
}

static int loaded_find(int screen, const char *path, const char *palpath)
{
    for (int i = 0; i < g_loaded_count; i++) {
        if (g_loaded[i].screen == screen &&
            strcmp(g_loaded[i].path, path) == 0 &&
            strcmp(g_loaded[i].palpath, palpath) == 0)
            return i;
    }
    return -1;
}

static int pal_find(int screen, const char *path)
{
    for (int i = 0; i < SLOTS_VRAM_PAL; i++) {
        if (g_pals[screen][i].path[0] &&
            strcmp(g_pals[screen][i].path, path) == 0)
            return i;
    }
    return -1;
}

static int sprite_alloc(int screen)
{
    for (int i = 0; i < SLOTS_SPRITE; i++) {
        if (!NF_SPRITEOAM[screen][i].created)
            return i;
    }
    return -1;
}

static void pal_register(int screen, uint16_t vpal, const char *path)
{
    strncpy(g_pals[screen][vpal].path, path,
            sizeof(g_pals[screen][vpal].path) - 1);
    g_pals[screen][vpal].path[sizeof(g_pals[screen][vpal].path) - 1] = '\0';
}

bool sprite_load(int screen, const SpriteDef *def, SpriteGfx *gfx)
{
    const char *palpath = def->pal_path ? def->pal_path : def->path;

    int idx = loaded_find(screen, def->path, palpath);
    if (idx >= 0) {
        gfx->vgfx = g_loaded[idx].vgfx;
        gfx->vpal = g_loaded[idx].vpal;
        gfx->layer = g_loaded[idx].layer;
        return true;
    }

    if (g_loaded_count >= (int)(sizeof(g_loaded) / sizeof(g_loaded[0]))) {
        printf("sprite load table full: %s\n", def->path);
        return false;
    }

    uint16_t vpal;
    idx = pal_find(screen, palpath);
    if (idx >= 0) {
        vpal = idx;
    } else {
        uint16_t rampal;
        if (!ram_pal_alloc(&rampal) || !vram_pal_alloc(screen, &vpal)) {
            printf("no free palette slot for %s\n", palpath);
            return false;
        }
        NF_LoadSpritePal(palpath, rampal);
        NF_VramSpritePal(screen, rampal, vpal);
        pal_register(screen, vpal, palpath);
    }

    uint16_t ramgfx, vgfx;
    if (!ram_gfx_alloc(&ramgfx) || !vram_gfx_alloc(screen, &vgfx)) {
        printf("no free gfx slot for %s\n", def->path);
        return false;
    }
    NF_LoadSpriteGfx(def->path, ramgfx, def->width, def->height);
    NF_VramSpriteGfx(screen, ramgfx, vgfx, false);

    gfx->vgfx = vgfx;
    gfx->vpal = vpal;
    gfx->layer = def->layer;

    LoadedGfx *entry = &g_loaded[g_loaded_count++];
    strncpy(entry->path, def->path, sizeof(entry->path) - 1);
    entry->path[sizeof(entry->path) - 1] = '\0';
    strncpy(entry->palpath, palpath, sizeof(entry->palpath) - 1);
    entry->palpath[sizeof(entry->palpath) - 1] = '\0';
    entry->screen = screen;
    entry->vgfx = vgfx;
    entry->vpal = vpal;
    entry->layer = def->layer;

    return true;
}

int sprite_create(int screen, const SpriteGfx *gfx, s32 x, s32 y)
{
    int sprite = sprite_alloc(screen);
    if (sprite < 0) {
        printf("no free sprite slot on screen %d\n", screen);
        return -1;
    }

    NF_CreateSprite(screen, sprite, gfx->vgfx, gfx->vpal, x, y);
    NF_SpriteLayer(screen, sprite, gfx->layer);
    return sprite;
}

void sprite_move(int screen, int sprite, s32 x, s32 y)
{
    NF_MoveSprite(screen, sprite, x, y);
}

void sprite_hflip(int screen, int sprite, bool flip)
{
    NF_HflipSprite(screen, sprite, flip);
}
