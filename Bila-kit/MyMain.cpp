/******************************************************************************
// MyMain.cpp
//
// This code is the heart of Bila-kit. Most of the functions are called from this code.
//
******************************************************************************/
// This code contains...
//   *  [Setup]                 Set up an amplifier or load data, prepare for serial communication etc.
//   *  [SenseBlock]            Acquire raw current from an amplifier / a local file
//   *  [ProcessingBlock]       Determine the number of proteins, and calculate protein features (conductance / single-molecule time)
//   *  [ActuationBlock]        Drive peripheral devices like motors or bluetooth
//   *  [UIBlock]               Manage UI

#include "MyMain.h"
#include "MyHelper.h"
#include "qcustomplot.h"

#include <QTimer>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <math.h>

// Important variables
const int SAMPLE_FREQ = 5000;   // 5kHz sampling
int dataSource = 1;             // The source where the current is acquired from.  0: Amplifier, 1: Local ATF, 2: Local CSV
int proteinType = 0;            // The type of target membrane protein.  0: Nanopores (AHL), 1: Ion channels (BK), 2: Ion channels (OR8)
int postprocessType = 0;        // The type of postprocessing. 0: None, 1: Measuring conductance, 2: Measuring open probability (Po), 3: measuring Po + estimating stimuli
double opProb = 0;              // The open probability of this 1 s signal.
double stimuli = 0;             // The estimated stimuli, which leads to the predetermined opProb.

// Variables for calling 1 Hz callback
int dataIndex_loop_num = -2;     // The number of loops from the time when "Acquire" button is pushed.  -2 : reset signal
QTimer dataTimer_1Hz;
double dataStartTime = 0;        // (Local data only) the time of the first row.

// Variables for Processing Block
int number_of_channel = 0;      // Number of channels (proteins) in the lipid bilayer during 1s period. 
int prev_num_channels = 0;      // number_of_channel at the previous (1 s ahead) timestep.
double current_per_channel = 0.0;     // Current per single channel [pA]. Equal to (conductance) * (bial voltage). AHL = 44.5, BK = 10, OR = 2 (roughly).
double current_per_channel_user_specified = 0.0;      // User input of current_per_channel.
double baseline = 0.0;          // The baseline currents. Equal to the mean current when all channels are closed (lastOpenNumber == 0). 
bool rupture_flag = false;      // Indicates whether the bilayer is ruptured during the measuring period (1s). 
bool recovery_flag = false;     // Indicates whether the bilayer is recovering from rupture during the measuring period (1s)
int lastOpenNumber = 0;
double startOneMolTime = -1;
bool oneMolContinueFlag = false;

MyMain::MyMain(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);       // If this sentence raises "No member setupUI" error, please review whether your UI (MyMain.ui) is correctly updated and saved. 
    QCommonStyle style;
    ui.pushButton_4->setIcon(style.standardIcon(QStyle::SP_MediaSkipBackward));
    ui.pushButton_5->setIcon(style.standardIcon(QStyle::SP_MediaSeekBackward));
    ui.pushButton_6->setIcon(style.standardIcon(QStyle::SP_MediaPause));
    ui.comboBox->addItem("s");
    ui.comboBox->addItem("ms");
    setupSerial(this);

    displayInfo("**------**");
    ui.pushButton_2->setEnabled(false);
    ui.pushButton_3->setEnabled(false);
    // clock_t start_time = clock();
    // clock_t end_time = clock();
    // std::cout << "elapsed time: " << double(end_time - start_time) / CLOCKS_PER_SEC << "sec" << std::endl;
}

MyMain::~MyMain() {
    closeSerial();
    if (dataSource == 0) finalizeAmplifier(); // Disconnect amplifier and release memories associated with it.
}


// ********************************************************************************************************
//   Pushbutton slots (A) ... Measurement related functions
// ********************************************************************************************************

