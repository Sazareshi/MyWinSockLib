#include "stdafx.h"
#include "Communicator.h"


CMCtransaction CCommunicator::mc_handler;
CSock CCommunicator::sock_handler;

CCommunicator::CCommunicator(){}

CCommunicator::~CCommunicator(){}

void CCommunicator::routine_work(void *param) {
	MCMsgMng* pMCMsgMng = (MCMsgMng*)(&(mc_handler.mcifmng));			//MC�v���g�R�������Ǘ��p�\���̂ւ̃|�C���^
	int MC_transaction_status;

	MC_transaction_status = mc_handler.req_transaction(0);

};

unsigned __stdcall CCommunicator::MCprotoThread(void *pVoid) 
{
	DWORD dwRet, event_index, isock;
	wostringstream ws;
	MCMsgMng* pMCMsgMng = (MCMsgMng*)(&(mc_handler.mcifmng));			//MC�v���g�R�������Ǘ��p�\���̂ւ̃|�C���^
	CCommunicator* pcomm = (CCommunicator*)pVoid;	//�Ăяo�����C���X�^���X�̃|�C���^ ���C���E�B���h�E�ւ̃��b�Z�[�W�\�����p
	
	
	///# �\�P�b�g��p�ӂ���Connect�v���܂�
		pMCMsgMng->sock_index = pcomm->start_MCsock(pMCMsgMng->sock_ipaddr, pMCMsgMng->sock_port, SOCK_STREAM, CLIENT_SOCKET);
		pMCMsgMng->hsock_event = sock_handler.hEvents[pMCMsgMng->sock_index];
	
	while (1) {
		dwRet = WSAWaitForMultipleEvents(MC_SOCK_USE, &(pMCMsgMng ->hsock_event), FALSE, MC_EVENT_TIMEOUT, FALSE);
		if (dwRet != WSA_WAIT_FAILED) {
			event_index = dwRet - WSA_WAIT_EVENT_0;		//���������C�x���g�̃C���f�b�N�X
			isock = pMCMsgMng->sock_index;	//���������C�x���g�̃\�P�b�g�̃C���f�N�X
			
			//�ǂ̃C�x���g�������������𔻕ʂ���
			WSANETWORKEVENTS events;
			DWORD errCode;

			dwRet = WSAEnumNetworkEvents(sock_handler.sock_packs[isock].socket, sock_handler.hEvents[isock], &events);
			if (dwRet == SOCKET_ERROR) {
				;
			}
			if (events.lNetworkEvents & FD_CONNECT) {
				errCode = events.iErrorCode[FD_CONNECT_BIT];
				if(errCode == 0){
					sock_handler.sock_connected(isock);
					pMCMsgMng->sock_event_status |= FD_CONNECT;
					ws << L"index:" << isock << L"  Connection OK"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
				}
				else {
					pMCMsgMng->sock_event_status &= ~FD_CONNECT;
					ws << L"index:" << isock << L"  Connection ERROR iErrorCode  " << errCode; pcomm->tweet2owner(ws.str()); //ws.str(L""); ws.clear();
					int r = sock_handler.make_connection(pMCMsgMng->sock_index);
					if (r == S_OK || r == WSAEWOULDBLOCK) {
						ws << L"   Retry and Waiting Connection again"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
						sock_handler.sock_packs[pMCMsgMng->sock_index].current_step = WAIT_CONNECTION;
					}
				}

			}
			if (events.lNetworkEvents & FD_READ) {
				errCode = events.iErrorCode[FD_READ_BIT];
				pMCMsgMng->sock_event_status |= FD_READ;
				sock_handler.sock_recv(isock);
				ws << L"index:" << isock << L"  Read OK"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
				pMCMsgMng->sock_event_status &= ~FD_READ;
				sock_handler.rcvbufpack;//�f�o�b�O�����j�^�p

				for (int ires = 0; ires < pMCMsgMng->nCommandSet; ires++) {
					if (pMCMsgMng->com_step[ires] == MC_STP_WAIT_RES) {
						pMCMsgMng->com_step[ires] = MC_STP_IDLE;
						ws << L"index:" << isock  << L"  Res OK RCV Buf -> " << sock_handler.rcvbufpack[isock].wptr ; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
						break;
					}
				}
			}
			if (events.lNetworkEvents & FD_WRITE) {
				errCode = events.iErrorCode[FD_WRITE_BIT];
				pMCMsgMng->sock_event_status |= FD_WRITE;
				int icom;
				for (icom = 0; icom < pMCMsgMng->nCommandSet; icom++) {
					if (pMCMsgMng->com_step[icom] == MC_STP_START) {
						sock_handler.sock_send(isock, (const char*)(&pMCMsgMng->com_msg[icom].cmd0401), pMCMsgMng->com_msg_len[icom]);
						pMCMsgMng->com_step[icom] = MC_STP_IDLE;
						ws << L"index:" << isock << L"  Write OK"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
						break;
					}
				}
				;
			}
			if (events.lNetworkEvents & FD_CLOSE) {
				errCode = events.iErrorCode[FD_CLOSE_BIT];
				pMCMsgMng->sock_event_status |= FD_CLOSE;
				sock_handler.sock_close(isock);
				ws << L"index:" << isock << L"  FD_CLOSE is triggered"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
			}
			if (events.lNetworkEvents & FD_ACCEPT) {
				errCode = events.iErrorCode[FD_ACCEPT_BIT];
				pMCMsgMng->sock_event_status |= FD_ACCEPT;
				ws << L"index:" << isock << L"  FD_ACCEPT is triggered"; pcomm->tweet2owner(ws.str()); ws.str(L""); ws.clear();
				;
			}
		}
		else {
			;
		}

	}

	sock_handler.exit();

	return 1;
};

void CCommunicator::init_task(void* pobj) {
	unsigned ThreadAddr;

	///# MC�v���g�R�������I�u�W�F�N�g�̏�����
	mc_handler.init();
	///# MC�v���g�R���ŗ��p����\�P�b�g��IP�ƃ|�[�g�Z�b�g
	mc_handler.mcifmng.sock_ipaddr = IPADDR_MCSERVER;
	mc_handler.mcifmng.sock_port = NPORT_MCSERVER;
			
	//MC�v���g�R���������s
	_beginthreadex(
		NULL,					//Security
		0,						//Stack size
		MCprotoThread,			//�X���b�h�֐�
		pobj,					//���^�X�N�̃I�u�W�F�N�g�ւ̃|�C���^
		0,						//�����t���O
		&ThreadAddr				//�X���b�hID���󂯎��A�h���X
	);
}

unsigned CCommunicator::start_MCsock(PCSTR ipaddr, USHORT port,int protoco, int type) {

	CSock sock_handler;
	int ID;
	IN_ADDR sa;

	if (sock_handler.init_ok == NOT_INITIALIZED) {
		if (sock_handler.wsa_init() == S_OK) {
			ws << L"WSA Start OK"; tweet2owner(ws.str()); ws.str(L""); ws.clear();
			sock_handler.init_ok = WSA_INITIAL_OK;
		}
	}

	int r = inet_pton(AF_INET, ipaddr, &(sa.S_un.S_addr));

	if (sock_handler.create(&ID, sa, port, protoco, type) == S_OK) {
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
