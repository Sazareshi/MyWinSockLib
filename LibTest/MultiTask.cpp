// MultiTask.cpp: アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "MultiTask.h"

#include "ThreadObj.h"		//タスクスレッドのクラス
#include "Communicator.h"	//コミュニケーションタスクスレッドのクラス
#include "Helper.h"		//補助関数集合クラス

#include <windowsx.h> //コモンコントロール用
#include <commctrl.h> //コモンコントロール用

using namespace std;

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名


///*****************************************************************************************************************
/// #ここにアプリケーション専用の関数を追加しています。

LRESULT CALLBACK TaskTabDlgProc(HWND, UINT, WPARAM, LPARAM);//個別タスク設定タブウィンドウ用メッセージハンドリング関数
HWND	CreateStatusbarMain(HWND);	//メインウィンドウステータスバー作成関数
VOID	CALLBACK alarmHandlar(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);	//マルチメディアタイマ処理関数　スレッドのイベントオブジェクト処理
int		Init_tasks(HWND hWnd);	//アプリケーション毎のタスクオブジェクトの初期設定
DWORD	knlTaskStartUp();	//実行させるタスクの起動処理
static unsigned __stdcall thread_gate_func(void * pObj) { //スレッド実行のためのゲート関数
	CThreadObj * pthread_obj = (CThreadObj *)pObj;  
	return pthread_obj->run(pObj); 
}
///*****************************************************************************************************************
/// #ここにアプリケーション専用のグローバル変数　または、スタティック変数を追加しています。
static HWND hWnd_status_bar;//ステータスバーのウィンドウのハンドル
static HWND hTabWnd;//操作パネル用タブコントロールウィンドウのハンドル
static ST_KNL_MANAGE_SET knl_manage_set;//マルチスレッド管理用構造体
static vector<HWND>	VectTweetHandle;	//メインウィンドウのスレッドツイートメッセージ表示Staticハンドル
static vector<HANDLE>	VectHevent;		//マルチスレッド用イベントのハンドル
static vector<void*>	VectpThreadObj;	//スレッドオブジェクト
static HIMAGELIST	hImgListTaskIcon;	//タスクアイコン用イメージリスト

///*****************************************************************************************************************

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


// # 関数: wWinMain ************************************
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化しています。
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTITASK, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーションの初期化を実行します:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTITASK));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

// # 関数: MyRegisterClass ウィンドウ クラスを登録します。***
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTITASK));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTITASK);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// # 関数: InitInstance インスタンス ハンドルを保存して、メイン ウィンドウを作成します。***
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	   MAIN_WND_INIT_POS_X, MAIN_WND_INIT_POS_Y,
	   TAB_DIALOG_W + 40, (MSG_WND_H + MSG_WND_Y_SPACE)*TASK_NUM + TAB_DIALOG_H + 130,
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd) return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   
   /// -タスク設定	
   Init_tasks(hWnd);//タスク個別設定
 
  /// -タスク起動									
   knlTaskStartUp(); 

   /// -マルチメディアタイマ起動
   {
	/// > マルチメディアタイマ精度設定
	TIMECAPS wTc;//マルチメディアタイマ精度構造体
	if (timeGetDevCaps(&wTc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 	return((DWORD)FALSE);
	 knl_manage_set.mmt_resolution = MIN(MAX(wTc.wPeriodMin, TARGET_RESOLUTION), wTc.wPeriodMax);
	if (timeBeginPeriod(knl_manage_set.mmt_resolution) != TIMERR_NOERROR) return((DWORD)FALSE);

	 _RPT1(_CRT_WARN, "MMTimer Period = %d\n", knl_manage_set.mmt_resolution);

	 /// > マルチメディアタイマセット
	 knl_manage_set.KnlTick_TimerID = timeSetEvent(knl_manage_set.cycle_base, knl_manage_set.mmt_resolution, (LPTIMECALLBACK)alarmHandlar, 0, TIME_PERIODIC);

	 /// >マルチメディアタイマー起動失敗判定　メッセージBOX出してFALSE　returen
	 if (knl_manage_set.KnlTick_TimerID == 0) {	 //失敗確認表示
	   LPVOID lpMsgBuf;
	   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		   0, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language*/(LPTSTR)&lpMsgBuf, 0, NULL);
	   MessageBox(NULL, (LPCWSTR)lpMsgBuf, L"MMT Failed!!", MB_OK | MB_ICONINFORMATION);// Display the string.
	   LocalFree(lpMsgBuf);// Free the buffer.
	   return((DWORD)FALSE);
	}
   }
   return TRUE;
}

