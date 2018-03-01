
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <math.h>

#include <stdio.h>
#include <ctime>
#include <sys/time.h>

#define	LAN_Tr_Sync1		                0xAE
#define	LAN_Tr_Sync2		                0xEA
#define	LAN_Rx_Sync1		                0xFA
#define	LAN_Rx_Sync2		                0xAF

#define     LAN_Conf_Tr_ID		            0x17
#define     LAN_RAWDATA_Tr_ID	            0x12
#define     LAN_GET_CONF_RX_ID              0x17
#define     LAN_SET_CONF_RX_ID              0xAE
#define     LAN_START_RAWDATA_RX_D          0x12
#define     LAN_STOP_RAWDATA_RX_ID          0xAC
#define     LAN_Raspi_Shotdown              0xA9
#define     LAN_Hole_System_reset           0xC9
#define     LAN_Slaves_Reset                0xB9
#define     LAN_Intro_get_Firsted           0x84

#define     LAN_RAWDATA_Online_Start        0x9E
#define     LAN_RAWDATA_Online_STOP         0x9F
#define     LAN_RAWDATA_Online_Tr_ID        0x9D

extern pthread_mutex_t Lan_mutex;
extern pthread_cond_t  Lan_condition_var;
extern void Parse_Byte( char DATA);
extern void Parse_Message();
///void *Thread_LAN(void* );
extern void transmitMessage_LAN(unsigned char Msg_ID,  int  msg_len,unsigned char out_msg[]);
extern int connfd;
extern char msg[50],msgConf[50];
extern bool reset_State;
