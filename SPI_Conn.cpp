#include "SPI_Conn.h"
#include "Lan_Conn.h"
#include "Slave_Card.h"
#include "Sin_array.h"


using namespace std;

unsigned char rxbuf[RAWDATA_NO], txbuf[RAWDATA_NO]; /// Bufer for Send & get Data SPI
unsigned char Mainbuf[Mainbuf_NO]; /// Main

pthread_mutex_t  spi_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   spi_condition_var   = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  itrr_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   itrr_condv     = PTHREAD_COND_INITIALIZER;

pthread_mutex_t  rawDataOnline_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   rawDataOnline_condv     = PTHREAD_COND_INITIALIZER;

char State_Config,RawData_Flag, Mainbuf_Cnt_RW_Differ;
int Lan_bufLenght;
_uni16_8	Data16to8;
_uni32_8	Data32to8;

pthread_t Thread_Online_Raw_Data_Cancelable_type,thread_GetRawData_Interrupt;

void select_Slave(char p);
void init_gpio();
void set_Config(char Slave_no);
void get_Conf_MSG(void);
void* Thread_Spi(void*);
void* Thread_Prepare_GetRawData_Interrupt_fnc(void*);
void* Thread_rawData_Online(void*);
void* Thread_rawData_Online_Cancelable(void*);
void init_spi_bcm();
void get_Introduce_Slaves();
void get_rawData_Multi();
void get_intro_Firsted();
void get_Introduce_Slaves_final();
void get_rawData_online();
void find_max_buff_size();
inline void interrupt_slave_func(char slv_gpioNumber,char slv_index);

short int spi_sendID(char sv_idx,short int Data16bit);
unsigned char* spi_transfer(char slv_idx,unsigned char* tx_array,unsigned char* rx_array,uint32_t lent);
//void spi_send(void* _p, unsigned char* tx_array, short unsigned int lent);
void spi_send(unsigned char* tx_array, short unsigned int lent);
void spi_receive(unsigned char* rx_array, short unsigned int lent);

Slave_Card *mySlave[8];
char count_intro = 0;

bool raw_Data_online_flg = false;
bool online_cmd_to_Slave = false;

struct timeval btime,etime;
bool Handle_Online_State = false;
void *tData;
void *Thread_Spi(void*)
{
    init_spi_bcm();
    printf("Thread Spi_init is runnig \n");
    gettimeofday(&btime,NULL);
    usleep(100000);
    gettimeofday(&etime,NULL);
    for(char i= 0;i<8;i++){
        mySlave[i] = new Slave_Card();}

    long double tim = etime.tv_usec - btime.tv_usec;

    printf("time : %8.0f \n" , tim);

    while(1)
    {
        pthread_mutex_lock(&spi_mutex);
        pthread_cond_wait(&spi_condition_var, &spi_mutex);
        switch(State_Config)
        {
            case 0:
                get_Introduce_Slaves_final();
            break;

            case 1:
                get_rawData_Multi();
            break;

            case 2:
            for(char slaveidx = 0; slaveidx <= 7; slaveidx++)
                {
                if(mySlave[slaveidx]->Exist & mySlave[slaveidx]->Activated_with_channel)
                    {
                    spi_sendID(slaveidx,SPI_RAWDATA_Online);
                    }
                }

            Handle_Online_State = true;
            bcm2835_gpio_write(GPIO_handle_online,     HIGH);
            pthread_create(&thread_GetRawData_Interrupt, NULL, &Thread_Prepare_GetRawData_Interrupt_fnc, NULL);
            usleep(200);
            pthread_create(&Thread_Online_Raw_Data_Cancelable_type,NULL,&Thread_rawData_Online_Cancelable,NULL);
            usleep(200);

            break;

            case 3:
            bcm2835_gpio_write(GPIO_handle_online,     LOW);
            Handle_Online_State = false;
            pthread_mutex_lock  (&itrr_mutex );
            pthread_cond_signal (&itrr_condv);
            pthread_mutex_unlock(&itrr_mutex);
            pthread_join(thread_GetRawData_Interrupt,&tData);
            cout << "Thread interrupt joined";
            pthread_join(Thread_Online_Raw_Data_Cancelable_type,&tData);
            cout << "Threads RawData joined";
            usleep(2000);
            break;
        }
        pthread_mutex_unlock(&spi_mutex);
    }
    usleep(100);
}

