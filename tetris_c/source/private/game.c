#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../public/game.h"
#include "../public/assert.h"
#include "../public/console_util.h"

#define MAX(a, b) ((a) >= (b) ? (a) : (b))

typedef enum key_state
{
    KEY_STATE_UP = 0x00,
    KEY_STATE_DOWN = 0x01,
    KEY_STATE_PRESSED = 0x11
} key_state_t;

static const int_vec2_t BLOCK_OFFSETS[][NUM_OFFSETS] =
{
    { { -1, 0 }, { 1, 0 }, { 2, 0 } },          // I
    { { -1, 0 }, { -1, -1 }, { 1, 0 } },        // L
    { { -1, 0 }, { 1, 0 }, { 1, -1 } },         // J
    { { -1, 0 }, { 1, 0 }, { 0, -1 } },         // T
    { { 1, 0 }, { 0, 1 }, { 1, 1 } },           // O
    { { -1, 0 }, { 0, -1 }, { 1, -1 } },        // S
    { { 1, 0 }, { 0, -1 }, { -1, -1 } },        // Z
};

static key_state_t s_left_key_state;
static key_state_t s_right_key_state;
static key_state_t s_up_key_state;
static key_state_t s_down_key_state;
static key_state_t s_space_key_state;

static void init_block(game_t* p_game);
static void make_block_bundle(game_t* p_game);
static void init_block_pos(game_t* p_game);
static void next_block(game_t* p_game);

static void update_key(int v_key, key_state_t* key_state);

static bool try_block_to_board(game_t* p_game, const int_vec2_t* p_pos, const int_vec2_t* p_offsets, const board_state_t state);
static bool rotate(game_t* p_game);
static void remove_line(game_t* p_game);

bool init_game(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    // 게임 전역 초기화
    {
        srand((unsigned int)time(NULL));
        set_visible_cursor(false);
    }

    // 타이머 초기화
    {
        init_timer(&p_game->game_frame_timer, 16.67f);  // 60 프레임
        init_timer(&p_game->fall_timer, 1000.0f);       // 1초에 한 번씩 떨어짐
    }

    // 보드 초기화
    memset(p_game->board, BOARD_STATE_SPACE, sizeof(p_game->board));

    // 블럭 초기화
    init_block(p_game);

    return true;
}

void update_game(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    update_timer(&p_game->game_frame_timer);
    if (!is_on_tick_timer(&p_game->game_frame_timer))
    {
        return;
    }

    // 잔상 지우기
    try_block_to_board(p_game, &p_game->block_pos, p_game->block_offsets, BOARD_STATE_SPACE);

    int_vec2_t next_pos = p_game->block_pos;

    // 떨어지는 주기가 되면 떨어뜨리기
    update_timer(&p_game->fall_timer);
    if (is_on_tick_timer(&p_game->fall_timer))
    {
        ++next_pos.y;
    }

    // 키 입력
    {
        update_key(VK_LEFT, &s_left_key_state);
        update_key(VK_RIGHT, &s_right_key_state);
        update_key(VK_DOWN, &s_down_key_state);
        update_key(VK_UP, &s_up_key_state);
        update_key(VK_SPACE, &s_space_key_state);
    }

    // 키 처리
    {
        if (s_left_key_state == KEY_STATE_DOWN)
        {
            --next_pos.x;
        }

        if (s_right_key_state == KEY_STATE_DOWN)
        {
            ++next_pos.x;
        }

        if (s_down_key_state == KEY_STATE_DOWN)
        {
            ++next_pos.y;
        }

        if (s_up_key_state == KEY_STATE_DOWN)
        {
            if (p_game->blocks[p_game->block_index] != BLOCK_O)
            {
                if (rotate(p_game))
                {
                    next_pos = p_game->block_pos;
                }
            }
        }

        if (s_space_key_state == KEY_STATE_DOWN)
        {
            do
            {
                try_block_to_board(p_game, &next_pos, p_game->block_offsets, BOARD_STATE_SPACE);
                ++next_pos.y;
            } while (try_block_to_board(p_game, &next_pos, p_game->block_offsets, BOARD_STATE_BLOCK));

            p_game->block_pos.y = next_pos.y - 1;
        }
    }

    // 그림자 그리기
    {
        static int_vec2_t shadow_pos;
        static int_vec2_t shadow_offsets[NUM_OFFSETS];

        try_block_to_board(p_game, &shadow_pos, shadow_offsets, BOARD_STATE_SPACE);

        shadow_pos.x = p_game->block_pos.x;
        shadow_pos.y = p_game->block_pos.y;
        memcpy(shadow_offsets, p_game->block_offsets, sizeof(int_vec2_t) * NUM_OFFSETS);
        do
        {
            try_block_to_board(p_game, &shadow_pos, shadow_offsets, BOARD_STATE_SPACE);
            ++shadow_pos.y;
        } while (try_block_to_board(p_game, &shadow_pos, shadow_offsets, BOARD_STATE_SHADOW));
        --shadow_pos.y;
        try_block_to_board(p_game, &shadow_pos, shadow_offsets, BOARD_STATE_SHADOW);
    }

    // 블럭 그리기
    if (!try_block_to_board(p_game, &next_pos, p_game->block_offsets, BOARD_STATE_BLOCK))
    {
        if (s_left_key_state == KEY_STATE_DOWN
            || s_right_key_state == KEY_STATE_DOWN)
        {
            return;
        }

        try_block_to_board(p_game, &p_game->block_pos, p_game->block_offsets, BOARD_STATE_FIXED);

        remove_line(p_game);
        next_block(p_game);

        return;
    }

    p_game->block_pos = next_pos;
}

