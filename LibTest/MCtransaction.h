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

//���ʃw�b�_
typedef struct _stMCCommonHeader
{
	BYTE subheader[2];			//�T�u�w�b�_
	BYTE accessRoute_nwno;		//�A�N�Z�X�o�H�@�l�b�g���[�NNo.
	BYTE accessRoute_pcno;		//�A�N�Z�X�o�H�@�l�b�g���[�NNo.
	BYTE accessRoute_unitio[3];	//�A�N�Z�X�o�H�@���j�b�gIO�ԍ��A�ǔ�
}MCComHeader, *LPMCComHeader;

//�f�o�C�X�ǂݏo���t�H�[�}�b�g
typedef struct _stMCFormatCmd0401
{
	MCComHeader common_header;	//���ʃw�b�_
	BYTE data_len[2];			//�v���f�[�^��
	BYTE wait_time[2];			//�Ď��^�C�}�[

	BYTE cmd[2];
	BYTE sub_cmd[2];
	BYTE device_top[3];
	BYTE device_code;
	BYTE device_num[2];
}MCCmd0401ReadData, *LPMCCmd0401ReadData;

typedef struct _stMCFormatRes0401
{
	MCComHeader common_header;	//���ʃw�b�_
	BYTE data_len[2];			//�v���f�[�^��
	BYTE end_code[2];			//�I���R�[�h
	
	BYTE data[MC_DATA_MAX];		//�ǂݏo���f�[�^ or �G���[���
}MCRes0401ReadData, *LPMCRes0401ReadData;

//�f�o�C�X�������݃t�H�[�}�b�g
typedef struct _stMCFormatCmd1401
{
	MCComHeader common_header;	//���ʃw�b�_
	BYTE data_len[2];			//�v���f�[�^��
	BYTE wait_time[2];			//�Ď��^�C�}�[

	BYTE cmd[2];
	BYTE sub_cmd[2];
	BYTE device_top[3];
	BYTE device_code;
	BYTE device_num[2];

	BYTE data[MC_DATA_MAX];//�������݃f�[�^

}MCCmd1401WriteData, *LPMCCmd1401WriteData;

typedef struct _stMCFormatRes1401
{
	MCComHeader common_header;	//���ʃw�b�_
	BYTE data_len[2];			//�v���f�[�^��
	BYTE end_code[2];			//�I���R�[�h

	//�G���[���
	BYTE err_accessRoute_nwno;		//�A�N�Z�X�o�H�@�l�b�g���[�NNo.
	BYTE err_accessRoute_pcno;		//�A�N�Z�X�o�H�@PCNo.
	BYTE err_accessRoute_unitio[3];	//�A�N�Z�X�o�H�@���j�b�gIO�ԍ��A�ǔ�
	BYTE err_cmd[2];				//�G���[�R�}���h�R�[�h
	BYTE err_sub_cmd[2];			//�G���[�R�}���h�T�u�R�[�h

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
	int n_act;									//�������̃g�����U�N�V������
	MCComHeader	mc_header;						//MC�v���g�R�����ʃw�b�_
	MCMsgMng mcifmng;
	int req_transaction(int nCommand);			//�g�����U�N�V�����v��
	int set_com_msg(int pos, int type, int writelen, ...);//�R�}���h���b�Z�[�W�Z�b�g

public:
	CMCtransaction();
	~CMCtransaction();

};

