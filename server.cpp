#include "server.h"
#include "game.h"

using namespace std;
#pragma comment(lib, "ws2_32.lib")

mutex cout_mutex;
mutex clients_mutex;

void safe_print(const string& msg)
{
    lock_guard<mutex> lock(cout_mutex);
    cout << msg;
}

void Add_Client(int ID, SOCKET sock) {
    lock_guard<mutex> lock(clients_mutex);
    clients.emplace_back(ID, sock);
}

void Remove_Client(int client_id, SOCKET clientSock) {
    //remove client from registar
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(
        remove_if(clients.begin(), clients.end(),
            [client_id](const auto& client) {
                return client.first == client_id;
            }),
        clients.end()
    );
    //CLose socket
    if (closesocket(clientSock) != 0) {
        cout << "closesocket: " << WSAGetLastError() << endl;
    }
    //print which client left
    string msg = "Client " + client_id;
    msg.append(" disconnected\n");
    safe_print(msg);
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
    init_game();

    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);

    while (true)
    {
        //accept incoming clients,
        SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            if (WSAGetLastError() != WSAEWOULDBLOCK)
                cout << "accept: " << WSAGetLastError() << endl;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else {
            //Set socket ID
            
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

    //close sockets, cleanup
    res = closesocket(sock);
    if (res != 0) {
        cout << "closesocket: " << WSAGetLastError() << endl;
    }


    //Shutting Down Library
    res = WSACleanup();
    if (res != 0) {
        cout << "WSACleanup: " << WSAGetLastError() << endl;
        return;
    }
}
int main(int argc, char** argv)
{
        // get port from argv 
        int port = 0;

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

        start_server(port);
    

    return 0;
}