char pins_gpio_array[8] = {GPIO_Slave0_itr,GPIO_Slave1_itr,GPIO_Slave2_itr,
GPIO_Slave3_itr,GPIO_Slave4_itr,GPIO_Slave5_itr,GPIO_Slave6_itr,GPIO_Slave7_itr};

int32_t cnt_Intr_slv_array[8]={0,0,0,0,0,0,0,0};
std::queue<char> queue_slv_Intr;
int p_exit =100,p_exit2 = 100;

void* Thread_Prepare_GetRawData_Interrupt_fnc(void*){
        printf("the tread prepare running\n");
    while(Handle_Online_State){
            if(Handle_Online_State)
            {
            for(char i=0;i<3;i++)
                {
                  interrupt_slave_func(pins_gpio_array[i],i);
                }
            }
        }
    }

 void inline interrupt_slave_func(char slv_gpioNumber,char slv_index){

    if(bcm2835_gpio_lev(slv_gpioNumber))
        {
            cnt_Intr_slv_array[slv_index]++;
            if(cnt_Intr_slv_array[slv_index]==3)
                {
                    pthread_mutex_lock  (&itrr_mutex );
                    queue_slv_Intr.push (slv_index  );
                    pthread_cond_signal (&itrr_condv);
                    pthread_mutex_unlock(&itrr_mutex);
                }
        }
    else
        cnt_Intr_slv_array[slv_index] = 0;
}

unsigned char rxbuf_2[450000];
void* Thread_rawData_Online_Cancelable(void*){
    unsigned char Slave_activeChannel_no[8] = {0,0,0,0,0,0,0,0};
    char active_channel_no =0;

    for(char i=0; i < 8 ;i++)
    {
        if(mySlave[i]->Exist)
        {
            active_channel_no = 0;
            for (char mb = 0; mb < 4; mb++)
            {
                if((mySlave[i]->Active_Channel & (1<<mb)))
                    active_channel_no++;
            }
            Slave_activeChannel_no[i] = active_channel_no;
        }
    }
    usleep(100);

    while(Handle_Online_State)
    {
        if(Handle_Online_State)
        {
            pthread_mutex_lock(&itrr_mutex);
            if(queue_slv_Intr.size()==0)
            {
               printf("Queue is empty\n");
               pthread_cond_wait(&itrr_condv,&itrr_mutex);
            }
            unsigned char slv_in_queue = queue_slv_Intr.front();
            if(Handle_Online_State){
                queue_slv_Intr.pop();}
            pthread_mutex_unlock(&itrr_mutex);

            printf("number of slave =%d\n",slv_in_queue);

            int RawData_Slave_Online_No  = (mySlave[slv_in_queue]->frequency)*(Slave_activeChannel_no[slv_in_queue]*2)/20;
            rxbuf_2[0] = slv_in_queue;
            spi_transfer(slv_in_queue,rxbuf_2, &rxbuf_2[1], RawData_Slave_Online_No);
            transmitMessage_LAN(LAN_RAWDATA_Online_Tr_ID, (RawData_Slave_Online_No+1),rxbuf_2);
        }
        else
        {
            cout << "Befor exit consumer";
            Handle_Online_State = false;
           /// pthread_exit(&p_exit);
            cout << "after exit consumer";
        }
    }
    usleep(20);
}


unsigned char mySlave_Buf_toPC[12];
char Slave_index;
uint16_t u16Dummy;
short int GetConfig_Response;

