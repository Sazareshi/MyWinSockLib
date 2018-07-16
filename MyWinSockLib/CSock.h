#pragma once

#define CLIENT_SOCKET	0
#define SERVER_SOCKET	1
#define SOCKET_MAX_NUM	16

class CSock
{
public:
	static int sock_num;
	static int sock_type[SOCKET_MAX_NUM];
	static SOCKET sockets[SOCKET_MAX_NUM];
	static SOCKADDR_IN sa[SOCKET_MAX_NUM];

public:
	CSock();
	~CSock();

	static int wsa_init();
	static int socket_init(int si);
	static int exit() { return WSACleanup(); }
};

