
#include <getopt.h>
#include <sys/ioctl.h>
#include "bcm2835.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <ctime>
#include <sys/time.h>
#include <queue>
//#include <bcm2835.h>


#include "Messaging.h"

#define	    SPI_RAWDATA_RID		    	    0xCD24
#define	    SPI_RAWDATA_Online		    	0x2020
#define	    SPI_GET_CONFIG_ID	            0x1410
#define     SPI_LIVECHECK_ID                0x5051
#define	    SPI_SET_CONFIG_ID	            0x1716
#define	    SPI_GET_CONF_Return_ID          0x2019
#define     SPI_GET_RAWDATA                 0xFEAB



#define    GPIO_Decoder_Enable              2
#define    GPIO_no_2                   2
#define    GPIO_no_3                   3
#define    GPIO_no_4    			   4
//#define    GPIO_init_Raw_data		  3
#define    RAWDATA_NO				  500000
#define    Mainbuf_NO				  500000

//#define    GPIO_Program_Workig_mo_set_now	     	    26
//#define    GPIO_TCP_Connection_mo_set_now			    19

#define     GPIO_Program_working                13
#define     GPIO_LAN_Connected                  6
#define     GPIO_DataLogging_bussy	            5
#define     GPIO_Program_Failed	    		    4
#define     GPIO_Reset_Micro                    21
#define     GPIO_Slave0_itr                     7
#define     GPIO_Slave1_itr                     8
#define     GPIO_Slave2_itr                     25
#define     GPIO_Slave3_itr                     24
#define     GPIO_Slave4_itr                     23
#define     GPIO_Slave5_itr                     18
#define     GPIO_Slave6_itr                     15
#define     GPIO_Slave7_itr                     14
#define     GPIO_handle_online                  20
#define     RAWDATA_Online_No                   4500

#define     Time_priof_RawData_Online           0.2

typedef union{
    uint8_t u08[2];
    uint16_t u16;
}   _uni16_8;

typedef union{
    uint8_t u08[4];
    int u32;
}   _uni32_8;

typedef enum{
    UINIT = 0,
    GOT_SYNC1,
    GOT_SYNC2,
    GOT_ID,
    GOT_LEN,
    GOT_PAYLOAD
}   Message_Status_Type;

extern char State_Config;
extern unsigned char mySlave_Buf_toPC[12], rxbuf[RAWDATA_NO], txbuf[RAWDATA_NO], Mainbuf[Mainbuf_NO];

extern unsigned char mySlave_Buf_toMicro[13];
extern pthread_mutex_t spi_mutex   ;
extern pthread_cond_t  spi_condition_var ;

extern pthread_mutex_t spiLAN_mutex   ;
extern pthread_cond_t  spiLAN_condition_var ;
extern void init_spi_bcm();
extern void* Thread_Spi(void*);
extern void* Thread_Prepare_GetRawData_Interrupt_fnc(void*);
extern void* Thread_rawData_Online(void*);

extern void set_Config(char Slave_no);
extern int  Lan_bufLenght;
extern void get_Introduce_Slaves_final();
extern void get_rawData_online();

extern bool raw_Data_online_flg;
//extern  void get_intro_Firsted();


