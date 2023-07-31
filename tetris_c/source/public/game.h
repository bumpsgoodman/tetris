#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stddef.h>

#include "timer.h"
#include "matrix.h"

#define BOARD_ROWS 24
#define BOARD_COLS 10

#define NUM_OFFSETS 3

typedef enum block
{
    BLOCK_I,
    BLOCK_L,
    BLOCK_J,
    BLOCK_T,
    BLOCK_O,
    BLOCK_S,
    BLOCK_Z,
    NUM_MAX_BLOCKS
} block_t;

typedef enum board_state
{
    BOARD_STATE_SPACE,
    BOARD_STATE_BLOCK,
    BOARD_STATE_FIXED,
    BOARD_STATE_SHADOW
} board_state_t;

typedef struct game
{
    timer_t game_frame_timer;
    timer_t fall_timer;

    board_state_t board[BOARD_ROWS][BOARD_COLS];

    block_t blocks[NUM_MAX_BLOCKS];
    size_t block_index;

    int_vec2_t block_pos;
    int_vec2_t block_offsets[NUM_OFFSETS];
    size_t block_rotation_count;
} game_t;

bool init_game(game_t* p_game);
void update_game(game_t* p_game);
void draw_game(const game_t* p_game);

bool is_running_game(const game_t* p_game);

#endif // GAME_H