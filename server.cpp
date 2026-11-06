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
    //initialize winsock
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        cout << "WSAStartup:" << WSAGetLastError() << endl;
        return;
    }
    //initialize the game
    init_game();//?????????????????????????????

    //create the listen socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cout << "socket: " << WSAGetLastError() << endl;
        return;
    }
    // set address and port 
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // bind the socket
    res = bind(sock, (sockaddr*)&addr, sizeof(addr));
    if (res == SOCKET_ERROR) {
        cout << "bind: " << WSAGetLastError() << endl;
        return;
    }

    // begin listening for connections
    res = listen(sock, SOMAXCONN);
    if (res == SOCKET_ERROR) {
        cout << "listen: " << WSAGetLastError() << endl;
        return;
    }
    safe_print("Server listening on port " + to_string(listen_port) + "\n");

    //Set Socket to non-blocking
    u_long nonBlocking = 1;
    res = ioctlsocket(sock, FIONBIO, &nonBlocking);
    if (res == SOCKET_ERROR) {
        cout << "ioctlsocket: " << WSAGetLastError() << endl;
        return;
    }

    // Initialize the game before accepting:
    init_game();//??????????????????????


    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);

    while (true)
    {
        // TODO accept incoming clients,
        SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                cout << "accept: " << WSAGetLastError() << endl;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else {
            //run thread
            thread worker(ClientGameLogic, clientSock);
            worker.detach();
        }
        // TODO handle clients (select or threads)
        if (is_game_over())
        {
            safe_print("Game Over. Resetting game ...\n");
            reset_game();
        }
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