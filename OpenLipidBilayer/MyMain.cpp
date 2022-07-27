// User interface

#include "MyMain.h"
#include "Measurement.h"
#include "TecellaAmp.h"
#include <math.h> // roundのため
#include <string> // to_stringのため
#include <windows.h> // Sleep()のため
#include <vector> //ベクトルの平均計算のため
#include <numeric> //同上
#include <sstream> //値を簡単に文字列にするため
#include <iomanip>  //時間取得のため
#include <iostream>
#include <time.h>  //処理時間計測のため

const int SAMPLE_FREQ = 5000;   // 5kHz sampling on Tecella PICO and KISTEC PocketAmpUSB

MyMain::MyMain(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this); //ここが「メンバーsetupUIがありません」とかエラーを吐いている際には，UIがしっかり更新されていない．テキストボックスを文字で埋めてセーブすると治るかも．
    ui.pushButton_2->setEnabled(false);
    ui.pushButton_3->setEnabled(false);
    QCommonStyle style;
    ui.pushButton_4->setIcon(style.standardIcon(QStyle::SP_MediaSkipBackward));
    ui.pushButton_5->setIcon(style.standardIcon(QStyle::SP_MediaSeekBackward));
    ui.pushButton_6->setIcon(style.standardIcon(QStyle::SP_MediaPause));
    ui.comboBox->addItem("s");
    ui.comboBox->addItem("ms");
    this->data_from_local = true;
    this->display_Infos("**------**");
    this->setup_mySerial();  // シリアル通信を行う関数のセットアップ

    clock_t start_time = clock();
    clock_t end_time = clock();
    std::cout << "elapsed time: " << double(end_time - start_time) / CLOCKS_PER_SEC << "sec" << std::endl;
}

MyMain::~MyMain() {
    this->close_mySerial();
    // Disconnect and release memories.
    if (!this->data_from_local) measure_finalize(this);
}

// 右上に情報を追記する関数
void MyMain::display_Infos(const char* str) {
    // 文字を入れて
    ui.textBrowser_9->append(QString::fromLocal8Bit(str));
    // スクロールバーを末尾に持っていく
    QScrollBar* sb = ui.textBrowser_9->verticalScrollBar();
    sb->setValue(sb->maximum());
}


// Set up the amplifier.
void MyMain::on_pushBtnClicked() {
    // 入力されているラジオボタンによって分岐して処理を行う．
    // データのソース
    if (ui.radioButton->isChecked()) {
        // Direct current acquisition from Amplifier PICO.
        this->display_Infos("Data source: Amplifier PICO");
        // Suggest user to cover the device with faraday cage. If just debugging, the user can skip the calibration.
        int choice = QMessageBox::question(this, tr("Confirmation"), tr("Make sure the PICO terminals are open and covered with a Faraday cage. Do you want to calibrate PICO?"), tr("Yes"), tr("No, just passing by"));
        // choice == 0 : yes  choice == 1 : no  choice == -1 : escaped
        //QMessageBox::information(this, tr("Confirmation"), tr("Make sure the PICO terminals are open and covered with a Faraday cage."));
        // Connect to the amplifier and conduct setup.
        if (measure_init(this, choice) != 0) {
            this->display_Infos("Unable to open PICO. Check connection and retry.");
            return;
        }
        this->data_from_local = false;
    }
    else if (ui.radioButton_2->isChecked()) {
        // Reading from an ATF file.
        this->display_Infos("Data source: ATF file");
        // Open the dialog to choose ATF file.
        QString filename = QFileDialog::getOpenFileName(this, tr("Choose an ATF file."), "data", tr("ATF files(*.atf);;All Files(*.*)"));
        if (filename.isEmpty()) {
            this->display_Infos("Please specify the file. Try again.");
            return;
        }
        // Obtain the current data.
        if (this->load_data_from_local(filename.toLocal8Bit().constData(), true) == -1) {
            this->display_Infos("Couldn't open the specified file. Try again.");
            return;
        }
        this->display_Infos("Local deta loading completed.");
        this->data_from_local = true;
    }
    else if (ui.radioButton_3->isChecked()) {
        // Reading from a CSV file
        this->display_Infos("Data source: CSV file");
        // Open the dialog to choose CSV file.
        QString filename = QFileDialog::getOpenFileName(this, tr("Choose a CSV file."), "data", tr("CSV files(*.csv);;All Files(*.*)"));
        if (filename.isEmpty()) {
            this->display_Infos("Please specify the file. Try again.");
            return;
        }
        // Obtain the current data.
        if (this->load_data_from_local(filename.toLocal8Bit().constData(), false) == -1) {
            this->display_Infos("Couldn't open the specified file. Try again.");
            return;
        }
        this->display_Infos("Local deta loading completed.");
        this->data_from_local = true;
    }
    else {
        this->display_Infos("Data source is not specified. Try again.");
        return;
    }

    // 解析するタンパク質の種類
    if (ui.radioButton_4->isChecked()) {
        // AlphaHL from Staphylococcus aureus  
        this->display_Infos("Protein: Alpha hemolysin from Staphylococcus aureus");
        baseline = 0;
        current_per_channel_user_specified = 44.5;
        ui.textBrowser_2->setEnabled(false);
        ui.textBrowser_3->setEnabled(false);
        ui.textBrowser_5->setEnabled(false);
        //ui.textBrowser_5->setText(QString::fromLocal8Bit("-"));
        ui.textBrowser_6->setEnabled(false);
        //ui.textBrowser_6->setText(QString::fromLocal8Bit("-"));
        this->protein_type = 0;
    }
    else if (ui.radioButton_5->isChecked()) {
        // pigBK
        this->display_Infos("Protein: Big Potassium (BK) ion channel from Pig");
        baseline = 0;
        current_per_channel_user_specified = -11.5;
        ui.textBrowser_2->setEnabled(true);
        ui.textBrowser_3->setEnabled(true);
        ui.textBrowser_4->setText(QString::fromLocal8Bit("Holding voltage"));
        ui.textBrowser_5->setEnabled(true);
        ui.textBrowser_6->setEnabled(true);
        ui.textBrowser_6->setText(QString::fromLocal8Bit("mV"));
        this->protein_type = 1;
    }
    else if (ui.radioButton_6->isChecked()) {
        // AaOR8
        this->display_Infos("Protein: Olfactory Receptor (OR) 8 from Aedes aegypti");
        this->baseline = 0;
        this->current_per_channel_user_specified = 2.0;
        ui.textBrowser_2->setEnabled(true);
        ui.textBrowser_3->setEnabled(true);
        ui.textBrowser_4->setText(QString::fromLocal8Bit("Odor Concentration"));
        ui.textBrowser_5->setEnabled(true);
        ui.textBrowser_6->setEnabled(true);
        ui.textBrowser_6->setText(QString::fromLocal8Bit("uM"));
        this->protein_type = 2;
    }
    else {
        this->display_Infos("Protein type is not specified. Try again.");
        return;
    }

    //*** Common work
    // Modification of the current_per_channel_user_specified (if necessary)
    bool ok;
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
        tr("Do you want to modify the current value per channel?"), this->current_per_channel_user_specified, -100, 100, 1, &ok,
        Qt::WindowFlags(), 0.5);
    if (ok) {
        this->current_per_channel_user_specified = d;
    }
    //initialization of graphs
    this->initialize_graphs();

    this->display_Infos("Setting up completed.");
    this->display_Infos("**------**");
    this->ui.pushButton_2->setEnabled(true);
}