void draw_game(const game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    if (!is_on_tick_timer(&p_game->game_frame_timer))
    {
        return;
    }

    gotoxy(0, 0);

    for (size_t i = 3; i < BOARD_ROWS; ++i)
    {
        printf("| ");
        for (size_t j = 0; j < BOARD_COLS; ++j)
        {
            switch (p_game->board[i][j])
            {
            case BOARD_STATE_SPACE:
                printf("· ");
                break;
            case BOARD_STATE_BLOCK:
                printf("■ ");
                break;
            case BOARD_STATE_FIXED:
                printf("▨ ");
                break;
            case BOARD_STATE_SHADOW:
                printf("□ ");
                break;
            default:
                break;
            }
        }
        printf("|\n");
    }

    for (size_t i = 0; i < BOARD_COLS + 1; ++i)
    {
        printf(" -");
    }
    printf("\n");
}

bool is_running_game(const game_t* p_game)
{
    for (size_t i = 0; i < BOARD_COLS; ++i)
    {
        if (p_game->board[2][i] == BOARD_STATE_FIXED)
        {
            return false;
        }
    }

    return true;
}

static void init_block(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    make_block_bundle(p_game);
    p_game->block_index = 0;
    p_game->block_rotation_count = 0;

    memcpy(p_game->block_offsets, BLOCK_OFFSETS[p_game->blocks[0]], sizeof(int_vec2_t) * NUM_OFFSETS);

    init_block_pos(p_game);
}

static void make_block_bundle(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    uint8_t flag = 0;
    size_t index = 0;
    do
    {
        const block_t block = rand() % NUM_MAX_BLOCKS;
        const uint8_t mask = (1 << block);
        if ((flag & mask) == 0)
        {
            p_game->blocks[index] = block;
            ++index;

            flag |= mask;
        }
    } while (flag != 0x7f);
}

static void init_block_pos(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    int_vec2_t* p_pos = &p_game->block_pos;
    p_pos->x = BOARD_COLS / 2 - 1;

    switch (p_game->blocks[p_game->block_index])
    {
    case BLOCK_I:
    case BLOCK_O:
        p_pos->y = 2;
        break;
    case BLOCK_S:
    case BLOCK_Z:
    case BLOCK_L:
    case BLOCK_J:
    case BLOCK_T:
        p_pos->y = 3;
        break;
    default:
        ASSERT(false, "Invalid block");
        break;
    }
}

static void next_block(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    p_game->block_rotation_count = 0;

    ++p_game->block_index;
    if (p_game->block_index >= NUM_MAX_BLOCKS)
    {
        init_block(p_game);
    }
    else
    {
        const size_t offset_index = p_game->blocks[p_game->block_index];
        memcpy(p_game->block_offsets, BLOCK_OFFSETS[offset_index], sizeof(int_vec2_t) * NUM_OFFSETS);
        init_block_pos(p_game);
    }

    reset_timer(&p_game->fall_timer);
}

static void update_key(int v_key, key_state_t* key_state)
{
    ASSERT(key_state != NULL, "key_state == NULL");

    if (GetAsyncKeyState(v_key))
    {
        *key_state = (*key_state == KEY_STATE_UP) ? KEY_STATE_DOWN : KEY_STATE_PRESSED;
    }
    else
    {
        *key_state = KEY_STATE_UP;
    }
}

