#include "FTDI_USB.h"


//************************************************************************************************************
//FTDI_USB

//FTDI_USB::FTDI_USB(): port(new FTDI_USB){}
FTDI_USB::FTDI_USB(){
    this->devInfo = new FT_DEVICE_LIST_INFO_NODE();
    this->ftdi_pHandle = this->devInfo->ftHandle;
}

//open()
FT_STATUS FTDI_USB::ftdi_open(int deviceNumber){
    //**********************************************************************
    FT_STATUS ftStatus;
    //**********************************************************************
    //FT_HANDLE ftHandle;
    DWORD numDevs;

    int local_baud_rate = 9600;
    // Create the device information list
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    // get the device information list
    ftStatus = FT_GetDeviceInfoList(this->devInfo,&numDevs);
    // Open user's selection.
    // Allocate storage for list based on numDevs.
    this->devInfo  = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
    this->ftdi_pHandle = this->devInfo->ftHandle;
    //ftStatus = FT_Open(deviceNumber, &this->devInfo[deviceNumber].ftHandle);
    ftStatus = FT_Open(deviceNumber, &this->ftdi_pHandle);
    //ftStatus = FT_Open(0,&ftHandle);
    if (ftStatus != FT_OK) {
        QMessageBox::warning(NULL,"Tip",QStringLiteral("Open失败"));
    }
    else {
        QMessageBox::warning(NULL,"Tip",QStringLiteral("Open成功"));
    }
    Sleep(50);
    FT_SetBaudRate(this->devInfo[deviceNumber].ftHandle, local_baud_rate);
    Sleep(50);
    ftStatus = FT_SetDataCharacteristics(this->ftdi_pHandle, FT_BITS_8, FT_STOP_BITS_2,FT_PARITY_EVEN);
    Sleep(50);

    //*********************************************************************

    if(ftStatus != FT_OK){
        qDebug() << "Open失败";
    }else{
        qDebug() << "Open成功";
    }
    /*
        ftHandle Handle of the device.
        dwReadTimeout Read timeout in milliseconds.
        dwWriteTimeout Write timeout in milliseconds.
    */
    FT_SetTimeouts(this->ftdi_pHandle,500,500);
    Sleep(50);
    return  ftStatus;
}

//close()
FT_STATUS FTDI_USB::ftdi_close(){
    FT_STATUS ftStatus = FT_OK;
    ftStatus = FT_Close(this->ftdi_pHandle);
    if(ftStatus != FT_OK){
        qDebug() << "Close失败";
    }else{
        qDebug() << "Close成功";
    }
    return ftStatus;
}

//write
FT_STATUS FTDI_USB::ftdi_write(byte *dat, int len){
    FT_STATUS ftStatus;
    byte* TxBuffer = new byte[len];
    DWORD BytesWritten = 0;//useless
    for(int i = 0; i < len; ++i){
        TxBuffer[i] = (char)dat[i];
    }
    ftStatus = FT_Write(this->ftdi_pHandle, TxBuffer, len, &BytesWritten);
    return ftStatus;
}

//read
FT_STATUS FTDI_USB::ftdi_read(byte *dat, int len){
    FT_STATUS ftStatus;
    DWORD RxBytes = len;
    byte RxBuffer[1024];
    DWORD BytesReceived = 0;
    ftStatus = FT_Read(this->ftdi_pHandle, RxBuffer, RxBytes, &BytesReceived);
    for(int i = 0; i < (int)BytesReceived; ++i){
        dat[i] = RxBuffer[i];
    }
    return ftStatus;
}

//************************************************************************************************************
//For Spartan-6
//DATA_INT::DATA_INT(): usb_port(new FTDI_USB){}
DATA_INT::DATA_INT(){
    this->usb_port = new FTDI_USB();
}


void DATA_INT::open(){
    this->usb_port->ftdi_open(0);
    //复位
    this->write(ADDR_CONT, 0x0004);
    this->write(ADDR_CONT, 0x0000);
}

void DATA_INT::close(){
    this->usb_port->ftdi_close();
}

void DATA_INT::writeText(byte *text, int len){
    this->writeBurst(ADDR_ITEXT0, text, len);
    this->write(ADDR_CONT, 0x0001);
    while(DATA_INT::read(ADDR_CONT) != 0){

    }
}

void DATA_INT::readText(byte *text, int len){
    this->readBurst(ADDR_OTEXT0, text, len);
}

void DATA_INT::write(int addr, int dat){
    byte* buf = new byte[5];
    buf[0] = (byte)0x01;
    buf[1] = (byte)((addr >> 8) & 0xFF);
    buf[2] = (byte)((addr) & 0xFF);
    buf[3] = (byte)((dat >> 8) & 0xFF);
    buf[4] = (byte)((dat) & 0xFF);
    this->usb_port->ftdi_write(buf, 5);
}

