#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "visa.h"
#include "visatype.h"
#include <QTcpServer>
#include <QTcpSocket>
#include<QTime>
#include<QDate>
#include <windows.h>
#include <cstdlib>
#include <ctime>

// --- 全局参数定义 (无改动) ---
char Time[] = "HORIZONTAL:SCALE 4E-5";
char Trig_Source[] ="TRIGger:A:EDGE:SOUrce CH1";
char Trig_Vol[]    = "TRIGger:A:LEVel:CH1 1.4";
char Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe RISe";
char Read_CH[] = "DATa:SOUrce CH2";
char Data_Start[] = "DATa:START 90000";
char Data_Stop [32] = "DATa:STOP 430000";
char Data_Mode[]  = "DATa:ENCdg RPBinary";
char NTT_Pipline_Time[] ="HORIZONTAL:SCALE 4E-7";
char NTT_Pipline_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";
char NTT_Pipline_Data_Start[32] =   "DATa:START 2000";
char NTT_Pipline_Data_Stop [32] = "DATa:STOP 7000";
const int NTT_Pipeline_capture_count = 5;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//******************* tcp_server *********************//
    mServer = new QTcpServer(this);
    connect(mServer,&QTcpServer::newConnection,this,&MainWindow::NTT_Pipeline_Attack); //连接客户端
    //启动服务器监听
    mServer->listen(QHostAddress::LocalHost,9988);
    Connected=0;
}

MainWindow::~MainWindow()
{
    delete ui;
}

