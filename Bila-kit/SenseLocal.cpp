/******************************************************************************
// SenseLocal.cpp
//
// This code obtains the current data from local ATF/CSV files.
//
******************************************************************************/

#include "MyHelper.h"
#include "MyMain.h"

QVector<double> localTimeStamp;
QVector<double> localCurrentData;

// Specify the target file, conduct some preprocessing (like dropping headers), and record all data to local variables. 
// extension: 0 = ATF, 1 = CSV
int setupLocal(MyMain* mainwindow, int extension, bool isSeconds, double* dataStartTime) {
    // Specify the target file.
    QString filename;
    if(extension == 0) filename = QFileDialog::getOpenFileName(mainwindow, "Choose an ATF file.", "data", "ATF files(*.atf);;All Files(*.*)");
    else filename = QFileDialog::getOpenFileName(mainwindow, "Choose a CSV file.", "data", "CSV files(*.csv);;All Files(*.*)");
    if (filename.isEmpty()) return -1;

    // Open the file and obtain data.
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return -2;
    QTextStream in(&file);
    QString line;
    localTimeStamp.clear(); // Release the data before newly import one.
    localCurrentData.clear();

    /************************************************************************
    * style specification:
    * Several rows at the top might be headers to be disposed.
    * Column 1 must be Time [s].  ATF file matches this condition.  However, in the exported CSV file, the unit become [ms], so adaptation (*0.001) must be applied.
    * Column 2 must be Current [pA].
    * Separator of columns should be tab (ATF) or conma(CSV).
    ************************************************************************/
    if (extension == 0) {
        // ATF files ... The files which are exported from Clampfit or other software by "export" function.
        // THe first 10 rows are headers to be disposed of.
        for (int i = 0; i < 10; i++) in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split('\t');      // split the string by tab
            localTimeStamp.append(fields.at(0).toDouble());     // Time [s]
            localCurrentData.append(fields.at(1).toDouble());     // Current [pA]
        }
    }
    else {
        // CSV files ... The files which are exported from Clampfit or other software by "Transfer Traces" function.
        // The first row is a header to be disposed of.
        in.readLine();
        while (!in.atEnd()) {
            line = in.readLine();
            QStringList fields = line.split(',');       // split the string by conma
            if (isSeconds) {         // In case the time unit is [s]. (e.g. The re-usage of the data recorded by this system)
                localTimeStamp.append(fields.at(0).toDouble());
            }
            else{  // In case the time unit is [ms]. (e.g. The Transfer Traces function.)
                localTimeStamp.append(fields.at(0).toDouble() * 0.001);       // [ms] -> [s]
            }
            localCurrentData.append(fields.at(1).toDouble());         // Current [pA]
        }
    }
    *dataStartTime = localTimeStamp.at(0);

    file.close();
    return 0;
}


// Conduct the acquisition from the file.
int readLocal(double* timestamp, double* destination, int dataIndex_loop_num) {

    // Check if the data is out of range from the file or not.
    if ((dataIndex_loop_num + 1) * SAMPLE_FREQ > localTimeStamp.size()) return -1;

    // Copy the required data to the destination.
    for (int idx = 0; idx < SAMPLE_FREQ; idx++) {
        timestamp[idx] = localTimeStamp.at(idx + dataIndex_loop_num * SAMPLE_FREQ);
        destination[idx] = localCurrentData.at(idx + dataIndex_loop_num * SAMPLE_FREQ);
    }
    return 0;
}

