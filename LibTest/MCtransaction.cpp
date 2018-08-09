#include "stdafx.h"
#include "MCtransaction.h"


CMCtransaction::CMCtransaction(){}

CMCtransaction::~CMCtransaction(){}


int CMCtransaction::set_com_msg(int pos, int type, int writelen, ...)//�������ރ��W�X�^�̃o�C�g��
{
	va_list arguments;
	
	//���ʃw�b�_
	mcifmng.com_msg[pos].cmd0401.common_header.subheader[0] = 0x50;//�T�u�w�b�_
	mcifmng.com_msg[pos].cmd0401.common_header.subheader[1] = 0x00;//�T�u�w�b�_
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_nwno = 0x00;//�l�b�g���[�N�ԍ�
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_pcno = 0xff;//PC�ԍ�
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[0] = 0xff;//���j�b�gIO
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[1] = 0x03;//���j�b�gIO
	mcifmng.com_msg[pos].cmd0401.common_header.accessRoute_unitio[2] = 0x00;//���j�b�gIO

	//�ʃ��b�Z�[�W	
	va_start(arguments, writelen);//�����̏����� writelen�����̑S���������X�g�Ɋi�[

	if (type & (MC_TYP_READ_D | MC_TYP_READ_R | MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {

		//�v���f�[�^��	
		*((PUINT16)(mcifmng.com_msg[pos].cmd1401.data_len)) = 2 + 10 + writelen; //�Ď��^�C�} (2byte)+ �v���f�[�^(10+writelen byte) 

		 //�Ď��^�C�}�[�@1�b=250msx4	
		mcifmng.com_msg[pos].cmd0401.wait_time[0] = 0x04;
		mcifmng.com_msg[pos].cmd0401.wait_time[1] = 0x00;

		//�R�}���h
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
		//�T�u�R�}���h
		mcifmng.com_msg[pos].cmd1401.sub_cmd[0] = 0x00;
		mcifmng.com_msg[pos].cmd1401.sub_cmd[1] = 0x00;

		//�v���f�[�^
		DWORD dw = va_arg(arguments, DWORD);//�擪�f�o�C�X�ԍ�
		mcifmng.com_msg[pos].cmd0401.device_top[0] = (BYTE)(0xff & dw);
		mcifmng.com_msg[pos].cmd0401.device_top[1] = (BYTE)(0xff & (dw>>8));
		mcifmng.com_msg[pos].cmd0401.device_top[2] = (BYTE)(0xff & (dw>>16));

		//�f�o�C�X�R�[�h
		if (type & (MC_TYP_WRITE_R | MC_TYP_READ_R)) mcifmng.com_msg[pos].cmd0401.device_code = 0xb0;
		else mcifmng.com_msg[pos].cmd0401.device_code = 0xa8;

		//�f�o�C�X�_��
		USHORT us = va_arg(arguments, USHORT);
		mcifmng.com_msg[pos].cmd0401.device_num[0] = (BYTE)(0xff & us);
		mcifmng.com_msg[pos].cmd0401.device_num[1] = (BYTE)(0xff & (us) >> 8);

		//�������݃f�[�^
		if (type & (MC_TYP_WRITE_D | MC_TYP_WRITE_R)) {
			BYTE* pch = va_arg(arguments, BYTE*);
			for (int i = 0; i < writelen; i++, pch++) mcifmng.com_msg[pos].cmd1401.data[i] = *pch;//�f�[�^���̒������̒�`�ŏ�������
		}
		//�\�P�b�g���M�p�f�[�^��
		mcifmng.com_msg_len[pos] = sizeof(MCComHeader) + 2 + 2 + 10 + writelen;
	}

	va_end(arguments);//�����̏����� type�����̑S���������X�g�Ɋi�[
	return 0;
}

int CMCtransaction::init() {
	
	//�\�P�b�g�p�f�[�^�N���A
	mcifmng.sock_index = MC_SOCK_NOT_REGISTERED;
	mcifmng.hsock_event = WSA_INVALID_EVENT;
	mcifmng.sock_event_status = 0;
		
	//�R�}���h���s�X�e�b�v������
	mcifmng.sock_index = MC_SOCK_NOT_REGISTERED;
	mcifmng.hsock_event = WSA_INVALID_EVENT;

	
	//�R�}���h���b�Z�[�W�쐬
	//D100�ǂݏo���v���@com_msg[0]
	set_com_msg(COMPOS_READ_D100, MC_TYP_READ_D, 0,100,7);

	//D110�������ݗv���@com_msg[1]
	set_com_msg(COMPOS_WRITE_D110, MC_TYP_WRITE_D, 9, 110, 7, mcifmng.com_msg[COMPOS_WRITE_D110].cmd1401.data);

	//R���W�X�^�ǂݏo���v���@com_msg[2]
	set_com_msg(COMPOS_READ_R0_R303, MC_TYP_READ_R, 0, 0, SIZE_OF_FAULT_TRIGGER);
	set_com_msg(COMPOS_READ_R304__, MC_TYP_READ_R, 304, SIZE_OF_TRACE_RECORD);

	//�Z�b�g����Ă���R�}���h�̐�
	mcifmng.nCommandSet = 4;
	//�R�}���h�̎��s�X�e�b�v������
	for (int i = 0; i < mcifmng.nCommandSet; i++) mcifmng.com_step[i] = MC_STP_IDLE;

	return 0;

};	


int CMCtransaction::Is_tranzaction_ready() {

	for (int i = 0; i < mcifmng.nCommandSet; i++) 
		if(mcifmng.com_step[i] != MC_STP_IDLE) return TRANZACTION_BUSY;

	return TRANZACTION_READY;
};
;	//�R�}���h���M�۔���

	//�g�����U�N�V�����v����t
int CMCtransaction::req_transaction(int nCommand) {
	CSock sock_handler;
	int stat;

	if (stat = Is_tranzaction_ready()) return stat;//���g�����U�N�V�������s��

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

