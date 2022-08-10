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
    TECELLA_HNDL h;   // Amplifier handle variable. 

private:
    Ui::MyMainClass ui;
    
    int load_data_from_local(const char* filepath, bool isATF, bool isHeader=true);
    void initialize_graphs();
    void start_graphs();
    void stop_graphs();
    QVector<double> data_x;
    QVector<double> data_y;
    double first_data_x;
    QTimer dataTimer_1Hz;
    bool timer_restart_flag_1Hz = false;
    std::string myFileName_raw;
    std::string myFileName_processed;
    std::string myFileName_postprocessed;
    bool data_from_local = false;
    int protein_type;  // 0: AHL, 1: BK, 2:OR8

    int number_of_channel = 0;      // Number of channels (proteins) in the lipid bilayer during 1s period. 
    double current_per_channel;     // Current per single channel [pA]. Equal to (conductance) * (bial voltage)ÅD AHL = 44.5, BK = 10ÅCOR = 2 (roughly).
    double current_per_channel_user_specified;      // User input of current_per_channel.
    double baseline = 0.0;          // The baseline currents. Equal to the mean current when all channels are closed (lastOpenNumber == 0). 
    bool rupture_flag = false;      // Indicates whether the bilayer is ruptured during the measuring period (1s). 
    bool recovery_flag = false;     // Indicates whether the bilayer is recovering from rupture during the measuring period (1s).

    QSerialPort port;   // Serial port related members.  [Ref] https://qiita.com/sh4869/items/b514483fff70b1319af1
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
    void update_graph_1Hz();
};
