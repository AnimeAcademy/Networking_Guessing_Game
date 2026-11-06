#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "server.h"
#include "game.h"


using namespace std;
#pragma comment(lib, "ws2_32.lib")

mutex cout_mutex;

void safe_print(const string& msg)
{
    lock_guard<mutex> lock(cout_mutex);
    cout << msg;
}

void start_server(int listen_port)
{
    // TODO: initialize winsock

    // TODO: initialize the game

    // TODO: create the listen socket

    // TODO: set address and port 

    // TODO: bind the socket

    // TODO: begin listening for connections
    safe_print("Server listening on port " + to_string(listen_port) + "\n");
    // Initialize the game before accepting:
    //init_game();

    while (true)
    {
        // TODO accept incoming clients,
        // TODO handle clients (select or threads)
        //if (is_game_over())
        //{
        //    safe_print("Game Over. Resetting game ...\n");
        //    reset_game();
        //}
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    // TODO: close sockets, cleanup
}
int main(int argc, char** argv)
{
    try
    {
        // TODO: get port from argv 
        // Is the port valid? (1 <= port < 65536)
        //start_server(0);
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}