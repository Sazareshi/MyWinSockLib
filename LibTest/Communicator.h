#pragma once
#include "ThreadObj.h"
#include "CSock.h"
#include "MCtransaction.h"

#define MC_COMMAND_MAX 8

typedef struct _stMCComSet
{
	DWORD threadID;
	int nCommand;	//���s�R�}���h��
	int errCode;	//�X���b�h���s���̃G���[�R�[�h
	int nComplete;	//���s�R�}���h���ѐ� 0:Start, 1-nCommand:������, -�l:error
	char*	commsg[MC_COMMAND_MAX];
	char*	resmsg[MC_COMMAND_MAX];
	int		nlen_commsg[MC_COMMAND_MAX];
	int		nlen_resmsg[MC_COMMAND_MAX];
	HANDLE	hexit;	//�X���b�h���I��������C�x���g�n���h��
	DWORD	start_time;
}StMCCom, *LPStMCCom;


class CCommunicator : public CThreadObj
{
public:
	CCommunicator();
	~CCommunicator();
	void routine_work(void *param);
	static unsigned __stdcall MCprotoThread(void *pVoid);
	unsigned start_sock();
	void init_task();
};

