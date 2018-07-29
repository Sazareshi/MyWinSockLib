#include "stdafx.h"
#include "MCtransaction.h"


CMCtransaction::CMCtransaction(){}

CMCtransaction::~CMCtransaction(){}

//トランザクション要求
int CMCtransaction::req_transaction(int nCommand) {
	return 1;
}

int CMCtransaction::set_com_msg(int pos, int type, int writelen, ...)
{
	va_list arguments;
	BYTE * pch;

	va_start(arguments, type);//引数の初期化 typeより後ろの全引数をリストに格納

	if (type & (MC_TYP_READ_D | MC_TYP_READ_R | MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {

		//要求データ長	
		*((PUINT16)(mcifmng.com_msg[pos].cmd1401.data_len)) = 2 + 10 + writelen; //監視タイマ (2byte)+ 要求データ(10+writelen byte) 

		 //監視タイマー　1秒=250msx4	
		mcifmng.com_msg[pos].cmd0401.wait_time[0] = 0x00;
		mcifmng.com_msg[pos].cmd0401.wait_time[0] = 0x04;

		//コマンド
		if (type & (MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {
			mcifmng.com_msg[pos].cmd1401.cmd[0] = 0x04;
			mcifmng.com_msg[pos].cmd1401.cmd[1] = 0x01;
		}
		else if (type & (MC_TYP_READ_D | MC_TYP_READ_R))
		{
			mcifmng.com_msg[pos].cmd1401.cmd[0] = 0x14;
			mcifmng.com_msg[pos].cmd1401.cmd[1] = 0x01;
		}
		else;

		//要求データ
		mcifmng.com_msg[pos].cmd1401.sub_cmd[0] = 0x00;
		mcifmng.com_msg[pos].cmd1401.sub_cmd[1] = 0x00;

		pch = va_arg(arguments, BYTE*);
		mcifmng.com_msg[pos].cmd0401.device_top[0] = *pch; pch++;
		mcifmng.com_msg[pos].cmd0401.device_top[1] = *pch; pch++;
		mcifmng.com_msg[pos].cmd0401.device_top[2] = *pch; pch++;

		mcifmng.com_msg[pos].cmd0401.device_code = *(va_arg(arguments, BYTE*));

		pch = va_arg(arguments, BYTE*);
		mcifmng.com_msg[pos].cmd0401.device_num[0] = *pch; pch++;
		mcifmng.com_msg[pos].cmd0401.device_num[1] = *pch;

		pch = va_arg(arguments, BYTE*);//書き込みデータ
		for (int i = 0; i<writelen; i++, pch++) mcifmng.com_msg[pos].cmd1401.data[i] = *pch;

		//ソケット送信用データ長
		mcifmng.com_msg_len[pos] = sizeof(MCComHeader) + 2 + 2 + 10 + writelen;

	}

	va_end(arguments);//引数の初期化 typeより後ろの全引数をリストに格納
	return 0;
}