// Start acquisition.
void MyMain::on_pushBtn2Clicked() {
    this->start_graphs();
    // measure_conduct内部でマルチスレッド計測してるはずなので，ここではいらない
    //if (!this->data_from_local) measure_start(this);  // 別スレッドを立てて，アンプからソフトウェアキューにデータの読み出しを開始する

    this->ui.pushButton->setEnabled(false);
    this->ui.pushButton_2->setEnabled(false);
    this->ui.pushButton_3->setEnabled(true);

    this->display_Infos("Acquisition has started.  Push Stop button to stop acquisition.");
    this->display_Infos("**------**");
}


// Stop acquisition.
void MyMain::on_pushBtn3Clicked() {
    this->stop_graphs();
    //こちらもいらない
    //if (!this->data_from_local) measure_stop(this); // 計測用スレッドを終了

    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
    this->ui.pushButton_3->setEnabled(false);

    this->display_Infos("Acquisition has stopped.  Push Acquire button to restart acquisition.");
    this->display_Infos("**------**");
}


// Send instructions to the stepper motor to rotate until the STOP button is pressed.
void MyMain::on_pushBtn4Clicked() {
    if (this->port.isOpen()) {
        this->port.write("z\n");
    }
    else {
    }
}

// Send instructions to the stepper motor to rotate single step.
void MyMain::on_pushBtn5Clicked() {
    if (this->port.isOpen()) {
        this->port.write("x\n");
    }
    else {
    }
}

// Send instructions to the stepper motor to stop.
void MyMain::on_pushBtn6Clicked() {
    if (this->port.isOpen()) {
        this->port.write("c\n");
    }
    else {
    }
}


//ATF/CSVなどのローカルファイルからデータを読み込んでおく関数
int MyMain::load_data_from_local(const char* filepath, bool isATF, bool isHeader) {
    // Open a file.
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    QTextStream in(&file);
    QString line;

    // Release the data before newly import one.
    data_x.clear();
    data_y.clear();

    // Dispose a header and Read data.
    /************************************************************************
    * style specification:
    * Several rows at the top might be headers to be disposed. 
    * Column 1 must be Time [s].  ATF file matches this condition.  However, in the exported CSV file, the unit become [ms], so adaptation (*0.001) must be applied.
    * Column 2 must be Current [pA].
    * Separator of columns should be tab (ATF) or conma(CSV). 
    ************************************************************************/
    if (isATF) {
        //「export」で出力されたATFファイルの場合，はじめ10行がヘッダ
        for (int i = 0; i < 10; i++) in.readLine();  //読み捨て
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split('\t');      // split the string by tab
            data_x.append(fields.at(0).toDouble());     // 時刻[s]
            data_y.append(fields.at(1).toDouble());     // 電流[pA]
        }
    }
    else {
        //「Transfer Traces」で出力されたCSVファイルの場合，はじめ1行がヘッダ．なんか前処理してヘッダがない場合もあるが，基本的にヘッダはつけておくこと．
        if (isHeader) in.readLine();  //読み捨て
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split(',');               // split the string by conma
            if (ui.comboBox->currentIndex() == 0) { // CSVが[s]で出力されているとき．このシステムで記録されたデータの読み込みとか．
                data_x.append(fields.at(0).toDouble());
            } else if (ui.comboBox->currentIndex() == 1) { //CSVが[ms]で出力されているとき．Transfer Tracesとか．
                data_x.append(fields.at(0).toDouble() * 0.001);       // 時刻[ms] → [s] になるように修正
            }
            data_y.append(fields.at(1).toDouble());             // 電流[pA]
        }
    }

    file.close();
    return data_x.size();
}


