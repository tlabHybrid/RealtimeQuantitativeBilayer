//*********************
// MyMain.cpp contains...
//   *  [Setup]                 Set up an amplifier or load data, prepare for serial communication etc.
//   *  [SenseBlock]            Acquire raw current from an amplifier / a local file
//   *  [PreprocessingBlock]    Determine the number of proteins
//   *  [PostprocessingBlock]   Calculate conductance / single-molecule time  
//   *  [ActuationBlock]        Drive peripheral devices like motors or bluetooth
//   *  [InterfaceBlock]        Manage UI
//*********************

#include "MyMain.h"
#include "Measurement.h"
#include "MyHelper.h"
#include "TecellaAmp.h"
#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <time.h>
// <Qt/PrintSupport> is used in qcustomplot.cpp


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
    this->setup_mySerial();
    this->data_from_local = true;

    this->display_Infos("**------**");
    ui.pushButton_2->setEnabled(false);
    ui.pushButton_3->setEnabled(false);
    // clock_t start_time = clock();
    // clock_t end_time = clock();
    // std::cout << "elapsed time: " << double(end_time - start_time) / CLOCKS_PER_SEC << "sec" << std::endl;
}

MyMain::~MyMain() {
    this->close_mySerial();
    if (!this->data_from_local) measure_finalize(this); // Disconnect amplifier and release memories associated with it.
}


// ********************************************************************************************************
//   Pushbutton slots (A) ... Measurement related functions
// ********************************************************************************************************

// Function called when "Setup" button is pressed.
// Set up the amplifier.
void MyMain::on_pushBtnClicked() {
    // ****** Determine the source of input data from the toggled radiobox.
    if (ui.radioButton->isChecked()) {
        // Direct current acquisition from Amplifier PICO.
        this->display_Infos("Data source: Amplifier PICO");
        // Suggest user to cover the device with faraday cage. If just debugging, the user can skip the calibration.
        // choice == 0 : yes  choice == 1 : no  choice == -1 : escaped
        int choice = QMessageBox::question(this, tr("Confirmation"), tr("Make sure the PICO terminals are open and covered with a Faraday cage. Do you want to calibrate PICO?"), tr("Yes"), tr("No, just passing by"));
        // Connect to the amplifier and conduct setup.
        if (measure_init(this, choice) != 0) {
            this->display_Infos("Unable to open PICO. Check connection and retry.");
            return;
        }
        this->data_from_local = false;
    }
    else if (ui.radioButton_2->isChecked()) {
        // Reading current data from a local ATF file.
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
        // Reading current data from a local CSV file.
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

    // ****** Determine the type of protein from the toggled radiobuttons.
    if (ui.radioButton_4->isChecked()) {
        // Alpha Hemolysin from Staphylococcus aureus
        // Perhaps in the future, this option will be changed to "Nanopores".
        this->display_Infos("Protein: Alpha hemolysin from Staphylococcus aureus");
        baseline = 0;
        current_per_channel_user_specified = 44.5;
        ui.textBrowser_2->setEnabled(false);
        ui.textBrowser_3->setEnabled(false);
        ui.textBrowser_5->setEnabled(false);
        ui.textBrowser_6->setEnabled(false);
        this->protein_type = 0;
    }
    else if (ui.radioButton_5->isChecked()) {
        // Big Potassium (K+) Channel from Pig
        // Perhaps in the future, this option will be changed to "Ion channels" altogether with the below option.
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
        // Olfactory Receptor (OR) 8 from Aedes aegypti
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

    // ****** Modification of the current_per_channel_user_specified (which corresponds to channel conductance) if necessary.
    bool ok;
    double d = QInputDialog::getDouble(this, tr("QInputDialog::getDouble()"),
        tr("Do you want to modify the current value per channel?"), this->current_per_channel_user_specified, -100, 100, 1, &ok,
        Qt::WindowFlags(), 0.5);
    if (ok) {
        this->current_per_channel_user_specified = d;
    }

    // ****** Initialization of graphs
    this->initialize_graphs();

    this->display_Infos("Setting up completed.");
    this->display_Infos("**------**");
    this->ui.pushButton_2->setEnabled(true);
}

// Function called when "Acquire" button is pressed.
// Start acquisition and drawing graphs.
void MyMain::on_pushBtn2Clicked() {
    // ****** Start graphs and register the 1 Hz callback function [update_graph_1Hz()].
    this->start_graphs();
    
    this->display_Infos("Acquisition has started.  Push Stop button to stop acquisition.");
    this->display_Infos("**------**");
    this->ui.pushButton->setEnabled(false);
    this->ui.pushButton_2->setEnabled(false);
    this->ui.pushButton_3->setEnabled(true);
}

// Function called when "Stop" button is pressed.
// Stop acquisition and drawing graphs.
void MyMain::on_pushBtn3Clicked() {
    // ****** Stop graphs and cancel the 1 Hz callback function [update_graph_1Hz()].
    this->stop_graphs();

    this->display_Infos("Acquisition has stopped.  Push Acquire button to restart acquisition.");
    this->display_Infos("**------**");
    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
    this->ui.pushButton_3->setEnabled(false);
}


// ********************************************************************************************************
//   Pushbutton slots (B) ... Motor drive related functions
// ********************************************************************************************************
// Note: the used characters (z, x, c...) is predefined in the Arduino source code.

// Function called when "|<<" button below the messagebox is pressed.
// Send instructions to the stepper motor to rotate until the STOP button is pressed.
void MyMain::on_pushBtn4Clicked() {
    if (this->port.isOpen()) {
        this->port.write("z\n");
    }
}

// Function called when "<<" button below the messagebox is pressed.
// Send instructions to the stepper motor to rotate only a single step.
void MyMain::on_pushBtn5Clicked() {
    if (this->port.isOpen()) {
        this->port.write("x\n");
    }
}

// Function called when "||" button below the messagebox is pressed.
// Send instructions to the stepper motor to stop.
void MyMain::on_pushBtn6Clicked() {
    if (this->port.isOpen()) {
        this->port.write("c\n");
    }
}


// ********************************************************************************************************
//   Utility functions
// ********************************************************************************************************

// Display information into the messagebox on the upperright.
void MyMain::display_Infos(const char* str) {
    ui.textBrowser_9->append(QString::fromLocal8Bit(str));
    QScrollBar* sb = ui.textBrowser_9->verticalScrollBar();
    sb->setValue(sb->maximum());
}

// Data loading function which read data from a local ATF/CSV file.
// The data are stored in MyMain::data_x and MyMain::data_y vectors.
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
        // ATF files ... The files which are exported from Clampfit or other software by "export" function.
        // THe first 10 rows are headers to be disposed of.
        for (int i = 0; i < 10; i++) in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split('\t');      // split the string by tab
            data_x.append(fields.at(0).toDouble());     // Time [s]
            data_y.append(fields.at(1).toDouble());     // Current [pA]
        }
    }
    else {
        // CSV files ... The files which are exported from Clampfit or other software by "Transfer Traces" function.
        // The first row is a header to be disposed of.
        if (isHeader) in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split(',');       // split the string by conma
            if (ui.comboBox->currentIndex() == 0) {         // In case the time unit is [s]. (e.g. The re-usage of the data recorded by this system)
                data_x.append(fields.at(0).toDouble());
            } else if (ui.comboBox->currentIndex() == 1) {  // In case the time unit is [ms]. (e.g. The Transfer Traces function.)
                data_x.append(fields.at(0).toDouble() * 0.001);       // [ms] -> [s]
            }
            data_y.append(fields.at(1).toDouble());         // Current [pA]
        }
    }
    file.close();
    return data_x.size();
}