// # 関数: メイン ウィンドウのメッセージを処理します。*********************************
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CREATE: {
		///メインウィンドウ　ステータスバー　クリエイト
		hWnd_status_bar = CreateStatusbarMain(hWnd);
		}
		break;

	case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			//タスクツィートメッセージアイコン描画
			for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) ImageList_Draw(hImgListTaskIcon, i, hdc, 0, i*(MSG_WND_H+ MSG_WND_Y_SPACE), ILD_NORMAL);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// # 関数:バージョン情報ボックスのメッセージ ハンドラーです。**************************
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

///# 関数: スレッドタスクの登録、設定 ***
int  Init_tasks(HWND hWnd) {

	HBITMAP hBmp;
	CThreadObj *ptempobj;

	InitCommonControls();//コモンコントロール初期化
	hImgListTaskIcon = ImageList_Create(32, 32, ILC_COLOR | ILC_MASK, 2, 0);//タスクアイコン表示用イメージリスト設定

	//###Task1 設定
	{
		/// -タスクインスタンス作成->リスト登録
		ptempobj = new CCommunicator;
		VectpThreadObj.push_back((void*)ptempobj);

		/// -イベントオブジェクトクリエイト->リスト登録	
		VectHevent.push_back(ptempobj->inf.hevent = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

		/// -スレッド起動周期セット
		ptempobj->inf.cycle_ms = 100;

		/// -ツイートメッセージ用iconセット
		hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_COMM");//ビットマップ割り当て
		ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255)); 
		DeleteObject(hBmp);

		 ///オブジェクト名セット
		DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, COMMUNICATOR_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name), PATH_OF_INIFILE);
				str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, COMMUNICATOR_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname), PATH_OF_INIFILE);

		///実行関数選択
				ptempobj->inf.work_select = THREAD_WORK_ROUTINE;
	}
	//###Task2 設定
	{
		/// -タスクインスタンス作成->リスト登録
		ptempobj = new CThreadObj;
		VectpThreadObj.push_back((void*)ptempobj);

		/// -イベントオブジェクトクリエイト->リスト登録	
		VectHevent.push_back(ptempobj->inf.hevent = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

																						   /// -スレッド起動周期セット
		ptempobj->inf.cycle_ms = 1000;

		/// -ツイートメッセージ用iconセット
		hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_YU");//ビットマップ割り当て
		ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
		DeleteObject(hBmp);

		///オブジェクト名セット
		DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, DEFAULT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name), PATH_OF_INIFILE);
		str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, DEFAULT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname), PATH_OF_INIFILE);

		///実行関数選択
		ptempobj->inf.work_select = THREAD_WORK_ROUTINE;

	}
	
	//各タスク残パラメータセット
	knl_manage_set.num_of_task = (unsigned int)VectpThreadObj.size();//タスク数登録
	for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) {
		CThreadObj * pobj = (CThreadObj *)VectpThreadObj[i];
	
		pobj->inf.index = i;	//タスクインデックスセット
		
		// -ツイートメッセージ用Static window作成->リスト登録	
		pobj->inf.hWnd_msgStatics = CreateWindow(L"STATIC", L"...", WS_CHILD | WS_VISIBLE, MSG_WND_ORG_X, MSG_WND_ORG_Y + MSG_WND_H * i + i * MSG_WND_Y_SPACE, MSG_WND_W, MSG_WND_H, hWnd, (HMENU)ID_STATIC_MAIN, hInst, NULL);
		VectTweetHandle.push_back(pobj->inf.hWnd_msgStatics);
		
		//その他設定
		pobj->inf.psys_counter = &knl_manage_set.sys_counter;
		pobj->inf.act_count = 0;//起動チェック用カウンタリセット
		 //起動周期カウント値
		if (pobj->inf.cycle_ms >= SYSTEM_TICK_ms)	pobj->inf.cycle_count = pobj->inf.cycle_ms / SYSTEM_TICK_ms;
		else pobj->inf.cycle_count = 1;

		///最後に初期化
		pobj->init_task(pobj);

	}
		
	//WM_PAINTを発生させてアイコンを描画させる
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);

	return 1;
}