// Function called when "Setup" button is pressed.
// Set up the amplifier.
void MyMain::on_pushBtnClicked() {
    // ****** Determine the source of input data from the toggled radiobox.
    if (ui.radioButton->isChecked()) {
        displayInfo("Data source: Amplifier PICO");
        // Suggest user to cover the device with faraday cage. If just debugging, the user can skip the calibration.  choice == 0 : yes  choice == 1 : no  choice == -1 : escaped
        int choice = QMessageBox::question(this, tr("Confirmation"), tr("Make sure the PICO terminals are open and covered with a Faraday cage. Do you want to calibrate PICO?"), tr("Yes"), tr("No, just passing by"));
        // Connect to the amplifier and conduct setup.
        if (setupAmplifier(choice) != 0) {
            displayInfo("Unable to open PICO. Check connection and retry.");
            return;
        }
        dataSource = 0;
    }
    else if (ui.radioButton_2->isChecked()) {
        displayInfo("Data source: ATF file");
        // Load data from a local ATF file.
        if (setupLocal(this, 0, true, &dataStartTime) != 0) {
            displayInfo("Unable to open the selected file. Please retry.");
            return;
        }
        dataSource = 1;
    }
    else if (ui.radioButton_3->isChecked()) {
        displayInfo("Data source: CSV file");
        // Load data from a local CSV file.
        if (setupLocal(this, 1, (ui.comboBox->currentIndex() == 0), &dataStartTime) != 0) {
            displayInfo("Unable to open the selected file. Please retry.");
            return;
        }
        dataSource = 2;
    }
    else {
        displayInfo("Data source is not specified. Try again.");
        return;
    }

    // ****** Determine the type of protein from the toggled radiobuttons.
    if (ui.radioButton_4->isChecked()) {
        // Perhaps in the future, this option will be changed to "Nanopores".
        displayInfo("Protein: Alpha hemolysin from Staphylococcus aureus");
        baseline = 0;
        current_per_channel_user_specified = 44.5;
        ui.textBrowser_2->setEnabled(false);
        ui.textBrowser_3->setEnabled(false);
        ui.textBrowser_5->setEnabled(false);
        ui.textBrowser_6->setEnabled(false);
        proteinType = 0;
    }
    else if (ui.radioButton_5->isChecked()) {
        // Perhaps in the future, this option will be changed to "Ion channels" altogether with the below option.
        displayInfo("Protein: Big Potassium (BK) ion channel from Pig");
        baseline = 0;
        current_per_channel_user_specified = -11.5;
        ui.textBrowser_2->setEnabled(true);
        ui.textBrowser_3->setEnabled(true);
        ui.textBrowser_4->setText(QString::fromLocal8Bit("Holding voltage"));
        ui.textBrowser_5->setEnabled(true);
        ui.textBrowser_6->setEnabled(true);
        ui.textBrowser_6->setText(QString::fromLocal8Bit("mV"));
        proteinType = 1;
    }
    else if (ui.radioButton_6->isChecked()) {
        displayInfo("Protein: Olfactory Receptor (OR) 8 from Aedes aegypti");
        baseline = 0;
        current_per_channel_user_specified = 2.0;
        ui.textBrowser_2->setEnabled(true);
        ui.textBrowser_3->setEnabled(true);
        ui.textBrowser_4->setText(QString::fromLocal8Bit("Odor Concentration"));
        ui.textBrowser_5->setEnabled(true);
        ui.textBrowser_6->setEnabled(true);
        ui.textBrowser_6->setText(QString::fromLocal8Bit("uM"));
        proteinType = 2;
    }
    else {
        displayInfo("Protein type is not specified. Try again.");
        return;
    }

    // ****** Modification of the current_per_channel_user_specified (which corresponds to channel conductance) if necessary.
    bool ok;
    double d = QInputDialog::getDouble(this, "QInputDialog::getDouble()",
        "Do you want to modify the current value per channel?", current_per_channel_user_specified, -100, 100, 1, &ok,
        Qt::WindowFlags(), 0.5);
    if (ok) {
        current_per_channel_user_specified = d;
    }


    // ****** Initialization of graphs
    this->initialize_graphs();

    displayInfo("Setting up completed.");
    displayInfo("**------**");
    this->ui.pushButton_2->setEnabled(true);
}

