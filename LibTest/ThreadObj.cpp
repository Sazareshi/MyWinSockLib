#pragma once

#include "stdafx.h"
#include "ThreadObj.h"

#include <windowsx.h>
#include <commctrl.h> //コモンコントロール用


using namespace std;

//# マルチスレッド基本クラス メンバー関数 

CThreadObj::CThreadObj() {//コンストラクタ
	inf.hndl = NULL; 
	inf.cycle_ms = DEFAUT_TASK_CYCLE;
	inf.thread_com = REPEAT_INFINIT;
	wsprintf(inf.name, L"Who am I?"); 
	inf.act_count = inf.act_time = 0;
	inf.priority = THREAD_PRIORITY_NORMAL;
	inf.work_select = THREAD_WORK_IDLE;
	inf.hWnd_parent = inf.hWnd_work = inf.hWnd_opepane = inf.hWnd_msgStatics = NULL;
	inf.hBmp = NULL;
};

CThreadObj::~CThreadObj() {
	delete inf.hBmp;
};//デストラクタ

//# スレッドメイン関数
unsigned CThreadObj::run(void *param) {

	while (this->inf.thread_com != TERMINATE_THREAD){
		WaitForSingleObject(((CThreadObj*)param)->inf.hevent, INFINITE);//メインスレッドからのSIGNAL状態待ち

		//処理周期確認用
		DWORD start_time = timeGetTime();
		inf.period = start_time - inf.start_time;
		inf.start_time = start_time;

		if (inf.work_select == THREAD_WORK_ROUTINE) routine_work(param);
		else if (inf.work_select == THREAD_WORK_OPTION1) optional_work1(param);
		else if (inf.work_select == THREAD_WORK_OPTION2) optional_work2(param);
		else default_work(param);//実処理

		inf.act_time = timeGetTime() - start_time;//実処理時間測定用
		if (this->inf.thread_com != REPEAT_INFINIT) break;//逐次処理の場合は抜ける
	}
	return 0;
}

void CThreadObj::routine_work(void *param) {

	ws << L"I am working!"<< *(inf.psys_counter);
	tweet2owner(ws.str()); ws.str(L"") ; ws.clear();
};

//# メインウィンドウへのショートメッセージ表示　string
void CThreadObj::tweet2owner(const std::string &src) {
	wstring srcw;
	tool.Str2Wstr(src, srcw);
	if(inf.hWnd_msgStatics != NULL) SetWindowText(inf.hWnd_msgStatics, srcw.c_str()); return;
};

//# メインウィンドウへのショートメッセージ表示　wstring
void CThreadObj::tweet2owner(const std::wstring &srcw) {
	if (inf.hWnd_msgStatics != NULL) SetWindowText(inf.hWnd_msgStatics, srcw.c_str()); return;
}; //メインウィンドウへのショートメッセージ表示

void CThreadObj::txout2msg_listbox(std::wstring str) {

};
void CThreadObj::txout2msg_listbox(std::string str) {

};
LRESULT CALLBACK CThreadObj::DlgProcDefault(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
	return FALSE;
};
LRESULT CALLBACK CThreadObj::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	return 0;
};
HWND CThreadObj::CreateOwnWindow(HWND h_parent_wnd) {

	return inf.hWnd_work;
};

void CThreadObj::set_PNLparam_value(float p1, float p2, float p3, float p4, float p5, float p6) {

}
void CThreadObj::set_panel_tip_txt()
{

}

