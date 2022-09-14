/******************************************************************************
// subWin.cpp
//
// This code provides an interface to subWindow from mainWindow. 
// [Note]	The subWindow should be on a sub display, which is connected via "Connection application (Miracast)" of Windows 10.
//			If the connection is not working well, try disconnection and reconnection to the internet by switching Airplane mode.
//			Also, the "Connection application" won't work well unless the display settings of each PC is properly configured (e.g. match the magnification rate and resolution).
******************************************************************************/

#include "subWin.h"

subWin::subWin(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

subWin::~subWin()
{}

void subWin::changeFontColor(int num) {
	if (num == 0) {
		ui.label->setStyleSheet("QLabel { color : black; }");
		ui.label_2->setStyleSheet("QLabel { color : black; }");
		ui.label_3->setStyleSheet("QLabel { color : black; }");
		ui.label_4->setStyleSheet("QLabel { color : black; }");
		ui.label_5->setStyleSheet("QLabel { color : black; }");
	}
	else if (num == 1) {
		ui.label->setStyleSheet("QLabel { color : gray; }");
		ui.label_2->setStyleSheet("QLabel { color : gray; }");
		ui.label_3->setStyleSheet("QLabel { color : gray; }");
		ui.label_4->setStyleSheet("QLabel { color : gray; }");
		ui.label_5->setStyleSheet("QLabel { color : gray; }");
	}
	else if (num == 2) {
		ui.label->setStyleSheet("QLabel { color : red; }");
		ui.label_2->setStyleSheet("QLabel { color : red; }");
		ui.label_3->setStyleSheet("QLabel { color : red; }");
		ui.label_4->setStyleSheet("QLabel { color : red; }");
		ui.label_5->setStyleSheet("QLabel { color : red; }");
	}
	
}

void subWin::changeString(int num, const char* str) {
	if (num == 0) {
		ui.label_2->setText(str);
		ui.label_2->setAlignment(Qt::AlignRight);
	}
	else if (num == 1) {
		ui.label_4->setText(str);
		ui.label_4->setAlignment(Qt::AlignRight);
	}
	else if (num == 2) {
		ui.label_5->setText(str);
		ui.label_5->setAlignment(Qt::AlignRight);
	}
}