///# 関数: マルチタスクスタートアップ処理関数 ***
DWORD knlTaskStartUp()	//実行させるオブジェクトのリストのスレッドを起動
{
	//機能	：[KNL]システム/ユーザタスクスタートアップ関数
	//処理	：自プロセスのプライオリティ設定，カーネルの初期設定,タスク生成，基本周期設定
	//戻り値：Win32APIエラーコード
	
	HANDLE	myPrcsHndl;	/* 本プログラムのプロセスハンドル */
						///# 自プロセスプライオリティ設定処理
						//-プロセスハンドル取得
	if ((myPrcsHndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, _getpid())) == NULL)	return(GetLastError());
	_RPT1(_CRT_WARN, "KNL Priority For Windows(before) = %d \n", GetPriorityClass(myPrcsHndl));

	//-自プロセスのプライオリティを最優先ランクに設定
	if (SetPriorityClass(myPrcsHndl, REALTIME_PRIORITY_CLASS) == 0)	return(GetLastError());
	_RPT1(_CRT_WARN, "KNL Priority For NT(after) = %d \n", GetPriorityClass(myPrcsHndl));

	///# アプリケーションタスク数が最大数を超えた場合は終了
	if (VectpThreadObj.size() >= MAX_APL_TASK)	return((DWORD)ERROR_BAD_ENVIRONMENT);

	///#    アプリケーションスレッド生成処理	
	for (unsigned i = 0; i < VectpThreadObj.size(); i++) {

		CThreadObj * pobj = (CThreadObj *)VectpThreadObj[i];

		// タスク生成(スレッド生成)
		// 他ﾌﾟﾛｾｽとの共有なし,スタック初期サイズ　デフォルト, スレッド実行関数　引数で渡すオブジェクトで対象切り替え,スレッド関数の引数（対象のオブジェクトのポインタ）, 即実行Createflags, スレッドID取り込み
		pobj->inf.hndl = (HANDLE)_beginthreadex((void *)NULL, 0, thread_gate_func, VectpThreadObj[i], (unsigned)0, (unsigned *)&(pobj->inf.ID));

		// タスクプライオリティ設定
		if (SetThreadPriority(pobj->inf.hndl, pobj->inf.priority) == 0)
			return(GetLastError());
		_RPT2(_CRT_WARN, "Task[%d]_priority = %d\n", i, GetThreadPriority(pobj->inf.hndl));

		pobj->inf.act_count = 0;		// 基本ティックのカウンタ変数クリア
		pobj->inf.time_over_count = 0;	// 予定周期オーバーカウントクリア
	}

	return 1;
}

///# 関数: マルチタイマーイベント処理関数 ****************************************************
VOID CALLBACK alarmHandlar( UINT uID, UINT uMsg, DWORD	dwUser,	DWORD dw1, DWORD dw2){

	knl_manage_set.sys_counter++;

	//起動後経過時間計算
	knl_manage_set.Knl_Time.MS += knl_manage_set.cycle_base;
	if (knl_manage_set.Knl_Time.MS >= 1000) {
		knl_manage_set.Knl_Time.MS -= 1000; knl_manage_set.Knl_Time.SS++;
		if (knl_manage_set.Knl_Time.SS >= 60) {
			knl_manage_set.Knl_Time.SS -= 60; knl_manage_set.Knl_Time.MM++;
			if (knl_manage_set.Knl_Time.MM >= 60) {
				knl_manage_set.Knl_Time.MM -= 60; knl_manage_set.Knl_Time.HH++;
				if (knl_manage_set.Knl_Time.HH >= 24) {
					knl_manage_set.Knl_Time.HH -= 24; knl_manage_set.Knl_Time.DD++;
				}
			}
		}
	}

	//スレッド起動イベント処理
	for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) {
		CThreadObj * pobj = (CThreadObj *)VectpThreadObj[i];
		pobj->inf.act_count++;
		if (pobj->inf.act_count >= pobj->inf.cycle_count) {
			PulseEvent(VectHevent[i]);
			pobj->inf.act_count = 0;
			pobj->inf.total_act++;
		}
	}

	//Statusバーに経過時間表示
	if (knl_manage_set.Knl_Time.MS % 1000 == 0) {// 1sec毎
		TCHAR tbuf[32];
		wsprintf(tbuf, L"%3dD %02d:%02d:%02d", knl_manage_set.Knl_Time.DD, knl_manage_set.Knl_Time.HH, knl_manage_set.Knl_Time.MM, knl_manage_set.Knl_Time.SS);
		SendMessage(hWnd_status_bar, SB_SETTEXT, 5, (LPARAM)tbuf);
	}
}

