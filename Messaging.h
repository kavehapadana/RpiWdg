#ifndef _COM_INTERFACE_H_
#define _COM_INTERFACE_H_

//#include "stm32f10x.h"
//#include "sdraSerial_f10x.h"
#include "CommIf_SiemensST.h"
#include "MemoryMap.h"



void ParseMsg(cParser::Message_t &newMsg);
void write_one_register(uint8_t ID, uint8_t FUN,table_struct data);

extern cSender SPI_Sender;
extern cParser SPI_Parser;

//extern SerialPort ReportPort;
#endif
