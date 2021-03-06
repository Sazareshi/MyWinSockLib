#pragma once

#include "resource.h"

///# ベース設定

//-タスク設定表示部
#define TARGET_RESOLUTION			1		//マルチメディアタイマーイベント分解能
#define SYSTEM_TICK_ms				25		//メインスレッド周期
#define MAX_APL_TASK				8		//スレッド最大数
#define INITIAL_TASK_STACK_SIZE		16384	//各スレッド用スタックサイズ

//-Main Windowの初期サイズ、位置設定
#define MAIN_WND_INIT_SIZE_W		640
#define MAIN_WND_INIT_SIZE_H		480
#define MAIN_WND_INIT_POS_X			20
#define MAIN_WND_INIT_POS_Y			20

//-Tweet Message表示用Static Windowのサイズ設定値
#define MSG_WND_H					20	//Height of window
#define MSG_WND_W					570	//Width of window
#define MSG_WND_ORG_X				35	//Origin X of window arrangement 
#define MSG_WND_ORG_Y				10	//Origin Y of window arrangement 
#define MSG_WND_Y_SPACE				12	//Space between windows Y direction 
#define TWEET_IMG_ORG_X				5	//Origin X of bitmap icon arrangement

//-各タスクアイコン用イメージリスト設定値
#define ICON_IMG_W					32	//イメージ幅
#define ICON_IMG_H					32	//イメージ高さ

//-各タスク設定用タブ関連
#define TAB_DIALOG_W				600
#define TAB_DIALOG_H				250
#define TAB_POS_X					10
#define TAB_POS_Y					MSG_WND_ORG_Y+(MSG_WND_H+MSG_WND_Y_SPACE)*TASK_NUM+10
#define TAB_SIZE_H					25

//-ID定義 Mainスレッド用　WM_USER + 1000 + α
#define ID_STATUS					WM_USER + 1001
#define ID_STATIC_MAIN				WM_USER + 1002
#define ID_TASK_SET_TAB				WM_USER + 1003


//-.ini 関連設定


#define PATH_OF_INIFILE					L"./app.ini"	//オプショナルパス

#define OPATH_SECT_OF_INIFILE			L"PATH"			//オプショナルパスセクション
#define OBJ_NAME_SECT_OF_INIFILE		L"OBJECT_NAME"	//オブジェクトフルネームセクション
#define OBJ_SNAME_SECT_OF_INIFILE		L"OBJECT_SNAME"	//オブジェクト略称セクション

//各セクションの各タスク用の情報を参照するためのキー
#define COMMUNICATOR_KEY_OF_INIFILE		L"COMM"
#define ANALYST_KEY_OF_INIFILE			L"ANALYST"
#define DRIVER_KEY_OF_INIFILE			L"DRIVER"
#define COODINATOR_KEY_OF_INIFILE		L"COODINATOR"
#define DEFAULT_KEY_OF_INIFILE			L"DEFAULT"
#define HELPER_KEY_OF_INIFILE			L"HELPER"
#define DUMMY_KEY_OF_INIFILE			L"DUMMY"

///#  アプリケーション起動からの時刻記録用構造体
typedef struct
{
	unsigned short DD;// day
	unsigned short HH;// hour
	unsigned short MM;// minute
	unsigned short SS;// second
	unsigned short MS;// millisecond
} TIME_DEF;

///# マルチスレッド管理用構造体
typedef struct knl_manage_set_tag {
	WORD mmt_resolution = TARGET_RESOLUTION;	//マルチメディアタイマーの分解能
	unsigned int cycle_base = SYSTEM_TICK_ms;
	WORD KnlTick_TimerID;						//マルチメディアタイマーのID
	unsigned int num_of_task = 0;		//アプリケーションで利用するスレッド数
	unsigned long sys_counter = 0;
	TIME_DEF Knl_Time;

	unsigned int stackSize = INITIAL_TASK_STACK_SIZE;	//タスクの初期スタックサイズ
}ST_KNL_MANAGE_SET, *P_ST_MANAGE_SET;


#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

