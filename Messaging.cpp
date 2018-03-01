/**
  ******************************************************************************
  * @file    Messaging.cpp
  * @brief   parses and reports
  ******************************************************************************
  */

#include <string.h>
#include <stdio.h>
//#include "Timer.h"
#include "Messaging.h"
//#include "LED.h"
//#include "flash103_sdra.h"
//#include "sdraSerial_f10x.h"
#include "MemoryMap.h"
#include <unistd.h>



cSender SPI_Sender;
cParser SPI_Parser;
/**
  * @brief  Parses
  * @param  cParser::Message_t &newMsg		a Message_t var from cParser
  */

uint16_t cntPayload = 0;
void ParseMsg(cParser::Message_t &newMsg)
{
	uint8_t ID   = newMsg.ID;
	uint8_t FUN  = newMsg.FUN;
	uint16_t L   = newMsg.Length,ReadPayload = 0;
	uint8_t type = 0;
	typedef_regStruct  reg;
	typedef_valueStruct value;

	while (ReadPayload<L)
	{
		type = newMsg.Payload[ReadPayload];
		ReadPayload++;
		memcpy(&reg.c[0],&newMsg.Payload[ReadPayload],2);
		ReadPayload = ReadPayload+2;

		if(FUN == 0)  // receive request and prepare to send answer
		{
			switch(type)
			{
				case 1:
					value.U8 = MemMAP[reg.U16].Value.U8;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],1);
					ReadPayload++;
					break;
				case 2:
					value.I8 = MemMAP[reg.U16].Value.I8;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],1);
					ReadPayload++;
					break;
				case 3:
					value.U16 = MemMAP[reg.U16].Value.U16;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],2);
					ReadPayload = ReadPayload + 2;
					break;
				case 4:
					value.I16 = MemMAP[reg.U16].Value.I16;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],2);
					ReadPayload = ReadPayload + 2;
					break;
				case 5:
					value.U32 = MemMAP[reg.U16].Value.U32;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],4);
					ReadPayload = ReadPayload + 4;
					break;
				case 6:
					value.I32 = MemMAP[reg.U16].Value.I32 ;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],4);
					ReadPayload = ReadPayload + 4;
					break;
				case 7:
					value.f = MemMAP[reg.U16].Value.f;
					memcpy(&newMsg.Payload[ReadPayload],&value.c[0],4);
					ReadPayload = ReadPayload + 4;
					break;
			}

		}
		else if(FUN == 1)   // Receive Data store in MemMAP
		{
			switch(type)
			{
				case 1:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],1);
					MemMAP[reg.U16].Value.U8 = value.U8;
					ReadPayload++;
					break;
				case 2:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],1);
					MemMAP[reg.U16].Value.I8 = value.I8;
					ReadPayload++;
					break;
				case 3:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],2);
					MemMAP[reg.U16].Value.U16 = value.U16;
					ReadPayload = ReadPayload + 2;
					break;
				case 4:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],2);
					MemMAP[reg.U16].Value.I16 = value.I16;
					ReadPayload = ReadPayload + 2;
					break;
				case 5:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],4);
					MemMAP[reg.U16].Value.U32 = value.U32;
					ReadPayload = ReadPayload + 4;
					break;
				case 6:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],4);
					MemMAP[reg.U16].Value.I32 = value.I32;
					ReadPayload = ReadPayload + 4;
					break;
				case 7:
					memcpy(&value.c[0],&newMsg.Payload[ReadPayload],4);
					MemMAP[reg.U16].Value.f = value.f;
					ReadPayload = ReadPayload + 4;
					break;
			}
		}
	}

	if(FUN == 0)
	{
		SPI_Sender.SendMessage(ID,FUN,&newMsg.Payload[0],L);
	}


}

void write_one_register(uint8_t ID, uint8_t FUN, table_struct data){
    uint8_t p[100];//p[7 + 8];
    uint16_t length;
    p[0] = data.Type;
    uint8_t a=4;
    memcpy(&p[1], (uint8_t*)&data.Reg, 2);


        switch(data.Type)
			{
				case 1:
					memcpy(&p[3],&data.Value.c,1);
					length = 4;
					break;
				case 2:
					memcpy(&p[3],&data.Value.I8,1);
					length = 4;
					break;
				case 3:
					memcpy(&p[3],&data.Value.U16,2);
					length = 5;
					break;
				case 4:
					memcpy(&p[3],&data.Value.I16,2);
					length = 5;
					break;
				case 5:
					memcpy(&p[3],&data.Value.U32,4);
					length = 7;
					break;
				case 6:
					memcpy(&p[3],&data.Value.I32,4);
					length = 7;
					break;
				case 7:
					memcpy(&p[3],&data.Value.f,4);
					length = 7;
					break;
			}

        SPI_Sender.SendMessage(ID, FUN, p, length);
        usleep(10000);

        if (FUN == 0){

            SPI_Sender.ReceiveBytes(p, length + 8);  //   Sending dummy bytes and receiving data.
            //data.Reg =
            //data.Type =
            switch(data.Type)
			{
				case 1:
					memcpy(&data.Value.c, &p[9], 1);
					length = 4;
					break;
				case 2:
					memcpy(&data.Value.I8, &p[9], 1);
					length = 4;
					break;
				case 3:
					memcpy(&data.Value.U16, &p[9], 2);
					length = 5;
					break;
				case 4:
					memcpy(&data.Value.I16, &p[9], 2);
					length = 5;
					break;
				case 5:
					memcpy(&data.Value.U32, &p[9], 4);
					length = 7;
					break;
				case 6:
					memcpy(&data.Value.I32, &p[9], 4);
					length = 7;
					break;
				case 7:
					memcpy(&data.Value.f, &p[9], 4);
					length = 7;
					break;
			}
        }


}
