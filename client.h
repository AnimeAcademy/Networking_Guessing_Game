#pragma once

static void safe_print(const std::string& msg);

static void receive_loop(SOCKET sock);

static void input_loop(SOCKET sock);

static void run_client(const std::string& server_ip, int port);

int main(int argc, char* argv[]);
