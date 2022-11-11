#pragma once

#include <QtWidgets/QWidget>
#include "ui_MyMain.h"
#include "subWin.h"

class MyMain : public QWidget
{
    Q_OBJECT

public:
    MyMain(QWidget *parent = Q_NULLPTR);
    ~MyMain();
    void displayInfo(const char*);

private:
    Ui::MyMainClass ui;
    
    // Graph drawing
    void initialize_graphs();
    void start_graphs();
    void stop_graphs();

    // Data export
    std::string myFileName_raw;
    std::string myFileName_processed;
    std::string myFileName_postprocessed;

    // Second window
    subWin* subWindow;

private slots:
    // Push buttons
    void on_pushBtnClicked();
    void on_pushBtn2Clicked();
    void on_pushBtn3Clicked();
    void on_pushBtn4Clicked();
    void on_pushBtn5Clicked();
    void on_pushBtn6Clicked();
    void on_pushBtn7Clicked();
    void on_spinBoxChanged(int value);
    void on_pushBtn10Clicked();
    void on_pushBtn11Clicked();
    void on_pushBtn12Clicked();
    void on_pushBtn13Clicked();

    // Main function (1 Hz callback)
    void update_graph_1Hz();
};