static bool try_block_to_board(game_t* p_game, const int_vec2_t* p_pos, const int_vec2_t* p_offsets, const board_state_t state)
{
    ASSERT(p_game != NULL, "p_game == NULL");
    ASSERT(p_pos != NULL, "p_pos == NULL");
    ASSERT(p_offsets != NULL, "p_offsets == NULL");

    if (p_pos->x < 0 || p_pos->x >= BOARD_COLS
        || p_pos->y >= BOARD_ROWS
        || p_game->board[p_pos->y][p_pos->x] == BOARD_STATE_FIXED)
    {
        return false;
    }

    for (size_t i = 0; i < NUM_OFFSETS; ++i)
    {
        int_vec2_t actual_pos = { 0, };
        actual_pos.x = p_pos->x + p_offsets[i].x;
        actual_pos.y = p_pos->y + p_offsets[i].y;

        if (actual_pos.x < 0 || actual_pos.x >= BOARD_COLS
            || actual_pos.y >= BOARD_ROWS
            || p_game->board[actual_pos.y][actual_pos.x] == BOARD_STATE_FIXED)
        {
            return false;
        }
    }

    // 성공했다면 보드에 그려넣기
    p_game->board[p_pos->y][p_pos->x] = state;
    for (size_t i = 0; i < NUM_OFFSETS; ++i)
    {
        int_vec2_t actual_pos = { 0, };
        actual_pos.x = p_pos->x + p_offsets[i].x;
        actual_pos.y = p_pos->y + p_offsets[i].y;

        p_game->board[actual_pos.y][actual_pos.x] = state;
    }

    return true;
}

static bool rotate(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    static const int_vec2_t JLSZT_TEST[5][4] =
    {
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { -1, 0 }, { 1, 0 }, { 1, 0 }, { -1, 0 } },
        { { -1, 1 }, { 1, -1 }, { 1, 1 }, { -1, -1 } },
        { { 0, -2 }, { 0, 2 }, { 0, -2 }, { 0, 2 } },
        { { -1, -2 }, { 1, 2 }, { 1, -2 }, { -1, 2 } },
    };

    static const int_vec2_t I_TEST[5][4] =
    {
        { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
        { { -2, 0 }, { -1, 0 }, { 2, 0 }, { 1, 0 } },
        { { 1, 0 }, { 2, 0 }, { -1, 0 }, { -2, 0 } },
        { { -2, -1 }, { -1, 2 }, { 2, 1 }, { 1, -2 } },
        { { 1, 2 }, { 2, -1 }, { -1, -2 }, { -2, 1 } },
    };

    const int_mat2_t ROTATE90_MAT_TR = { { 0, 1 }, { -1, 0 } };
    int_vec2_t result_offsets[NUM_OFFSETS] = { 0, };

    const block_t block = p_game->blocks[p_game->block_index];
    const int_vec2_t (*test_table)[4] = (block == BLOCK_I) ? I_TEST : JLSZT_TEST;

    int_vec2_t test_pos = { 0, };
    for (size_t test_index = 0; test_index < 5; ++test_index)
    {
        size_t count = 0;

        test_pos.x = test_table[test_index][p_game->block_rotation_count].x;
        test_pos.y = test_table[test_index][p_game->block_rotation_count].y;
        for (size_t i = 0; i < NUM_OFFSETS; ++i)
        {
            int_vec2_t offset = { 0, };
            offset.x = -p_game->block_offsets[i].x;
            offset.y = -p_game->block_offsets[i].y;

            transform22(&result_offsets[i], &offset, &ROTATE90_MAT_TR);

            int_vec2_t actual_pos = { 0, };
            actual_pos.x = p_game->block_pos.x + result_offsets[i].x + test_pos.x;
            actual_pos.y = p_game->block_pos.y + result_offsets[i].y + test_pos.y;

            // 회전 이후 범위를 벗어나거나
            // 해당 위치에 이미 블럭이 있으면 회전 실패
            if (actual_pos.x < 0 || actual_pos.x >= BOARD_COLS
                || actual_pos.y >= BOARD_ROWS
                || p_game->board[actual_pos.y][actual_pos.x] == BOARD_STATE_FIXED)
            {
                break;
            }

            ++count;
        }

        // 회전 성공
        if (count == NUM_OFFSETS)
        {
            memcpy(p_game->block_offsets, result_offsets, sizeof(int_vec2_t) * NUM_OFFSETS);
            p_game->block_pos.x += test_pos.x;
            p_game->block_pos.y += test_pos.y;
            p_game->block_rotation_count = (p_game->block_rotation_count + 1) % 4;
            return true;
        }
    }

    return false;
}

static void remove_line(game_t* p_game)
{
    ASSERT(p_game != NULL, "p_game == NULL");

    size_t y = p_game->block_pos.y;
    for (size_t i = 0; i < NUM_OFFSETS; ++i)
    {
        y = MAX(y, p_game->block_pos.y + p_game->block_offsets[i].y);
    }
    const size_t end_y = MAX(y - 4, 1);

    while (y >= end_y)
    {
        size_t count = 0;
        for (size_t x = 0; x < BOARD_COLS; ++x)
        {
            if (p_game->board[y][x] == BOARD_STATE_FIXED)
            {
                ++count;
            }
        }

        if (count == BOARD_COLS)
        {
            for (size_t i = y; i >= 1; --i)
            {
                memcpy(p_game->board[i], p_game->board[i - 1], sizeof(board_state_t) * BOARD_COLS);
            }
        }
        else
        {
            --y;
        }
    }
}