// Function called when "Acquire" button is pressed.
// Start acquisition and drawing graphs.
void MyMain::on_pushBtn2Clicked() {
    // ****** Start graphs and register the 1 Hz callback function [update_graph_1Hz()].
    this->start_graphs();
    
    displayInfo("Acquisition has started.  Push Stop button to stop acquisition.");
    displayInfo("**------**");
    this->ui.pushButton->setEnabled(false);
    this->ui.pushButton_2->setEnabled(false);
    this->ui.pushButton_3->setEnabled(true);
}

// Function called when "Stop" button is pressed.
// Stop acquisition and drawing graphs.
void MyMain::on_pushBtn3Clicked() {
    // ****** Stop graphs and cancel the 1 Hz callback function [update_graph_1Hz()].
    this->stop_graphs();

    displayInfo("Acquisition has stopped.  Push Acquire button to restart acquisition.");
    displayInfo("**------**");
    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
    this->ui.pushButton_3->setEnabled(false);
}


// ********************************************************************************************************
//   Pushbutton slots (B) ... Motor drive related functions
// ********************************************************************************************************
// Note: the used characters (z, x, c...) is predefined in the Arduino source code.

// Function called when "|<<" button below the messagebox is pressed.
void MyMain::on_pushBtn4Clicked() {
    sendSerial("z\n");
}

// Function called when "<<" button below the messagebox is pressed.
void MyMain::on_pushBtn5Clicked() {
    sendSerial("x\n");
}

// Function called when "||" button below the messagebox is pressed.
void MyMain::on_pushBtn6Clicked() {
    sendSerial("c\n");
}


// ********************************************************************************************************
//   Utility functions
// ********************************************************************************************************

// Display information into the messagebox on the upperright.
void MyMain::displayInfo(const char* str) {
    ui.textBrowser_9->append(QString::fromLocal8Bit(str));
    QScrollBar* sb = ui.textBrowser_9->verticalScrollBar();
    sb->setValue(sb->maximum());
}


// ********************************************************************************************************
//   Graph drawing functions
// ********************************************************************************************************

// Initialize the qCustomPlot graphs by preparing qCustomPlot canvas.
void MyMain::initialize_graphs() {
    // ****** ui.customPlot is the ABOVE, BLUE graph. It shows the raw current.   
    ui.customPlot->addGraph();
    ui.customPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
    // Background color which will be used in the post-process block.
    ui.customPlot->addGraph();
    ui.customPlot->graph(1)->setPen(QPen(QColor(255, 255, 255, 0)));
    ui.customPlot->graph(1)->setBrush(QBrush(QColor(255, 165, 0, 127)));;
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui.customPlot->xAxis->setTicker(timeTicker);
    ui.customPlot->axisRect()->setupFullAxesBox();
    ui.customPlot->yAxis->setRange(-2, 5);
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui.customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // ****** ui.customPlot_2 is the BELOW, ORANGE graph. It shows the processed data (i.e. the number of proteins)
    ui.customPlot_2->addGraph();
    const int PEN_WIDTH = 1;
    ui.customPlot_2->graph(0)->setPen(QPen(QColor(255, 110, 40), PEN_WIDTH));   // You can increase the PEN-WIDTH for better visibility, but it significantly affects performance (it is officially discussed in the qCustomPlot forum).
    QSharedPointer<QCPAxisTickerTime> timeTicker2(new QCPAxisTickerTime);
    timeTicker2->setTimeFormat("%h:%m:%s");
    ui.customPlot_2->xAxis->setTicker(timeTicker2);
    ui.customPlot_2->axisRect()->setupFullAxesBox();
    ui.customPlot_2->yAxis->setRange(-1, 1);
    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui.customPlot_2->xAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_2->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui.customPlot_2->yAxis, SIGNAL(rangeChanged(QCPRange)), ui.customPlot_2->yAxis2, SLOT(setRange(QCPRange)));
}

