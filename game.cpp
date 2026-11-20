#include "game.h"
#include "server.h"
#include "NetworkHelper.h"
#include <thread>


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
        return { "Game already won!\r\n" };
    }
    if (guess > 100) {
        return { "Number is higher than guessing range.\r\n" };
    }
    if (guess < secret_number)
    {
        return { "Too low.\r\n" };
    }
    else if (guess > secret_number)
    {
        return { "Too high.\r\n" };
    }
    else
    {
        winner_client = client_id;
        game_over_flag = true;
        return { "Correct! You win!\r\n" };
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

void ClientGameLogic(SOCKET clientSocket)
{

//Set Client ID
    int client_id = ++client_counter;
    atomic<bool> receiving{ true };
    string msg;
    mutex msg_mutex;
    Add_Client(client_id, clientSocket);
    
    thread recv(RecieveTCPMessage, clientSocket, ref(msg), ref(msg_mutex), ref(receiving));

    auto cleanup = [&]() {
        receiving = false;
        shutdown(clientSocket, SD_BOTH);

        if (recv.joinable()) {
            recv.join();
        }

        Remove_Client(client_id, clientSocket);
        };

    //Print Client connected
    string message = "Client " + to_string(client_id) + " connected\n";
    safe_print(message);

    message = "Welcome to the Guessing Game! Client " + to_string(client_id) + "\n";
    int res, playerGuess;
    //Welcome to game
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        cleanup();
        return;
    }
    //Explain the rules
    message = "Guess a number between 1 and 100. \n"
        "You will be racing against other players. \n"
        "First one to guess the number WINS!\n";
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        cleanup();
        return;
    }
    //TODO: Ready UP?

    //Get Started
    message = "Let's start guessing!\n";
    res = SendTCPMessage(clientSocket, message);
    if (res == 0) {
        cleanup();
        return;
    }
   

    while (!game_over_flag && receiving) {
        //Ask for Guess
        if (game_over_flag) break;
        message = "Your Guess: \n";
        res = SendTCPMessage(clientSocket, message);
        if (res == 0) {
            cleanup();
            return;
        }
        //get players guess
        while (receiving && !game_over_flag) {
            if (game_over_flag) break;
            this_thread::sleep_for(chrono::milliseconds(10));
            lock_guard<mutex> lock(msg_mutex);
            if (!msg.empty()) break;
        }
        if (!receiving) {
            cleanup();
            return;
        }
        if (game_over_flag) break;

        {
            lock_guard<mutex> lock(msg_mutex);
            message = msg;
            //removed msg data
            msg.clear();
        }
        //check player guess 
        size_t start = message.find("Player Guess: ");
        if (start != string::npos) {
            // Move start past "Player Guess: "
            start += string("Player Guess: ").length();
            try {
                //procces guess
                string input = message.substr(start);
                size_t pos = 0;

                playerGuess = stoi(input, &pos);

                if (pos != input.length() && input.substr(pos) != "\n" &&
                    input.substr(pos) != "\r\n") {
                    throw invalid_argument("Invalid characters in guess");
                }

                message = process_guess(playerGuess, client_id);
            }//checks to see if they sent an int or not
            catch (const invalid_argument&) {
                message = "Please enter a valid number.\n";
            }//makes sure the int isn't too large
            catch (const out_of_range&) {
                message = "Number is too large.\n";
            }
            //found guess now send response to player
            //Too high / low or correct
            res = SendTCPMessage(clientSocket, message);
            if (res == 0) {
                break;
            }
        }
        else {
            //failed to get player guess try again
            message = "Wrong input, please put in a valid number\n";
            res = SendTCPMessage(clientSocket, message);
            if (res == 0) {
                cleanup();
                return;
            }
        }
    }
    //After game ended
    if (client_id != winner_client) {
        //If player didn't win say who did
        message = "Client " + to_string(winner_client) + " has won!\n";
        message.append("Winning Guess was " + to_string(secret_number) + "!\n");
        res = SendTCPMessage(clientSocket, message);
        if (res == 0) {
            cleanup();
            return;
        }
    }
    //After recieving who won (themselves or someone else)
    //tell client we are done and close socket
    cleanup();
    return;
}

bool GetGameOverFlag()
{
    return game_over_flag;
}
