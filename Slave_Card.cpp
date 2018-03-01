
#include "Slave_Card.h"

Slave_Card::Slave_Card()
{
    frequency = 50000;
    ID = 0x00;
    Channel_No = 4;
    Active_Channel = 0x0F;
    Exist = true;
}

//Slave_Card::~Slave_Card(){}

void Slave_Card::get_Conf_MSG()
{
}

uint16_t Slave_Card::get_frequency()
{
    return frequency;
}

int Slave_Card::get_ID()
{
    return ID;
}

char Slave_Card::get_Channel_No()
{
    return Channel_No;
}

char Slave_Card::get_Active_Channel()
{
    return Active_Channel;
}

void Slave_Card::set_ID(int id)
{
    ID = id;
}

void Slave_Card::set_Channel_No(char ch_no)
{
    Channel_No = ch_no;
}

void Slave_Card::set_Active_Channel(char ac_ch)
{
    Active_Channel = ac_ch;
}
