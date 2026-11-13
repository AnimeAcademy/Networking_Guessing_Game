#pragma once
#include <utility> 
#include <string>  

using namespace std;

void init_game();

pair<string, bool> process_guess(int guess, int client_id);

int get_winning_client();

bool is_game_over();

void reset_game();

void ClientGameLogic(SOCKET clientSocket);