void get_Introduce_Slaves_final(){

    for(Slave_index = 0; Slave_index <= 7; Slave_index++)
    {
        usleep(10000);
        u16Dummy = spi_sendID(Slave_index,SPI_LIVECHECK_ID);
        if(u16Dummy == SPI_GET_CONF_Return_ID)

        {
            usleep(100000);
            mySlave[Slave_index]->Exist = true;
            GetConfig_Response = spi_sendID(Slave_index,SPI_GET_CONFIG_ID);
            usleep(10000);
            spi_transfer(Slave_index,&txbuf[2], rxbuf, 11);
            mySlave_Buf_toPC[0] = Slave_index;
            memcpy(mySlave_Buf_toPC+1,rxbuf,11);
            memcpy((char *)(&mySlave[Slave_index]->ID),rxbuf,4); //  slave_ID
            mySlave[Slave_index]->Active_Channel = rxbuf[4]; // Active channel
            memcpy((char *)(&mySlave[Slave_index]->frequency),&rxbuf[5],2);    //frequency
            memcpy((char *)(&mySlave[Slave_index]->buf_Length),&rxbuf[7],4); //Lenght
            transmitMessage_LAN(LAN_Conf_Tr_ID, sizeof(mySlave_Buf_toPC), mySlave_Buf_toPC);
        }
        else
        {
            mySlave[Slave_index]->Exist = false;
        }
    }
}

char active_ch_no, buf_idx_Max = 0, Slv_buf[8];
int  buf_spi_max = 50000;
int  timspi, max_slv_RawDATA_time = 0;
uint32_t jj;

void find_max_buff_size()
{
    for(char d = 0; d <= 7; d++)
    {
        if(mySlave[d]->Exist)
        {
            jj = (((long double)(mySlave[d]->buf_Length))*1000000)/(mySlave[d]->frequency);
            if(jj > max_slv_RawDATA_time)
                max_slv_RawDATA_time = jj;
        }
    }
}

timeval  xtime,ytime;

void get_rawData_Multi()
{
    bcm2835_gpio_write(GPIO_DataLogging_bussy, HIGH);
    std::queue<char> queue_offLine_Slv;
    find_max_buff_size();
    for(char slaveidx = 0; slaveidx <= 7; slaveidx++){
        if(mySlave[slaveidx]->Exist & mySlave[slaveidx]->Activated_with_channel)
        {
            spi_sendID(slaveidx,SPI_RAWDATA_RID);
            queue_offLine_Slv.push(slaveidx);
        }
    }

    while(queue_offLine_Slv.size()){
            if(bcm2835_gpio_lev(pins_gpio_array[queue_offLine_Slv.front()]))
                {
                    queue_offLine_Slv.pop();
                }
        usleep(100);
    }

    for(char slaveidx = 0; slaveidx <= 7; slaveidx++)
    {
        if(mySlave[slaveidx]->Exist & mySlave[slaveidx]->Activated_with_channel)
        {
            usleep(100);

			active_ch_no = 0;
            for (char mb = 0; mb < 4; mb++)
            {
                if((mySlave[slaveidx]->Active_Channel & (1<<mb)))
                    active_ch_no++;
            }

            buf_idx_Max = (char) (((active_ch_no*mySlave[slaveidx]->buf_Length*2)-1)/buf_spi_max);
            buf_idx_Max++;
            if(gettimeofday(&xtime,NULL))
            printf("ERRORX");

            for(char buf_idx = 0; buf_idx < buf_idx_Max; buf_idx++){

                rxbuf[0] = slaveidx;

                spi_transfer(slaveidx,txbuf, &rxbuf[1], buf_spi_max);
                memcpy(&Mainbuf[1],rxbuf,(buf_spi_max));

                printf("buf_idx= %d,\t buf_idx_max = %d \n",buf_idx,buf_idx_Max);
                transmitMessage_LAN(LAN_RAWDATA_Tr_ID,(buf_spi_max + 1),rxbuf);

            }
            if(gettimeofday(&xtime,NULL))
            printf("ERRORY");
            long int time_hole  = ((ytime.tv_usec - xtime.tv_usec)/100000) ;
            printf("time-x=  %d \n",time_hole);

        }
    }
     bcm2835_gpio_write(GPIO_DataLogging_bussy, LOW);
}