///# 関数: ステータスバー作成 ***************
HWND CreateStatusbarMain(HWND hWnd)
{
	HWND hSBWnd;
	int sb_size[] = { 100,200,300,400,525,615 };//ステータス区切り位置

	InitCommonControls();
	hSBWnd = CreateWindowEx(
		0, //拡張スタイル
		STATUSCLASSNAME, //ウィンドウクラス
		NULL, //タイトル
		WS_CHILD | SBS_SIZEGRIP, //ウィンドウスタイル
		0, 0, //位置
		0, 0, //幅、高さ
		hWnd, //親ウィンドウ
		(HMENU)ID_STATUS, //ウィンドウのＩＤ
		hInst, //インスタンスハンドル
		NULL);
	SendMessage(hSBWnd, SB_SETPARTS, (WPARAM)6, (LPARAM)(LPINT)sb_size);//6枠で各枠の仕切り位置をパラーメータ指定
	ShowWindow(hSBWnd, SW_SHOW);
	return hSBWnd;
} 

///# 関数: タブ付タスクウィンドウ作成 *******
HWND CreateTaskSettingWnd(HWND hWnd) {
	
	RECT rc;
	TC_ITEM tc[TASK_NUM];

	GetClientRect(hWnd, &rc);
	HWND hTab = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		rc.left + TAB_POS_X, rc.top + TAB_POS_Y, TAB_DIALOG_W, TAB_DIALOG_H, hWnd, (HMENU)ID_TASK_SET_TAB, hInst, NULL);

		for (unsigned i = 0; i < VectpThreadObj.size(); i++) {//Task Setting用タブ作成
		CThreadObj* pObj = (CThreadObj *)VectpThreadObj[i];

		tc[i].mask = (TCIF_TEXT | TCIF_IMAGE);
		tc[i].pszText = pObj->inf.sname;
		tc[i].iImage = pObj->inf.index;
		SendMessage(hTab, TCM_INSERTITEM, (WPARAM)0, (LPARAM)&tc[i]);
		pObj->inf.hWnd_opepane = CreateDialog(hInst, L"IDD_DIALOG_TASKSET1", hWnd, (DLGPROC)TaskTabDlgProc);
		pObj->set_panel_pb_txt();
	
		//初期値はindex 0のウィンドウを表示
		if (i == 0) {
			ShowWindow(pObj->inf.hWnd_opepane, SW_SHOW);
			SetWindowText(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TAB_TASKNAME), pObj->inf.name);//タスク名をスタティックテキストに表示
		}
		else ShowWindow(pObj->inf.hWnd_opepane, SW_HIDE);
	}
	
	//タブコントロールにイメージリストセット
	SendMessage(hTab, TCM_SETIMAGELIST, (WPARAM)0, (LPARAM)hImgListTaskIcon);
	return hTab;
}

static LVCOLUMNA lvcol;

LRESULT CALLBACK TaskTabDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG: {
		//メッセージ用リスト
		InitCommonControls();

		LVCOLUMN lvcol;
		LPTSTR strItem0[] = { (LPTSTR)"time", (LPTSTR)"message" };
		int CX[] = { 50, 600 };

		//リストコントロール設定
		HWND hList = GetDlgItem(hDlg, IDC_LIST1);
		lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvcol.fmt = LVCFMT_LEFT;
		//列追加
		for (int i = 0; i < 2; i++) {
			lvcol.cx = CX[i];             // 表示位置
			lvcol.pszText = strItem0[i];  // 見出し
			lvcol.iSubItem = i;           // サブアイテムの番号
			ListView_InsertColumn(hList, i, &lvcol);
		}
		//行追加
		LVITEM item;
		item.mask = LVIF_TEXT;
		for (int i = 0; i < MSG_LIST_MAX; i++) {
			item.pszText = (LPWSTR)L"";   // テキスト
			item.iItem = i;               // 番号
			item.iSubItem = 0;            // サブアイテムの番号
			ListView_InsertItem(hList, &item);
		}
		return TRUE;
	}break;
	case WM_COMMAND:

		CThreadObj * pObj = (CThreadObj *) VectpThreadObj[VectpThreadObj.size() - TabCtrl_GetCurSel(hTabWnd) - 1];
		pObj->PanelProc(hDlg, msg, wp, lp);

		switch (LOWORD(wp)) {
		
			case IDSET: // 更新ボタンが押されました
		/*
			swprintf_s(str, L"SETPB_OK");
			Edit_SetText(GetDlgItem(hDlg, IDC_TASK_EDIT2),str, sizeof(str));
			InvalidateRect(GetParent(hDlg),NULL,TRUE);
			return TRUE;
			case IDRESET:
			swprintf_s(str, L"RESETPB_OK");
			Edit_SetText(GetDlgItem(hDlg, IDC_TASK_EDIT2), str, sizeof(str));
			InvalidateRect(GetParent(hDlg), NULL, TRUE);
			return TRUE;
		*/
			break;

			default: break;
		}
		break;
	}
	return FALSE;
}





