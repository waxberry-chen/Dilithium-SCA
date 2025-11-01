#ifndef CONTROL_H
#define CONTROL_H

#include <QTime>
#include <QRandomGenerator>
#include <QDebug>
#include <QMessageBox>


#include "FTDI_USB.h"
//************************************************************************************************************
//Main Control

class HW_CONTROL{
public:
    explicit HW_CONTROL();
    DATA_INT* pData_In;
public:
    int num_trace;
    //byte* key;
    const int BYTE_LEN = 16;
    const int BYTELEN_TEST = 256;
    const int LOOP = 1;//记得更改循环次数
    bool main_loop();

    byte* text_in ;
    byte* text_out;
    byte* text_ans;
};



#endif // CONTROL_H
