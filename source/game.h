// SPDX-License-Identifier: CC0-1.0

#ifndef GAME_H
#define GAME_H

enum GameState {
  GAME_TITLE,
  GAME_PLAYING,
  GAME_OVER,
};

extern int game_state;

void game_init(void);
void game_update(void);
void game_deinit(void);
void game_start_play(void);

#endif
