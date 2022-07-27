#pragma once

#include <QtWidgets/QWidget>
#include "ui_MyMain.h"
#include "qcustomplot.h"
#include <QTimer>
#include "TecellaAmp.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class MyMain : public QWidget
{
    Q_OBJECT

public:
    MyMain(QWidget *parent = Q_NULLPTR);
    ~MyMain();
    void display_Infos(const char*);
    TECELLA_HNDL h;   // �A���v���g�����߂̃����o�ϐ��D

private:
    Ui::MyMainClass ui;


    int load_data_from_local(const char* filepath, bool isATF, bool isHeader=true); // ���[�J������ATF, CSV�Ȃǂœd���l�����O���[�h����֐��DATF��CSV�Ńw�b�_�[�̂�����R���}�E�^�u���Ⴄ�̂ŕ�����D�܂��CCSV�̓w�b�_�[���t���Ă��Ȃ����̂ɂ��Ή�
    void initialize_graphs();
    void start_graphs();
    void stop_graphs();
    QVector<double> data_x;
    QVector<double> data_y;
    double first_data_x;  // ���[�J���t�@�C��(ATF/CSV)��1�s�ڂ̎����D������L�^���Ă����Ȃ��ƃO���t�`��ʒu�������D

    QTimer dataTimer_50Hz;  // ��ʍX�V�̂��߂�callback�֐����Ăяo�����߂̃^�C�}�[
    QTimer dataTimer_1Hz; // �f�[�^�擾�ƐM�������̂��߂�callback�֐����Ăяo�����߂̃^�C�}�[
    bool timer_restart_flag_50Hz = false; // �ēxacquire���������ۂɂ͂�����true�ɂ���D
    bool timer_restart_flag_1Hz = false;

    std::string myFileName_raw;  // �o�͐�̃t�@�C���� (5kHz���̓d���l)
    std::string myFileName_processed;  // �o�͐�̃t�@�C���� (1Hz �M�������ς�)
    std::string myFileName_postprocessed;  // �o�͐�̃t�@�C���� (�ꕪ�q���Ԃ�ꕪ�q�R���_�N�^���X�ȂǁCpostprocess��������̃f�[�^�̏o�͐�D)

    bool data_from_local = false;  // ATF, CSV�ǂݍ��ݎ���true�ɂȂ�D�A���v���烊�A���^�C���擾����Ƃ��ɂ�false�D
    int protein_type;  // ���^���p�N���̎�ށD 0: AHL, 1: BK, 2:OR8

    int number_of_channel = 0;   // ���A���^�C���ɐ��肵���C�I���`���l���̐��D
    double current_per_channel;  // ��`���l��������̂����悻�̓d���l�D�R���_�N�^���X�~�o�C�A�X�d���DBK��10�COR��2���炢�D������ɉ����Ď����ŕς��Ă���
    double current_per_channel_user_specified;  //���[�U�[�����͂����l��ۑ����Ă����ϐ��D����}20���͈͈̔ȊO�ɂ͔�яo�����Ȃ��D
    double baseline = 0.0; // �S�Ẵ`���l�������Ă���Ƃ��̓d���l�D������ɉ����Ď����ŕς��Ă���
    bool rupture_flag = false;  // �������ꂽ���ǂ����̔�����L�^����ϐ��D
    bool recovery_flag = false;  // �����񕜒����ǂ����̔�����L�^����ϐ��D





    // QSerialport���g���ăV���A���ʐM�������Ă݂悤
    // https://qiita.com/sh4869/items/b514483fff70b1319af1
    QSerialPort port;   // �V���A���ʐM�p
    void setup_mySerial();
    void actuation_sendSerial(int data = 0);
    void close_mySerial();

private slots:
    void on_pushBtnClicked();
    void on_pushBtn2Clicked();
    void on_pushBtn3Clicked();
    void on_pushBtn4Clicked();
    void on_pushBtn5Clicked();
    void on_pushBtn6Clicked();
    void update_graph_50Hz();  // 50Hz�ŉ�ʍX�V�C1Hz�Ńf�[�^�擾
    void update_graph_1Hz();
};
