#pragma once

#include <QtWidgets/QWidget>
#include "ui_MyMain.h"
#include "qcustomplot.h"
#include <QTimer>
#include "TecellaAmp.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class MyMain : public QWidget
{
    Q_OBJECT

public:
    MyMain(QWidget *parent = Q_NULLPTR);
    ~MyMain();
    void display_Infos(const char*);
    TECELLA_HNDL h;   // アンプを使うためのメンバ変数．

private:
    Ui::MyMainClass ui;


    int load_data_from_local(const char* filepath, bool isATF, bool isHeader=true); // ローカルからATF, CSVなどで電流値を事前ロードする関数．ATFとCSVでヘッダーのつき方やコンマ・タブが違うので分ける．また，CSVはヘッダーが付いていないものにも対応
    void initialize_graphs();
    void start_graphs();
    void stop_graphs();
    QVector<double> data_x;
    QVector<double> data_y;
    double first_data_x;  // ローカルファイル(ATF/CSV)の1行目の時刻．これを記録しておかないとグラフ描画位置がずれる．

    QTimer dataTimer_50Hz;  // 画面更新のためのcallback関数を呼び出すためのタイマー
    QTimer dataTimer_1Hz; // データ取得と信号処理のためのcallback関数を呼び出すためのタイマー
    bool timer_restart_flag_50Hz = false; // 再度acquireを押した際にはこれらをtrueにする．
    bool timer_restart_flag_1Hz = false;

    std::string myFileName_raw;  // 出力先のファイル名 (5kHz生の電流値)
    std::string myFileName_processed;  // 出力先のファイル名 (1Hz 信号処理済み)
    std::string myFileName_postprocessed;  // 出力先のファイル名 (一分子時間や一分子コンダクタンスなど，postprocessをした後のデータの出力先．)

    bool data_from_local = false;  // ATF, CSV読み込み時はtrueになる．アンプからリアルタイム取得するときにはfalse．
    int protein_type;  // 膜タンパク質の種類． 0: AHL, 1: BK, 2:OR8

    int number_of_channel = 0;   // リアルタイムに推定したイオンチャネルの数．
    double current_per_channel;  // 一チャネル当たりのおおよその電流値．コンダクタンス×バイアス電圧．BKは10，ORは2くらい．個数推定に応じて自動で変えていく
    double current_per_channel_user_specified;  //ユーザーが入力した値を保存しておく変数．これ±20％の範囲以外には飛び出させない．
    double baseline = 0.0; // 全てのチャネルが閉じているときの電流値．個数推定に応じて自動で変えていく
    bool rupture_flag = false;  // 膜が割れたかどうかの判定を記録する変数．
    bool recovery_flag = false;  // 膜が回復中かどうかの判定を記録する変数．





    // QSerialportを使ってシリアル通信を試してみよう
    // https://qiita.com/sh4869/items/b514483fff70b1319af1
    QSerialPort port;   // シリアル通信用
    void setup_mySerial();
    void actuation_sendSerial(int data = 0);
    void close_mySerial();

private slots:
    void on_pushBtnClicked();
    void on_pushBtn2Clicked();
    void on_pushBtn3Clicked();
    void on_pushBtn4Clicked();
    void on_pushBtn5Clicked();
    void on_pushBtn6Clicked();
    void update_graph_50Hz();  // 50Hzで画面更新，1Hzでデータ取得
    void update_graph_1Hz();
};
