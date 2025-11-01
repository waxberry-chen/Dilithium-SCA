#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QList>
#include <QMessageBox>
#include <QDateTime>
#include "QDebug"
#include "qexcel.h"
#include "control.h"
//#include "ftdi/ftd2xx.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>



#include <QtCharts>
QT_CHARTS_USE_NAMESPACE
extern "C" {
    #include "ps3000aApi.h"
}
#define NoOf_Samples 1024*64
#define M 3000
#define Trace 83
#define K 500
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    int16_t BufferA[NoOf_Samples];
    int A_enable;
    int16_t BufferB[NoOf_Samples];
    int B_enable;
    int16_t BufferC[NoOf_Samples];
    int C_enable;
    int16_t BufferD[NoOf_Samples];
    int D_enable;
    int16_t Buffer[NoOf_Samples];
    int32_t Buffer_Sum[2][NoOf_Samples] ={0};
    uint32_t Time_Base;
    double corr[6];
    double r[K][Trace];
    int16_t r_wirte[K][Trace];
    //uint32_t NoOf_Samples=1024;

    int Capture_count = 10;

    QExcel * myexcel;
    QExcel * ReadExcel;
    QExcel * WriteExcel;
    QChart* chart ;
    QLineSeries *series;
    int  Current_col =1;
    int  Current_index=0;

    QFile HD_File;

    QTcpSocket *mSocket;



    void Excel_test(int16_t BufferA[],int16_t BufferB[],int16_t BufferC[],int16_t BufferD[]);
    void Excel_write_col(int col,int16_t Buffer[],int length);
    void Run_Pico();
    void Set_Pico();
    void Run_Shake();
    void Set_Wots(int tree_index,int tree_height,int layer,int keypair,int Mode);
    void Set_Adder(int16_t a, int16_t b);
    void Set_PWM4(int16_t s[],int16_t u[]);
    void PWM4_Loop();
    void PWM_Decode(int16_t u[4], char str[]);
    double Corrcoef(int16_t A[],int16_t B[], int Length);
    void GetH(int16_t HD[],int sk_num,int coef_index);
    void GetT(int coef_index,int Time);
    void GetT2(int coef_index,int Time);
    void Sign_Attack();
    double Calcuate_r(int16_t HD[],int16_t T[], int length);
    void Set_Key();

    void Genreuslt();

    void Get_Sever_inst();
    double Gen_t_value(int16_t a[],int16_t b[], int length);
    void Gen_SHA4_TVLA();


//    int16_t H[M];
//    int16_t T[M];
    ~MainWindow();

protected:
    void findFreePorts();
    bool initSerialPort();
    void sendMsg(const QString &msg);

public slots:
    void recvMsg();


private:
    Ui::MainWindow *ui;
    QSerialPort *serialPort;
    HW_CONTROL* hw;
    void openOSC(int ena,int enb,  int enc, int end,int index);
};

#endif // MAINWINDOW_H