// シリアル通信用の関数
void MyMain::setup_mySerial()
{
    QSerialPortInfo info; //NULL

    foreach(const QSerialPortInfo & info_tmp, QSerialPortInfo::availablePorts()) { //availablePorts()で利用可能なすべてのシリアルポートが取得できる
        qDebug() << "Name        : " << info_tmp.portName();
        qDebug() << "Description : " << info_tmp.description();
        qDebug() << "Manufacturer: " << info_tmp.manufacturer();

        if (info_tmp.description() == "Arduino Mega 2560") {
            info = info_tmp;
        }
    }
    if (info.isNull()) {
        QMessageBox::information(this, tr("Info"), "Arduino Mega is not connected. Continue with no serial communication.");
    }
    else {
        this->port.setPort(info);
        this->port.setBaudRate(QSerialPort::Baud9600);
        this->port.setDataBits(QSerialPort::Data8);
        this->port.setParity(QSerialPort::NoParity);
        this->port.setStopBits(QSerialPort::OneStop);
        if (this->port.open(QIODevice::ReadWrite)) {

        }
        else {
            QMessageBox::critical(this, tr("Error"), "Couldn't open the serial port. Maybe the port is used by other software?");
        }
    }

}

void MyMain::actuation_sendSerial(int data) {
    if (this->port.isOpen()) {
        //this->port.write(std::to_string(data).c_str());
        this->port.write("r\n");
        this->display_Infos("Sent data to serial port.");
    }
    else {
        //QMessageBox::critical(this, tr("Write Error"), this->port.errorString());
    }
}

void MyMain::close_mySerial()
{
    if (this->port.isOpen()) this->port.close();
}



// Initialize the qCustomPlot graphs.
void MyMain::initialize_graphs() {
    // graph1 (上のグラフ，生の電流値)   
    ui.customPlot->addGraph(); // blue line
    ui.customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    // 使用場所強調のための背景色ペン
    ui.customPlot->addGraph();
    ui.customPlot->graph(1)->setPen(QPen(QColor(255, 255, 255, 0)));
    ui.customPlot->graph(1)->setBrush(QBrush(QColor(255, 165, 0, 127)));;

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui.customPlot->xAxis->setTicker(timeTicker);
    ui.customPlot->axisRect()->setupFullAxesBox();
    ui.customPlot->yAxis->setRange(-2, 5);  //y軸の範囲も，個数推定に応じて自動で変えていく．ここは初期値．

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));


    // graph2 下のグラフ，処理した波形
    ui.customPlot_2->addGraph(); // red line
    const int PEN_WIDTH = 1; // 3でもOK
    ui.customPlot_2->graph(0)->setPen(QPen(QColor(255, 110, 40), PEN_WIDTH)); //PEN-WIDTHを太くするとパフォーマンスががた落ちする...が，もともと1秒以上処理落ちするプログラムなら何の問題もなく使えるぜ

    QSharedPointer<QCPAxisTickerTime> timeTicker2(new QCPAxisTickerTime);
    timeTicker2->setTimeFormat("%h:%m:%s");
    ui.customPlot_2->xAxis->setTicker(timeTicker2);
    ui.customPlot_2->axisRect()->setupFullAxesBox();
    ui.customPlot_2->yAxis->setRange(-1, 1); //y軸の範囲も，個数推定に応じて自動で変えていく．ここは初期値．

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui.customPlot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot_2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_2->yAxis2, SLOT(setRange(QCPRange)));
}


// 計測開始に伴って，グラフをスクロールさせ始める．
// TecellaLab等のアンプと同じく，一度Stopしてもう一度Acquireすると，一番初めから表示し直すようにする．
// (Resume機能は搭載しない)
void MyMain::start_graphs() {
    // グラフを更新する処理 (50Hzスクロール + 1Hzデータ追加と信号処理)
    //20220612~ 「8秒表示して，終わったら8秒ずらす，の方が見やすいし使いやすそう．」
    //connect(&dataTimer_50Hz, SIGNAL(timeout()), this, SLOT(update_graph_50Hz()));
    //this->dataTimer_50Hz.start(0); // Interval 0 means to refresh as fast as possible
    connect(&dataTimer_1Hz, SIGNAL(timeout()), this, SLOT(update_graph_1Hz()));
    this->dataTimer_1Hz.start(0); // Interval 0 means to refresh as fast as possible

    // ローカルデータ読み込みの際には，しばしば開始時刻がずれるので，データの初期時刻を記録
    if (this->data_from_local) this->first_data_x = data_x.at(0);
    else this->first_data_x = 0;
    // グラフを開始した時刻を記録するため
    //this->timer_restart_flag_50Hz = true;
    this->timer_restart_flag_1Hz = true;
    //よく考えたらqcustomplot上のデータを消さないといけない．
    ui.customPlot->graph(0)->data()->clear();
    ui.customPlot->graph(1)->data()->clear();
    ui.customPlot_2->graph(0)->data()->clear();

    // 初期位置へ移動
    ui.customPlot->xAxis->setRange(first_data_x, 8, Qt::AlignLeft);
    ui.customPlot_2->xAxis->setRange(first_data_x, 8, Qt::AlignLeft);
    ui.customPlot->replot();
    ui.customPlot_2->replot();


    //出力ログ準備  http://rinov.sakura.ne.jp/wp/cpp-date を参考に時刻を取得
    //現在日時を取得する
    time_t t = time(nullptr);
    //形式を変換する    
    const tm* lt = localtime(&t);
    //sに独自フォーマットになるように連結していく
    std::stringstream s;
    s << "log\\";   // 出力先のフォルダ名
    s << lt->tm_year + 1900; //1900を足すことで20xxになる
    s << std::setw(2) << std::setfill('0') << lt->tm_mon + 1; //月を0からカウントしているため
    s << std::setw(2) << std::setfill('0') << lt->tm_mday; //そのまま
    s << "-";
    s << std::setw(2) << std::setfill('0') << lt->tm_hour;
    s << std::setw(2) << std::setfill('0') << lt->tm_min;
    s << std::setw(2) << std::setfill('0') << lt->tm_sec;
    switch (this->protein_type)
    {
    case 0:
        s << "-AHL-";
        break;
    case 1:
        s << "-BK-";
        break;
    case 2:
        s << "-OR8-";
        break;
    }
    //result = "log\\20220619-094610-AHL-" 
    std::string result = s.str();
    this->myFileName_raw = result + "Raw.csv";
    this->myFileName_processed = result + "Processed.csv";
    this->myFileName_postprocessed = result + "POSTProcessed.csv";
    
    
    FILE* fp;
    // タンパク質ごとに処理内容が違うので，出力の中身も調整する
    switch (this->protein_type)
    {
    case 0:
        fopen_s(&fp, myFileName_processed.c_str(), "w");
        if (fp) {
            fprintf(fp, "time [s],num [-]\n");
            fclose(fp);
        }
        break;
    case 1:
        fopen_s(&fp, myFileName_processed.c_str(), "w");
        if (fp) {
            fprintf(fp, "time [s],opProb,voltage [mV]\n");
            fclose(fp);
        }
        break;
    case 2:
        fopen_s(&fp, myFileName_processed.c_str(), "w");
        if (fp) {
            fprintf(fp, "time [s],opProb,log10concentration [M]\n");
            fclose(fp);
        }
        break;
    }
    // アンプから取る場合は生データ(5KHz)も出力する
    if (!this->data_from_local) {
        fopen_s(&fp, myFileName_raw.c_str(), "w");
        if (fp) {
            fprintf(fp, "time [s],current [pA]\n");
            fclose(fp);
        }
    }
    
    if (ui.radioButton_13->isChecked()) {
        fopen_s(&fp, myFileName_postprocessed.c_str(), "w");
        if (fp) {
            fprintf(fp, "start_time [s],duration [s]\n");
            fclose(fp);
        }
    }
    else if (ui.radioButton_14->isChecked()) {
        fopen_s(&fp, myFileName_postprocessed.c_str(), "w");
        if (fp) {
            fprintf(fp, "step_time [s],conductance [pS]\n");  // これαHLに完全に最適化してるから，本当はタンパク質ごとに分けた方がいいかも
            fclose(fp);
        }
    }

}


