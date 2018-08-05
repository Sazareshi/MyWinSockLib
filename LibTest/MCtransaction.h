#pragma once

#include "CSock.h"

#define MC_DATA_MAX 2048
#define MC_TRANSACTION_MAX 16

#define MC_STP_IDLE		0
#define MC_STP_START	1
#define MC_STP_WAIT_RES 2


#define MC_TYP_READ_D	1
#define MC_TYP_READ_R	2
#define MC_TYP_WRITE_D	4
#define MC_TYP_WRITE_R	8

#define MC_SOCK_NOT_REGISTERED -1
#define MC_SOCK_USE	 1

#define COMPOS_READ_D100	0
#define COMPOS_WRITE_D110	1
#define COMPOS_READ_R0_R303	2
#define COMPOS_READ_R304__	3

#define SIZE_OF_FAULT_TRIGGER	304
#define SIZE_OF_TRACE_RECORD	96

#define NEW_TRANZACTION_READY	1
#define NEW_TRANZACTION_BUSY	0


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
	int nCommandSet;//�Z�b�g����Ă���R�}���h��
	int com_step[MC_TRANSACTION_MAX];
	int com_msg_len[MC_TRANSACTION_MAX];
	MCCMD com_msg[MC_TRANSACTION_MAX];//�R�}���h�d��
 	MCRES res_msg[MC_TRANSACTION_MAX];//���X�|���X�d��

	HANDLE hsock_event;
	int sock_index;//CSockf��芄�蓖�Ă�ꂽ�\�P�b�g�̃C���f�b�N�X
	USHORT sock_port;//���p����\�P�b�g�̃|�[�g
	PCSTR  sock_ipaddr;//���p����\�P�b�g��IP�A�h���X
	WORD   sock_event_status;
}MCMsgMng, LPMCMsgMng;

class CMCtransaction
{
public:
	MCMsgMng mcifmng;//MC�v���g�R�������Ǘ��\����
	int req_transaction(int nCommand);	//�g�����U�N�V�����v��
	int set_com_msg(int pos, int type, int writelen, ...);//�R�}���h���b�Z�[�W�Z�b�g
	int init();	//������
	int Is_tranzaction_ready();	//�R�}���h���M�۔���

public:
	CMCtransaction();
	~CMCtransaction();

};

