#pragma once

#include <QWidget>
#include "ui_subWin.h"

class subWin : public QWidget
{
	Q_OBJECT

public:
	subWin(QWidget *parent = nullptr);
	~subWin();
	void changeFontColor(int num);  // 0:black, 1: gray, 2: red
	void changeString(const char* str);

private:
	Ui::subWinClass ui;
};
