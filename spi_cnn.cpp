#include "spi_cnn.h"
#include <CommonDefines.h>

#ifdef SPI_Raspberri
#include <bcm2835.h>
#include <CommIf_SiemensST.h>

#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "SPI_Conn.h"
#include "Lan_Conn.h"
#include <ctime>
#include <time.h>
#include "bcm2835.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include "Sin_array.h"


#include "Messaging.h"
#include "MemoryMap.h"
#endif
SPI_Cnn::SPI_Cnn(QObject *parent) :
    QObject(parent)
{
}

void testSpi()
{



//    while(1){
//        cout << "------" <<endl;
//        data2send.Value.U8 = 0x16;
//        write_one_register(0x12, 1,data2send);
//        data2send.Value.U8 = 0x0;
//        write_one_register(0x12, 1,data2send);
//        data2send.Value.U8 = 0x8;
//        write_one_register(0x12, 1,data2send);
//        data2send.Value.U8 = 0x0;
//        write_one_register(0x12, 1,data2send);
//        //SPI_Sender.SendMessage(0x12, 0x01, p, sizeof(myobject));
//        cout << "======" <<endl;

//    }
}

void SPI_Cnn::SPI_RPI_Init()
{
#ifdef SPI_Raspberri
    init_spi_bcm();
//    if(!bcm2835_init())
//    {
//        qDebug() << "Not init bcm";
//    }
//    //init_gpio();

//    bcm2835_spi_begin();
//    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
//    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
//    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
//    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
//    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0,LOW);
//    QThread::msleep(100);
#endif
}

int nn = 0;
void SPI_Cnn::rxSPI_DataSlot()
{
#ifdef SPI_Raspberri
    table_struct data2send;
    data2send.Type = 1;
    data2send.Reg = 1;
    data2send.Value.U8 = bitRelaysReset;

    write_one_register(0x12, 1,data2send);
    QThread::msleep(2000);
    data2send.Value.U8 = 0x00;
    write_one_register(0x12, 1,data2send);

    if(false)
    {
            nn++;
            qDebug() << "nn = " << nn << "\n";
            switch(nn%5)
            {
            case 0:
                data2send.Value.U8 = 0x0;
                write_one_register(0x12, 1,data2send);
                data2send.Value.U8 = 0x16;
                write_one_register(0x12, 1,data2send);
                qDebug() << "case 0 ---> \n ";
                break;
            case 1:
                data2send.Value.U8 = 0x0;
                write_one_register(0x12, 1,data2send);
                data2send.Value.U8 = 0x08;
                write_one_register(0x12, 1,data2send);
                qDebug() << "case 1 ---> \n ";
                break;
            case 2:
                data2send.Value.U8 = 0x0;
                write_one_register(0x12, 1,data2send);
                data2send.Value.U8 = 0x04;
                write_one_register(0x12, 1,data2send);
                qDebug() << "case 2 ---> \n ";
                break;
            case 3:
                data2send.Value.U8 = 0x0;
                write_one_register(0x12, 1,data2send);
                data2send.Value.U8 = 0x02;
                write_one_register(0x12, 1,data2send);
                qDebug() << "case 3 ---> \n ";
                break;
            case 4:
                data2send.Value.U8 = 0x0;
                write_one_register(0x12, 1,data2send);
                data2send.Value.U8 = 0x01;
                write_one_register(0x12, 1,data2send);
                qDebug() << "case 4 ---> \n ";
                break;
            }
        }
//    uint8_t tx_data[1] = {0x55};
//    uint8_t rx_data[1] = {0x00};

//    cSender SPI_Sender;
//    cParser SPI_Parser;

//    SPI_Sender.TransmitBytes = &spiSendCnt;

//    SPI_Sender.SendMessage(0x12, 0x01, tx_data, sizeof(tx_data));
#endif
}

unsigned char* SPI_Cnn::spi_ExchangeData(unsigned char* tx_array,unsigned char* rx_array,u_int32_t lent)
{

#ifdef SPI_Raspberri
    bcm2835_spi_transfernb((char*)tx_array,(char*)rx_array,lent);
#endif
    return  rx_array;
}
