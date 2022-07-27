#include "MyMain.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyMain w;  // myMainはQapplicationの前には.出せない．グローバル変数には出来ないか．．．
    w.show();
    return a.exec();
}