//計測停止，グラフも止める
void MyMain::stop_graphs() {
    //dataTimer_50Hz.stop();
    dataTimer_1Hz.stop();
}



// Update the qCustomPlot graphs. 
// 50Hzで画面更新スクロール処理を行う処理．
// ~20220612   50Hzで画面スクロール
// 20220612~   「画面スクロールせずに，8秒間表示してし終わったら次へ行く，方が使いやすそう？」→ 一時的に無効にしてみる
void MyMain::update_graph_50Hz() {
    // set the timer;
    static QTime time(QTime::currentTime());
    static double lastPointKey = 0;
    if (this->timer_restart_flag_50Hz) {
        time.restart();
        lastPointKey = time.elapsed();
        this->timer_restart_flag_50Hz = false;
    }

    // calculate two new data points:
    double key = time.elapsed() / 1000.0; // time elapsed since start of demo, in seconds
     // ローカルデータ読み込みの際には，読み込んだ範囲からはみ出さないようにする．1Hzの方でストップするので，こちらはただreturnする．
    if (this->data_from_local) {
        if (key * SAMPLE_FREQ > this->data_x.size()) {
            return;
        }
    }

    //***************************************
    // スクロール
    if (key - lastPointKey > 0.020)
    {
        // make key axis range scroll with the data (at a constant range size of 8):
        ui.customPlot->xAxis->setRange(key + first_data_x, 8, Qt::AlignLeft);
        ui.customPlot_2->xAxis->setRange(key + first_data_x, 8, Qt::AlignLeft);
        ui.customPlot->replot();
        ui.customPlot_2->replot();

        lastPointKey = key;
    }
}


