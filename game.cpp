#include "game.h"
#include "server.h"
#include "NetworkHelper.h"

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

string process_guess(int guess, int client_id)
{
    lock_guard<mutex> lock(game_mutex);
    if (game_over_flag)
    {
        return { "Game already won!\r\n", false };
    }
    if (guess > 100) {
        return { "Number is higher than guessing range.\r\n", false };
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
    //Set Client ID
    int client_id = ++client_counter;
    Add_Client(client_id, clientSocket);

    //Print Client connected
    string message = "Client " + to_string(client_id) + " connected";
    safe_print(message);

    message = "Welcome to the Guessing Game!";
    int res, playerGuess;
    //Welcome to game
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        Remove_Client(client_id, clientSocket);
        return;
    }
    //Explain the rules
    message = "Guess a number between 1 and 100. \n"
        "You will be racing against other players. \n"
        "First one to guess the number WINS!\n";
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        Remove_Client(client_id, clientSocket);
        return;
    }
    //TODO: Ready UP?

    //Get Started
    message = "Let's start guessing!";
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        Remove_Client(client_id, clientSocket);
        return;
    }

    while (!game_over_flag) {
        //Ask for Guess
        message = "Your Guess: ";
        res = SendTCPMessage(clientSocket, message);
        if (res == 0) {
            Remove_Client(client_id, clientSocket);
            return;
        }

        //get players guess
        res = RecieveTCPMessage(clientSocket, message);
        if (res == 0) {
            Remove_Client(client_id, clientSocket);
            return;
        }

        //check player guess 
        size_t start = message.find("Player Guess: ");
        if (start != string::npos) {

            start += string("Player Guess: ").length();
            playerGuess = stoi(message, &start);
            //TODO save output bool
            message = process_guess(playerGuess, client_id);
            //found guess now send response to player
            //Too high / low or correct
            res = SendTCPMessage(clientSocket, message);
            if (res == 0) {
                Remove_Client(client_id, clientSocket);
                return;
            }
        }
        else {
            //failed to get player guess try again
            message = "Wrong input, please put in a valid number";
            res = SendTCPMessage(clientSocket, message);
            if (res == 0) {
                Remove_Client(client_id, clientSocket);
                return;
            }
        }
    }
    //After game ended
    if (client_id != winner_client) {
        //If player didn't win say who did
        message = "Client " + to_string(winner_client) + " has won!";
        SendTCPMessage(clientSocket, message);
        if (res == 0) {
            Remove_Client(client_id, clientSocket);
            return;
        }
    }
    //After recieving who won (themselves or someone else)

    //tell client we are done and close socket
    res = shutdown(clientSocket, SD_SEND);
    if (res == SOCKET_ERROR) {
        cout << "shutdown: " << WSAGetLastError() << endl;
    }

    Remove_Client(client_id, clientSocket);

    return;
}