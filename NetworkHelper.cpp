#include "NetworkHelper.h"
#include "game.h"
#include <thread>

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
	return 1;
}

int RecieveTCPMessage(SOCKET sock, string& msg, mutex& msg_mutex, atomic<bool>& receiving)
{
#define BUF_LEN 1400
    string message;
    char buf[BUF_LEN + 1]; //leave room for \0
    int res;
    while (receiving && !GetGameOverFlag()) {
        res = recv(sock, buf, BUF_LEN, 0); // resv = num of bytes recieved
        if (res == SOCKET_ERROR) {
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                receiving = false; 
                return 0;
            }
            else {
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        else if (res == 0) {//done receiving
            receiving = false;
            return 1;
        }
        else {//take recieved bytes and store full message then check to see if we have the request
            message.append(buf, res);

            size_t pos;
            while ((pos = message.find('\n')) != string::npos)
            {
                lock_guard<mutex> lock(msg_mutex);
                msg.append(message.substr(0, pos + 1));

                // Remove the printed line from the buffer.
                message.erase(0, pos + 1);
            }
        }

    }
}

