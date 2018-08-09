#include "stdafx.h"
#include "CSock.h"

using namespace std;

CSock::CSock() {}

CSock::~CSock() {}

StSOCKPACKAGE CSock::sock_packs[SOCKET_MAX_NUM];
StSOCKERR CSock::sock_err[SOCKET_MAX_NUM];
StRCVBUFPACK CSock::rcvbufpack[SOCKET_MAX_NUM];
WSAEVENT  CSock::hEvents[SOCKET_MAX_NUM];
BOOL CSock::init_ok;

int CSock::wsa_init() {

	WSADATA wsaData;

	HANDLE hDefEvent = CreateEvent(NULL, FALSE, FALSE, NULL);//セキュリティ,自動リセット,初期シグナル状態,名前文字列

	for (int i = 0; i < SOCKET_MAX_NUM; i++) {
		sock_packs[i].current_step = NULL;
		sock_packs[i].socket = INVALID_SOCKET;
		// defaultイベントハンドルでhEvent配列初期化
		hEvents[i] = hDefEvent;
	}
	hEvents[SOCKET_MAX_NUM] = hDefEvent;

	//WinSock.dllを初期化する
	if (int nRet = WSAStartup(WINSOCK_VERSION, &wsaData)) {
		fprintf(stderr, "\nWSAStartup() : %d\n", nRet);
		WSACleanup();
		return nRet;
	}

	//WinSockのバージョンを調べる
	if (wsaData.wVersion != WINSOCK_VERSION) {
		WSACleanup();
		return wsaData.wVersion;
	}

	for (int i = 0; i < SOCKET_MAX_NUM; i++) {
		sock_packs[i].current_step = SOCK_NOT_CREATED;
		rcvbufpack[i].rptr = rcvbufpack[i].wptr = 0;
	}

	return S_OK;
}

int CSock::create(int * index, IN_ADDR ip_addr, USHORT port, int protocol, int type) {
	
	int si;
	for (si = 0; si < SOCKET_MAX_NUM; si++) {
		if (sock_packs[si].socket == INVALID_SOCKET) break;
	}

	if ((si >= SOCKET_MAX_NUM) || ((protocol != SOCK_STREAM) && (protocol != SOCK_DGRAM))) {
		sock_packs[si].current_step = SOCK_NOT_CREATED;
		return APP_ERROR;	//ソケット数オーバー orプロトコル指定が範囲外
	}

	//サーバーアドレス構造体を埋める
	sock_packs[si].sa.sin_family = AF_INET;
	sock_packs[si].sa.sin_port = htons(port);
	sock_packs[si].sa.sin_addr = ip_addr;

	//ソケットを作成する
	sock_packs[si].socket = socket(AF_INET, protocol, 0);
	if (sock_packs[si].socket == INVALID_SOCKET) {
		sock_packs[si].current_step = SOCK_NOT_CREATED;
		return SOCK_ERROR;
	}
	sock_packs[si].sock_protocol = protocol;
	sock_packs[si].sock_type = type;

	//ソケットで使用するイベントオブジェクトを作成する
	hEvents[si] = WSACreateEvent();
	if (hEvents[si] == WSA_INVALID_EVENT) {
		closesocket(sock_packs[si].socket);
		sock_packs[si].socket = INVALID_SOCKET;
		sock_packs[si].current_step = SOCK_NOT_CREATED;
		return SOCK_ERROR;
	}


	//ソケットを非ブロッキングにし、ネットワークイベントを関連付ける
	int nRet;
	if (protocol == SOCK_STREAM) {
		if (type == CLIENT_SOCKET)	nRet = WSAEventSelect(sock_packs[si].socket, hEvents[si], FD_READ | FD_WRITE | FD_CONNECT | FD_CLOSE);
		else	nRet = WSAEventSelect(sock_packs[si].socket, hEvents[si], FD_READ | FD_WRITE | FD_ACCEPT | FD_CLOSE);
	}
	else nRet = WSAEventSelect(sock_packs[si].socket, hEvents[si], FD_READ | FD_WRITE);

	if (nRet) {
		closesocket(sock_packs[si].socket);
		WSACloseEvent(hEvents[si]);
		sock_packs[si].socket = INVALID_SOCKET;
		sock_packs[si].current_step = SOCK_NOT_CREATED;
		hEvents[si] = hEvents[SOCKET_MAX_NUM];
		return SOCK_ERROR;
	}

	sock_packs[si].current_step = SOCK_PREPARED;
	*index = si;
	
	return S_OK;
}