int DATA_INT::read(int addr)
{
  byte* buf = new byte[3];
  buf[0] = (byte)0x00;
  buf[1] = (byte)((addr >> 8) & 0xFF);
  buf[2] = (byte)((addr) & 0xFF);
  this->usb_port->ftdi_write(buf, 3);
  this->usb_port->ftdi_read(buf, 2);
  return ((int)buf[0] << 8) + (int)buf[1];
}

void DATA_INT::writeBurst(int addr, byte* dat, int len)
{
  byte* buf = new byte[5 * len / 2];
  for (int i = 0; i < len / 2; i++)
  {
    buf[i * 5 + 0] = (byte)0x01;
    buf[i * 5 + 1] = (byte)(((addr + i * 2) >> 8) & 0xFF);
    buf[i * 5 + 2] = (byte)(((addr + i * 2)) & 0xFF);
    buf[i * 5 + 3] = dat[i * 2];
    buf[i * 5 + 4] = dat[i * 2 + 1];
  }
  this->usb_port->ftdi_write(buf, 5 * (len / 2));
}

void DATA_INT::readBurst(int addr, byte* dat, int len)
{
  byte* buf = new byte[3 * len / 2];
  for (int i = 0; i < len / 2; i++)
  {
    buf[i * 3 + 0] = (byte)0x00;
    buf[i * 3 + 1] = (byte)(((addr + i * 2) >> 8) & 0xFF);
    buf[i * 3 + 2] = (byte)(((addr + i * 2)) & 0xFF);
  }
  this->usb_port->ftdi_write(buf, 3 * (len / 2));
  this->usb_port->ftdi_read(dat, len);
}


void DATA_INT::core_write_control(byte* all_data, int all_byte_len, int ram_sel, int initial_start)
//void DATA_INT::core_write_control(byte* all_data, int all_byte_len, int ram_sel)
{
    int pc_data_send = 0x0000;
    int group        = (int) ( (all_byte_len-2) >> 4 );//一组有16个byte，减1是因为硬件端从0计数
    int round        = 0;
    int data_last    = 0;
    int hw_addr_start= 0;
    int sw_addr_start= 0;
    byte* text       = new byte[16];
    int data_len     = 0;

    //发送前，先置无效
    this->write(WRITE_GROUP,0xFFFF);

    while(true){
        if(round == group){
          data_last    = 0x1;
          data_len     = (int)(all_byte_len - group*16)/2 - 1;//因为PC端是字节，而FPGA是16bits
        }else{
          data_last = 0x0;
          data_len  = 0x0007;
        }
        hw_addr_start = round;
        sw_addr_start = round * 16;
        pc_data_send  = this->encode(data_last, data_len, ram_sel, hw_addr_start);

        //PC向K7发送 数据格式：pc_data_send
        this->write(PC_DATA_SEND, pc_data_send);

        //PC发送第round批数据
          //关于Array.Copy函数：
              /*1. 源数组
              2. 源数组起始地址
              3. 目的数组
              4. 目的数组的起始地址
              5. 要复制的长度*/
        //Array.Copy(all_data, sw_addr_start, text, 0, 2*(data_len+1));
        this->Array_Copy(all_data, sw_addr_start, text, 0, 2*(data_len+1));
        this->writeBurst(ADDR_ITEXT0, text, 2*(data_len+1));
        //writeBurst(ADDR_ITEXT0, all_data, 16);

        //发送数据有效信号
        //Rising(WRITE_GROUP);
        this->write(WRITE_GROUP,this->Gen_group_sc(round));
        qDebug()<< "core_write_control ===> Start inner While()";

        while (this->read(CORE_DATA_REV) != pc_data_send) { /*nop*/
        //while (read(CORE_DATA_REV) != 0x0) { /*nop*/
          //K7已经接收完毕，数据无效 电平信号
          //必须置零，因为K7检测的是DATA_VLD[0]的上升沿

          //write(WRITE_GROUP,0x00);//我放在While循环里，是怕硬件采不到（想来应该是我多虑了）
        }
        /*if(round == 6){
            while(true);
        }*/
        qDebug() << "core_write_control ===> Over inner While()";
        qDebug() << "group is" << group;
        qDebug() << "round is" << round;
        qDebug() << "data_last is" << data_last;

        if(data_last != 0) break;
        ++ round;
    }
    qDebug() << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
}

int DATA_INT::encode(int data_last, int data_len, int ram_sel, int addr_start){
       //pc_data_last[0], pc_data_len[2:0], pc_data_ram_sel[4:0], pc_data_start[6:0]
       int data_send = ( ((data_last & 0x0001) << 15) |  ((data_len & 0x0007) << 12) |
                       ( (ram_sel & 0x001F) << 7) | (addr_start & 0x007F)) & 0xFFFF ;
       return data_send;
   }


