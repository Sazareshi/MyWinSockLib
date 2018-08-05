#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>

#define CLIENT_SOCKET	0
#define SERVER_SOCKET	1
#define SOCKET_MAX_NUM	16
#define ERR_MSG_MAX		256

#define NOT_INITIALIZED			0
#define WSA_INITIAL_OK			1
#define SOCK_NOT_CREATED		1
#define SOCK_PREPARED			2
#define WAIT_CONNECTION			4
#define CONNECTED				8

#define APP_ERROR				-1
#define SOCK_ERROR				-2
#define BUF_OVERFLOW			-3

#define NUM_OF_RBUF				16
#define SIZE_OF_RBUF			4096


typedef struct _stErrStatus
{
	BOOL	errlogout;
	int	wErrCode;
	_TCHAR errmsg[ERR_MSG_MAX];
	_TCHAR nameoferrlog[_MAX_FNAME];
	FILE *fp;

}StSOCKERR, *LPStSOCKERR;

typedef struct _stRcvBufPack
{
	char rbuf[NUM_OF_RBUF][SIZE_OF_RBUF];//ring buffer
	unsigned datsize[NUM_OF_RBUF];
	unsigned rptr;
	unsigned wptr;
}StRCVBUFPACK, *LPStRCVBUFPACK;

typedef struct _stSockPack
{
	int sock_type;//CLIENT_SOCKET or SERVER_SOCKET
	int current_step;
	SOCKET socket;
	SOCKADDR_IN sa;
}StSOCKPACKAGE, *LPStSOCKPACKAGE;

class CSock
{
public:
	static StSOCKPACKAGE sock_packs[SOCKET_MAX_NUM];
	static StSOCKERR sock_err[SOCKET_MAX_NUM];
	static StRCVBUFPACK rcvbufpack[SOCKET_MAX_NUM];
	static WSAEVENT hEvents[SOCKET_MAX_NUM];
	static BOOL init_ok;

public:
	CSock();
	~CSock();

	static int wsa_init();
	static int create(int *index, IN_ADDR ip_addr, USHORT port, int protocol, int type);
	static int sock_connected(int index);
	static int sock_recv(int index);
	static int sock_close(int index);
	static int sock_send(int index, const char* buf, int length);
	static HRESULT make_connection(int index);
	static int exit();
	static HRESULT GetSockMsg(int nError, LPWSTR pszMessage, DWORD dwSize);
	static int msg_pickup(int index, const char* buf, int n);
	static int rcv_check(int index);
};