char msg_idx ;
unsigned char mySlave_Buf_toMicro[13];

void set_Config(char Slv_idx)
{
    //select_Slave(Slv_idx);
    msg_idx = 0;
    u16Dummy =spi_sendID(Slv_idx,SPI_SET_CONFIG_ID);

    usleep(100);

        mySlave[Slv_idx]->Active_Channel = msgConf[1];
        mySlave_Buf_toMicro[0] = msgConf[1];
        if(msgConf[1] == 0x00){
            mySlave[Slv_idx]->Activated_with_channel = false;}
        else
        {
            mySlave[Slv_idx]->Activated_with_channel = true;
        }

    memcpy((char *) (&mySlave[Slv_idx]->frequency),&msgConf[2], 2);
    memcpy((char *) (&mySlave[Slv_idx]->buf_Length),&msgConf[4], 4);

    memcpy(mySlave_Buf_toMicro, msgConf+1, 7);
    spi_transfer(Slv_idx,mySlave_Buf_toMicro, rxbuf, 7);
    for(char r = 0; r < 7; r++)
        printf("Bufer to Micro Data: %d \t \n",mySlave_Buf_toMicro[r]);
}

void init_spi_bcm()
{
    if(!bcm2835_init())
    {
        cout << "Not init bcm";
    }
    init_gpio();

    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4096);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0,LOW);

    SPI_Sender.TransmitBytes = &spi_send;
    SPI_Sender.ReceiveBytes = &spi_receive;

    usleep(100);
}

