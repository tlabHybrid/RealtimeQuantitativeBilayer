#pragma once

#include "TecellaAmp.h"
#include "MyMain.h"

//�A���v�̃n���h����MyMain�N���X�̃����o�ϐ��ɂ���

int measure_init(MyMain* m, int choice);		//�������Ə��o�^
void measure_conduct(MyMain* m, double* timestamp, double* destination);	//�f�[�^�̓ǂݏo�� (destination�́C5000�T���v���̊i�[��ϐ��̃|�C���^, timestamp�͂��̎���)
void measure_stop(MyMain* m);		//Acquire stop
int measure_finalize(MyMain* m);	//�I��