int DATA_INT::core_read_control(byte* all_data)
{
    int len = 0;
    int data_len      = 16;
    //int read_group_sc = 0x0;
    int round         = 0;
    int data_rev      = 0x0;
    byte* text       = new byte[16];

    this->write(PC_DATA_REV, 0xFFFF);//PC先将pc_data_rev置为无效信号

    while (true){
        while ( this->read(READ_GROUP) != this->Gen_group_sc(round)) { /*nop*/
        //while ( read(READ_GROUP) != Gen_group_sc(0x00)) { /*nop*/
          //data_rev = read(CORE_DATA_SEND);
        }
        //System.Threading.Thread.Sleep(336);//335 | 336
         data_rev = this->read(CORE_DATA_SEND);
         //data_rev = read(CORE_DATA_SEND);
         //data_rev = read(CORE_DATA_SEND);
        // while(true){
        //     if(data_rev == 0xF001){
        //       return 0;
        //     }
        //     if(data_rev == 0x7000){
        //         break;
        //     }

        // }



        this->write(PC_DATA_REV,data_rev);//PC告诉K7：我已收到
        if( ((data_rev >> 15) & 0x01) != 0){
          data_len = 2 * (int)((data_rev >> 12) & 0x0007) + 2;//2*：FPGA发的是16bits，而PC接收的是byte
        }else{
          data_len = 16;
        }

        //数组拼接，摘自：https://www.techiedelight.com/zh/concatenate-two-arrays-in-csharp/
        this->readText(text, data_len);
        //将text复制到all_data的指定起始位置
        //Array.Copy (text, 0, all_data, 16*round, data_len);
        this->Array_Copy (text, 0, all_data, 16*round, data_len);

        //write(PC_DATA_REV,data_rev);//PC告诉K7：我已收到

        //write(PC_DATA_REV,0xF000);//PC告诉K7：我已收到
        //System.Threading.Thread.Sleep(1 * 3000);

        if( ((data_rev >> 15) & 0x01) != 0 ){
                    break;
        }


        /*while(true){
            write(PC_DATA_REV,0xF000);//PC告诉K7：我已收到
        }*/
        ++ round;
    }
    len = round * 16 + data_len;
    return len;
}


int DATA_INT::De_group_sc(int group_sc){
   int group = 0xFFFF;//无效状态
   if(group_sc == 0x0001)  group = 0   ;
   if(group_sc == 0x0003)  group = 1   ;
   if(group_sc == 0x0007)  group = 2   ;
   if(group_sc == 0x000F)  group = 3   ;
   if(group_sc == 0x001F)  group = 4   ;
   if(group_sc == 0x003F)  group = 5   ;
   if(group_sc == 0x007F)  group = 6   ;
   if(group_sc == 0x00FF)  group = 7   ;
   if(group_sc == 0x01FF)  group = 8   ;
   if(group_sc == 0x03FF)  group = 9   ;
   if(group_sc == 0x07FF)  group = 10  ;
   if(group_sc == 0x0FFF)  group = 11  ;
   if(group_sc == 0x1FFF)  group = 12  ;
   if(group_sc == 0x3FFF)  group = 13  ;
   if(group_sc == 0x7FFF)  group = 14  ;
   if(group_sc == 0xFFFF)  group = 15  ;
   return group;
}

int DATA_INT::Gen_group_sc(int round)
   {
       int gen_group_sc = 0x0000;
       if( round == 0   )   gen_group_sc = 0x0001  ;
       if( round == 1   )   gen_group_sc = 0x0003  ;
       if( round == 2   )   gen_group_sc = 0x0007  ;
       if( round == 3   )   gen_group_sc = 0x000F  ;
       if( round == 4   )   gen_group_sc = 0x001F  ;
       if( round == 5   )   gen_group_sc = 0x003F  ;
       if( round == 6   )   gen_group_sc = 0x007F  ;
       if( round == 7   )   gen_group_sc = 0x00FF  ;
       if( round == 8   )   gen_group_sc = 0x01FF  ;
       if( round == 9   )   gen_group_sc = 0x03FF  ;
       if( round == 10  )   gen_group_sc = 0x07FF  ;
       if( round == 11  )   gen_group_sc = 0x0FFF  ;
       if( round == 12  )   gen_group_sc = 0x1FFF  ;
       if( round == 13  )   gen_group_sc = 0x3FFF  ;
       if( round == 14  )   gen_group_sc = 0x7FFF  ;
       if( round == 15  )   gen_group_sc = 0xFFFF  ;
       return gen_group_sc;
   }
void DATA_INT::Rising(int addr){
//因为硬件那边是检测上升沿来判断复位，怕硬件检测不到，因此多发送几个0x0000
//事实证明我是对的，确实要多发几个，不然硬件收不到
    for(int i=0; i < 100; ++i){
        this->write(addr,0x0000);
    }

    for(int i=0; i < 100; ++i){
        this->write(addr,0x0001);
    }

    for(int i=0; i < 100; ++i){
        this->write(addr,0x0000);
    }
}

//拷贝数组
/*
    1. 源数组
    2. 源数组起始地址
    3. 目的数组
    4. 目的数组的起始地址
    5. 要复制的长度
*/
void DATA_INT::Array_Copy(byte* s_array, int s_start, byte* d_array, int d_start, int len){
    for(int i = 0; i < len; ++i){
        d_array[d_start + i] = s_array[s_start + i];
    }
}











