#ifndef FTDI_USB_H
#define FTDI_USB_H

#include <windows.h>

#include "ftdi/ftd2xx.h"
#include <QString>
#include <QDebug>
#include <QMessageBox>

/*namespace FTDI {
    //class FTDI_USB;//不能加，加了会报错
    //class DATA_INT;
}*/


class FTDI_USB{
//调用前必须先执行ftdi_open,因为open后才会有ftdi_pHandle,这个指针是其他函数的基础

public:
    explicit FTDI_USB();
private:
    //FTDI_USB *port;
    FT_DEVICE_LIST_INFO_NODE *devInfo;
    FT_HANDLE ftdi_pHandle;
//public:
//    FT_HANDLE* ftdi_pHandle;

public:
    //------------------------------------------------ open()
    //FT_STATUS ftdi_open(int deviceNumber, FT_HANDLE* pHandle);
    FT_STATUS ftdi_open(int deviceNumber);
    //------------------------------------------------ close()
    //FT_STATUS ftdi_close(FT_HANDLE pHandle);
    FT_STATUS ftdi_close();
    //------------------------------------------------ write()
    //FT_STATUS write(byte[] dat, int len);
    FT_STATUS ftdi_write(byte* dat, int len);
    //------------------------------------------------ read()
    //FT_STATUS read(byte[] dat, int len);
    FT_STATUS ftdi_read(byte* dat, int len);
};

//************************************************************************************************************
//For Spartan-6
//data interaction
class DATA_INT{
public:
    explicit DATA_INT();
    FTDI_USB *usb_port;
public:
    //************************************************ Variable
    // Localbus address of AIST LSI Version 1
    int ADDR_CONT    = 0x0002;
    int ADDR_IPSEL   = 0x0004;
    int ADDR_OUTSEL  = 0x0008;
    //-----------------------------------------------
    //ys_debug begin
    int ADDR_MODE    = 0x000C;
    //public int ADDR_RANGE    = 0x000C;
    //ys_debug end
    //-----------------------------------------------
    int ADDR_RSEL    = 0x000E;
    int ADDR_KEY0    = 0x0100;
    int ADDR_ITEXT0  = 0x0140;
    int ADDR_OTEXT0  = 0x0180;
    int ADDR_VERSION = 0xFFFC;
    //---------------------------------------------------------
    //ys_debug begin
    int CORE_MAIN_CTR    = 0x0010;//ys added

    int CORE_DATA_REV    = 0x0012;//K7向PC发送接收完信号，PC由该地址确认K7是否接收完毕
    int CORE_DATA_SEND   = 0x0014;//K7向PC发送的数据格式
    int PC_DATA_REV      = 0x0016;//PC在接收完比来自K7的数据后，PC向K7发送：确认收到
    int PC_DATA_SEND     = 0x0018;//PC向K7发送数据格式
    int READ_GROUP       = 0x001A;//K7向PC最多可以发16组，因为read_group是[3:0]
    int WRITE_GROUP      = 0x001C;//第0位表示PC向K7发送数据有效
    int CMD_CTR          = 0x001E;//向硬件发送复位信号,其中第0位为高时表示复位
    //ys_debug end
    //---------------------------------------------------------

    // Cipher mode
    int MODE_ENC = 0x0000;
    int MODE_DEC = 0x0001;
    void open();
    void close();
    void writeText(byte* text, int len);
    void readText(byte* text, int len);
    void write(int addr, int dat);
    int  read(int addr);
    void writeBurst(int addr, byte* dat, int len);
    void readBurst(int addr, byte* dat, int len);
    void core_write_control(byte* all_data, int all_byte_len, int ram_sel, int initial_start);
    //void core_write_control(byte* all_data, int all_byte_len, int ram_sel);
    int encode(int data_last, int data_len, int ram_sel, int addr_start);
    int data_encode(int data_last, int data_len, int ram_sel, int addr_start);
    int core_read_control(byte* all_data);
    int De_group_sc(int group_sc);
    int Gen_group_sc(int round);
    void Rising(int addr);

    void Array_Copy(byte* s_array, int s_start, byte* d_array, int d_start, int len);
};

#endif // FTDI_USB_H