/******************************
 * TEST FUNCTION 3:
 * NTT_Pipeline_Attack (EVENT-DRIVEN OPTIMIZED VERSION)
******************************/
void MainWindow:: NTT_Pipeline_Attack()
{
    qDebug()<<"新客户段连接";
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");

    Connected=1;
    mSocket =new QTcpSocket(this);
    mSocket = mServer->nextPendingConnection();//与客户端通信的套接字

    uint8_t * Data_read       = new uint8_t[5000 * 2 + 10];
    int16_t * Data_Write      = new int16_t[5000];
    int32_t * Data_Sum        = new int32_t[5000];
    int Data_locate;

    //******************** MDO与VISA配置 ************************//
    ViSession	rm = VI_NULL, vi = VI_NULL;
    ViUInt32	retCnt;
    ViEventType eventType;
    ViEvent     eventContext;
    ViStatus    status;

    char Device[256] = "USB::0x0699::0x0456::C013077::INSTR";
    // Open a default Session
    viOpenDefaultRM(&rm);
    viOpen(rm, Device, VI_NULL, VI_NULL, &vi);

    // --- 核心改进 1: 配置示波器以生成事件 ---
    // SCPI (Standard Commands for Programmable Instruments) 命令用于设置事件
    viWrite(vi, (ViBuf)"*CLS", 4, &retCnt);      // 1. 清除状态寄存器
    viWrite(vi, (ViBuf)"*ESE 1", 6, &retCnt);      // 2. 事件状态使能: 只关心 "Operation Complete" (第0位)
    viWrite(vi, (ViBuf)"*SRE 32", 7, &retCnt);     // 3. 服务请求使能: 当有标准事件发生时 (第5位), 产生服务请求(SRQ)

    // --- 核心改进 2: 在PC端启用VISA事件队列 ---
    // 我们告诉VISA库，我们想要监听来自仪器的服务请求(SRQ)事件
    status = viEnableEvent(vi, VI_EVENT_SERVICE_REQ, VI_QUEUE, VI_NULL);
    if (status < VI_SUCCESS) {
        qDebug() << "无法启用VISA事件!";
        viClose(vi);
        viClose(rm);
        return;
    }

    // --- MDO基础配置 (与原版相同) ---
    viWrite(vi, (ViBuf)"ACQuire:MODe HIRes",18,&retCnt);
    viWrite(vi, (ViBuf) NTT_Pipline_Time, sizeof(NTT_Pipline_Time), &retCnt);
    viWrite(vi, (ViBuf) Trig_Source, sizeof(Trig_Source), &retCnt);
    viWrite(vi, (ViBuf) Trig_Vol, sizeof(Trig_Vol), &retCnt);
    viWrite(vi, (ViBuf) NTT_Pipline_Trig_Mode, sizeof(NTT_Pipline_Trig_Mode), &retCnt);
    viWrite(vi, (ViBuf) Read_CH, sizeof(Read_CH), &retCnt);
    viWrite(vi, (ViBuf) NTT_Pipline_Data_Start, sizeof(NTT_Pipline_Data_Start), &retCnt);
    viWrite(vi, (ViBuf) NTT_Pipline_Data_Stop, sizeof(NTT_Pipline_Data_Stop), &retCnt);
    viWrite(vi, (ViBuf) Data_Mode, sizeof(Data_Mode),&retCnt);

    viSetAttribute(vi, VI_ATTR_WR_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);
    viSetAttribute(vi, VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);

    char str1[64] ="header off\n";
    char str2[64]="DATA:ENCDG RIBINARY;WIDTH 2\n";
    viPrintf(vi, str1);
    viPrintf(vi, str2);

    QString a_Qstr;
    char u_str[10];
    const int fixed_b = 666;

    for (int loop = 0; loop < 10 ; loop++) {
        QString filename = "D:\\ntt_pipeline_traces-loop"+QString::number(loop)+".txt" ;
        QFile f(filename);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << ("打开文件失败");
            return;
        }
        QTextStream txtOutput(&f);

        for(int a_value = 0; a_value < 3329; a_value++) {
            for(int k=0;k<5000;k++) Data_Sum[k] = 0;

            for(int k=0;k<NTT_Pipeline_capture_count;k++) {

                // --- 核心改进 3: 用事件等待取代轮询 ---
                // 1. 发送一个耗时操作的命令. "*OPC" 会在该操作完成后设置事件状态位.
                //    ACQUIRE:STOPAFTER SEQUENCE 使示波器准备好进行一次采集.
                viWrite(vi, (ViBuf) "ACQUIRE:STOPAFTER SEQUENCE;*OPC", 30, &retCnt);

                // 2. 发送指令给FPGA，让它开始运算并触发示波器
                a_Qstr = QString::number(a_value, 10);
                strcpy(u_str, a_Qstr.toLatin1().data());
                mSocket->write("NTT_Pipe");
                mSocket->write(u_str);
                mSocket->waitForBytesWritten();

                // 3. 高效等待事件. 程序将在此挂起, 直到示波器采集完成并发出SRQ事件.
                //    CPU占用率几乎为零. 超时设为5000毫秒.
                status = viWaitOnEvent(vi, VI_EVENT_SERVICE_REQ, 5000, &eventType, &eventContext);

                viClose(eventContext); // 必须关闭事件句柄以释放资源

                if (status < VI_SUCCESS) {
                    qDebug() << "ERROR: Oscilloscope timeout or fail.";
                    // 在此可以添加错误处理或重试逻辑
                    goto acquisition_failed; // 跳转到循环末尾
                }

                // 4. (可选) 读取状态字节，确认是SRQ事件并清除它
                ViUInt16 stb;
                viReadSTB(vi, &stb);

                // --- 数据读取 (与原版类似) ---
                viWrite(vi, (ViBuf) "CURVE?", 6, &retCnt);
                viFlush(vi, VI_WRITE_BUF | VI_READ_BUF_DISCARD);
                viSetAttribute(vi,VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_DISABLE);
                viRead(vi, (ViBuf) Data_read, 5000*2+10, &retCnt);
                Data_locate = Data_read[1] - '0' + 2;
                for(int m=0;m<5000;m++) // 修正了循环变量冲突
                {
                    Data_Sum[m] += (Data_read[2*m+ 1 + Data_locate] |(int16_t)(Data_read[2*m+ Data_locate] << 8));
                }
            acquisition_failed:
                viWrite(vi, (ViBuf) "ACQUIRE:STATE RUN", 17, &retCnt);  // 让示波器重新开始运行，为下一次采集做准备
            }

            // --- 数据处理与保存 (无改动) ---
            txtOutput << QString::number(a_value, 10) << ":";
            for(int k=0;k<5000;k++)
            {
                Data_Write[k] = Data_Sum[k]/NTT_Pipeline_capture_count;
                txtOutput << " " << Data_Write[k];
            }
            txtOutput << endl;

            current_date_time =QDateTime::currentDateTime();
            current_date = current_date_time.toString("dd hh:mm:ss.zzz ddd");
            qDebug() << current_date;
            qDebug() << "Loop" << loop << "Trace " << a_value << ": a=" << a_value << ", b=" << fixed_b;
        }
        f.close();
        qDebug() << "NTT Pipeline Attack completed loop " << loop;
    }

    // --- 核心改进 4: 清理和关闭事件及VISA会话 ---
    viDisableEvent(vi, VI_EVENT_SERVICE_REQ, VI_QUEUE);
    viClose(vi);
    viClose(rm);
}

void MainWindow::Set_Data_range(int Data_Start,int Data_End)
{
    QString Data_Start_str = "DATa:START ";
    QString Data_End_str = "DATa:STOP ";
    Data_Start_str += QString::number(Data_Start,10);
    Data_End_str += QString::number(Data_End,10);

    // 这两个变量在NTT_Pipeline_Attack中并未使用，但保留函数以兼容其他部分
    // strcpy(PWM_Template_Data_Start,Data_Start_str.toLatin1().data());
    // strcpy(PWM_Template_Data_Stop,Data_End_str.toLatin1().data());
}

void MainWindow::client_dis()
{
    QTcpSocket *obj = (QTcpSocket*)sender();
    qDebug()<<obj->peerAddress().toString();
    Connected=0;
}