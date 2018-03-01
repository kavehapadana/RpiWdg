#include "Lan_Conn.h"
#include "SPI_Conn.h"
#include "Slave_Card.h"
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include "Sin_array.h"

using namespace std;
int connfd;
pthread_mutex_t         Lan_mutex           = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t          Lan_condition_var   = PTHREAD_COND_INITIALIZER;
Message_Status_Type     msgStatus           = UINIT;

void transmitMessage_LAN(unsigned char Msg_ID, int msg_len, char out_msg[]);
///void *Thread_LAN(void*);
void Parse_Byte(unsigned char DATA);
void Parse_Message();
char msg[50], msgConf[50];
char byteCnt = 0;
char CheckSumm,msgID,msgLen,errCnt,msgIndex;

_uni16_8    DATA16to8;
_uni32_8    DATA32to8;
unsigned char PayloadLAN[60000];

void transmitMessage_LAN(unsigned char Msg_ID,  int  msg_len,unsigned char out_msg[])
{
    unsigned char ck_sum = 0;
    PayloadLAN[0] = LAN_Tr_Sync1;
    PayloadLAN[1] = LAN_Tr_Sync2;
    PayloadLAN[2] = Msg_ID;
    DATA32to8.u32 = msg_len;
    memcpy(&PayloadLAN[3],DATA32to8.u08,4);
    for(char cc =2; cc<7;cc++)
    ck_sum += PayloadLAN[cc];

    for(int j = 0; j < msg_len; j++)
    {
        PayloadLAN[j+7]=out_msg[j];
        ck_sum+= PayloadLAN[j+7];
    }

    PayloadLAN[msg_len+7] = ck_sum;
    send(connfd, PayloadLAN, (msg_len+8), 0);
}
char idx_Lan_Max;

pid_t pid;
bool reset_State = false;
void Parse_Message()
{
    switch (msgID)
    {
        case LAN_GET_CONF_RX_ID:
            pthread_mutex_lock(&spi_mutex);
            State_Config = 0;
            pthread_cond_signal(&spi_condition_var);
            pthread_mutex_unlock(&spi_mutex);
        break;

        case LAN_START_RAWDATA_RX_D:
            pthread_mutex_lock(&spi_mutex);
            State_Config = 1;
            pthread_cond_signal(&spi_condition_var);
            pthread_mutex_unlock(&spi_mutex);
        break;

        case LAN_SET_CONF_RX_ID:
            memcpy(msgConf, msg, sizeof(msg));
            set_Config(msg[0]);
        break;
        case    LAN_Hole_System_reset:

       /// //    bcm2835_gpio_write(GPIO_Reset_Micro, HIGH);
        ///    usleep(500);
        ///    bcm2835_gpio_write(GPIO_Reset_Micro, LOW);

        break;

        case  LAN_Slaves_Reset:
         //   bcm2835_gpio_write(GPIO_Reset_Micro, HIGH);
        //    usleep(500);
          //  bcm2835_gpio_write(GPIO_Reset_Micro, LOW);
        break;
        case    LAN_Raspi_Shotdown:
        system("sudo shutdown -h now");
        break;

        case  LAN_RAWDATA_Online_Start:

            pthread_mutex_lock(&spi_mutex);
            State_Config = 2;
            pthread_cond_signal(&spi_condition_var);
            pthread_mutex_unlock(&spi_mutex);

        break;

        case  LAN_RAWDATA_Online_STOP:
            pthread_mutex_lock(&spi_mutex);
            State_Config =3;
            pthread_cond_signal(&spi_condition_var);
            pthread_mutex_unlock(&spi_mutex);

        break;

    }
}

void Parse_Byte(char DATA)
{
    byteCnt++;
    switch (msgStatus)
    {
        case UINIT:
            if (DATA == LAN_Rx_Sync1)
                msgStatus = GOT_SYNC1;
            break;

        case GOT_SYNC1:
            if (DATA == LAN_Rx_Sync2)
            {
                msgStatus = GOT_SYNC2;
                CheckSumm = 0;
            }
            else
                goto error;
            break;

        case GOT_SYNC2:
            msgID = DATA;
            msgStatus = GOT_ID;
            CheckSumm+=DATA;
            break;

        case GOT_ID:
            msgLen = DATA;
            msgStatus = GOT_LEN;
            msgIndex = 0;
            CheckSumm += DATA;
            break;

        case GOT_LEN:
            msg[msgIndex] = DATA;
            CheckSumm +=DATA;
            msgIndex++;

            if (msgIndex >= msgLen)
            {
                msgStatus = GOT_PAYLOAD;
            }
          break;

        case GOT_PAYLOAD:
            if ( DATA != CheckSumm)
            {
                goto error;
                printf(" errro \n");
            }
            Parse_Message();
            goto restart;
    }
    return;
    error: errCnt++;
    restart: msgStatus = UINIT;
    return;
}