// Start the qCustomPlot graphs. (e.g. start the 1 Hz callback function)
// Also, this function conducts all other necessary procedures to start acquisition. (e.g. delete the previously recorded data / prepare the log file with the first row)
void MyMain::start_graphs() {
    // ****** Start the 1 Hz callback function.
    connect(&dataTimer_1Hz, SIGNAL(timeout()), this, SLOT(update_graph_1Hz()));
    dataTimer_1Hz.start(0);
    // ****** Set a flag to initialize the local variables in [update_graph_1Hz()].
    dataIndex_loop_num = -2;
    // ****** Delete the previously recorded data.
    ui.customPlot->graph(0)->data()->clear();
    ui.customPlot->graph(1)->data()->clear();
    ui.customPlot_2->graph(0)->data()->clear();
    // ****** Move the graphs to their initial positions.
    ui.customPlot->xAxis->setRange(dataStartTime, 8, Qt::AlignLeft);
    ui.customPlot->replot();
    ui.customPlot_2->xAxis->setRange(dataStartTime, 8, Qt::AlignLeft);
    ui.customPlot_2->replot();

    // ****** Prepare the logging output (1. filename)  [Ref] http://rinov.sakura.ne.jp/wp/cpp-date 
    time_t t = time(nullptr);
    const tm* lt = localtime(&t);
    std::stringstream s;
    s << "log\\";
    s << lt->tm_year + 1900;
    s << std::setw(2) << std::setfill('0') << lt->tm_mon + 1;
    s << std::setw(2) << std::setfill('0') << lt->tm_mday;
    s << "-";
    s << std::setw(2) << std::setfill('0') << lt->tm_hour;
    s << std::setw(2) << std::setfill('0') << lt->tm_min;
    s << std::setw(2) << std::setfill('0') << lt->tm_sec;
    switch (proteinType)
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
    std::string result = s.str();  // result = "log\\20220619-094610-AHL-" 
    this->myFileName_raw = result + "Raw.csv";
    this->myFileName_processed = result + "Processed.csv";
    this->myFileName_postprocessed = result + "POSTProcessed.csv";

    // ****** Prepare the logging output (2. initial rows)  
    // The output will be different based on the type of proteins (nanopore -> number only, ion channel -> open probability and magnitude of stimuli), so the first row should be adjusted.
    FILE* fp;
    // Processed data files
    switch (proteinType)
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

    // In case the current data is acquired from a real amplifier, then the raw value will also be output to file.
    if (dataSource == 0) {
        fopen_s(&fp, myFileName_raw.c_str(), "w");
        if (fp) {
            fprintf(fp, "time [s],current [pA]\n");
            fclose(fp);
        }
    }
    
    // PostProcessed Data files
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
            fprintf(fp, "step_time [s],conductance [pS]\n");  // [Note: This is completely adjusted to AHL, so I have to extend this to other proteins.] 
            fclose(fp);
        }
    }
}

// Stop the qCustomPlot graphs. 
void MyMain::stop_graphs() {
    dataTimer_1Hz.stop();
}





// ********************************************************************************************************
// ********************************************************************************************************
// 
//   The Main Function ... 1 Hz callback to acquire currents, analyze the data, drive peripheral devices, and update UI.
// 
// ********************************************************************************************************
// ********************************************************************************************************

