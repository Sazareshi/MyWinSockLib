#include "stdafx.h"
#include "Communicator.h"


static CMCtransaction mc_handler;

CCommunicator::CCommunicator(){}

CCommunicator::~CCommunicator(){}

void CCommunicator::routine_work(void *param) {
	CSock sock_handler;
	
	ws << L"Sock Step:" << sock_handler.sock_packs[0].current_step; tweet2owner(ws.str()); ws.str(L""); ws.clear();

};

unsigned __stdcall CCommunicator::MCprotoThread(void *pVoid) 
{
	CSock sock_handler;

	DWORD dwRet, index;
	ostringstream ws;

	//�����C�x���g����������̂�ҋ@����
	dwRet = WSAWaitForMultipleEvents(SOCKET_MAX_NUM, sock_handler.hEvents, FALSE, 1000, FALSE);

	if (dwRet != WSA_WAIT_FAILED) {
		index = dwRet - WSA_WAIT_EVENT_0;//���������C�x���g�n���h��
										 //�ǂ̃C�x���g�������������𔻕ʂ���
		WSANETWORKEVENTS events;
		DWORD errCode;

		dwRet = WSAEnumNetworkEvents(sock_handler.sock_packs[index].socket, sock_handler.hEvents[index], &events);
		if (dwRet == SOCKET_ERROR) {
			;
		}
		if (events.lNetworkEvents & FD_CONNECT) {
			errCode = events.iErrorCode[FD_CONNECT_BIT];
			sock_handler.sock_connected(index);
	//		ws << L"index:" << index << L"  Connection OK"; tweet2owner(ws.str()); ws.str(L""); ws.clear();
		}
		if (events.lNetworkEvents & FD_READ) {
			errCode = events.iErrorCode[FD_READ_BIT];
			sock_handler.sock_recv(index);
		}
		if (events.lNetworkEvents & FD_WRITE) {
			errCode = events.iErrorCode[FD_WRITE_BIT];
			;
		}
		if (events.lNetworkEvents & FD_CLOSE) {
			errCode = events.iErrorCode[FD_CLOSE_BIT];
			sock_handler.sock_close(index);
		}
		if (events.lNetworkEvents & FD_ACCEPT) {
			errCode = events.iErrorCode[FD_ACCEPT_BIT];
			;
		}
		;
	}
	else {
		;
	}


	return 1;
};

void CCommunicator::init_task() {
	unsigned ThreadAddr;

	//�\�P�b�g��p�ӂ���Connect�v���܂�
	int id = start_sock();

	//MC�v���g�R���������s
	_beginthreadex(
		NULL,					//Security
		0,						//Stack size
		MCprotoThread,			//�X���b�h�֐�
		&mc_handler.mcifmng,	//����
		0,						//�����t���O
		&ThreadAddr				//�X���b�hID���󂯎��A�h���X
	);
}

unsigned CCommunicator::start_sock() {

	CSock sock_handler;
	int ID;
	IN_ADDR sa;

	if (sock_handler.init_ok == NOT_INITIALIZED) {
		if (sock_handler.wsa_init() == S_OK) {
			ws << L"WSA Start OK"; tweet2owner(ws.str()); ws.str(L""); ws.clear();
			sock_handler.init_ok = WSA_INITIAL_OK;
		}
	}

	int r = inet_pton(AF_INET, "192.168.200.10", &(sa.S_un.S_addr));

	if (sock_handler.create(&ID, sa, 30000, SOCK_STREAM, CLIENT_SOCKET) == S_OK) {
		ws << L"Creat Sock OK"; tweet2owner(ws.str()); ws.str(L""); ws.clear();
		sock_handler.sock_packs[ID].current_step = SOCK_PREPARED;
	}

	r = sock_handler.make_connection(ID);
	if (r == S_OK || r == WSAEWOULDBLOCK) {
		ws << L"index:" << ID << L"  Waiting Connection"; tweet2owner(ws.str()); ws.str(L""); ws.clear();
		sock_handler.sock_packs[ID].current_step = WAIT_CONNECTION;
	}

	return ID;
}
