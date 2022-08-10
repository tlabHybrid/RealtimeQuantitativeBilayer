
#include "Measurement.h"
#include "MyMain.h"
#include "TecellaAmp.h"
#include "TecellaAmpExample_00.h"

TECELLA_HNDL aaa;

// 計測器と接続・設定の初期化を行う
int measure_init(MyMain* m, int choice) {

	// TecellaAmpExample_00のmainからのコピー
	tecella_debug("tecella_debug.log");

	// コンソールへの表示を同時に確認するために，
	// プロジェクト(P) > プロパティ(P) > 構成プロパティ > リンカー > システム > サブシステム
	// を「コンソール」にしている．これを「WINDOWS」にすることでコンソールは表示されなくなる．

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



// 計測を実行する (readする：ソフトウェアキュー→適当な変数destination)
void measure_conduct(MyMain* m, double* timestamp, double* destination) {
	//Acquire using different methods
	acquire_without_callback(m->h, timestamp, destination);  // 「1秒間のデータを，1秒に1回取得する」ようにする．これで今回は充分．
}


//計測を停止する (stop：acquireスレッドを止める)
void measure_stop(MyMain* m) {
	acquire_stop(m->h);
}


// 完全にメモリを開放し，他のアプリ(TecellaLabとか)からPICOが使えるようにする
int measure_finalize(MyMain* m) {
	//Clean up and exit.
	tecella_finalize(m->h);
	return 0;
}