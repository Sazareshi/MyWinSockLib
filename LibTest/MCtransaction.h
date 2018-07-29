#pragma once

#include "CSock.h"

#define MC_DATA_MAX 2048
#define MC_TRANSACTION_MAX 16

#define MC_STP_IDLE		0
#define MC_STP_WAIT_RES 1
#define MC_STP_END		2

#define MC_TYP_READ_D	1
#define MC_TYP_READ_R	2
#define MC_TYP_WRITE_D	4
#define MC_TYP_WRITE_R	8

//共通ヘッダ
typedef struct _stMCCommonHeader
{
	BYTE subheader[2];			//サブヘッダ
	BYTE accessRoute_nwno;		//アクセス経路　ネットワークNo.
	BYTE accessRoute_pcno;		//アクセス経路　ネットワークNo.
	BYTE accessRoute_unitio[3];	//アクセス経路　ユニットIO番号、局番
}MCComHeader, *LPMCComHeader;

//デバイス読み出しフォーマット
typedef struct _stMCFormatCmd0401
{
	MCComHeader common_header;	//共通ヘッダ
	BYTE data_len[2];			//要求データ長
	BYTE wait_time[2];			//監視タイマー

	BYTE cmd[2];
	BYTE sub_cmd[2];
	BYTE device_top[3];
	BYTE device_code;
	BYTE device_num[2];
}MCCmd0401ReadData, *LPMCCmd0401ReadData;

typedef struct _stMCFormatRes0401
{
	MCComHeader common_header;	//共通ヘッダ
	BYTE data_len[2];			//要求データ長
	BYTE end_code[2];			//終了コード
	
	BYTE data[MC_DATA_MAX];		//読み出しデータ or エラー情報
}MCRes0401ReadData, *LPMCRes0401ReadData;

//デバイス書き込みフォーマット
typedef struct _stMCFormatCmd1401
{
	MCComHeader common_header;	//共通ヘッダ
	BYTE data_len[2];			//要求データ長
	BYTE wait_time[2];			//監視タイマー

	BYTE cmd[2];
	BYTE sub_cmd[2];
	BYTE device_top[3];
	BYTE device_code;
	BYTE device_num[2];

	BYTE data[MC_DATA_MAX];//書き込みデータ

}MCCmd1401WriteData, *LPMCCmd1401WriteData;

typedef struct _stMCFormatRes1401
{
	MCComHeader common_header;	//共通ヘッダ
	BYTE data_len[2];			//要求データ長
	BYTE end_code[2];			//終了コード

	//エラー情報
	BYTE err_accessRoute_nwno;		//アクセス経路　ネットワークNo.
	BYTE err_accessRoute_pcno;		//アクセス経路　PCNo.
	BYTE err_accessRoute_unitio[3];	//アクセス経路　ユニットIO番号、局番
	BYTE err_cmd[2];				//エラーコマンドコード
	BYTE err_sub_cmd[2];			//エラーコマンドサブコード

}MCRes1401WriteData, *LPMCRes1401WriteData;

typedef struct _stMCCmdData
{
	union
	{
		MCCmd0401ReadData cmd0401;
		MCCmd1401WriteData cmd1401;
	};
}MCCMD, *LPMCCMD;

typedef struct _stMCResData
{
	union
	{
		MCRes0401ReadData res0401;
		MCRes1401WriteData res1401;
	};
}MCRES, *LPMCRES;


typedef struct _stMCTransactionMng
{
	int nCommand;
	int pregress;// -1:idle, 0:start taransaction, 1-nComm: num of complete
	int step[MC_TRANSACTION_MAX];
	int com_msg_len[MC_TRANSACTION_MAX];
	MCCMD com_msg[MC_TRANSACTION_MAX];
	MCRES res_msg[MC_TRANSACTION_MAX];
}MCMsgMng, LPMCMsgMng;

class CMCtransaction
{
public:
	int n_act;									//活動中のトランザクション数
	MCComHeader	mc_header;						//MCプロトコル共通ヘッダ
	MCMsgMng mcifmng;
	int req_transaction(int nCommand);			//トランザクション要求
	int set_com_msg(int pos, int type, int writelen, ...);//コマンドメッセージセット

public:
	CMCtransaction();
	~CMCtransaction();

};

