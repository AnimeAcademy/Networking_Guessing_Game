#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>

extern std::atomic<int> client_counter;

void safe_print(const std::string& msg);

void Add_Client(int ID, SOCKET sock);

void Remove_Client(int client_id, SOCKET clientSock, int error = 1);

bool Are_Clients_Empty();

void start_server(int listen_port);

int main(int argc, char** argv);









