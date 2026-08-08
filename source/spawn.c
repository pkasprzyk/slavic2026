#include "spawn.h"
#include "sprites.h"

#include <stdio.h>
#include <stdlib.h>

static u16 s_player_x;
static u16 s_player_y;
static u8  s_bunny_count;
static u16 s_bunny_x[BUNNIES_MAX];
static u16 s_bunny_y[BUNNIES_MAX];
static bool s_loaded = false;

void spawn_load(void) {
  if (s_loaded)
    return;

  FILE *f = fopen("nitro:/data/spawn.dat", "rb");
  if (!f) {
    s_player_x = 200;
    s_player_y = 200;
    s_bunny_count = 5;
    s_bunny_x[0] = 60;  s_bunny_y[0] = 60;
    s_bunny_x[1] = 350; s_bunny_y[1] = 80;
    s_bunny_x[2] = 150; s_bunny_y[2] = 280;
    s_bunny_x[3] = 400; s_bunny_y[3] = 300;
    s_bunny_x[4] = 300; s_bunny_y[4] = 380;
    s_loaded = true;
    return;
  }

  u16 buf[2];
  if (fread(buf, 2, 2, f) != 2)
    goto fail;
  s_player_x = buf[0];
  s_player_y = buf[1];

  u8 count;
  if (fread(&count, 1, 1, f) != 1)
    goto fail;
  if (count > BUNNIES_MAX)
    count = BUNNIES_MAX;
  s_bunny_count = count;

  for (u8 i = 0; i < count; i++) {
    if (fread(buf, 2, 2, f) != 2)
      goto fail;
    s_bunny_x[i] = buf[0];
    s_bunny_y[i] = buf[1];
  }

  fclose(f);
  s_loaded = true;
  return;

fail:
  fclose(f);
  s_player_x = 200;
  s_player_y = 200;
  s_bunny_count = 0;
  s_loaded = true;
}

u16 spawn_player_x(void) { return s_player_x; }
u16 spawn_player_y(void) { return s_player_y; }
u8  spawn_bunny_count(void) { return s_bunny_count; }
u16 spawn_bunny_x(u8 index) { return s_bunny_x[index]; }
u16 spawn_bunny_y(u8 index) { return s_bunny_y[index]; }
