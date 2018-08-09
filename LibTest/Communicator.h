#pragma once
#include "ThreadObj.h"
#include "CSock.h"
#include "MCtransaction.h"

#define IPADDR_MCSERVER "192.168.200.10"
#define NPORT_MCSERVER 30000
#define MC_EVENT_TIMEOUT 5000 //ソケット状態確認周期

class CCommunicator : public CThreadObj
{
public:
	static CMCtransaction mc_handler;
public:
	CCommunicator();
	~CCommunicator();
	void routine_work(void *param);
	static unsigned __stdcall MCprotoThread(void *pVoid);
	unsigned start_MCsock(PCSTR ipaddr, USHORT port, int protocol, int type);
	void init_task(void* pobj);

private:
	static CSock sock_handler;
	int	index_MCsock;
};

