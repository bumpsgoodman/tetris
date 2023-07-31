#include "../public/game.h"

int main(int argc, char* argv[])
{
    game_t game;

    init_game(&game);

    while (is_running_game(&game))
    {
        update_game(&game);
        draw_game(&game);
    }

    return 0;
}