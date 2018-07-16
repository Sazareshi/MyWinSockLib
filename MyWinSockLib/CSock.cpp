#include "stdafx.h"
#include "CSock.h"

using namespace std;

CSock::CSock(){}

CSock::~CSock(){}

int CSock::sock_num;
int CSock::sock_type[SOCKET_MAX_NUM];
SOCKADDR_IN CSock::sa[SOCKET_MAX_NUM];

int CSock::wsa_init() {

	WSADATA wsaData;
	//WinSock.dll‚ğ‰Šú‰»‚·‚é
	if (int nRet = WSAStartup(WINSOCK_VERSION, &wsaData)) {
		WSACleanup();
		return nRet;
	}

	//WinSock‚Ìƒo[ƒWƒ‡ƒ“‚ğ’²‚×‚é
	if (wsaData.wVersion != WINSOCK_VERSION) {
		WSACleanup();
		return wsaData.wVersion;
	}

	return 0;
}

int CSock::socket_init(int si) {
	sa[si].sin_family = AF_INET;
	return 1;
}