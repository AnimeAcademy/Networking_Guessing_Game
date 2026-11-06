#include <winsock2.h>
#include <ws2tcpip.h>
#include "game.h"
#include <string>
#include <mutex>

using namespace std;

namespace
{
    int secret_number = 0;
    int winner_client = -1;
    mutex game_mutex;
    atomic<bool> game_over_flag(false);
}

void init_game()
{
    srand(static_cast<unsigned int>(time(nullptr)));
    lock_guard<mutex> lock(game_mutex);
    secret_number = rand() % 100 + 1;
    winner_client = -1;
    game_over_flag = false;
}

pair<string, bool> process_guess(int guess, int client_id)
{
    lock_guard<mutex> lock(game_mutex);
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
    lock_guard<mutex> lock(game_mutex);
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

void ClientGameLogic(SOCKET clientSocket) {
    //TODO: Welcome to game

    //TODO: Explain the rules

    //TODO: Ready UP?

    //TODO:Ask player to guess number
    //TODO:get players guess

    //TODO:check player guess 
    //TODO:Send too high low or correct
}