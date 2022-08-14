#pragma once

#include <QtWidgets/QWidget>
#include "ui_MyMain.h"

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

private slots:
    // Push buttons
    void on_pushBtnClicked();
    void on_pushBtn2Clicked();
    void on_pushBtn3Clicked();
    void on_pushBtn4Clicked();
    void on_pushBtn5Clicked();
    void on_pushBtn6Clicked();

    // Main function (1 Hz callback)
    void update_graph_1Hz();
};