// 1Hzで画面更新 (データ取得処理＋信号処理) を行う関数
void MyMain::update_graph_1Hz() {
    // set the timer;
    static QTime time(QTime::currentTime());
    static double lastPointKey = 0;
    static int lastOpenNumber = 0; // 直前の時間に開いていたチャンネルの数． 全てのチャンネルの推定数を示すnumber_of_channelとは別．
    static int prev_num_channels = 0;  //今までの時間に開いていた最大のチャネル数．number_of_channelの前時間．
    static int dataIndex_loop_num = 0;  // この関数が呼び出された回数．これをもとに，どの範囲を読み込むかを決める

    static double startOneMolTime = -1;
    static bool oneMolContinueFlag = false;

    if (this->timer_restart_flag_1Hz) {
        time.restart();
        if (this->data_from_local) lastPointKey = 0; //ローカルデータを読む際には，1秒後から表示し始める
        else lastPointKey = -1;     // アンプから読みだすときには，startを押した瞬間から計測を始める (すると1秒後から表示される)
        lastOpenNumber = -1;    // 初回に必ずグラフを更新してもらうために，はじめは0ではなく-1にしておく
        dataIndex_loop_num = -1;      // 見やすさのために初めに++を持ってきたので，はじめは-1にしておく．
        prev_num_channels = -1;
        this->number_of_channel = -1;
        this->current_per_channel = this->current_per_channel_user_specified;
        this->baseline = 0;
        this->rupture_flag = false;
        this->recovery_flag = false;

        startOneMolTime = -1;
        oneMolContinueFlag = false;

        this->timer_restart_flag_1Hz = false;
    }


    // calculate two new data points:
    double key = time.elapsed() / 1000.0; // time elapsed since start of demo, in seconds 
    // ローカルデータ読み込みの際には，読み込んだ範囲からはみ出さないようにする
    if (this->data_from_local) {
        if (key * SAMPLE_FREQ > this->data_x.size()) {
            this->on_pushBtn3Clicked();
            return;
        }
    }


    //***************************************
    // 1Hzでデータ取得と信号処理
    if (key - lastPointKey > 1)
    {

        //***************************************
        // 0.125 Hzでの画面位置移動
        dataIndex_loop_num++;
        if (dataIndex_loop_num % 8 == 0) {
            ui.customPlot->xAxis->setRange(dataIndex_loop_num + first_data_x, 8, Qt::AlignLeft);
            ui.customPlot_2->xAxis->setRange(dataIndex_loop_num + first_data_x, 8, Qt::AlignLeft);
            //ui.customPlot->replot();
            //ui.customPlot_2->replot();
        }

        
        //***************************************************************************************
        // Sense: アンプからリアルタイムに電流値を取得．一応ローカルデータからの読み込みも可能．
        //***************************************************************************************
        // ① データ取得
        double currentTime[SAMPLE_FREQ]; // 処理する5000データの取得時刻[s]
        double currentData[SAMPLE_FREQ]; // 処理する5000データの電流値[pA]  現在currentの電流currentという激ウマ変数
        int processedData[SAMPLE_FREQ];  // 処理後の5000データの開閉バイナリ[開受容体数] 
        for (int i = 0; i < SAMPLE_FREQ; i++) {
            currentTime[i] = 0;
            currentData[i] = 0;  // 初期化
            processedData[i] = -1;
        }

        //int dataIndex = floor(key) * SAMPLE_FREQ; // 5kHZ sampling
        if (!this->data_from_local) {
            //アンプから5kHzでデータを取得
            measure_conduct(this, currentTime, currentData);
            //currentTime,currentDataにデータを導入
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                currentTime[idx] = dataIndex_loop_num + idx / double(SAMPLE_FREQ);  // 5kHz = 0.0002 sec per sample
            }
        }
        else {
            // data_x, data_yから5000サンプルを読みだす
            int start_num = dataIndex_loop_num * SAMPLE_FREQ;
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                currentTime[idx] = data_x.at(idx + start_num);
                currentData[idx] = data_y.at(idx + start_num);
            }
        }
        lastPointKey = floor(key); // 無理やり値を1秒ごとにする


        //***************************************************************************************
        // Processing: 取得した電流値を信号処理し，分子数とか目標物質の濃度を推定する．
        //***************************************************************************************
        // 
        clock_t start_time = clock();
        // ② 信号処理
        //QVector<double> current_per_channels;  // 開閉の変化を検出した時，その変化量を記録しておく 
        //QVector<double> zeros;  // 全てのチャネルが閉じているときの電流値を記録しておく．

        // (a) ヒステリシスコンパレータの処理によるチャネル開閉推定．
        // 20220722追記；ナノポアについては，あまりヒューリスティック的な電流値を使用せず，ノイズの大きさより大きいステップをすべて計測できるようにしたい
        const double threshold = 0.75;  // 0.5だとopen/closeの50%で閾値を取る． 0.75とか0.8にするとヒステリシスコンパレータになる
        if (this->rupture_flag) {
            prev_num_channels = -1;
            this->number_of_channel = -1;
            lastOpenNumber = -1;

            startOneMolTime = -1;
            oneMolContinueFlag = false;

            this->rupture_flag = false;
            this->recovery_flag = false;
        }
        int maxOpenNumber = -1;  // 分子数更新のための記録変数．5000サンプル中での最大の開分子数．
        const int rupture_threshold = 500;
        for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
            double y_now = currentData[idx];
            //*******
            // 膜が割れた場合には，「割れた」フラグをtrueにして脱出する．
            // その閾値は，適当に±500pAとする．

            /* 20220705追記：これだと膜が割れた(はじめ数字→X，もしくはX→X)のと膜が回復した(はじめX→数字)の区別がつかない．
            * もう一つ recovery_flagを立てて，それがtrueの時には回転信号を出さないことにする (割れてはいるのでrupture_flagもtrueにし，画面更新はしない)
            */
            if (y_now > rupture_threshold || y_now < -rupture_threshold) {
                this->rupture_flag = true;
                break;
            }
            //*******
            //ここからはまともなチャネル開閉推定
            if (!this->rupture_flag) {
                if (current_per_channel >= 0) {   // 電流が上の方向に振れるとき
                    if (y_now > (lastOpenNumber + threshold) * current_per_channel + baseline) {
                        lastOpenNumber++;
                    }
                    else if (y_now < (lastOpenNumber - threshold) * current_per_channel + baseline) {
                        lastOpenNumber--;
                        if (lastOpenNumber < 0) lastOpenNumber = 0;
                    }
                }
                else {  //電流が下の方向に振れるとき．コンパレータの符号が変わる．
                    if (y_now < (lastOpenNumber + threshold) * current_per_channel + baseline) {
                        lastOpenNumber++;
                    }
                    else if (y_now > (lastOpenNumber - threshold) * current_per_channel + baseline) {
                        lastOpenNumber--;
                        if (lastOpenNumber < 0) lastOpenNumber = 0;
                    }
                }

                //記録
                processedData[idx] = lastOpenNumber;
                if (lastOpenNumber > maxOpenNumber) maxOpenNumber = lastOpenNumber;
            }
        }
        // recovery_flag
        if (this->rupture_flag) {
            if (-rupture_threshold < currentData[SAMPLE_FREQ - 1] && currentData[SAMPLE_FREQ - 1] < rupture_threshold) {
                this->recovery_flag = true;
            }
        }
        // 20220719 分子数2で無限に回り続けるバグの修正
        if (maxOpenNumber >= 2 && processedData[SAMPLE_FREQ - 1] == 0) {
            this->rupture_flag = true;
            this->recovery_flag = true;
        }

        //**************膜が割れている場合，この後のほとんどの工程を無視する．****************
        
        // (b) チャンネルの数の更新． (グラフの縦軸の修正は，number_of_channelを介して⑤で行う)．
        // MaxOpenNumber (今回の5000サンプルでの最大開分子数) が，this->number_of_channel (前回までの最大開分子数)と違う場合には更新する．
        if (!this->rupture_flag) {
            if (maxOpenNumber != number_of_channel) {
                number_of_channel = maxOpenNumber; //チャネル数の更新 (チャネル数が減ってもよい)
                ui.textBrowser_12->setText(QString::fromLocal8Bit(std::to_string(maxOpenNumber).c_str()));
            }
        }
        else {
            //割れた場合
            ui.textBrowser_12->setText(QString::fromLocal8Bit("X"));
        }



        // (c) ベースラインと一分子あたりの開コンダクタンスを補正.
        // processedDataをもとに，「分子数がが0」「分子数が1」のところを適当に平均をとる
        int zero_num = 0;
        if (!this->rupture_flag) {
            double zero_value = 0;
            double one_value = 0;
            int one_num = 0;
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                if (processedData[idx] == 0) {
                    zero_value += currentData[idx];
                    zero_num += 1;
                }
                if (processedData[idx] == 1) {
                    one_value += currentData[idx];
                    one_num += 1;
                }
            }
            bool updated = false;
            //どちらとも，カウント数が500(1秒データの10%)未満であるときには，信用できないとみなして更新しないことにする．恣意的だが．．．
            if (zero_num >= 500) {
                //ベースラインは，もともと「0」だが，「1分子コンダクタンスの±50%以内」であれば更新する．(それ以上は普通に1分子目が入っているとみなす)
                //current_per_channelの符号要注意！！！
                double tmp = zero_value / zero_num;
                if (current_per_channel > 0) {
                    if (-0.5 * this->current_per_channel < tmp && tmp < 0.5 * this->current_per_channel) {
                        if (ui.checkBox_2->isChecked()) this->current_per_channel -= (tmp - this->baseline);  // 一分子コンダクタンスも連動して修正(ベースライン変動分を修正)
                        if (ui.checkBox->isChecked()) this->baseline = tmp;
                        updated = true;
                    }
                }
                else {
                    if (0.5 * this->current_per_channel < tmp && tmp < -0.5 * this->current_per_channel) {
                        if (ui.checkBox_2->isChecked()) this->current_per_channel -= (tmp - this->baseline);  // 一分子コンダクタンスも連動して修正(ベースライン変動分を修正)
                        if (ui.checkBox->isChecked()) this->baseline = tmp;
                        updated = true;
                    }
                }
            }

            //一分子コンダクタンスは，もともと「タンパク質ごとに指定した値」だが，「もとの1分子コンダクタンス±25%以内」であれば更新する．(それ以上は普通に2分子目が入っているとみなす)
            //current_per_channelの符号要注意！！！
            //あまり変えすぎると人間の解析とずれるので，ベースライン補正のみにしてこちらは無効化するのも選択肢として提示する．(チェックボックスで切り替え可)
            if (ui.checkBox_2->isChecked()) {
                if (one_num >= 500) {
                    double tmp = one_value / one_num - this->baseline;
                    if (current_per_channel > 0) {
                        if (0.75 * this->current_per_channel < tmp && tmp < 1.25 * this->current_per_channel) {
                            this->current_per_channel = tmp;
                            updated = true;
                        }
                    }
                    else {
                        if (1.25 * this->current_per_channel < tmp && tmp < 0.75 * this->current_per_channel) {
                            this->current_per_channel = tmp;
                            updated = true;
                        }
                    }
                }
            }

            if (updated) {
                std::string disp_str = "Current per Channel: ";
                disp_str = disp_str + std::to_string(current_per_channel);
                disp_str = disp_str + "   Baseline: ";
                disp_str = disp_str + std::to_string(baseline);
                this->display_Infos(disp_str.c_str());
            }

        }


        //***************************************
        // ③ 開確率の計算
        // zero_numは上で計算したものを使う
        //本来は各 0 < c < number_of_channel について計算するのが良いのだろうが，今回は0だけで妥協．
        // double opProb = p; 
        // zero_num / 5000 = (1-p)^number_of_channel
        // p = 1 - pow(zero_num/5000, 1/number_of_channel)
        // if (number_of_channel = 0) p = 0;
        double opProb = -99;    // 膜が割れてたり，分子数がたまたま0だったりするとopProbは負の値になる
        if (!this->rupture_flag) {
            if (number_of_channel != 0) {
                opProb = 1 - pow(zero_num / double(SAMPLE_FREQ), 1.0 / number_of_channel);
                if (opProb < 0.001) opProb = 0.001;
                if (opProb > 0.999) opProb = 0.999;
            }
        }


        //***************************************
        // ④ 生理学的刺激の計算，およびファイルへの書き出し．あと時刻と開確率も．
        FILE* fp;
        double millivolts = 0;
        double x_con = 0;
        int nowTime = round(dataIndex_loop_num + first_data_x) + 1;
        switch (protein_type)
        {
        case 0:
            // if AHL 何もしない
            //5000サンプル目の開状態の分子数 (最大値: number_of_channel もしくは MaxOpenNumber) をそのまま出力
            if (this->rupture_flag) {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A\n", nowTime);
                }
            }
            else {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%d\n", nowTime, number_of_channel);
                }
            }
            break;
        case 1:
            // if BK 電圧推定
            // ブタBK, 250mM KCl, 100 uM CaCl2のデータから，シグモイド関数で逆推定
            // p = 1 / (1 + exp(-a*(millivolts-x0)))
            // つまり millivolts = x0 + ln(p/(1-p))/a
            // Excelソルバーにより a = 0.070469599, x0 = -28.3978081 
            if (opProb < 0 || this->rupture_flag) {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A,#N/A\n", nowTime);
                }
            }
            else {
                millivolts = -28.3978081 + log(opProb / (1 - opProb)) / 0.070469599;
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%lf,%lf\n", nowTime, opProb, millivolts);
                }
            }
            break;
        case 2:
            // if OR8 濃度推定
            // シグモイド関数から逆張りする．
            // Dekel 2016 のTaOR8 vs R-Octenol のグラフを用いる．
            // x = log10(concentration)
            // p = 1 / (1 + exp(-a*(x-x0)))
            // つまり x = x0 + ln(p/(1-p))/a
            // つまり concentration = exp10(x0 + ln(p/(1-p))/a) (上の方が扱いやすいかも)
            // Excelソルバーにより a = 1.597072388, x0 = -6.495105404 
            // xの値に応じて条件分岐．
            // if(x > -6) concentration = exp10(x - (-6)) [uM]
            // else  concentration = exp10(x - (-9)) [nM]
            if (opProb < 0 || this->rupture_flag) {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A,#N/A\n", nowTime);
                }
            }
            else {
                x_con = -6.495105404 + log(opProb / (1 - opProb)) / 1.597072388;
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%lf,%lf\n", nowTime, opProb, x_con);
                }
            }
            break;
        }
        if (fp) fclose(fp);


        // アンプから取る場合は生データ(5KHz)も出力する
        if (!this->data_from_local) {
            fopen_s(&fp, myFileName_raw.c_str(), "a");
            if (fp) {
                for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                    fprintf(fp, "%lf,%lf\n", currentTime[idx], currentData[idx]);
                }
                fclose(fp);
            }
        }

        clock_t end_time = clock();
        //std::cout << "elapsed time: " << double(end_time - start_time) / CLOCKS_PER_SEC << "sec" << std::endl;

        //***************************************
        // ⑤ 上のグラフ(生データ)・下のグラフ(処理済みデータ)への反映

        // デバッグ：背景色テスト．一分子時間やコンダクタンスジャンプについて，「ここを計算に使った」アピールをする．

        if (ui.radioButton_13->isChecked()) {
            //一分子時間をできるだけ延長したい，その集計を自動でやりたい場合
            // processedDataが1であれば黄色にする．
            //ここ，完全にαHL向けにしてしまってるような．．．？ひょっとするとタンパク質ごとの区別が必要かも

            // 割れてモーターで回しているときのごたごたも「分子数の変動」として見ている？ → 割れたところから1秒間は計測しないことにする．
            double startTime = -1; double endTime = -1;
            bool oneMolFlag = false;
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                if ((processedData[idx] == 1 && oneMolFlag == false) || (idx == 0 && oneMolContinueFlag == true)) {
                    startTime = currentTime[idx];
                    oneMolFlag = true;
                    if (!oneMolContinueFlag) {
                        startOneMolTime = startTime;
                        oneMolContinueFlag = true;
                    }
                }
                if (processedData[idx] != 1 && oneMolFlag == true) {
                    endTime = currentTime[idx];
                    double tmpx[] = { startTime, startTime, endTime, endTime };
                    double tmpy[] = { 0, 500, 500, 0 };
                    for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                    oneMolFlag = false;
                    if (oneMolContinueFlag) {
                        double oneMolTime = endTime - startOneMolTime;
                        if (oneMolTime > 0.02) { // あまりにも短すぎるのは除外する
                            this->display_Infos(std::to_string(oneMolTime).c_str());
                            //******************* ここらへんで「開始時刻，そこからの一分子時間」をCSVに出力する． ****************************
                            fopen_s(&fp, myFileName_postprocessed.c_str(), "a");
                            if (fp) {
                                fprintf(fp, "%lf,%lf\n", std::round(startOneMolTime*100)/100, std::round(oneMolTime * 100) / 100);
                                fclose(fp);
                            }
                        }
                        oneMolContinueFlag = false;
                    }
                }
                if (processedData[idx] == -1) {
                    //全ての処理を中断してループを抜ける
                    oneMolFlag = false;
                    startOneMolTime = -1;
                    oneMolContinueFlag = false;
                    this->display_Infos("rupture detected");
                    break;
                }
            }
            if (oneMolFlag) {
                //次の時間へ継続
                endTime = currentTime[SAMPLE_FREQ - 1];
                double tmpx[] = { startTime, startTime, endTime, endTime };
                double tmpy[] = { 0, 500, 500, 0 };
                for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                oneMolFlag = false;
            }
        }

        if (ui.radioButton_14->isChecked()) {
            // 一分子コンダクタンスを評価したい，そのコンダクタンスの個性分布を作りたい場合
            // processedDataが0から1に飛んだ周辺100msとかを黄色くする．
            // 追記；0→1だけじゃなくて，2→3とか，3→4とかも含めていいんじゃない？→ちょっと怪しい．正しいデータを取りたければ0→1だけを使うがいいかも
            //ここ，完全にαHL向けにしてしまってるような．．．？ひょっとするとタンパク質ごとの区別が必要かも
            for (int idx = 0; idx < SAMPLE_FREQ-1; idx++) {
                //ジャンプしたところの周辺を観察して，平均をとる
                if (processedData[idx + 1] - processedData[idx] == 1 && processedData[idx] != -1) {  //processedData[idx] == 0 && processedData[idx + 1] == 1
                    //必要な時間の計算
                    int zero_start_idx = -1;
                    if (idx < 500) zero_start_idx = 0;
                    else zero_start_idx = idx - 500;
                    int zero_end_idx = idx;
                    if (zero_start_idx >= zero_end_idx) continue;  //ゼロ割を起こさないために，計算できない場合は処理しない
                    if (zero_end_idx - zero_start_idx < 200) continue;  //また，計算精度を保証するために，200サンプル(40ms)以下のデータしかない場合は計算しない
                    
                    //1分子側は，膜が割れていないことを保証する必要あり
                    int one_start_idx = idx + 1;
                    int one_end_idx = -1;
                    for (one_end_idx = one_start_idx; one_end_idx < SAMPLE_FREQ - 1; one_end_idx++) {
                        if (processedData[one_end_idx] != processedData[one_start_idx]) break;  //分子数がさらに増えたり割れたりしたら抜ける
                        if (one_end_idx > one_start_idx + 500) break;
                    }
                    one_end_idx--; // 割れた瞬間の1分子ではない状態を取り除く
                    if (one_start_idx >= one_end_idx) continue;  //ゼロ割を起こさないために，計算できない場合は処理しない
                    if (one_end_idx - one_start_idx < 200) continue;  //また，計算精度を保証するために，200サンプル(40ms)以下のデータしかない場合は計算しない
                    //色付け
                    double tmpx[] = { currentTime[zero_start_idx], currentTime[zero_start_idx], currentTime[one_end_idx], currentTime[one_end_idx] };
                    double tmpy[] = { 0, 500, 500, 0 };
                    for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                    //******************* ここらへんで「ジャンプ時刻，そこでの一分子コンダクタンス」をCSVに出力する． ****************************
                    double zero_value = 0;
                    for (int i = zero_start_idx; i <= zero_end_idx; i++) zero_value += currentData[i];
                    zero_value = zero_value / (zero_end_idx - zero_start_idx);
                    double one_value = 0;
                    for (int i = one_start_idx; i <= one_end_idx; i++) one_value += currentData[i];
                    one_value = one_value / (one_end_idx - one_start_idx);
                    //ここまで電流[pA]なので，ヒューリスティックス(50mV)でコンダクタンス[pS]に変換する
                    double one_conductance = (one_value - zero_value) * 1000 / 50;
                    this->display_Infos(std::to_string(one_conductance).c_str());
                    fopen_s(&fp, myFileName_postprocessed.c_str(), "a");
                    if (fp) {
                        fprintf(fp, "%lf,%lf\n", std::round(currentTime[one_start_idx] * 100) / 100, std::round(one_conductance * 100) / 100);
                        fclose(fp);
                    }

                }

                if (processedData[idx] == -1) {
                    //全ての処理を中断してループを抜ける
                    this->display_Infos("rupture detected");
                    break;
                }
            }

        }


        // 20220722追記：Qvectorが長くなりすぎると，1630秒(28分)付近でMicrosoft C++ の例外: std::bad_alloc が発生してアプリが落ちる．
        // では，このQvectorを定期的に開放するようにしたら，このエラーは治るだろうか？
        // 8秒ごとにclearすれば，ユーザは気づかないはず．
        // 例えば，4分(8秒と1分の最小公倍数)ごとにgraphをclearしてみる．
        if (dataIndex_loop_num % 240 == 0) {
            ui.customPlot->graph(0)->data()->clear();
            ui.customPlot->graph(1)->data()->clear();
            ui.customPlot_2->graph(0)->data()->clear();
        }


        if (!this->rupture_flag) {
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                ui.customPlot->graph(0)->addData(currentTime[idx], currentData[idx]);
                ui.customPlot_2->graph(0)->addData(currentTime[idx], processedData[idx]);
            }
            // チャンネル数・コンダクタンスに応じてy軸の範囲を変更する
            // 20220722追記：膜が割れた場合でも，その時点でのMaxOpenNumberに応じて画面の範囲を変更できるようにしたい (黄色い枠が正しいのかを見たい)
            if (prev_num_channels != number_of_channel) {
                //上
                if (current_per_channel > 0) {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(-2, tmp);
                }
                else {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(tmp, 2);
                }
                //下
                ui.customPlot_2->yAxis->setRange(-1.0, 1.0 + number_of_channel);
            }
            prev_num_channels = number_of_channel;
        }
        else {
            // 膜が割れている場合は，グラフの軸も変更せず，ただ上へ吹き飛ばす
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                ui.customPlot->graph(0)->addData(currentTime[idx], currentData[idx]); //生データは書く．
                // 処理済みデータは書かない．
            }
            //y軸の範囲を変更しない．
        }
        ui.customPlot->replot();
        ui.customPlot_2->replot();


        //***************************************
        // ⑥ 画面表示への反映 (分子数については②で反映ずみ)
        if (!this->rupture_flag) {
            switch (protein_type)
            {
            case 0:
                // if AHL 何もしない
                break;
            case 1:
                // if BK
                if (opProb < 0) {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
                    ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
                }
                else {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit(std::to_string(int(opProb * 100)).c_str()));
                    ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(millivolts))).c_str()));
                    ui.textBrowser_6->setText(QString::fromLocal8Bit("mV"));
                }
                break;
            case 2:
                // if OR8 
                if (opProb < 0) {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
                    ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
                }
                else {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit(std::to_string(int(opProb * 100)).c_str()));
                    if (x_con > -6) {
                        double concentration_uM = pow(10, x_con - (-6));
                        ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(concentration_uM))).c_str()));
                        ui.textBrowser_6->setText(QString::fromLocal8Bit("uM"));
                    }
                    else {
                        double concentration_nM = pow(10, x_con - (-9));
                        ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(concentration_nM))).c_str()));
                        ui.textBrowser_6->setText(QString::fromLocal8Bit("nM"));
                    }
                }
                break;
            }
        }
        else {
            //割れてた場合は全てXにする
            ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
            ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
        }


        //***************************************************************************************
        // Actuation: Processingの結果をもとに，SerialPort出力を介してモーターなどを駆動する
        //***************************************************************************************
        if (!this->rupture_flag) {
            switch (protein_type)
            {
            case 0:
                // if AHL
                // 「1分子計測の時間を可能な限り伸ばす」＝「2分子目が入ったら膜を自動で張りなおす」
                if (this->number_of_channel >= 2) {
                    this->actuation_sendSerial(2);  // 回転指示．データは適当．
                }
                break;
            case 1:
                // if BK
                if (millivolts > -40) {
                    this->actuation_sendSerial((int)millivolts);  // 適当にデータを送信してみる
                }
                break;
            case 2:
                // if OR8 
                break;
            }
        }
        else {
            //割れてた場合
            if(!this->recovery_flag) this->actuation_sendSerial(-1);  // 割れた場合はすぐに張りなおすように指示を飛ばす
        }
    }
}