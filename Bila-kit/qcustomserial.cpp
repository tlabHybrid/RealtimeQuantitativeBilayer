/******************************************************************************
// qcustomserial.cpp
//
// This code provide methods to access serial port to the connected microcomputers.

// reference: https://qiita.com/sh4869/items/b514483fff70b1319af1 (in Japanese)
******************************************************************************/

#include "MyHelper.h"
#include "MyMain.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

QSerialPort port;
int serialTarget = -1; // -1: no serial, 0: Arduino, 1: Legato180


// Function to set up the serial communication.
void setupSerial(MyMain* mainwindow)
{
    QSerialPortInfo info;
    foreach(const QSerialPortInfo & info_tmp, QSerialPortInfo::availablePorts()) {
        qDebug() << "Name        : " << info_tmp.portName();
        qDebug() << "Description : " << info_tmp.description();
        qDebug() << "Manufacturer: " << info_tmp.manufacturer();
        if (info_tmp.description() == "Arduino Mega 2560") {
            info = info_tmp;
            serialTarget = 0;
        }else if (info_tmp.manufacturer() == "Microsoft") {
            info = info_tmp;
            serialTarget = 1;
        }
    }
    if (info.isNull()) {
        QMessageBox::information(mainwindow, "Info", "Neither Arduino Mega nor KDS LEGATO 180 is connected. Continue with no serial communication.");
    }
    else if (serialTarget == 0){
        port.setPort(info);
        port.setBaudRate(QSerialPort::Baud9600);
        port.setDataBits(QSerialPort::Data8);
        port.setParity(QSerialPort::NoParity);
        port.setStopBits(QSerialPort::OneStop);
        if (port.open(QIODevice::ReadWrite)) {

        }
        else {
            QMessageBox::critical(mainwindow, "Error", "Couldn't open the serial port. Maybe the port is used by other software (like Serial Monitor)?");
        }
    }
    else if (serialTarget == 1) {
        port.setPort(info);
        port.setBaudRate(QSerialPort::Baud115200);
        port.setDataBits(QSerialPort::Data8);
        port.setParity(QSerialPort::NoParity);
        port.setStopBits(QSerialPort::OneStop);
        if (port.open(QIODevice::ReadWrite)) {

        }
        else {
            QMessageBox::critical(mainwindow, "Error", "Couldn't open the serial port. Maybe the port is used by other software (like Serial Monitor)?");
        }
    }
    else {

    }
}

// Function to send a character to Arduino MEGA by the serial communication.
// Example: "r\n": Instruct the stepper motor to move back and forth once for bilayer reformation.
// "z\n": Instruct the stepper motor to rotate until the STOP button is pressed.
// "x\n": Instruct the stepper motor to rotate only a single step.
// "c\n": Instruct the stepper motor to stop.

void sendSerial(const char* data) {
    if (port.isOpen()) {
        port.write(data);
    }
}

void sendSerial_pump() {
    if (port.isOpen()) {
        //QByteArray ba;
        //ba.append("run");
        //port.write(ba);
        //QString buffer = "run\n";
        //port.write(buffer.toStdString().c_str(), buffer.size());
        //port.waitForBytesWritten(-1);
        //QString data = "run\n";
        //char* transsmit_data = data.toUtf8().data();
        //port.write(transsmit_data);
        //port.write("run\n");

    }
}

// Function to release Arduino MEGA from the serial communication.
void closeSerial()
{
    if (port.isOpen()) port.close();
}

