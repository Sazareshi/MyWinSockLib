#pragma once

#include "helper.h"
#include <process.h>
#include "resource.h"

using namespace std;

//#�^�X�N�����ݒ蕔
#define TARGET_RESOLUTION			1
#define SYSTEM_TICK_ms				25
#define MAX_APL_TASK				8
#define TASK_NUM					6
#define INITIAL_TASK_STACK_SIZE		16384
#define DEFAUT_TASK_CYCLE			50

//# �e�^�X�N�p�����[�^�ݒ�pDialog�A�^�u�֘A�T�C�Y�ݒ�l
#define TAB_DIALOG_W				600
#define TAB_DIALOG_H				250
#define TAB_POS_X					10
#define TAB_POS_Y					MSG_WND_ORG_Y+(MSG_WND_H+MSG_WND_Y_SPACE)*TASK_NUM+10
#define TAB_SIZE_H					25

//inf.thread_com�p�@�X���b�h���[�v����p
#define REPEAT_INFINIT		0	//�i�v���[�v
#define TERMINATE_THREAD	1	//�X���b�h���f
#define REPEAT_ONCE			-1	//��������

//�X���b�h�Ŏ�������֐��̑I��p
#define THREAD_WORK_IDLE		0
#define THREAD_WORK_ROUTINE		1
#define THREAD_WORK_OPTION1		2
#define THREAD_WORK_OPTION2		3

#define MSG_LIST_MAX			14


/***********************************************************************
�^�X�N�I�u�W�F�N�g�̌ʊǗ����\����
************************************************************************/
typedef struct {
	//-�I�u�W�F�N�g���ʏ��
	TCHAR			name[24];						//�I�u�W�F�N�g��
	TCHAR			sname[8];						//�I�u�W�F�N�g����
	HBITMAP			hBmp;							//�I�u�W�F�N�g���ʗp�r�b�g�}�b�v

	//-�X���b�h�ݒ���e
	int				index;							//�X���b�hIndex
	unsigned int	ID;								//�X���b�hID
	HANDLE			hndl;							//�X���b�h�n���h��
	HANDLE			hevent;							//�C�x���g�n���h��
	unsigned int	cycle_ms;						//�X���b�h���s�ݒ�����@ms
	unsigned int	cycle_count;					//�X���b�h���s�ݒ�����@Tick count�ims/system tick)
	int				trigger_type;					//�X���b�h�N�������@�����or�C�x���g
	int				priority;						//�X���b�h�̃v���C�I���e�B
	int				thread_com;						//�X���b�h����t���O �@0:�J��Ԃ��@1:���f�@����ȊO�F��������:

	//-�X���b�h���j�^���
	DWORD			start_time;						//���X�L�����̃X���b�h�J�n����
	DWORD			act_time;						//1�X�L�����̃X���b�h����������
	DWORD			period;							//�X���b�h���[�v��������
	DWORD			act_count;						//�X���b�h���[�v�J�E���^
	DWORD			total_act;						//�N���ώZ�J�E���^
	unsigned int	time_over_count;				//�\��������I�[�o�[������

	//-�֘A�E�B���h�E�n���h��
	HWND			hWnd_parent;					//�e�E�B���h�E�̃n���h��
	HWND			hWnd_msgStatics;				//�e�E�B���h�E���b�Z�[�W�\���p�E�B���h�E�ւ̃n���h��
	HWND			hWnd_opepane;					//�����C���E�B���h�E�̃n���h���i���C���t���[����ɔz�u�j
	HWND			hWnd_msgList;					//�����C���E�B���h�E�̃��b�Z�[�W�\���p���X�g�R���g���[���ւ̃n���h��
	HWND			hWnd_work;						//����p��Ɨp�E�B���h�E�̃n���h��

	//-�O���C���^�[�t�F�[�X
	unsigned long	*psys_counter;					//���C���V�X�e���J�E���^�[�̎Q�Ɛ�|�C���^
	unsigned		work_select=0;					//�X���b�h���s�̊֐��̎�ނ��w��

}ST_THREAD_INFO, *PST_THREAD_INFO;

/***********************************************************************
�^�X�N�X���b�h�I�u�W�F�N�g�x�[�X�N���X
************************************************************************/
class CThreadObj {
public:
	ST_THREAD_INFO inf;

	CThreadObj();
	virtual ~CThreadObj();//�f�X�g���N�^

	virtual void init_task(void* pobj) { return; };
	unsigned __stdcall run(void *param);//�X���b�h���s�Ώۊ֐�

	//�X���b�h���s�Ώۊ֐����ŌĂ΂�鏈�� run()���ł����ꂩ���I�����{�����
	// >set_work()�ł���������{���邩�ݒ肷�� 
	virtual void routine_work(void *param);
	virtual void optional_work1(void *param) { Sleep(1000); };
	virtual void optional_work2(void *param) { Sleep(1000); };
	virtual void default_work(void *param) { Sleep(1000); };
	unsigned int set_work(int work_id) { inf.work_select = work_id; return(inf.work_select); };

	virtual void output_name() { const type_info &t = typeid(this); std::cout << "My type is " << t.name() << std::endl; }

	virtual HWND CreateOwnWindow(HWND h_parent_wnd);

	virtual LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	virtual void set_panel_tip_txt();//�^�u�p�l����Static�e�L�X�g��ݒ�
	virtual void set_panel_pb_txt() { return; };//�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
	virtual void set_PNLparam_value(float p1, float p2, float p3, float p4, float p5, float p6);//�p�����[�^�����\���l�ݒ�



private:
	static LRESULT CALLBACK DlgProcDefault(HWND, UINT, WPARAM, LPARAM);

protected:
	CHelper tool;
	ostringstream s;
	wostringstream ws;
	wstring wstr;
	string  str;

	void tweet2owner(const std::string &src);	//���C���E�B���h�E�ւ̃V���[�g���b�Z�[�W�\��
	void tweet2owner(const std::wstring &srcw); //���C���E�B���h�E�ւ̃V���[�g���b�Z�[�W�\��
	void txout2msg_listbox(const std::wstring str);//���X�g�R���g���[���ւ̃V���[�g���b�Z�[�W�\��
	void txout2msg_listbox(const std::string str);//���X�g�R���g���[���ւ̃V���[�g���b�Z�[�W�\��
};

