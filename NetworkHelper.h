#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

using namespace std;

//Sends a message to the Socket  
// Params: SOCKET, STRING
// SOCKET is the socket of where to send the message
// STRING is the message to send
// returns 1 if succseeded and 0 if failed
int SendTCPMessage(SOCKET sock, string& message);

//Recieves a sting from the client
// Params: SOCKET, STRING
// SOCKET is where you are recieving data
// STRING is where the message will be stored
// returns 1 if succseeded and 0 if failed
int RecieveTCPMessage(SOCKET sock, string& message);