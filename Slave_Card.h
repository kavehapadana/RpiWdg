
#include "stdint.h"

class Slave_Card
{

public:
    uint16_t frequency;
    int ID;
    char Channel_No;
    char Active_Channel;
    bool Exist = false;
    bool Activated_with_channel = true;

    unsigned int buf_Length;

    Slave_Card(int freq,int id, char ch_no,char active_ch);
    Slave_Card();

    void get_Conf_MSG();
     uint16_t get_frequency();
    int get_ID();
    char get_Channel_No();
    char get_Active_Channel();
    void set_frequency(int freq);
    void set_ID(int id);
    void set_Channel_No(char ch_no);
    void set_Active_Channel(char ac_ch);

};
