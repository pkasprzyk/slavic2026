// SPDX-License-Identifier: CC0-1.0

#include <stdlib.h>

#include <nds.h>
#include <nf_lib.h>

#include "fire.h"
#include "ids.h"
#include "level.h"
#include "mech.h"
#include "reactor.h"

#define TILE_FIRE1 1
#define TILE_FIRE2 2
#define TILE_BURNED 3

#define INITIAL_HEAT_LEVEL 10

typedef struct {
  int tx, ty;
  int burn_life;
  int heat_level;
  int spread_cooldown;
  u8 active;
} fire_cell_t;

static fire_cell_t cells[FIRE_CELLS_MAX];
static int cell_count;
static u8 grid_state[MAX_GRID_Y][MAX_GRID_X];
static u16 fire_frame_cnt;

static int alloc_cell(void) {
  if (cell_count >= FIRE_CELLS_MAX)
    return -1;
  for (int i = 0; i < FIRE_CELLS_MAX; i++) {
    if (!cells[i].active) {
      cells[i].active = 1;
      cells[i].heat_level = INITIAL_HEAT_LEVEL;
      cell_count++;
      return i;
    }
  }
  return -1;
}

static void free_cell(int i) {
  cells[i].active = 0;
  cell_count--;
}

static void write_fire_tile(int tx, int ty, int tile) {
  NF_SetTileOfMap(SCR_WORLD, LAYER_WORLD_FIRE, tx, ty, tile);
}

void fire_start_cell_fire(int cell_index, int tx, int ty) {
  grid_state[ty][tx] = 1;
  cells[cell_index].tx = tx;
  cells[cell_index].ty = ty;
  cells[cell_index].burn_life = FIRE_BURN_DURATION;
  cells[cell_index].heat_level = INITIAL_HEAT_LEVEL;
  cells[cell_index].spread_cooldown =
      FIRE_SPREAD_MIN + (rand() % (FIRE_SPREAD_MAX - FIRE_SPREAD_MIN + 1));
  write_fire_tile(tx, ty, TILE_FIRE1);
}

void fire_init(void) {
  for (int y = 0; y < MAX_GRID_Y; y++)
    for (int x = 0; x < MAX_GRID_X; x++)
      grid_state[y][x] = 0;

  for (int i = 0; i < FIRE_CELLS_MAX; i++)
    cells[i].active = 0;
  cell_count = 0;
  fire_frame_cnt = 0;

  for (int ty = 0; ty < MAX_GRID_Y; ty++)
    for (int tx = 0; tx < MAX_GRID_X; tx++) {
      int t = NF_GetTile(COLMAP_SLOT, tx * 8, ty * 8);
      if (t == TILE_FIRE) {
        int ci = alloc_cell();
        if (ci < 0)
          break;
        fire_start_cell_fire(ci, tx, ty);
      }
    }

  NF_UpdateVramMap(SCR_WORLD, LAYER_WORLD_FIRE);
}

static const s8 dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

void fire_update(void) {
  fire_frame_cnt++;
  int frame = (fire_frame_cnt / FIRE_FRAME_TOGGLE) & 1;
  int fire_tile = frame ? TILE_FIRE2 : TILE_FIRE1;
  bool dirty = false;

  s32 mech_closest_dist_sq = 0x7FFFFFFF;

  for (int i = 0; i < FIRE_CELLS_MAX; i++) {
    if (!cells[i].active)
      continue;

    cells[i].burn_life--;
    if (cells[i].burn_life <= 0) {
      write_fire_tile(cells[i].tx, cells[i].ty, TILE_BURNED);
      grid_state[cells[i].ty][cells[i].tx] = 2;
      free_cell(i);
      dirty = true;
      continue;
    }

    write_fire_tile(cells[i].tx, cells[i].ty, fire_tile);
    dirty = true;

    cells[i].spread_cooldown--;
    if (cells[i].spread_cooldown <= 0) {
      int d = rand() & 3;
      int nx = cells[i].tx + dirs[d][0];
      int ny = cells[i].ty + dirs[d][1];
      if (nx >= 0 && nx < MAX_GRID_X && ny >= 0 && ny < MAX_GRID_Y &&
          grid_state[ny][nx] == 0) {
        int t = NF_GetTile(COLMAP_SLOT, nx * 8, ny * 8);
        if (t == TILE_TREE || t == TILE_BUSH) {
          int ci = alloc_cell();
          if (ci >= 0) {
            fire_start_cell_fire(ci, nx, ny);
          }
        }
      }
      cells[i].spread_cooldown =
          FIRE_SPREAD_MIN + (rand() % (FIRE_SPREAD_MAX - FIRE_SPREAD_MIN + 1));
    }

    if (fire_heat_cooldown == 0) {
      s32 mech_distance = mech_fire_distance_sq(cells[i].tx, cells[i].ty);
      if (mech_distance < mech_closest_dist_sq) {
        mech_closest_dist_sq = mech_distance;
      }
    }
  }

  if (dirty)
    NF_UpdateVramMap(SCR_WORLD, LAYER_WORLD_FIRE);

  if (fire_heat_cooldown == 0) {
    reactor_heat_from_fire(mech_closest_dist_sq);
  }

  NF_ScrollBg(SCR_WORLD, LAYER_WORLD_FIRE, level_cam_x(), level_cam_y());
}

int fire_is_burning(int tx, int ty) {
  if (tx < 0 || tx >= MAX_GRID_X || ty < 0 || ty >= MAX_GRID_Y)
    return 0;
  return grid_state[ty][tx] == 1;
}

void fire_extinguish(int tx, int ty) {
  if (tx < 0 || tx >= MAX_GRID_X || ty < 0 || ty >= MAX_GRID_Y)
    return;
  if (grid_state[ty][tx] != 1)
    return;
  grid_state[ty][tx] = 0;
  write_fire_tile(tx, ty, 0);
  NF_UpdateVramMap(SCR_WORLD, LAYER_WORLD_FIRE);
  for (int i = 0; i < FIRE_CELLS_MAX; i++) {
    if (cells[i].active && cells[i].tx == tx && cells[i].ty == ty) {
      free_cell(i);
      return;
    }
  }
}

void fire_partial_extinguish(int tx, int ty, int value) {
  if (tx < 0 || tx >= MAX_GRID_X || ty < 0 || ty >= MAX_GRID_Y)
    return;
  if (grid_state[ty][tx] != 1)
    return;
  for (int i = 0; i < FIRE_CELLS_MAX; i++) {
    if (cells[i].active && cells[i].tx == tx && cells[i].ty == ty) {
      cells[i].heat_level -= value;
      if (cells[i].heat_level <= 0) {
        grid_state[ty][tx] = 0;
        write_fire_tile(tx, ty, 0);
        NF_UpdateVramMap(SCR_WORLD, LAYER_WORLD_FIRE);
        free_cell(i);
      }
      return;
    }
  }
}
