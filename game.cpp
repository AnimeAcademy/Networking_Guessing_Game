#include "game.h"
#include <string>
#include <mutex>


namespace
{
    int secret_number = 0;
    int winner_client = -1;
    std::mutex game_mutex;
    std::atomic<bool> game_over_flag(false);
}

void init_game()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::lock_guard<std::mutex> lock(game_mutex);
    secret_number = std::rand() % 100 + 1;
    winner_client = -1;
    game_over_flag = false;
}

std::pair<std::string, bool> process_guess(int guess, int client_id)
{
    std::lock_guard<std::mutex> lock(game_mutex);
    if (game_over_flag)
    {
        return { "Game already won!\r\n", false };
    }

    if (guess < secret_number)
    {
        return { "Too low.\r\n", false };
    }
    else if (guess > secret_number)
    {
        return { "Too high.\r\n", false };
    }
    else
    {
        winner_client = client_id;
        game_over_flag = true;
        return { "Correct! You win!\r\n", true };
    }
}

int get_winning_client()
{
    std::lock_guard<std::mutex> lock(game_mutex);
    return winner_client;
}

bool is_game_over()
{
    return game_over_flag;
}

void reset_game()
{
    init_game();
}