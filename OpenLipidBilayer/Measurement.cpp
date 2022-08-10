
#include "Measurement.h"
#include "MyMain.h"
#include "TecellaAmp.h"
#include "TecellaAmpExample_00.h"

TECELLA_HNDL aaa;

// �v����Ɛڑ��E�ݒ�̏��������s��
int measure_init(MyMain* m, int choice) {

	// TecellaAmpExample_00��main����̃R�s�[
	tecella_debug("tecella_debug.log");

	// �R���\�[���ւ̕\���𓯎��Ɋm�F���邽�߂ɁC
	// �v���W�F�N�g(P) > �v���p�e�B(P) > �\���v���p�e�B > �����J�[ > �V�X�e�� > �T�u�V�X�e��
	// ���u�R���\�[���v�ɂ��Ă���D������uWINDOWS�v�ɂ��邱�ƂŃR���\�[���͕\������Ȃ��Ȃ�D

	//Initialize the device.
	m->display_Infos("Device connecting...");
	//Autodetect doesn't work yet, so you must explicitly specify the device model.
	TECELLA_ERRNUM err;
	//TECELLA_HNDL h;
	//if( err = tecella_initialize(&h, TECELLA_HW_MODEL_AUTO_DETECT) )	{
	if (err = tecella_initialize(&m->h, TECELLA_HW_MODEL_AUTO_DETECT)) {
		wprintf(tecella_error_message(err));
		return 1;
	}
	//Determine what's supported and configure the GUI.
	setup_gui(m->h);
	m->display_Infos("Device PICO connect completed.");

	//Utility functions
	// Use only if your hardware supports this.
	//test_utility_set_stimulus(h); wprintf(L"\nPress any key to continue.\n"); _getch();

	//Change various settings of the device
	if (choice == 0) {
		setup_auto_compensation(m->h);
		m->display_Infos("Device calibration completed.");
	}
	setup_source_and_gain(m->h);
	m->display_Infos("Source: Head");
	m->display_Infos("Gain: 1G");
	setup_per_channel_settings(m->h);
	m->display_Infos("Bessel filter: 1kHz");
	setup_stimulus(m->h);
	m->display_Infos("Holding voltage: 50 mV");
	m->display_Infos("Additional Stimulus: None");

	return 0;
}



// �v�������s���� (read����F�\�t�g�E�F�A�L���[���K���ȕϐ�destination)
void measure_conduct(MyMain* m, double* timestamp, double* destination) {
	//Acquire using different methods
	acquire_without_callback(m->h, timestamp, destination);  // �u1�b�Ԃ̃f�[�^���C1�b��1��擾����v�悤�ɂ���D����ō���͏[���D
}


//�v�����~���� (stop�Facquire�X���b�h���~�߂�)
void measure_stop(MyMain* m) {
	acquire_stop(m->h);
}


// ���S�Ƀ��������J�����C���̃A�v��(TecellaLab�Ƃ�)����PICO���g����悤�ɂ���
int measure_finalize(MyMain* m) {
	//Clean up and exit.
	tecella_finalize(m->h);
	return 0;
}