HRESULT CSock::make_connection(int index) {
	int nRet;
	switch (sock_packs[index].sock_type) {
		case CLIENT_SOCKET:{
			nRet = connect(sock_packs[index].socket, (LPSOCKADDR)&sock_packs[index].sa, sizeof(SOCKADDR_IN));
			if (nRet == SOCKET_ERROR) {
				sock_err[index].wErrCode = WSAGetLastError();

				if (sock_err[index].wErrCode == WSAEWOULDBLOCK) {
					return sock_err[index].wErrCode;
				}
				else {
					closesocket(sock_packs[index].socket);
					WSACloseEvent(hEvents[index]);
					sock_packs[index].socket = INVALID_SOCKET;
					sock_packs[index].current_step = SOCK_NOT_CREATED;
					hEvents[index] = hEvents[SOCKET_MAX_NUM];
					return sock_err[index].wErrCode;
				}
			}
			else {
				sock_packs[index].current_step = CONNECTED;
			}
		}break;
		case SERVER_SOCKET: {
			;
		}break;
		default: return SOCKET_ERROR;
	};

	return S_OK;
}

int CSock::sock_connected(int index) {
	sock_packs[index].current_step = CONNECTED;
	return S_OK;
}


int CSock::sock_send(int index, const char* buf, int length) {
	int nRet;
	if(sock_packs[index].sock_protocol == SOCK_STREAM)
		nRet = send(sock_packs[index].socket, buf, length, 0);
	else if (sock_packs[index].sock_protocol == SOCK_DGRAM) {
		;
	}
	else return APP_ERROR;

	return nRet;
}

int CSock::sock_recv(int index) {
	int nRet;
	if (sock_packs[index].sock_protocol == SOCK_STREAM) {
		if (nRet = recv(sock_packs[index].socket, rcvbufpack[index].rbuf[rcvbufpack[index].wptr], SIZE_OF_RBUF, 0)) {
			rcvbufpack[index].datsize[rcvbufpack[index].wptr] = nRet;
			rcvbufpack[index].wptr++;
			if (rcvbufpack[index].wptr >= NUM_OF_RBUF) rcvbufpack[index].wptr = 0;
		}
	}
	else if (sock_packs[index].sock_protocol == SOCK_DGRAM) {
		;
	}
	else return APP_ERROR;

	if(rcvbufpack[index].wptr == rcvbufpack[index].rptr) return BUF_OVERFLOW;

	return S_OK;
}

int CSock::sock_close(int index) {
	if(sock_packs[index].socket != INVALID_SOCKET) closesocket(sock_packs[index].socket);
	sock_packs[index].current_step = SOCK_NOT_CREATED;
	sock_packs[index].socket = INVALID_SOCKET;
	WSACloseEvent(hEvents[index]);
	return S_OK;
}

int CSock::exit(){
	for (int i = 0; i < SOCKET_MAX_NUM; i++) sock_close(i);
	init_ok = FALSE;
	return WSACleanup();
}

int CSock::msg_pickup(int index, const char* buf, int n) { return 1; };

int CSock::rcv_check(int index) {
	if (rcvbufpack[index].wptr >= rcvbufpack[index].rptr) return rcvbufpack[index].wptr - rcvbufpack[index].rptr;
	else return rcvbufpack[index].wptr + 1 + (NUM_OF_RBUF - rcvbufpack[index].rptr);
};


