#pragma once
#include "ThreadObj.h"
#include "CSock.h"
#include "MCtransaction.h"

#define MC_COMMAND_MAX 8

typedef struct _stMCComSet
{
	DWORD threadID;
	int nCommand;	//実行コマンド数
	int errCode;	//スレッド実行中のエラーコード
	int nComplete;	//実行コマンド実績数 0:Start, 1-nCommand:完了数, -値:error
	char*	commsg[MC_COMMAND_MAX];
	char*	resmsg[MC_COMMAND_MAX];
	int		nlen_commsg[MC_COMMAND_MAX];
	int		nlen_resmsg[MC_COMMAND_MAX];
	HANDLE	hexit;	//スレッドを終了させるイベントハンドル
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

