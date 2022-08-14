#pragma once

#include "MyMain.h"
extern const int SAMPLE_FREQ;

/*  Sense Block  *****************************************************************************/
// SenseAmplifier.cpp // 
int setupAmplifier(int choice);
void readAmplifier(double* timestamp, double* destination, int dataIndex_loop_num);
void stopAmplifier();
int finalizeAmplifier();
// SenseLocal.cpp // 
int setupLocal(MyMain* mainwindow, int extension, bool isSeconds, double* dataStartTime);
int readLocal(double* timestamp, double* destination, int dataIndex_loop_num);


/*  Processing Block  *****************************************************************************/
// See the comment at "Processing Block" in MyMain.cpp.


/*  Actuation Block  *****************************************************************************/
// qcustomserial.cpp //
void setupSerial(MyMain* mainwindow);
void sendSerial(const char*);
void closeSerial();
// ActuationSerial.cpp //
void conductActuationSerial(bool rupture_flag, bool recovery_flag, int number_of_channel);