HRESULT CSock::GetSockMsg(int nError, LPWSTR pszMessage, DWORD dwSize)
{

#define ERROR_BUF_LEN 100
	wchar_t	pszError[ERROR_BUF_LEN];
	DWORD	n;

	swprintf_s(pszError, sizeof(pszError), L"%d:", nError);

	switch (nError)
	{ //lstrcat 第1引数の文字列に第2引数の文字列を連結 ただし、pszErrorは、初期化されてない
	case WSAEINTR:				lstrcat(pszError, L"Interrupted system call");							break;
	case WSAEBADF:				lstrcat(pszError, L"Bad file number");									break;
	case WSAEACCES:				lstrcat(pszError, L"Permission denied");								break;
	case WSAEFAULT:				lstrcat(pszError, L"Bad address");										break;
	case WSAEINVAL:				lstrcat(pszError, L"Invalid argument");									break;
	case WSAEMFILE:				lstrcat(pszError, L"Too many open files");								break;
	case WSAEWOULDBLOCK:		lstrcat(pszError, L"Operation would block");								break;
	case WSAEINPROGRESS:		lstrcat(pszError, L"Operation now in progress");							break;
	case WSAEALREADY:			lstrcat(pszError, L"Operation already in progress");						break;
	case WSAENOTSOCK:			lstrcat(pszError, L"Socket operation on non-socket");					break;
	case WSAEDESTADDRREQ:		lstrcat(pszError, L"Destination address required");						break;
	case WSAEMSGSIZE:			lstrcat(pszError, L"Message too long");									break;
	case WSAEPROTOTYPE:			lstrcat(pszError, L"Protocol wrong type for socket");					break;
	case WSAENOPROTOOPT:		lstrcat(pszError, L"Protocol not available");							break;
	case WSAEPROTONOSUPPORT:	lstrcat(pszError, L"Protocol not supported");							break;
	case WSAESOCKTNOSUPPORT:	lstrcat(pszError, L"Socket type not supported");							break;
	case WSAEOPNOTSUPP:			lstrcat(pszError, L"Operation not supported on socket");					break;
	case WSAEPFNOSUPPORT:		lstrcat(pszError, L"Protocol family not supported");						break;
	case WSAEAFNOSUPPORT:		lstrcat(pszError, L"Address family not supported by protocol family");	break;
	case WSAEADDRINUSE:			lstrcat(pszError, L"Address already in use");							break;
	case WSAEADDRNOTAVAIL:		lstrcat(pszError, L"Can't assign requested address");					break;
	case WSAENETDOWN:			lstrcat(pszError, L"Network is down");									break;
	case WSAENETUNREACH:		lstrcat(pszError, L"Network is unreachable");							break;
	case WSAENETRESET:			lstrcat(pszError, L"Network dropped connection on reset");				break;
	case WSAECONNABORTED:		lstrcat(pszError, L"Software caused connection abort");					break;
	case WSAECONNRESET:			lstrcat(pszError, L"Connection reset by peer");							break;
	case WSAENOBUFS:			lstrcat(pszError, L"No buffer space available");							break;
	case WSAEISCONN:			lstrcat(pszError, L"Socket is already connected");						break;
	case WSAENOTCONN:			lstrcat(pszError, L"Socket is not connected");							break;
	case WSAESHUTDOWN:			lstrcat(pszError, L"Can't send after socket shutdown");					break;
	case WSAETOOMANYREFS:		lstrcat(pszError, L"Too many references: can't splice");					break;
	case WSAETIMEDOUT:			lstrcat(pszError, L"Connection timed out");								break;
	case WSAECONNREFUSED:		lstrcat(pszError, L"Connection refused");								break;
	case WSAELOOP:				lstrcat(pszError, L"Too many levels of symbolic links");					break;
	case WSAENAMETOOLONG:		lstrcat(pszError, L"File name too long");								break;
	case WSAEHOSTDOWN:			lstrcat(pszError, L"Host is down");										break;
	case WSAEHOSTUNREACH:		lstrcat(pszError, L"No route to host");									break;
	case WSAENOTEMPTY:			lstrcat(pszError, L"Directory not empty");								break;
	case WSAEPROCLIM:			lstrcat(pszError, L"Too many processes");								break;
	case WSAEUSERS:				lstrcat(pszError, L"Too many users");									break;
	case WSAEDQUOT:				lstrcat(pszError, L"Disc quota exceeded");								break;
	case WSAESTALE:				lstrcat(pszError, L"Stale NFS file handle");								break;
	case WSAEREMOTE:			lstrcat(pszError, L"Too many levels of remote in path");					break;
	case WSAEDISCON:			lstrcat(pszError, L"Disconnect");										break;
	case WSASYSNOTREADY:		lstrcat(pszError, L"Network sub-system is unusable");					break;
	case WSAVERNOTSUPPORTED:	lstrcat(pszError, L"WinSock DLL cannot support this application");		break;
	case WSANOTINITIALISED:		lstrcat(pszError, L"WinSock not initialized");							break;
	case WSAHOST_NOT_FOUND:		lstrcat(pszError, L"Host not found");									break;
	case WSATRY_AGAIN:			lstrcat(pszError, L"Non-authoritative host not found");					break;
	case WSANO_RECOVERY:		lstrcat(pszError, L"Non-recoverable error");								break;
	case WSANO_DATA:			lstrcat(pszError, L"Valid name, no data record of requested type");		break;
	default:					lstrcpy(pszError, L"Not a WinSock error");								break;
	}

	n = lstrlen(pszError);
	if (pszMessage != NULL)	n += lstrlen(pszMessage);
	if ((pszMessage != NULL) && (n < dwSize))	wcscpy_s(pszMessage, dwSize, pszError);           //dwSize:pszMessageのバッファサイズ
	else										wcsncpy_s(pszMessage, dwSize, pszError, dwSize);  //こちらはコピー上限有り

	return S_OK;
}