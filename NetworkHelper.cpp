#include "NetworkHelper.h"
#include <thread>

using namespace std;

int SendTCPMessage(SOCKET sock, string& message)
{
    int totalBytes = (int)message.length();
    int bytesSent = 0;
    int res = -1;
    while (bytesSent != totalBytes) {
        res = send(sock, (message.c_str() + bytesSent), (totalBytes - bytesSent), 0);
        if (res == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                return 0;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if (res == 0) {
            return 1;
        }
        else
            bytesSent += res;
    }
	return 0;
}

int RecieveTCPMessage(SOCKET sock, string& message)
{
#define BUF_LEN 1400
    message.clear();
    char buf[BUF_LEN + 1]; //leave room for \0
    SOCKET sock = INVALID_SOCKET;
    int res;
    while (true) {
        res = recv(sock, buf, BUF_LEN, 0); // resv = num of bytes recieved
        if (res == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                return 0;
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        else if (res == 0) {//done receiving
            return 1;
        }
        else {//take recieved bytes and store full message then check to see if we have the request
            message.append(buf, res);
        }
    }
}
