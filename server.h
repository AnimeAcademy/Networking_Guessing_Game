#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <vector>

vector<std::pair<int, SOCKET>> clients;
static atomic<int> client_counter{ 0 };



void Add_Client(int ID, SOCKET sock);

void Remove_Client(int client_id, SOCKET clientSock);

bool Are_Clients_Empty();










