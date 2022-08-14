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
        }
    }
    if (info.isNull()) {
        QMessageBox::information(mainwindow, "Info", "Arduino Mega is not connected. Continue with no serial communication.");
    }
    else {
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

// Function to release Arduino MEGA from the serial communication.
void closeSerial()
{
    if (port.isOpen()) port.close();
}

