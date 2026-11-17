#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include "client.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

static atomic<bool> running{ true };
static mutex cout_mutex;

// See server.h for why this helper exists. The client has two threads
// (receive_loop and input_loop) that both write status to the same terminal,
// so std::cout/std::cerr need to be serialized.
static void safe_print(const string& msg)
{
    lock_guard<std::mutex> lock(cout_mutex);
    cout << msg;
}


// Reads server messages and prints them. Runs on the main thread.
static void receive_loop(SOCKET sock)
{
    while (running)
    {
        // receive a message (block until receiving)
        // if successful, decode message and display text.
        // if server disconnect, print disconnection method and exit.
        // if error, print error message and code and exit.

        running = false;
    }
}

// Reads user input and sends it to the server. Runs on a background thread.
static void input_loop(SOCKET sock)
{
   string input;
    while (running && getline(cin, input))
    {
        if (!running) break;
        if (input.empty()) continue;

        // TODO: encode input and send to server 

        int sent = SOCKET_ERROR;
        if (sent == SOCKET_ERROR)
        {
            safe_print("Send failed: " + to_string(WSAGetLastError()) + "\n");
            running = false;
            break;
        }
    }
}

static void run_client(const string& server_ip, int port)
{
    // TODO: Initialize Winsock, create address, connect to server
    SOCKET sock;
    // Detached because std::getline blocks on stdin and there is no
    // portable way to unblock it from the recv thread. The process exits
    // when run_client returns, which tears the thread down.
    thread(input_loop, sock).detach();

    receive_loop(sock);

    // TODO: shutdown cleanup and prepare for exit
}

int main(int argc, char* argv[])
{
    // TODO: get port from argv (is it valid?)
    // Print usage instructions if port not given
    try
    {
        string server_ip = "127.0.0.1";
        int port = 0;
        run_client(server_ip, port);
    }
    catch (const exception& e)
    {
        cerr << "Invalid input: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

//// get port from argv 
//int port = 0;
//
//if (argc != 2) {
//    cerr << "Usage: " << argv[0] << " <port>\n";
//    return 1;
//}
//
//try {
//    size_t pos = 0;
//    port = stoi(argv[1], &pos);
//
//    if (pos != strlen(argv[1]) || port < 1 || port > 65535) {
//        throw invalid_argument("Invalid port");
//    }
//}
//catch (const exception&) {
//    cerr << "Usage: " << argv[0] << " <port>\n";
//    return 1;
//}