void MyMain::update_graph_1Hz() {
    // set the timer;
    static QTime time(QTime::currentTime());
    static double lastTimerKey = 0;

    // On the first loop after pushing "Acquire" button, reset variables.
    if (dataIndex_loop_num == -2) {
        time.restart();
        if (dataSource != 0) lastTimerKey = 0;
        else lastTimerKey = -1;

        lastOpenNumber = -1;
        number_of_channel = -1;
        prev_num_channels = -1;
        current_per_channel = current_per_channel_user_specified;
        baseline = 0;
        rupture_flag = false;
        recovery_flag = false;
        startOneMolTime = -1;
        oneMolContinueFlag = false;

        dataIndex_loop_num = -1;
    }

    //***************************************
    // 1 Hz watchdog 
    double key = time.elapsed() / 1000.0;
    if (key - lastTimerKey > 1)
    {
        lastTimerKey = floor(key);
        dataIndex_loop_num++;
        // 1/8 Hz scrolling
        if (dataIndex_loop_num % 8 == 0) {
            ui.customPlot->xAxis->setRange(dataIndex_loop_num + dataStartTime, 8, Qt::AlignLeft);
            ui.customPlot_2->xAxis->setRange(dataIndex_loop_num + dataStartTime, 8, Qt::AlignLeft);
        }
        // 1/240 Hz buffer clearing for preventing std::bad_alloc error.
        if (dataIndex_loop_num % 240 == 0) {
            ui.customPlot->graph(0)->data()->clear();
            ui.customPlot->graph(1)->data()->clear();
            ui.customPlot_2->graph(0)->data()->clear();
        }

        //***************************************************************************************
        // Sense Block: Acquire the raw (digitized) current data from either of the amplifier or the local file.
        //***************************************************************************************
        double currentTime[SAMPLE_FREQ]; // The timestamp of data.
        double currentData[SAMPLE_FREQ]; // The raw current data (5000 samples per second).
        if (dataSource == 0) {
            readAmplifier(currentTime, currentData, dataIndex_loop_num);
        }
        else {
            if (readLocal(currentTime, currentData, dataIndex_loop_num) == -1) {
                this->on_pushBtn3Clicked();  // If the data range is over, terminate the process.
                return;
            }
        }

        //***************************************************************************************
        // Processing Block: Process the raw current to the idealized data, then obtain features like open probability.
        //***************************************************************************************
        /*
        [Important] The autor's comment: 
        This Processing Block, like other blocks (Sense and Actuation), should be refactored into separate source code,
        which makes future extensions easier. However, because of the large number of variables used, the too many conditional branches,
        and the accompanying processing changes, I couldn't refactor this section into separate files. 
        
        We sincerely hope that someone will understand and better organize this code someday.
        */

        // Processing (the former half)*******************************************************
        // This code receives the raw (digitized) current values from "SenseAmplifier" or "SenseLocal", 
        //    then idealizes the data to the number of open nanopores/ion channels,
        int processedData[SAMPLE_FREQ]; // The idealized data (5000 samples per second).
        const double threshold = 0.75;
        if (rupture_flag) {
            prev_num_channels = -1;
            number_of_channel = -1;
            lastOpenNumber = -1;
            startOneMolTime = -1;
            oneMolContinueFlag = false;
            rupture_flag = false;
            recovery_flag = false;
        }
        int maxOpenNumber = -1;
        const int rupture_threshold = 500;

        for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
            double y_now = currentData[idx];
            //*******
            // If the bilayer is ruptured, break the loop after making "rupture_flag" true.
            // To distinguish "the beginning of rupture (number -> OVERFLOW)" and "the end of rupture (OVERFLOW -> number)",
            // we also use "recovery_flag", which prevents motor rotation on recovering.
            if (y_now > rupture_threshold || y_now < -rupture_threshold) {
                rupture_flag = true;
                break;
            }
            //*******
            // Open/close determination for each timestep
            if (!rupture_flag) {
                if (current_per_channel >= 0) {   // Positive bias voltage
                    if (y_now > (lastOpenNumber + threshold) * current_per_channel + baseline) {
                        lastOpenNumber++;
                    }
                    else if (y_now < (lastOpenNumber - threshold) * current_per_channel + baseline) {
                        lastOpenNumber--;
                        if (lastOpenNumber < 0) lastOpenNumber = 0;
                    }
                }
                else {  // Negative bias voltage
                    if (y_now < (lastOpenNumber + threshold) * current_per_channel + baseline) {
                        lastOpenNumber++;
                    }
                    else if (y_now > (lastOpenNumber - threshold) * current_per_channel + baseline) {
                        lastOpenNumber--;
                        if (lastOpenNumber < 0) lastOpenNumber = 0;
                    }
                }
                processedData[idx] = lastOpenNumber;
                if (lastOpenNumber > maxOpenNumber) maxOpenNumber = lastOpenNumber;
            }
        }
        // recovery_flag [true if OVERFLOW -> 0] [true if 2 -> 0]
        if (rupture_flag) {
            if (-rupture_threshold < currentData[SAMPLE_FREQ - 1] && currentData[SAMPLE_FREQ - 1] < rupture_threshold) {
                recovery_flag = true;
            }
        }
        if (maxOpenNumber >= 2 && processedData[SAMPLE_FREQ - 1] == 0) {
            rupture_flag = true;
            recovery_flag = true;
        }

        // Baseline correction
        int zero_num = 0;
        if (!rupture_flag) {
            double zero_value = 0;
            double one_value = 0;
            int one_num = 0;
            bool updated = false;

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
            if (zero_num >= 500) {
                // Update the baseline when the change is in }50% of the single-molecule current.
                double tmp = zero_value / zero_num;
                if (current_per_channel > 0) {
                    if (-0.5 * current_per_channel < tmp && tmp < 0.5 * current_per_channel) {
                        if (ui.checkBox_2->isChecked()) current_per_channel -= (tmp - baseline);
                        if (ui.checkBox->isChecked()) baseline = tmp;
                        updated = true;
                    }
                }
                else {
                    if (0.5 * current_per_channel < tmp && tmp < -0.5 * current_per_channel) {
                        if (ui.checkBox_2->isChecked()) current_per_channel -= (tmp - baseline);
                        if (ui.checkBox->isChecked()) baseline = tmp;
                        updated = true;
                    }
                }
            }

            // Update the conductance when the change is in }25% of the single-molecule current.
            if (ui.checkBox_2->isChecked()) {
                if (one_num >= 500) {
                    double tmp = one_value / one_num - baseline;
                    if (current_per_channel > 0) {
                        if (0.75 * current_per_channel < tmp && tmp < 1.25 * current_per_channel) {
                            current_per_channel = tmp;
                            updated = true;
                        }
                    }
                    else {
                        if (1.25 * current_per_channel < tmp && tmp < 0.75 * current_per_channel) {
                            current_per_channel = tmp;
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
                this->displayInfo(disp_str.c_str());
            }

        }

        // Processing (the latter half)*******************************************************
        // This code receives the raw current values AND the idealized data,
        //    then calculates the experiment-specific features (i.e. open probability of ion channels, conductance of nanopores).

        // Calculating the open probability
        // double opProb = p; 
        // zero_num / 5000 = (1-p)^number_of_channel
        // p = 1 - pow(zero_num/5000, 1/number_of_channel)
        // if (number_of_channel = 0) p = 0;
        opProb = -99;    // Cannot calculate opProb when the bilayer is ruptured or number_of_channel = 0.
        if (!rupture_flag) {
            if (number_of_channel != 0) {
                opProb = 1 - pow(zero_num / double(SAMPLE_FREQ), 1.0 / number_of_channel);
                if (opProb < 0.001) opProb = 0.001;
                if (opProb > 0.999) opProb = 0.999;
            }
        }

        // Feature extraction 1:  Estimating the magnitude of stimuli by the given mathematical relationship.
        FILE* fp;
        int nowTime = round(dataIndex_loop_num + dataStartTime) + 1;
        switch (proteinType)
        {
        case 0:
            // if AHL, do nothing for stimuli estimation.
            // Export the number_of_channel.
            if (!rupture_flag) {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%d\n", nowTime, number_of_channel);
                }
            }
            else {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A\n", nowTime);
                }
            }
            break;
        case 1:
            // if BK, estimate the applied voltage
            // Given the data of pig-BK, 250mM KCl, 100 uM CaCl2 [20220427_BK channel_mV •Ï‰».abf],
            // we can estimate the stimuli(x) from the open probability (p) by applying sigmoidal function.
            // p = 1 / (1 + exp(-a*(x-x0)))
            // x = x0 + ln(p/(1-p))/a
            // According to Excel solver, a = 0.070469599, x0 = -28.3978081 
            if (opProb > 0 && !rupture_flag) {
                stimuli = -28.3978081 + log(opProb / (1 - opProb)) / 0.070469599;
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%lf,%lf\n", nowTime, opProb, stimuli);
                }
            }
            else {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A,#N/A\n", nowTime);
                }
            }
            break;
        case 2:
            // if OR8, estimate the applied octenol concentration
            // Given the data of [Dekel et al., 2016] (https://www.nature.com/articles/srep37330) (TaOR8 vs R-Octenol),
            // x = log10(concentration)
            // p = 1 / (1 + exp(-a*(x-x0)))
            // x = x0 + ln(p/(1-p))/a
            // concentration = exp10(x0 + ln(p/(1-p))/a)
            // According to Excel solver, a = 1.597072388, x0 = -6.495105404 
            // if(x > -6) concentration = exp10(x - (-6)) [uM]
            // else  concentration = exp10(x - (-9)) [nM]
            if (opProb > 0 && !rupture_flag) {
                stimuli = -6.495105404 + log(opProb / (1 - opProb)) / 1.597072388;
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,%lf,%lf\n", nowTime, opProb, stimuli);
                }
            }
            else {
                fopen_s(&fp, myFileName_processed.c_str(), "a");
                if (fp) {
                    fprintf(fp, "%d,#N/A,#N/A\n", nowTime);
                }
            }
            break;
        }
        if (fp) fclose(fp);

        // Also export the raw data if the data is obtained from an amplifier.
        if (dataSource == 0) {
            fopen_s(&fp, myFileName_raw.c_str(), "a");
            if (fp) {
                for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                    fprintf(fp, "%lf,%lf\n", currentTime[idx], currentData[idx]);
                }
                fclose(fp);
            }
        }


        // Feature extraction 2: Calculation of single-molecule conductance of nanopores
        switch (proteinType)
        {
        case 0:
            if (!recovery_flag) {

                if (ui.radioButton_13->isChecked()) {
                    // Prolonging the single-molecule time
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
                            // Graph plotting
                            endTime = currentTime[idx];
                            double tmpx[] = { startTime, startTime, endTime, endTime };
                            double tmpy[] = { 0, 500, 500, 0 };
                            for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                            oneMolFlag = false;
                            if (oneMolContinueFlag) {
                                double oneMolTime = endTime - startOneMolTime;
                                if (oneMolTime > 0.02) {
                                    this->displayInfo(std::to_string(oneMolTime).c_str());
                                    // CSV export
                                    fopen_s(&fp, myFileName_postprocessed.c_str(), "a");
                                    if (fp) {
                                        fprintf(fp, "%lf,%lf\n", std::round(startOneMolTime * 100) / 100, std::round(oneMolTime * 100) / 100);
                                        fclose(fp);
                                    }
                                }
                                oneMolContinueFlag = false;
                            }
                        }
                        if (processedData[idx] == -1) {
                            // If the bilayer is ruptured, terminate all process and break.
                            oneMolFlag = false;
                            startOneMolTime = -1;
                            oneMolContinueFlag = false;
                            this->displayInfo("Rupture detected");
                            break;
                        }
                    }
                    if (oneMolFlag) {
                        // Continue to the next timestep.
                        endTime = currentTime[SAMPLE_FREQ - 1];
                        double tmpx[] = { startTime, startTime, endTime, endTime };
                        double tmpy[] = { 0, 500, 500, 0 };
                        for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                        oneMolFlag = false;
                    }
                }

                if (ui.radioButton_14->isChecked()) {
                    // Evaluating the single-molecule conducntance.
                    for (int idx = 0; idx < SAMPLE_FREQ - 1; idx++) {
                        // Find the "jumping" point, which corresponds to the nanopore incorporation.
                        if (processedData[idx] == 0 && processedData[idx + 1] == 1) { //processedData[idx + 1] - processedData[idx] == 1 && processedData[idx] != -1
                            if (idx < 200 || idx > SAMPLE_FREQ - 1 - 200) continue; // To ensure the calculation accuracy
                            int zero_start_idx = idx - 500;
                            if (zero_start_idx < 0)zero_start_idx = 0;
                            int zero_end_idx = idx - 100;
                            int one_start_idx = idx + 100;
                            int one_end_idx = -1;
                            for (one_end_idx = one_start_idx; one_end_idx < SAMPLE_FREQ - 1; one_end_idx++) {
                                if (processedData[one_end_idx] != processedData[one_start_idx]) break;
                                if (one_end_idx > idx + 500) break;
                            }
                            double zero_value = 0;
                            for (int i = zero_start_idx; i <= zero_end_idx; i++) zero_value += currentData[i];
                            zero_value = zero_value / (zero_end_idx - zero_start_idx);
                            double one_value = 0;
                            for (int i = one_start_idx; i <= one_end_idx; i++) one_value += currentData[i];
                            one_value = one_value / (one_end_idx - one_start_idx);
                            // If the value is too strange, not use one.
                            if (one_value - zero_value > current_per_channel * 1.9 || one_value - zero_value < current_per_channel * 0.1) continue;

                            // Graph plotting
                            double tmpx[] = { currentTime[zero_start_idx], currentTime[zero_start_idx], currentTime[one_end_idx], currentTime[one_end_idx] };
                            double tmpy[] = { 0, 500, 500, 0 };
                            for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);

                            // CSV export after converting the current [pA] into conductance [pS] using heuristic knowledge of the bias voltage (50 [mV])
                            double one_conductance = (one_value - zero_value) * 1000 / 50;
                            this->displayInfo(std::to_string(one_conductance).c_str());
                            fopen_s(&fp, myFileName_postprocessed.c_str(), "a");
                            if (fp) {
                                fprintf(fp, "%lf,%lf\n", std::round(currentTime[one_start_idx] * 100) / 100, std::round(one_conductance * 100) / 100);
                                fclose(fp);
                            }

                        }

                        if (processedData[idx] == -1) {
                            // If the bilayer is ruptured, terminate all process and break.
                            this->displayInfo("Rupture detected");
                            break;
                        }
                    }

                }
            }
            break;
        case 1:
            // if BK, do nothing
            break;
        case 2:
            // if OR8, do nothing
            break;
        }
        

        //***************************************************************************************
        // Actuation Block: Based on the processing results, drive peripheral devices like stepper motors.
        //***************************************************************************************
        conductActuationSerial(rupture_flag, recovery_flag, number_of_channel);
        

        //***************************************************************************************
        // UI Block: Update the UI.
        //***************************************************************************************
        
        // Update the number of channels on the UI. 
        if (!rupture_flag) {
            if (maxOpenNumber != number_of_channel) {
                number_of_channel = maxOpenNumber;
                ui.textBrowser_12->setText(QString::fromLocal8Bit(std::to_string(maxOpenNumber).c_str()));
            }
        }
        else {
            ui.textBrowser_12->setText(QString::fromLocal8Bit("X"));
        }

        // Add the data and update the graphs on the UI.
        if (!rupture_flag) {
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                ui.customPlot->graph(0)->addData(currentTime[idx], currentData[idx]);
                ui.customPlot_2->graph(0)->addData(currentTime[idx], processedData[idx]);
            }
            if (prev_num_channels != number_of_channel) {
                // The above graph (raw data)
                if (current_per_channel > 0) {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(-2, tmp);
                }
                else {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(tmp, 2);
                }
                // The bottom graph (idealized data)
                ui.customPlot_2->yAxis->setRange(-1.0, 1.0 + number_of_channel);
            }
            prev_num_channels = number_of_channel;
        }
        else {
            // In case the bilayer is ruptured, only replot the raw data (because the idealized data is not calculated).
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                ui.customPlot->graph(0)->addData(currentTime[idx], currentData[idx]);
            }
        }
        ui.customPlot->replot();
        ui.customPlot_2->replot();


        // Updating the calculated features (opProb and stimuli)
        if (!rupture_flag) {
            switch (proteinType)
            {
            case 0:
                // if AHL, do nothing
                break;
            case 1:
                // if BK
                if (opProb < 0) {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
                    ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
                }
                else {
                    ui.textBrowser_2->setText(QString::fromLocal8Bit(std::to_string(int(opProb * 100)).c_str()));
                    ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(stimuli))).c_str()));
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
                    if (stimuli > -6) {
                        double concentration_uM = pow(10, stimuli - (-6));
                        ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(concentration_uM))).c_str()));
                        ui.textBrowser_6->setText(QString::fromLocal8Bit("uM"));
                    }
                    else {
                        double concentration_nM = pow(10, stimuli - (-9));
                        ui.textBrowser_5->setText(QString::fromLocal8Bit(std::to_string(int(round(concentration_nM))).c_str()));
                        ui.textBrowser_6->setText(QString::fromLocal8Bit("nM"));
                    }
                }
                break;
            }
        }
        else {
            // ruptured
            ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
            ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
        }
    }
}