#include "MyMain.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyMain w;  // myMain��Qapplication�̑O�ɂ�.�o���Ȃ��D�O���[�o���ϐ��ɂ͏o���Ȃ����D�D�D
    w.show();
    return a.exec();
}
