// SPDX-License-Identifier: CC0-1.0

#include <stdlib.h>

#include <nds.h>
#include <nf_lib.h>

#include "fire.h"
#include "ids.h"
#include "level.h"
#include "sprites.h"

typedef struct {
    int tx, ty;
    int burn_life;
    int spread_cooldown;
    u8 active;
} fire_cell_t;

static fire_cell_t cells[FIRE_CELLS_MAX];
static int cell_count;
static u8 grid_state[MAX_GRID_Y][MAX_GRID_X];
static u16 fire_frame_cnt;

static int alloc_cell(void) {
    if (cell_count >= FIRE_CELLS_MAX) return -1;
    for (int i = 0; i < FIRE_CELLS_MAX; i++) {
        if (!cells[i].active) {
            cells[i].active = 1;
            cell_count++;
            return i;
        }
    }
    return -1;
}

static void free_cell(int i) {
    u16 sid = FIRE_OAM_BASE + i;
    NF_MoveSprite(SCR_WORLD, sid, -16, -16);
    cells[i].active = 0;
    cell_count--;
}

void fire_init(void) {
    for (int y = 0; y < MAX_GRID_Y; y++)
        for (int x = 0; x < MAX_GRID_X; x++)
            grid_state[y][x] = 0;

    for (int i = 0; i < FIRE_CELLS_MAX; i++) {
        cells[i].active = 0;
        u16 sid = FIRE_OAM_BASE + i;
        NF_CreateSprite(SCR_WORLD, sid, FIRE, DEFAULT_SPRITE_PALETTE, -16, -16);
        NF_SpriteLayer(SCR_WORLD, sid, LAYER_WORLD_BG);
    }
    cell_count = 0;
    fire_frame_cnt = 0;

    for (int ty = 0; ty < MAX_GRID_Y; ty++)
        for (int tx = 0; tx < MAX_GRID_X; tx++) {
            int t = NF_GetTile(COLMAP_SLOT, tx * 8, ty * 8);
            if (t == TILE_FIRE) {
                int ci = alloc_cell();
                if (ci < 0) break;
                grid_state[ty][tx] = 1;
                cells[ci].tx = tx;
                cells[ci].ty = ty;
                cells[ci].burn_life = FIRE_BURN_DURATION;
                cells[ci].spread_cooldown = FIRE_SPREAD_MIN + (rand() % (FIRE_SPREAD_MAX - FIRE_SPREAD_MIN + 1));
                NF_MoveSprite(SCR_WORLD, FIRE_OAM_BASE + ci, tx * 8, ty * 8);
            }
        }
}

static const s8 dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

void fire_update(void) {
    fire_frame_cnt++;
    int frame = (fire_frame_cnt / FIRE_FRAME_TOGGLE) & 1;

    for (int i = 0; i < FIRE_CELLS_MAX; i++) {
        if (!cells[i].active) continue;

        cells[i].burn_life--;
        if (cells[i].burn_life <= 0) {
            grid_state[cells[i].ty][cells[i].tx] = 2;
            free_cell(i);
            continue;
        }

        cells[i].spread_cooldown--;
        if (cells[i].spread_cooldown <= 0) {
            int d = rand() & 3;
            int nx = cells[i].tx + dirs[d][0];
            int ny = cells[i].ty + dirs[d][1];
            if (nx >= 0 && nx < MAX_GRID_X && ny >= 0 && ny < MAX_GRID_Y && grid_state[ny][nx] == 0) {
                int t = NF_GetTile(COLMAP_SLOT, nx * 8, ny * 8);
                if (t == TILE_TREE || t == TILE_BUSH) {
                    int ci = alloc_cell();
                    if (ci >= 0) {
                        grid_state[ny][nx] = 1;
                        cells[ci].tx = nx;
                        cells[ci].ty = ny;
                        cells[ci].burn_life = FIRE_BURN_DURATION;
                        cells[ci].spread_cooldown = FIRE_SPREAD_MIN + (rand() % (FIRE_SPREAD_MAX - FIRE_SPREAD_MIN + 1));
                        NF_MoveSprite(SCR_WORLD, FIRE_OAM_BASE + ci, nx * 8, ny * 8);
                    }
                }
            }
            cells[i].spread_cooldown = FIRE_SPREAD_MIN + (rand() % (FIRE_SPREAD_MAX - FIRE_SPREAD_MIN + 1));
        }

        NF_MoveSprite(SCR_WORLD, FIRE_OAM_BASE + i, cells[i].tx * 8 - level_cam_x(), cells[i].ty * 8 - level_cam_y());
        NF_SpriteFrame(SCR_WORLD, FIRE_OAM_BASE + i, frame);
    }
}

int fire_is_burning(int tx, int ty) {
    if (tx < 0 || tx >= MAX_GRID_X || ty < 0 || ty >= MAX_GRID_Y) return 0;
    return grid_state[ty][tx] == 1;
}

void fire_extinguish(int tx, int ty) {
    if (tx < 0 || tx >= MAX_GRID_X || ty < 0 || ty >= MAX_GRID_Y) return;
    if (grid_state[ty][tx] != 1) return;
    grid_state[ty][tx] = 0;
    for (int i = 0; i < FIRE_CELLS_MAX; i++) {
        if (cells[i].active && cells[i].tx == tx && cells[i].ty == ty) {
            free_cell(i);
            return;
        }
    }
}
