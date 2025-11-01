#include "control.h"


HW_CONTROL::HW_CONTROL(){
    this->pData_In = new DATA_INT();
    this->text_in  = new byte [this->BYTELEN_TEST];
    this->text_out = new byte [this->BYTELEN_TEST];
    this->text_ans = new byte [this->BYTELEN_TEST];
}

bool HW_CONTROL::main_loop(){
    qDebug()<< "main loop Enter...";
    //这里没有seKey,所以会不会影响硬件的复位呢?

    this->pData_In->Rising(0x001E);
    for(int loop = 0; loop < this->LOOP; loop++){
        //产生随机序列
        for (int i = 0; i < this->BYTELEN_TEST; i++) {
            //qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
            //this->text_in[i] = (byte)qrand() % (256);//产生0到255的随机数
            this->text_in[i] = byte(i);
        }

        //hw.core()
        this->pData_In->core_write_control(this->text_in, this->BYTELEN_TEST, 0, 0);
        this->pData_In->core_read_control(this->text_out);
    }
    qDebug()<< "main loop End";
    QMessageBox::warning(NULL,"Tip",QStringLiteral("Loop Over"));


    return true;
}