/*** Raspberry Pins

slave   rasp_40_ports   GPIO_N
1           26              12
2           24              10
3           22              6
4           18              5
6           16              4
5           12              1
7           10              16
8           8               15


RPI_GPIO_P1_03        =  0,  !< Version 1, Pin P1-03
RPI_GPIO_P1_05        =  1,  !< Version 1, Pin P1-05
RPI_GPIO_P1_07        =  4,  !< Version 1, Pin P1-07
RPI_GPIO_P1_08        = 14,  !< Version 1, Pin P1-08, defaults to alt function 0 UART0_TXD
RPI_GPIO_P1_10        = 15,  !< Version 1, Pin P1-10, defaults to alt function 0 UART0_RXD
RPI_GPIO_P1_11        = 17,  !< Version 1, Pin P1-11
RPI_GPIO_P1_12        = 18,  !< Version 1, Pin P1-12, can be PWM channel 0 in ALT FUN 5
RPI_GPIO_P1_13        = 21,  !< Version 1, Pin P1-13
RPI_GPIO_P1_15        = 22,  !< Version 1, Pin P1-15
RPI_GPIO_P1_16        = 23,  !< Version 1, Pin P1-16
RPI_GPIO_P1_18        = 24,  !< Version 1, Pin P1-18
RPI_GPIO_P1_19        = 10,  !< Version 1, Pin P1-19, MOSI when SPI0 in use
RPI_GPIO_P1_21        =  9,  !< Version 1, Pin P1-21, MISO when SPI0 in use
RPI_GPIO_P1_22        = 25,  !< Version 1, Pin P1-22
RPI_GPIO_P1_23        = 11,  !< Version 1, Pin P1-23, CLK when SPI0 in use
RPI_GPIO_P1_24        =  8,  !< Version 1, Pin P1-24, CE0 when SPI0 in use
RPI_GPIO_P1_26        =  7,  !< Version 1, Pin P1-26, CE1 when SPI0 in use

 RPi Version 2
RPI_V2_GPIO_P1_03     =  2,  !< Version 2, Pin P1-03
RPI_V2_GPIO_P1_05     =  3,  !< Version 2, Pin P1-05
RPI_V2_GPIO_P1_07     =  4,  !< Version 2, Pin P1-07
RPI_V2_GPIO_P1_08     = 14,  !< Version 2, Pin P1-08, defaults to alt function 0 UART0_TXD
RPI_V2_GPIO_P1_10     = 15,  !< Version 2, Pin P1-10, defaults to alt function 0 UART0_RXD
RPI_V2_GPIO_P1_11     = 17,  !< Version 2, Pin P1-11
RPI_V2_GPIO_P1_12     = 18,  !< Version 2, Pin P1-12, can be PWM channel 0 in ALT FUN 5
RPI_V2_GPIO_P1_13     = 27,  !< Version 2, Pin P1-13
RPI_V2_GPIO_P1_15     = 22,  !< Version 2, Pin P1-15
RPI_V2_GPIO_P1_16     = 23,  !< Version 2, Pin P1-16
RPI_V2_GPIO_P1_18     = 24,  !< Version 2, Pin P1-18
RPI_V2_GPIO_P1_19     = 10,  !< Version 2, Pin P1-19, MOSI when SPI0 in use
RPI_V2_GPIO_P1_21     =  9,  !< Version 2, Pin P1-21, MISO when SPI0 in use
RPI_V2_GPIO_P1_22     = 25,  !< Version 2, Pin P1-22
RPI_V2_GPIO_P1_23     = 11,  !< Version 2, Pin P1-23, CLK when SPI0 in use
RPI_V2_GPIO_P1_24     =  8,  !< Version 2, Pin P1-24, CE0 when SPI0 in use
RPI_V2_GPIO_P1_26     =  7,  !< Version 2, Pin P1-26, CE1 when SPI0 in use

 RPi Version 2, new plug P5
RPI_V2_GPIO_P5_03     = 28,  !< Version 2, Pin P5-03
RPI_V2_GPIO_P5_04     = 29,  !< Version 2, Pin P5-04
RPI_V2_GPIO_P5_05     = 30,  !< Version 2, Pin P5-05
RPI_V2_GPIO_P5_06     = 31,  !< Version 2, Pin P5-06

 RPi B+ J8 header
RPI_BPLUS_GPIO_J8_03     =  2,  !< B+, Pin J8-03
RPI_BPLUS_GPIO_J8_05     =  3,  !< B+, Pin J8-05
RPI_BPLUS_GPIO_J8_07     =  4,  !< B+, Pin J8-07
RPI_BPLUS_GPIO_J8_08     = 14,  !< B+, Pin J8-08, defaults to alt function 0 UART0_TXD
RPI_BPLUS_GPIO_J8_10     = 15,  !< B+, Pin J8-10, defaults to alt function 0 UART0_RXD
RPI_BPLUS_GPIO_J8_11     = 17,  !< B+, Pin J8-11
RPI_BPLUS_GPIO_J8_12     = 18,  !< B+, Pin J8-12, can be PWM channel 0 in ALT FUN 5
RPI_BPLUS_GPIO_J8_13     = 27,  !< B+, Pin J8-13
RPI_BPLUS_GPIO_J8_15     = 22,  !< B+, Pin J8-15
RPI_BPLUS_GPIO_J8_16     = 23,  !< B+, Pin J8-16
RPI_BPLUS_GPIO_J8_18     = 24,  !< B+, Pin J8-18
RPI_BPLUS_GPIO_J8_19     = 10,  !< B+, Pin J8-19, MOSI when SPI0 in use
RPI_BPLUS_GPIO_J8_21     =  9,  !< B+, Pin J8-21, MISO when SPI0 in use
RPI_BPLUS_GPIO_J8_22     = 25,  !< B+, Pin J8-22
RPI_BPLUS_GPIO_J8_23     = 11,  !< B+, Pin J8-23, CLK when SPI0 in use
RPI_BPLUS_GPIO_J8_24     =  8,  !< B+, Pin J8-24, CE0 when SPI0 in use
RPI_BPLUS_GPIO_J8_26     =  7,  !< B+, Pin J8-26, CE1 when SPI0 in use
RPI_BPLUS_GPIO_J8_29     =  5,  !< B+, Pin J8-29,
RPI_BPLUS_GPIO_J8_31     =  6,  !< B+, Pin J8-31,
RPI_BPLUS_GPIO_J8_32     =  12, !< B+, Pin J8-32,
RPI_BPLUS_GPIO_J8_33     =  13, !< B+, Pin J8-33,
RPI_BPLUS_GPIO_J8_35     =  19, !< B+, Pin J8-35,
RPI_BPLUS_GPIO_J8_36     =  16, !< B+, Pin J8-36,
RPI_BPLUS_GPIO_J8_37     =  26, !< B+, Pin J8-37,
RPI_BPLUS_GPIO_J8_38     =  20, !< B+, Pin J8-38,
RPI_BPLUS_GPIO_J8_40     =  21  !< B+, Pin J8-40,
*/
