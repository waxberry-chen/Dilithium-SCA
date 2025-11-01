#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <Qdir>
#include "qexcel.h"

#define NoOf_Samplers 340000

#define PWM4_NoOf_Samplers 10000000
#define Continue_Num       115
#define PWM4_M             3000 - Continue_Num
#define PWM_Tamplate_onetrace 150
#define PWM_Pipeline_onetrace 200

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    //unsigned int  NoOf_Samplers = 1000; //10k

    int Capture_count = 1000;
    int PWM4_capture_count = 100;
    int PWM_Tamplate_capture_count = 10;
    int NTT_Tamplate_capture_count = 1000;
    int PWM_Pipeline_capture_count = 100;

    int Sampler_capture_count = 100;
    int NTT_Pipeline_capture_count = 1;

    int NTT_TVLA_capture_count = 10;
    QTcpServer *mServer;
    QTcpSocket *mSocket;

    QExcel * myexcel;
    QExcel * ReadExcel;
    int Connected;
    int Current_row;

    void Sign_Attack();
    void Sign_Attack2();
    void PWM4_Attack();
    void PWM_Tamplate_Attack();
    void NTT_Tamplate_Attack();
    void NTT_Pipeline_Attack();
    void PWM_Pipeline_Attack();
    void NTT_TVLA_Attack();
    void Dili_NTT_TVLA_Attack();
    void Sampler_Attack();
    void Set_Data_range(int Data_Start,int Data_End);
    void client_dis();
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
