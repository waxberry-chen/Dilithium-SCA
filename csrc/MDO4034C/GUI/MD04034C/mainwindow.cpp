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
char Time[] = "HORIZONTAL:SCALE 4E-5";                   //时间40us
char Trig_Source[] ="TRIGger:A:EDGE:SOUrce CH1";         //触发源CH1
char Trig_Vol[]    = "TRIGger:A:LEVel:CH1 1.4";          //触发电平
char Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe RISe";        //触发方式 rise  fAll
char Read_CH[] = "DATa:SOUrce CH2";                  //读取通道
char Data_Start[] = "DATa:START 90000";                  //start 100k - 10k
char Data_Stop [32]                                                                                                                   = "DATa:STOP 430000";                //Stop 100k + 330k
char Data_Mode[]  = "DATa:ENCdg RPBinary";           //无符号数

char PWM4_Time[] ="HORIZONTAL:SCALE 4E-7";                   //时间400ns
char PWM4_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式 rise  fAll
char PWM4_Data_Start[] = "DATa:START 5000";                  //start 50k
char PWM4_Data_Stop [32] = "DATa:STOP 6000";                 //Stop 60k


char PWM_Template_Time[] ="HORIZONTAL:SCALE 4E-4";                   //时间400us
char PWM_Template_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char PWM_Template_Data_Start[32] =   "DATa:START 1000000";                  //start 1m
char PWM_Template_Data_Stop [32] = "DATa:STOP 9322500";                 //Stop 60k  //2500点一个运算 取 790-840

char NTT_Template_Time[] ="HORIZONTAL:SCALE 4E-7";                   //时间400Ns
char NTT_Template_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char NTT_Template_Data_Start[32] =   "DATa:START 5000";              //start 50K
char NTT_Template_Data_Stop [32] = "DATa:STOP 6000";                 //Stop 60k

char PWM_Pipeline_Time[] ="HORIZONTAL:SCALE 4E-4";                   //时间400us
char PWM_Pipeline_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char PWM_Pipeline_Data_Start[32] =   "DATa:START 1000000";              //start 1m
char PWM_Pipeline_Data_Stop [32] = "DATa:STOP 9000000";                 //Stop 9m


char Samper_Time[] ="HORIZONTAL:SCALE 4E-7";                   //时间400ns
char Samper_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char Samper_Data_Start[32] =   "DATa:START 5000";              //start 5k
char Samper_Data_Stop [32] = "DATa:STOP 7000";                 //Stop 7k

char NTT_Pipline_Time[] ="HORIZONTAL:SCALE 4E-7";                   //时间400ns
char NTT_Pipline_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char NTT_Pipline_Data_Start[32] =   "DATa:START 2000";              //start 2k
char NTT_Pipline_Data_Stop [32] = "DATa:STOP 7000";                 //Stop 7k


char NTT_TVLA_Time[] ="HORIZONTAL:SCALE 4E-5";                   //时间40us
char NTT_TVLA_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char NTT_TVLA_Data_Start[32] =   "DATa:START 100000";              //start 100k
char NTT_TVLA_Data_Stop [32] = "DATa:STOP 1000000";                 //Stop 1000k
const int number_data = 900000;//采样点个数，stop-start



