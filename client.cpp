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
    lock_guard<mutex> lock(cout_mutex);
    cout << msg;
}


// Reads server messages and prints them. Runs on the main thread.
static void receive_loop(SOCKET sock)
{
    while (running)
    {
        // receive a message (block until receiving)
#define BUF_LEN 1400
        char buf[BUF_LEN + 1]; //leave room for \0
        string msg;
        int res;
        while (true) {
            res = recv(sock, buf, BUF_LEN, 0); // resv = num of bytes recieved
            if (res == SOCKET_ERROR) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    cout << "recv: " << WSAGetLastError() << endl;
                    // if server disconnect, print disconnection method and exit.
                    safe_print("Error receiving data, please press ctl + c to disconect and try again");
                    break;
                }
                else {
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }
            else if (res == 0) {//done receiving
                break;
            }
            else {
                 // if successful, decode message and display text.
                msg.append(buf, res);

                size_t pos;
                while ((pos = msg.find('\n')) != string::npos)
                {
                    string line = msg.substr(0, pos + 1);

                    safe_print(line);

                    // Remove the printed line from the buffer.
                    msg.erase(0, pos + 1);
                }
            }
        }

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

        int res = SOCKET_ERROR;
        string msg = "Player Guess: ";
        msg.append(input);
        int totalBytes = (int)msg.length();
        int bytesSent = 0;
        while (bytesSent != totalBytes) {
            int res = send(sock, (msg.c_str() + bytesSent), (totalBytes - bytesSent), 0);
            if (res == SOCKET_ERROR) {
                if (WSAGetLastError() != WSAEWOULDBLOCK) {
                    safe_print("Send failed: " + to_string(WSAGetLastError()) + "\n");
                    running = false;
                    return;
                }
                this_thread::sleep_for(chrono::milliseconds(100));
            }
            else if (res == 0) {
                break;
            }
            else
                bytesSent += res;
        }

    }
}

static void run_client(const string& server_ip, int port)
{

    //Initialize the Library
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        cout << "WSAStartup:" << WSAGetLastError() << endl;
        return;
    }
    //Create a TCP Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        cout << "socket: " << WSAGetLastError() << endl;
        return;
    }
    //creat address
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//serverIP????????????

    //Connect Socket
    res = connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (res == SOCKET_ERROR) {
        cout << "connect: " << WSAGetLastError() << endl;
        if (closesocket(sock) != 0) {
            cout << "closesocket: " << WSAGetLastError() << endl;
        }
        return;
    }
   
    // Detached because std::getline blocks on stdin and there is no
    // portable way to unblock it from the recv thread. The process exits
    // when run_client returns, which tears the thread down.
    thread(input_loop, sock).detach();

    receive_loop(sock);

    // shutdown cleanup and prepare for exit
    res = closesocket(sock);
    if (res != 0) {
        cout << "closesocket: " << WSAGetLastError() << endl;
    }

    res = WSACleanup();
    if (res != 0) {
        cout << "WSACleanup: " << WSAGetLastError() << endl;
        return;
    }

}

int main(int argc, char* argv[])
{
    // get port from argv (is it valid?)
    int port = 0;
    string server_ip = "127.0.0.1";
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }

    try {
        size_t pos = 0;
        port = stoi(argv[1], &pos);

        if (pos != strlen(argv[1]) || port < 1 || port > 65535) {
            throw invalid_argument("Invalid port");
        }
    }
    catch (const exception&) {
        cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    
    run_client(server_ip, port);

    //// Print usage instructions if port not given
    //try
    //{
    //    string server_ip = "127.0.0.1";
    //    int port = 0;
    //    run_client(server_ip, port);
    //}
    //catch (const exception& e)
    //{
    //    cerr << "Invalid input: " << e.what() << "\n";
    //    return 1;
    //}

    return 0;
}
