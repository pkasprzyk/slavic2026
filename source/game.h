// SPDX-License-Identifier: CC0-1.0

#ifndef GAME_H
#define GAME_H

enum GameState
{
    GAME_TITLE,
    GAME_INSTRUCTIONS,
    GAME_PLAYING,
    GAME_BAD_ENDING,
    GAME_GOOD_ENDING,
    GAME_MID_ENDING,
};

extern int game_state;

void game_init(void);
void game_update(void);
void game_deinit(void);
void game_start_play(void);
void game_return_to_title(void);

#endif
