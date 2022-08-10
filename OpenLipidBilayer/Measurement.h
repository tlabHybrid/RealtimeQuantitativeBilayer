#pragma once

#include "TecellaAmp.h"
#include "MyMain.h"

//アンプのハンドルはMyMainクラスのメンバ変数にする

int measure_init(MyMain* m, int choice);		//初期化と情報登録
void measure_conduct(MyMain* m, double* timestamp, double* destination);	//データの読み出し (destinationは，5000サンプルの格納先変数のポインタ, timestampはその時刻)
void measure_stop(MyMain* m);		//Acquire stop
int measure_finalize(MyMain* m);	//終了