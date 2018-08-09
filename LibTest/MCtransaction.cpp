#include "stdafx.h"
#include "MCtransaction.h"


CMCtransaction::CMCtransaction(){}

CMCtransaction::~CMCtransaction(){}


int CMCtransaction::set_com_msg(int pos, int type, int writelen, ...)//書き込むレジスタのバイト数
{
	va_list arguments;
	
	//共通ヘッダ
	mcifmng.com_msg[pos].cmd0401.common_header.subheader[0] = 0x50;//サブヘッダ
	mcifmng.com_msg[pos].cmd0401.common_header.subheader[1] = 0x00;//サブヘッダ
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_nwno = 0x00;//ネットワーク番号
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_pcno = 0xff;//PC番号
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[0] = 0xff;//ユニットIO
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[1] = 0x03;//ユニットIO
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[2] = 0x00;//ユニットIO

	//個別メッセージ	
	va_start(arguments, writelen);//引数の初期化 writelenより後ろの全引数をリストに格納

	if (type & (MC_TYP_READ_D | MC_TYP_READ_R | MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {

		//要求データ長	
		*((PUINT16)(mcifmng.com_msg[pos].cmd1401.data_len)) = 2 + 10 + writelen; //監視タイマ (2byte)+ 要求データ(10+writelen byte) 

		 //監視タイマー　1秒=250msx4	
		mcifmng.com_msg[pos].cmd0401.wait_time[0] = 0x04;
		mcifmng.com_msg[pos].cmd0401.wait_time[1] = 0x00;

		//コマンド
		if (type & (MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {
			mcifmng.com_msg[pos].cmd1401.cmd[0] = 0x01;
			mcifmng.com_msg[pos].cmd1401.cmd[1] = 0x14;
		}
		else if (type & (MC_TYP_READ_D | MC_TYP_READ_R))
		{
			mcifmng.com_msg[pos].cmd0401.cmd[0] = 0x01;
			mcifmng.com_msg[pos].cmd0401.cmd[1] = 0x04;
		}
		else;
		//サブコマンド
		mcifmng.com_msg[pos].cmd1401.sub_cmd[0] = 0x00;
		mcifmng.com_msg[pos].cmd1401.sub_cmd[1] = 0x00;

		//要求データ
		DWORD dw = va_arg(arguments, DWORD);//先頭デバイス番号
		mcifmng.com_msg[pos].cmd0401.device_top[0] = (BYTE)(0xff & dw);
		mcifmng.com_msg[pos].cmd0401.device_top[1] = (BYTE)(0xff & (dw>>8));
		mcifmng.com_msg[pos].cmd0401.device_top[2] = (BYTE)(0xff & (dw>>16));

		//デバイスコード
		if (type & (MC_TYP_WRITE_R | MC_TYP_READ_R)) mcifmng.com_msg[pos].cmd0401.device_code = 0xb0;
		else mcifmng.com_msg[pos].cmd0401.device_code = 0xa8;

		//デバイス点数
		USHORT us = va_arg(arguments, USHORT);
		mcifmng.com_msg[pos].cmd0401.device_num[0] = (BYTE)(0xff & us);
		mcifmng.com_msg[pos].cmd0401.device_num[1] = (BYTE)(0xff & (us) >> 8);

		//書き込みデータ
		if (type & (MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {
			BYTE* pch = va_arg(arguments, BYTE*);
			for (int i = 0; i < writelen; i++, pch++) mcifmng.com_msg[pos].cmd1401.data[i] = *pch;//データ長の長い方の定義で書き込み
		}
		//ソケット送信用データ長
		mcifmng.com_msg_len[pos] = sizeof(MCComHeader) + 2 + 2 + 10 + writelen;
	}

	va_end(arguments);//引数の初期化 typeより後ろの全引数をリストに格納
	return 0;
}

int CMCtransaction::init() {
	
	//ソケット用データクリア
	mcifmng.sock_index = MC_SOCK_NOT_REGISTERED;
	mcifmng.hsock_event = WSA_INVALID_EVENT;
	mcifmng.sock_event_status = 0;
		
	//コマンド実行ステップ初期化
	mcifmng.sock_index = MC_SOCK_NOT_REGISTERED;
	mcifmng.hsock_event = WSA_INVALID_EVENT;

	
	//コマンドメッセージ作成
	//D100読み出し要求　com_msg[0]
	set_com_msg(COMPOS_READ_D100, MC_TYP_READ_D, 0,100,7);

	//D110書き込み要求　com_msg[1]
	set_com_msg(COMPOS_WRITE_D110, MC_TYP_WRITE_D, 9, 110, 7, mcifmng.com_msg[COMPOS_WRITE_D110].cmd1401.data);

	//Rレジスタ読み出し要求　com_msg[2]
	set_com_msg(COMPOS_READ_R0_R303, MC_TYP_READ_R, 0, 0, SIZE_OF_FAULT_TRIGGER);
	set_com_msg(COMPOS_READ_R304__, MC_TYP_READ_R, 304, SIZE_OF_TRACE_RECORD);

	//セットされているコマンドの数
	mcifmng.nCommandSet = 4;
	//コマンドの実行ステップ初期化
	for (int i = 0; i < mcifmng.nCommandSet; i++) mcifmng.com_step[i] = MC_STP_IDLE;

	return 0;

};	


int CMCtransaction::Is_tranzaction_ready() {

	for (int i = 0; i < mcifmng.nCommandSet; i++) 
		if(mcifmng.com_step[i] != MC_STP_IDLE) return TRANZACTION_BUSY;

	return TRANZACTION_READY;
};
;	//コマンド送信可否判定

	//トランザクション要求受付
int CMCtransaction::req_transaction(int nCommand) {
	CSock sock_handler;
	int stat;

	if (stat = Is_tranzaction_ready()) return stat;//他トランザクション実行中

	if (mcifmng.sock_event_status & FD_WRITE) {
		mcifmng.com_step[nCommand] = MC_STP_START;
		if (mcifmng.com_msg_len[nCommand] != sock_handler.sock_send(mcifmng.sock_index, (const char*)(&(mcifmng.com_msg[nCommand].cmd0401)), mcifmng.com_msg_len[nCommand]) ){
			stat = TRANZACTION_ERROR;
			mcifmng.com_step[nCommand] = MC_STP_IDLE;
			return TRANZACTION_ERROR;
		}
		mcifmng.com_step[nCommand] = MC_STP_WAIT_RES;
	}

	return stat = TRANZACTION_READY;
}