void init_gpio()
{
    if(!bcm2835_init())
        printf(" init bcm_2835 not work \n");

    /*Pins for Active Slaves*/{
        bcm2835_gpio_fsel(GPIO_Decoder_Enable,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_no_2,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_no_3,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_no_4,BCM2835_GPIO_FSEL_OUTP);
    }

    /*Pins LED for alarm to User*/{
        bcm2835_gpio_fsel(GPIO_Program_working,  BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_LAN_Connected,    BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_DataLogging_bussy,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_Reset_Micro,      BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(GPIO_Program_Failed,   BCM2835_GPIO_FSEL_OUTP);


    /*Default for pins is the low*/
        bcm2835_gpio_write(GPIO_Program_working ,       LOW);
        bcm2835_gpio_write(GPIO_LAN_Connected   ,       LOW);
        bcm2835_gpio_write(GPIO_DataLogging_bussy,      LOW);
        bcm2835_gpio_write(GPIO_Reset_Micro     ,       LOW);
    /*interrupt Pins Rasp_Slave*/
        bcm2835_gpio_fsel(GPIO_Slave0_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave1_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave2_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave3_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave4_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave5_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave6_itr,      BCM2835_GPIO_FSEL_INPT);
        bcm2835_gpio_fsel(GPIO_Slave7_itr,      BCM2835_GPIO_FSEL_INPT);

        bcm2835_gpio_set_pud(GPIO_Slave0_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave1_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave2_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave3_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave4_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave5_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave6_itr,BCM2835_GPIO_PUD_DOWN);
        bcm2835_gpio_set_pud(GPIO_Slave7_itr,BCM2835_GPIO_PUD_DOWN);

        bcm2835_gpio_fsel (GPIO_handle_online,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(GPIO_handle_online,       LOW);

    }
}

void select_Slave(char p)
{
  //  bcm2835_gpio_write(GPIO_Decoder_Enable, LOW);
    switch(p)
    {
        case 0:
            bcm2835_gpio_write(GPIO_no_2, LOW);
            bcm2835_gpio_write(GPIO_no_3, LOW);
            bcm2835_gpio_write(GPIO_no_4, LOW);
        break;
        case 1:
            bcm2835_gpio_write(GPIO_no_2, HIGH);
            bcm2835_gpio_write(GPIO_no_3, LOW);
            bcm2835_gpio_write(GPIO_no_4, LOW);
        break;
        case 2:
            bcm2835_gpio_write(GPIO_no_2, LOW);
            bcm2835_gpio_write(GPIO_no_3, HIGH);
            bcm2835_gpio_write(GPIO_no_4, LOW);
        break;
        case 3:
            bcm2835_gpio_write(GPIO_no_2, HIGH);
            bcm2835_gpio_write(GPIO_no_3, HIGH);
            bcm2835_gpio_write(GPIO_no_4, LOW);
        break;
        case 4:
            bcm2835_gpio_write(GPIO_no_2, LOW);
            bcm2835_gpio_write(GPIO_no_3, LOW);
            bcm2835_gpio_write(GPIO_no_4, HIGH);
        break;
        case 5:
            bcm2835_gpio_write(GPIO_no_2, HIGH);
            bcm2835_gpio_write(GPIO_no_3, LOW);
            bcm2835_gpio_write(GPIO_no_4, HIGH);
        break;
        case 6:
            bcm2835_gpio_write(GPIO_no_2, LOW);
            bcm2835_gpio_write(GPIO_no_3, HIGH);
            bcm2835_gpio_write(GPIO_no_4, HIGH);
        break;
        case 7:
            bcm2835_gpio_write(GPIO_no_2, HIGH);
            bcm2835_gpio_write(GPIO_no_3, HIGH);
            bcm2835_gpio_write(GPIO_no_4, HIGH);
        break;
    }
 //  bcm2835_gpio_write(GPIO_Decoder_Enable, HIGH);
}

short int spi_sendID(char sv_idx,short int Data16bit){  /// This function use for send ID_CMD
    _uni16_8 Internal_Data16to8;
    short int out_data;
    Internal_Data16to8.u16 = Data16bit;
    unsigned char internal_array_tx[2],internal_array_rx[2];
    internal_array_tx[0] = Internal_Data16to8.u08[0];
    internal_array_tx[1] = Internal_Data16to8.u08[1];
    //bcm2835_spi_transfernb(internal_array_tx, internal_array_rx, 2);
    spi_transfer(sv_idx,internal_array_tx, internal_array_rx, 2);
    Internal_Data16to8.u08[0] = internal_array_rx[0];
    Internal_Data16to8.u08[1] = internal_array_rx[1];
    out_data = Internal_Data16to8.u16;
    return out_data;
}

unsigned char* spi_transfer(char slv_idx, unsigned char* tx_array, unsigned char* rx_array, uint32_t lent){
    select_Slave(slv_idx);
    bcm2835_spi_transfernb((char*)tx_array,(char*)rx_array,lent);
    bcm2835_gpio_write(GPIO_Decoder_Enable, LOW);
    return  rx_array;
}

//void spi_send(void* _p, unsigned char* tx_array, short unsigned int lent){
void spi_send(unsigned char* tx_array, short unsigned int lent){
    uint8_t rx_array_i[lent];
    spi_transfer(0x01, tx_array, rx_array_i, lent);
}

void spi_receive(unsigned char* rx_array, short unsigned int lent){
    unsigned char* tx_array = new unsigned char [lent ];
    //unsigned char* rx_array = new unsigned char [lent];;
    memset(tx_array, 0x55, lent);
    memset(rx_array, 0xaa, lent);
    //memset(rx_array, 0, lent);
    //rx_array = spi_transfer(0x01, tx_array, rx_array, lent+8);
    for (int i = 0; i<lent; i++){
        spi_transfer(0x01, &tx_array[i], &rx_array[i], 1);
        usleep(100);
    }
    delete[] tx_array;
    //return rx_array;
}