char Dili_NTT_TVLA_Time[] ="HORIZONTAL:SCALE 4E-5";                   //时间40us
char Dili_NTT_TVLA_Trig_Mode[]   = "TRIGger:A:EDGE:SLOpe fAll";        //触发方式   fAll
char Dili_NTT_TVLA_Data_Start[32] =   "DATa:START 100000";              //start 100k
char Dili_NTT_TVLA_Data_Stop [32] = "DATa:STOP 600000";                 //Stop 600k
const int Dili_NTT_Sample_cnt = 500000;
const int NTT_Pipeline_capture_count = 1;  // NTT Pipeline取均值的分母


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
 *      NTT_Pipeline_Attack
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
    int Busy_cnt;
    int Data_locate;


    //******************** MDO配置 ************************//
    ViSession	rm = VI_NULL,vi = VI_NULL;
    ViUInt32	retCnt;
    char        BUSY[4] = {0,0,0,0};
    char Device[256] = "USB::0x0699::0x0456::C013077::INSTR";
    // Open a d efault Session
    viOpenDefaultRM(&rm);
    viOpen(rm, Device, VI_NULL, VI_NULL, &vi);

    //CONFIG
    viWrite(vi, (ViBuf)"ACQuire:MODe HIRes",18,&retCnt);                   //高精度模式
    viWrite(vi, (ViBuf) NTT_Pipline_Time, sizeof(NTT_Pipline_Time), &retCnt);  //时间刻度
    viWrite(vi, (ViBuf) Trig_Source, sizeof(Trig_Source), &retCnt);  //触发源
    viWrite(vi, (ViBuf) Trig_Vol, sizeof(Trig_Vol), &retCnt);        //触发电平
    viWrite(vi, (ViBuf) NTT_Pipline_Trig_Mode, sizeof(NTT_Pipline_Trig_Mode), &retCnt);      //触发方式

    viWrite(vi, (ViBuf) Read_CH, sizeof(Read_CH), &retCnt);         //读取通道
    viWrite(vi, (ViBuf) NTT_Pipline_Data_Start, sizeof(NTT_Pipline_Data_Start), &retCnt);   //起始
    viWrite(vi, (ViBuf) NTT_Pipline_Data_Stop, sizeof(NTT_Pipline_Data_Stop), &retCnt);
    viWrite(vi, (ViBuf) Data_Mode, sizeof(Data_Mode),&retCnt);      //数据模式


    viSetAttribute(vi, VI_ATTR_WR_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);
    viSetAttribute(vi, VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);//不用管 直接复制

    char str1[64] ="header off\n";
    char str2[64]="DATA:ENCDG RIBINARY;WIDTH 2\n"; //不用管
    viPrintf(vi, str1);
    viPrintf(vi, str2);


    QString a_Qstr;
    char u_str[10];
    const int fixed_b = 666;  // 固定的b值, ONLY DISPLAY
    for (int loop = 0; loop < 10 ; loop++) {
        QString filename = "D:\\ntt_pipeline_traces-loop"+QString::number(loop)+".txt" ;

        a_Qstr.toLatin1().data()// 打开文件用于写入所有曲线
        QFile f(filename);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << ("打开文件失败");
            return;
        }
        QTextStream txtOutput(&f);

        // 采集4096条曲线，a值从0到4095
        for(int a_value = 0; a_value < 3329; a_value++) {
            //清空Data_sum
            for(int k=0;k<5000;k++) Data_Sum[k] = 0;

            //测试循环 - 对同一个a值采集多次取平均
            for(int k=0;k<NTT_Pipeline_capture_count;k++) {
                viWrite(vi, (ViBuf) "ACQUIRE:STOPAFTER SEQUENCE", 26, &retCnt);  //Single
                BUSY[0] = '0';
                while(BUSY[0] != '1')
                {
                    viWrite(vi, (ViBuf) "BUSY?", 5, &retCnt);  //等待
                    viRead(vi, (ViBuf) BUSY, sizeof(BUSY), &retCnt);
                    Sleep(10);
                }
                BUSY[0] = '0';
                while(BUSY[0] != '1')
                {
                    viWrite(vi, (ViBuf) "BUSY?", 5, &retCnt);  //等待
                    viRead(vi, (ViBuf) BUSY, sizeof(BUSY), &retCnt);
                    Sleep(10);
                }

                Sleep(100);

                // 使用顺序的a值
                a_Qstr = QString::number(a_value, 10);
                strcpy(u_str, a_Qstr.toLatin1().data());

            send:
                mSocket->write("NTT_Pipe");
                mSocket->write(u_str);
                mSocket->waitForBytesWritten();
                //WAIT
                //BUSY[0] = '1';
                Busy_cnt = 0;
                while(BUSY[0] != '0')
                {
                    viWrite(vi, (ViBuf) "BUSY?", 5, &retCnt);  //等待
                    viRead(vi, (ViBuf) BUSY, sizeof(BUSY), &retCnt);
                    Sleep(10);
                    Busy_cnt ++;
                    if(Busy_cnt == 100) goto send;
                }
                viWrite(vi, (ViBuf) "ACQUIRE:STOPAFTER RUNSTop", 25, &retCnt);  //runstop

                //读取数据
                viWrite(vi, (ViBuf) "CURVE?", 6, &retCnt);  //获取数据
                viFlush(vi, VI_WRITE_BUF | VI_READ_BUF_DISCARD);
                viSetAttribute(vi,VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_DISABLE);
                viRead(vi, (ViBuf) Data_read, 5000*2+10, &retCnt);
                Data_locate = Data_read[1] - '0' + 2;
                for(int k=0;k<5000;k++)
                {
                    Data_Sum[k] = (Data_read[2*k+ 1 + Data_locate] |(int16_t)(Data_read[2*k+ Data_locate] << 8)) + Data_Sum[k];
                    //qDebug()<<Data_Sum[j];
                }
                viWrite(vi, (ViBuf) "ACQUIRE:STATE RUN", 17, &retCnt);  //run
            }

            // 计算平均值并写入文件，格式为: "a的十进制值: 采集值1 采集值2 采集值3..."
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
        qDebug() << "NTT Pipeline Attack completed. 4096 traces saved to ntt_pipeline_traces.txt";
    }
}

void MainWindow::Set_Data_range(int Data_Start,int Data_End)
{
    QString Data_Start_str = "DATa:START ";
    QString Data_End_str = "DATa:STOP ";
    Data_Start_str += QString::number(Data_Start,10);
    Data_End_str += QString::number(Data_End,10);

    strcpy(PWM_Template_Data_Start,Data_Start_str.toLatin1().data());
    strcpy(PWM_Template_Data_Stop,Data_End_str.toLatin1().data());
}

void MainWindow::client_dis()
{
    QTcpSocket *obj = (QTcpSocket*)sender();//掉线对象
    qDebug()<<obj->peerAddress().toString();//打印出掉线对象的ip
    Connected=0;
}