// Function to set up the serial communication.
void MyMain::setup_mySerial()
{
    QSerialPortInfo info;
    foreach(const QSerialPortInfo & info_tmp, QSerialPortInfo::availablePorts()) { //availablePorts()�ŗ��p�\�Ȃ��ׂẴV���A���|�[�g���擾�ł���
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
            QMessageBox::critical(this, tr("Error"), "Couldn't open the serial port. Maybe the port is used by other software (like Serial Monitor)?");
        }
    }
}

// Function to send a character to Arduino MEGA by the serial communication.
void MyMain::actuation_sendSerial(int data) {
    if (this->port.isOpen()) {
        if(data == 0) this->port.write("r\n");
        this->display_Infos("Sent data to serial port.");
    }
}

// Function to release Arduino MEGA from the serial communication.
void MyMain::close_mySerial()
{
    if (this->port.isOpen()) this->port.close();
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
    this->dataTimer_1Hz.start(0);
    // ****** In case the data are loaded from local files, the starting time may not be 0[s]. Thus we need to record the initial time to compensate for it.
    if (this->data_from_local) this->first_data_x = data_x.at(0);
    else this->first_data_x = 0;
    // ****** Set a flag to initialize the local variables in [update_graph_1Hz()].
    this->timer_restart_flag_1Hz = true;
    // ****** Delete the previously recorded data.
    ui.customPlot->graph(0)->data()->clear();
    ui.customPlot->graph(1)->data()->clear();
    ui.customPlot_2->graph(0)->data()->clear();
    // ****** Move the graphs to their initial positions.
    ui.customPlot->xAxis->setRange(first_data_x, 8, Qt::AlignLeft);
    ui.customPlot->replot();
    ui.customPlot_2->xAxis->setRange(first_data_x, 8, Qt::AlignLeft);
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
    std::string result = s.str();  // result = "log\\20220619-094610-AHL-" 
    this->myFileName_raw = result + "Raw.csv";
    this->myFileName_processed = result + "Processed.csv";
    this->myFileName_postprocessed = result + "POSTProcessed.csv";

    // ****** Prepare the logging output (2. initial rows)  
    // The output will be different based on the type of proteins (nanopore -> number only, ion channel -> open probability and magnitude of stimuli), so the first row should be adjusted.
    FILE* fp;
    // Processed data files
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

    // In case the current data is acquired from a real amplifier, then the raw value will also be output to file.
    if (!this->data_from_local) {
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
    static double lastPointKey = 0;
    static int lastOpenNumber = 0; // ���O�̎��ԂɊJ���Ă����`�����l���̐��D �S�Ẵ`�����l���̐��萔������number_of_channel�Ƃ͕ʁD
    static int prev_num_channels = 0;  //���܂ł̎��ԂɊJ���Ă����ő�̃`���l�����Dnumber_of_channel�̑O���ԁD
    static int dataIndex_loop_num = 0;  // ���̊֐����Ăяo���ꂽ�񐔁D��������ƂɁC�ǂ͈̔͂�ǂݍ��ނ������߂�

    static double startOneMolTime = -1;
    static bool oneMolContinueFlag = false;


    if (this->timer_restart_flag_1Hz) {
        time.restart();
        if (this->data_from_local) lastPointKey = 0; //���[�J���f�[�^��ǂލۂɂ́C1�b�ォ��\�����n�߂�
        else lastPointKey = -1;     // �A���v����ǂ݂����Ƃ��ɂ́Cstart���������u�Ԃ���v�����n�߂� (�����1�b�ォ��\�������)
        lastOpenNumber = -1;    // ����ɕK���O���t���X�V���Ă��炤���߂ɁC�͂��߂�0�ł͂Ȃ�-1�ɂ��Ă���
        dataIndex_loop_num = -1;      // ���₷���̂��߂ɏ��߂�++�������Ă����̂ŁC�͂��߂�-1�ɂ��Ă����D
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
    // ���[�J���f�[�^�ǂݍ��݂̍ۂɂ́C�ǂݍ��񂾔͈͂���͂ݏo���Ȃ��悤�ɂ���
    if (this->data_from_local) {
        if (key * SAMPLE_FREQ > this->data_x.size()) {
            this->on_pushBtn3Clicked();
            return;
        }
    }


    //***************************************
    // 1Hz�Ńf�[�^�擾�ƐM������
    if (key - lastPointKey > 1)
    {

        //***************************************
        // 0.125 Hz�ł̉�ʈʒu�ړ�
        dataIndex_loop_num++;
        if (dataIndex_loop_num % 8 == 0) {
            ui.customPlot->xAxis->setRange(dataIndex_loop_num + first_data_x, 8, Qt::AlignLeft);
            ui.customPlot_2->xAxis->setRange(dataIndex_loop_num + first_data_x, 8, Qt::AlignLeft);
            //ui.customPlot->replot();
            //ui.customPlot_2->replot();
        }

        
        //***************************************************************************************
        // Sense: �A���v���烊�A���^�C���ɓd���l���擾�D�ꉞ���[�J���f�[�^����̓ǂݍ��݂��\�D
        //***************************************************************************************
        // �@ �f�[�^�擾
        double currentTime[SAMPLE_FREQ]; // ��������5000�f�[�^�̎擾����[s]
        double currentData[SAMPLE_FREQ]; // ��������5000�f�[�^�̓d���l[pA]  ����current�̓d��current�Ƃ������E�}�ϐ�
        int processedData[SAMPLE_FREQ];  // �������5000�f�[�^�̊J�o�C�i��[�J��e�̐�] 
        for (int i = 0; i < SAMPLE_FREQ; i++) {
            currentTime[i] = 0;
            currentData[i] = 0;  // ������
            processedData[i] = -1;
        }

        //int dataIndex = floor(key) * SAMPLE_FREQ; // 5kHZ sampling
        if (!this->data_from_local) {
            //�A���v����5kHz�Ńf�[�^���擾
            measure_conduct(this, currentTime, currentData);
            //currentTime,currentData�Ƀf�[�^�𓱓�
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                currentTime[idx] = dataIndex_loop_num + idx / double(SAMPLE_FREQ);  // 5kHz = 0.0002 sec per sample
            }
        }
        else {
            // data_x, data_y����5000�T���v����ǂ݂���
            int start_num = dataIndex_loop_num * SAMPLE_FREQ;
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                currentTime[idx] = data_x.at(idx + start_num);
                currentData[idx] = data_y.at(idx + start_num);
            }
        }
        lastPointKey = floor(key); // �������l��1�b���Ƃɂ���


        //***************************************************************************************
        // Processing: �擾�����d���l��M���������C���q���Ƃ��ڕW�����̔Z�x�𐄒肷��D
        //***************************************************************************************
        // 
        clock_t start_time = clock();
        // �A �M������
        //QVector<double> current_per_channels;  // �J�̕ω������o�������C���̕ω��ʂ��L�^���Ă��� 
        //QVector<double> zeros;  // �S�Ẵ`���l�������Ă���Ƃ��̓d���l���L�^���Ă����D

        // (a) �q�X�e���V�X�R���p���[�^�̏����ɂ��`���l���J����D
        // 20220722�ǋL�G�i�m�|�A�ɂ��ẮC���܂�q���[���X�e�B�b�N�I�ȓd���l���g�p�����C�m�C�Y�̑傫�����傫���X�e�b�v�����ׂČv���ł���悤�ɂ�����
        const double threshold = 0.75;  // 0.5����open/close��50%��臒l�����D 0.75�Ƃ�0.8�ɂ���ƃq�X�e���V�X�R���p���[�^�ɂȂ�
        if (this->rupture_flag) {
            prev_num_channels = -1;
            this->number_of_channel = -1;
            lastOpenNumber = -1;

            startOneMolTime = -1;
            oneMolContinueFlag = false;


            this->rupture_flag = false;
            this->recovery_flag = false;
        }
        int maxOpenNumber = -1;  // ���q���X�V�̂��߂̋L�^�ϐ��D5000�T���v�����ł̍ő�̊J���q���D
        const int rupture_threshold = 500;
        // ����1�b�Ԃ̃x�[�X���C�� for nanopore
        double preprocessedBaseline = 0; // ���[�p�X�����d��
        bool no_response_flag = false;
        double prev_preprocessedData = 0;
        for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
            double y_now = currentData[idx];
            //*******
            // �������ꂽ�ꍇ�ɂ́C�u���ꂽ�v�t���O��true�ɂ��ĒE�o����D
            // ����臒l�́C�K���Ɂ}500pA�Ƃ���D

            /* 20220705�ǋL�F���ꂾ�Ɩ������ꂽ(�͂��ߐ�����X�C��������X��X)�̂Ɩ����񕜂���(�͂���X������)�̋�ʂ����Ȃ��D
            * ������� recovery_flag�𗧂ĂāC���ꂪtrue�̎��ɂ͉�]�M�����o���Ȃ����Ƃɂ��� (����Ă͂���̂�rupture_flag��true�ɂ��C��ʍX�V�͂��Ȃ�)
            */
            if (y_now > rupture_threshold || y_now < -rupture_threshold) {
                this->rupture_flag = true;
                break;
            }
            //*******
            //��������͂܂Ƃ��ȃ`���l���J����
            if (!this->rupture_flag) {

                switch (protein_type)
                {
                case 0:
                    // if AHL�C�m�C�Y�ȏ�̑傫���̃W�����v�Ȃ�S�Ĉꕪ�q�Ƃ݂Ȃ�

                    /*
                    if (current_per_channel >= 0) {   // �d������̕����ɐU���Ƃ�
                        if (y_now > (lastOpenNumber + threshold) * current_per_channel + baseline) {
                            lastOpenNumber++;
                        }
                        else if (y_now < (lastOpenNumber - threshold) * current_per_channel + baseline) {
                            lastOpenNumber--;
                            if (lastOpenNumber < 0) lastOpenNumber = 0;
                        }
                    }
                    else {  //�d�������̕����ɐU���Ƃ��D�R���p���[�^�̕������ς��D
                        if (y_now < (lastOpenNumber + threshold) * current_per_channel + baseline) {
                            lastOpenNumber++;
                        }
                        else if (y_now > (lastOpenNumber - threshold) * current_per_channel + baseline) {
                            lastOpenNumber--;
                            if (lastOpenNumber < 0) lastOpenNumber = 0;
                        }
                    }
                    */
                    
                    if (current_per_channel >= 0) {   // �d������̕����ɐU���Ƃ�
                        if (lastOpenNumber < 0) lastOpenNumber = 0;
                        
                        // �܂����[�p�X�t�B���^������
                        int start_idx = idx - 100;
                        if (start_idx < 0) start_idx = 0;
                        int end_idx = idx + 100;
                        if (end_idx > SAMPLE_FREQ - 1) end_idx = SAMPLE_FREQ - 1;
                        double preprocessedData = 0;
                        for (int lowpass_idx = start_idx; lowpass_idx < end_idx; lowpass_idx++) {
                            preprocessedData += currentData[lowpass_idx];
                        }
                        preprocessedData /= (end_idx - start_idx);

                        // �d�����}�C�i�X�ɂȂ��Ă���Ƃ��́C��������������ł���\���������̂�0�߂��ɂȂ�܂Ŗ���
                        if (preprocessedData < 0) {
                            no_response_flag = false;
                        }

                        if (idx == 0) {
                            preprocessedBaseline = preprocessedData;
                            no_response_flag = false;
                        }

                        // �s���т����Ȃ��ƁC��HL��W�����v��9���x�F������Ă��܂��D�t�ɁC�s���т���������ƁC�A�v�^�}�[��S�~�łӂ����������̃X�p�C�N�����m�ł��Ȃ��Ȃ�D
                        // �u���[�p�X�l�����肷��v�����܂ŃX�L�b�v����
                        if (no_response_flag == true) {
                            if (abs(prev_preprocessedData - preprocessedData) < 0.1 && preprocessedData > 0) {
                                preprocessedBaseline = preprocessedData;
                                no_response_flag = false;
                            }
                        }
                        else {
                            // ���̃��[�p�X�ς݂̒l���C�x�[�X���C�����傫��������u�W�����v�v�Ɣ��f����D
                            const int heuristic_noise = 5;
                            if (preprocessedData > preprocessedBaseline + heuristic_noise) {
                                no_response_flag = true;

                                // �������q���قړ����ɓ������ꍇ�C���[�p�X�̉e���ŋ�ʂł��Ȃ��Ȃ�C2�{�̃R���_�N�^���X�Ɗ��Ⴂ�����\��������D�ǂ����悤�H
                                lastOpenNumber++;
                            }
                            else if (preprocessedData < preprocessedBaseline - heuristic_noise) {
                                lastOpenNumber--;
                                no_response_flag = true;
                                if (lastOpenNumber < 0) lastOpenNumber = 0;
                            }
                        }
                        prev_preprocessedData = preprocessedData;

                    }
                    else {   // �d�������̕����ɐU���Ƃ�
                        // �o�O���Ȃ��Ȃ�����R�s�y
                    }
                    
                    
                    break;
                case 1:
                    // if BK �R���_�N�^���X�Œ�C75% threshold
                    if (current_per_channel >= 0) {   // �d������̕����ɐU���Ƃ�
                        if (y_now > (lastOpenNumber + threshold) * current_per_channel + baseline) {
                            lastOpenNumber++;
                        }
                        else if (y_now < (lastOpenNumber - threshold) * current_per_channel + baseline) {
                            lastOpenNumber--;
                            if (lastOpenNumber < 0) lastOpenNumber = 0;
                        }
                    }
                    else {  //�d�������̕����ɐU���Ƃ��D�R���p���[�^�̕������ς��D
                        if (y_now < (lastOpenNumber + threshold) * current_per_channel + baseline) {
                            lastOpenNumber++;
                        }
                        else if (y_now > (lastOpenNumber - threshold) * current_per_channel + baseline) {
                            lastOpenNumber--;
                            if (lastOpenNumber < 0) lastOpenNumber = 0;
                        }
                    }
                    break;
                }

                //�L�^
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
        // 20220719 ���q��2�Ŗ����ɉ�葱����o�O�̏C��
        if (maxOpenNumber >= 2 && processedData[SAMPLE_FREQ - 1] == 0) {
            this->rupture_flag = true;
            this->recovery_flag = true;
        }

        //**************��������Ă���ꍇ�C���̌�̂قƂ�ǂ̍H���𖳎�����D****************
        
        // (b) �`�����l���̐��̍X�V�D (�O���t�̏c���̏C���́Cnumber_of_channel����ćD�ōs��)�D
        // MaxOpenNumber (�����5000�T���v���ł̍ő�J���q��) ���Cthis->number_of_channel (�O��܂ł̍ő�J���q��)�ƈႤ�ꍇ�ɂ͍X�V����D
        if (!this->rupture_flag) {
            if (maxOpenNumber != number_of_channel) {
                number_of_channel = maxOpenNumber; //�`���l�����̍X�V (�`���l�����������Ă��悢)
                ui.textBrowser_12->setText(QString::fromLocal8Bit(std::to_string(maxOpenNumber).c_str()));
            }
        }
        else {
            //���ꂽ�ꍇ
            ui.textBrowser_12->setText(QString::fromLocal8Bit("X"));
        }



        // (c) �x�[�X���C���ƈꕪ�q������̊J�R���_�N�^���X��␳.
        // processedData�����ƂɁC�u���q������0�v�u���q����1�v�̂Ƃ����K���ɕ��ς��Ƃ�
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
            //�ǂ���Ƃ��C�J�E���g����500(1�b�f�[�^��10%)�����ł���Ƃ��ɂ́C�M�p�ł��Ȃ��Ƃ݂Ȃ��čX�V���Ȃ����Ƃɂ���D���ӓI�����D�D�D
            if (zero_num >= 500) {
                //�x�[�X���C���́C���Ƃ��Ɓu0�v�����C�u1���q�R���_�N�^���X�́}50%�ȓ��v�ł���΍X�V����D(����ȏ�͕��ʂ�1���q�ڂ������Ă���Ƃ݂Ȃ�)
                //current_per_channel�̕����v���ӁI�I�I
                double tmp = zero_value / zero_num;
                if (current_per_channel > 0) {
                    if (-0.5 * this->current_per_channel < tmp && tmp < 0.5 * this->current_per_channel) {
                        if (ui.checkBox_2->isChecked()) this->current_per_channel -= (tmp - this->baseline);  // �ꕪ�q�R���_�N�^���X���A�����ďC��(�x�[�X���C���ϓ������C��)
                        if (ui.checkBox->isChecked()) this->baseline = tmp;
                        updated = true;
                    }
                }
                else {
                    if (0.5 * this->current_per_channel < tmp && tmp < -0.5 * this->current_per_channel) {
                        if (ui.checkBox_2->isChecked()) this->current_per_channel -= (tmp - this->baseline);  // �ꕪ�q�R���_�N�^���X���A�����ďC��(�x�[�X���C���ϓ������C��)
                        if (ui.checkBox->isChecked()) this->baseline = tmp;
                        updated = true;
                    }
                }
            }

            //�ꕪ�q�R���_�N�^���X�́C���Ƃ��Ɓu�^���p�N�����ƂɎw�肵���l�v�����C�u���Ƃ�1���q�R���_�N�^���X�}25%�ȓ��v�ł���΍X�V����D(����ȏ�͕��ʂ�2���q�ڂ������Ă���Ƃ݂Ȃ�)
            //current_per_channel�̕����v���ӁI�I�I
            //���܂�ς�������Ɛl�Ԃ̉�͂Ƃ����̂ŁC�x�[�X���C���␳�݂̂ɂ��Ă�����͖���������̂��I�����Ƃ��Ē񎦂���D(�`�F�b�N�{�b�N�X�Ő؂�ւ���)
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
        // �B �J�m���̌v�Z
        // zero_num�͏�Ōv�Z�������̂��g��
        //�{���͊e 0 < c < number_of_channel �ɂ��Čv�Z����̂��ǂ��̂��낤���C�����0�����őË��D
        // double opProb = p; 
        // zero_num / 5000 = (1-p)^number_of_channel
        // p = 1 - pow(zero_num/5000, 1/number_of_channel)
        // if (number_of_channel = 0) p = 0;
        double opProb = -99;    // ��������Ă���C���q�������܂���0�������肷���opProb�͕��̒l�ɂȂ�
        if (!this->rupture_flag) {
            if (number_of_channel != 0) {
                opProb = 1 - pow(zero_num / double(SAMPLE_FREQ), 1.0 / number_of_channel);
                if (opProb < 0.001) opProb = 0.001;
                if (opProb > 0.999) opProb = 0.999;
            }
        }


        //***************************************
        // �C �����w�I�h���̌v�Z�C����уt�@�C���ւ̏����o���D���Ǝ����ƊJ�m�����D
        FILE* fp;
        double millivolts = 0;
        double x_con = 0;
        int nowTime = round(dataIndex_loop_num + first_data_x) + 1;
        switch (protein_type)
        {
        case 0:
            // if AHL �������Ȃ�
            //5000�T���v���ڂ̊J��Ԃ̕��q�� (�ő�l: number_of_channel �������� MaxOpenNumber) �����̂܂܏o��
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
            // if BK �d������
            // �u�^BK, 250mM KCl, 100 uM CaCl2�̃f�[�^����C�V�O���C�h�֐��ŋt����
            // p = 1 / (1 + exp(-a*(millivolts-x0)))
            // �܂� millivolts = x0 + ln(p/(1-p))/a
            // Excel�\���o�[�ɂ�� a = 0.070469599, x0 = -28.3978081 
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
            // if OR8 �Z�x����
            // �V�O���C�h�֐�����t���肷��D
            // Dekel 2016 ��TaOR8 vs R-Octenol �̃O���t��p����D
            // x = log10(concentration)
            // p = 1 / (1 + exp(-a*(x-x0)))
            // �܂� x = x0 + ln(p/(1-p))/a
            // �܂� concentration = exp10(x0 + ln(p/(1-p))/a) (��̕��������₷������)
            // Excel�\���o�[�ɂ�� a = 1.597072388, x0 = -6.495105404 
            // x�̒l�ɉ����ď�������D
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


        // �A���v������ꍇ�͐��f�[�^(5KHz)���o�͂���
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
        // �D ��̃O���t(���f�[�^)�E���̃O���t(�����ς݃f�[�^)�ւ̔��f

        // �f�o�b�O�F�w�i�F�e�X�g�D�ꕪ�q���Ԃ�R���_�N�^���X�W�����v�ɂ��āC�u�������v�Z�Ɏg�����v�A�s�[��������D
        if (!this->recovery_flag) {

            if (ui.radioButton_13->isChecked()) {
                //�ꕪ�q���Ԃ��ł��邾�������������C���̏W�v�������ł�肽���ꍇ
                // processedData��1�ł���Ή��F�ɂ���D
                //�����C���S�Ƀ�HL�����ɂ��Ă��܂��Ă�悤�ȁD�D�D�H�Ђ���Ƃ���ƃ^���p�N�����Ƃ̋�ʂ��K�v����

                // ����ă��[�^�[�ŉ񂵂Ă���Ƃ��̂����������u���q���̕ϓ��v�Ƃ��Č��Ă���H �� ���ꂽ�Ƃ��납��1�b�Ԃ͌v�����Ȃ����Ƃɂ���D
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
                            if (oneMolTime > 0.02) { // ���܂�ɂ��Z������̂͏��O����
                                this->display_Infos(std::to_string(oneMolTime).c_str());
                                //******************* ������ւ�Łu�J�n�����C��������̈ꕪ�q���ԁv��CSV�ɏo�͂���D ****************************
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
                        //�S�Ă̏����𒆒f���ă��[�v�𔲂���
                        oneMolFlag = false;
                        startOneMolTime = -1;
                        oneMolContinueFlag = false;
                        this->display_Infos("rupture detected");
                        break;
                    }
                }
                if (oneMolFlag) {
                    //���̎��Ԃ֌p��
                    endTime = currentTime[SAMPLE_FREQ - 1];
                    double tmpx[] = { startTime, startTime, endTime, endTime };
                    double tmpy[] = { 0, 500, 500, 0 };
                    for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);
                    oneMolFlag = false;
                }
            }

            if (ui.radioButton_14->isChecked()) {
                // �ꕪ�q�R���_�N�^���X��]���������C���̃R���_�N�^���X�̌����z����肽���ꍇ
                // processedData��0����1�ɔ�񂾎���100ms�Ƃ������F������D
                // �ǋL�G0��1��������Ȃ��āC2��3�Ƃ��C3��4�Ƃ����܂߂Ă����񂶂�Ȃ��H��������Ɖ������D�������f�[�^����肽�����0��1�������g������������
                //�����C���S�Ƀ�HL�����ɂ��Ă��܂��Ă�悤�ȁD�D�D�H�Ђ���Ƃ���ƃ^���p�N�����Ƃ̋�ʂ��K�v����
                for (int idx = 0; idx < SAMPLE_FREQ - 1; idx++) {
                    //�W�����v�����Ƃ���̎��ӂ��ώ@���āC���ς��Ƃ�
                    if (processedData[idx + 1] - processedData[idx] == 1 && processedData[idx] != -1) {  //processedData[idx] == 0 && processedData[idx + 1] == 1
                        //�K�v�Ȏ��Ԃ̌v�Z
                        if (idx < 200 || idx > SAMPLE_FREQ - 1 - 200) continue; // �v�Z���x���s���ȏꍇ�D�g���Ă������Ƃ͎v�����D�D�D

                        // ���q�������Ȃ���
                        int zero_start_idx = idx - 500;
                        if (zero_start_idx < 0)zero_start_idx = 0;
                        int zero_end_idx = idx-100;
                        // if (zero_start_idx >= zero_end_idx) continue;  //�[�������N�����Ȃ����߂ɁC�v�Z�ł��Ȃ��ꍇ�͏������Ȃ�
                        // if (zero_end_idx - zero_start_idx < 200) continue;  //�܂��C�v�Z���x��ۏ؂��邽�߂ɁC200�T���v��(40ms)�ȉ��̃f�[�^�����Ȃ��ꍇ�͌v�Z���Ȃ�

                        // ���q����������   ������́C��������Ă��Ȃ����Ƃ�ۏ؂���K�v����
                        int one_start_idx = idx + 100;
                        int one_end_idx = -1;
                        for (one_end_idx = one_start_idx; one_end_idx < SAMPLE_FREQ - 1; one_end_idx++) {
                            if (processedData[one_end_idx] != processedData[one_start_idx]) break;  //���q��������ɑ������芄�ꂽ�肵���甲����
                            if (one_end_idx > idx + 500) break;
                        }
                        // if (one_start_idx >= one_end_idx) continue;  //�[�������N�����Ȃ����߂ɁC�v�Z�ł��Ȃ��ꍇ�͏������Ȃ�
                        // if (one_end_idx - one_start_idx < 200) continue;  //�܂��C�v�Z���x��ۏ؂��邽�߂ɁC200�T���v��(40ms)�ȉ��̃f�[�^�����Ȃ��ꍇ�͌v�Z���Ȃ�
                        
                        //******************* ������ւ�Łu�W�����v�����C�����ł̈ꕪ�q�R���_�N�^���X�v��CSV�ɏo�͂���D ****************************
                        double zero_value = 0;
                        for (int i = zero_start_idx; i <= zero_end_idx; i++) zero_value += currentData[i];
                        zero_value = zero_value / (zero_end_idx - zero_start_idx);
                        double one_value = 0;
                        for (int i = one_start_idx; i <= one_end_idx; i++) one_value += currentData[i];
                        one_value = one_value / (one_end_idx - one_start_idx);

                        // ���܂�ɂ��������ꍇ�͍̗p���Ȃ�
                        if (one_value - zero_value > this->current_per_channel * 1.9 || one_value - zero_value < this->current_per_channel * 0.1) continue;


                        //�F�t��
                        double tmpx[] = { currentTime[zero_start_idx], currentTime[zero_start_idx], currentTime[one_end_idx], currentTime[one_end_idx] };
                        double tmpy[] = { 0, 500, 500, 0 };
                        for (int i = 0; i < 4; i++) ui.customPlot->graph(1)->addData(tmpx[i], tmpy[i]);


                        //�����܂œd��[pA]�Ȃ̂ŁC�q���[���X�e�B�b�N�X(50mV)�ŃR���_�N�^���X[pS]�ɕϊ�����
                        double one_conductance = (one_value - zero_value) * 1000 / 50;
                        this->display_Infos(std::to_string(one_conductance).c_str());
                        fopen_s(&fp, myFileName_postprocessed.c_str(), "a");
                        if (fp) {
                            fprintf(fp, "%lf,%lf\n", std::round(currentTime[one_start_idx] * 100) / 100, std::round(one_conductance * 100) / 100);
                            fclose(fp);
                        }

                    }

                    if (processedData[idx] == -1) {
                        //�S�Ă̏����𒆒f���ă��[�v�𔲂���
                        this->display_Infos("rupture detected");
                        break;
                    }
                }

            }
        }


        // 20220722�ǋL�FQvector�������Ȃ肷����ƁC1630�b(28��)�t�߂�Microsoft C++ �̗�O: std::bad_alloc ���������ăA�v����������D
        // �ł́C����Qvector�����I�ɊJ������悤�ɂ�����C���̃G���[�͎��邾�낤���H
        // 8�b���Ƃ�clear����΁C���[�U�͋C�Â��Ȃ��͂��D
        // �Ⴆ�΁C4��(8�b��1���̍ŏ����{��)���Ƃ�graph��clear���Ă݂�D
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
            // �`�����l�����E�R���_�N�^���X�ɉ�����y���͈̔͂�ύX����
            // 20220722�ǋL�F�������ꂽ�ꍇ�ł��C���̎��_�ł�MaxOpenNumber�ɉ����ĉ�ʂ͈̔͂�ύX�ł���悤�ɂ����� (���F���g���������̂���������)
            if (prev_num_channels != number_of_channel) {
                //��
                if (current_per_channel > 0) {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(-2, tmp);
                }
                else {
                    int tmp = int((number_of_channel + 0.75) * current_per_channel + baseline);
                    ui.customPlot->yAxis->setRange(tmp, 2);
                }
                //��
                ui.customPlot_2->yAxis->setRange(-1.0, 1.0 + number_of_channel);
            }
            prev_num_channels = number_of_channel;
        }
        else {
            // ��������Ă���ꍇ�́C�O���t�̎����ύX�����C������֐�����΂�
            for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
                ui.customPlot->graph(0)->addData(currentTime[idx], currentData[idx]); //���f�[�^�͏����D
                // �����ς݃f�[�^�͏����Ȃ��D
            }
            //y���͈̔͂�ύX���Ȃ��D
        }
        ui.customPlot->replot();
        ui.customPlot_2->replot();


        //***************************************
        // �E ��ʕ\���ւ̔��f (���q���ɂ��Ă͇A�Ŕ��f����)
        if (!this->rupture_flag) {
            switch (protein_type)
            {
            case 0:
                // if AHL �������Ȃ�
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
            //����Ă��ꍇ�͑S��X�ɂ���
            ui.textBrowser_2->setText(QString::fromLocal8Bit("X"));
            ui.textBrowser_5->setText(QString::fromLocal8Bit("X"));
        }


        //***************************************************************************************
        // Actuation: Processing�̌��ʂ����ƂɁCSerialPort�o�͂���ă��[�^�[�Ȃǂ��쓮����
        //***************************************************************************************
        if (!this->rupture_flag) {
            switch (protein_type)
            {
            case 0:
                // if AHL
                // �u1���q�v���̎��Ԃ��\�Ȍ���L�΂��v���u2���q�ڂ��������疌�������Œ���Ȃ����v
                if (this->number_of_channel >= 2) {
                    this->actuation_sendSerial(0);  // ��]�w���D�f�[�^�͓K���D
                }
                break;
            case 1:
                // if BK
                if (millivolts > -40) {
                    this->actuation_sendSerial(0);  // �K���Ƀf�[�^�𑗐M���Ă݂�
                }
                break;
            case 2:
                // if OR8 
                break;
            }
        }
        else {
            //����Ă��ꍇ
            if(!this->recovery_flag) this->actuation_sendSerial(0);  // ���ꂽ�ꍇ�͂����ɒ���Ȃ����悤�Ɏw�����΂�
        }
    }
}