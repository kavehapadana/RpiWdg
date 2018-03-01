#ifndef _COMMIF_H_
#define _COMMIF_H_

#include <stdint.h>
#include <string.h>
#include <iostream>

#define MSG_PAYLOAD_SIZE	255
#define SYNC1_DEFAULT		0xFA
#define SYNC2_DEFAULT		0x70

class cParser
{
	// Definitions
private: typedef enum
	{
		UNINIT = 0, GOT_SYNC1, GOT_SYNC2, GOT_ID, GOT_FUN, GOT_LEN1, GOT_LEN2, GOT_PAYLOAD, GOT_CHKSUM1
	}	MessageStatus_t;

public: typedef struct
	{
		uint8_t ID;
		uint8_t FUN;
		uint16_t Length;
		uint8_t Payload[MSG_PAYLOAD_SIZE];
	}	Message_t;

public: typedef  void  (*OnPayloadReceived)(Message_t &message);
public: typedef  void  (*OnFramingError)(void);
public: OnPayloadReceived   ParsePayload;
public: OnFramingError      FramingErrorEvent;

public: uint8_t SYNC1;
public: uint8_t SYNC2;

public: uint8_t ActiveCommIf;

	// Properties

private: MessageStatus_t msgStatus;
private: Message_t  msg;
private: uint32_t   msgIdx;
private: uint16_t   msgCheckSum;
private: uint16_t   CheckSum;

private: uint32_t   cntFrame, cntByte, cntError;
private: void* parent;

	// Methods
public: cParser()
	{
		SYNC1 = SYNC1_DEFAULT;
		SYNC2 = SYNC2_DEFAULT;
		reset();
	}

public: cParser(void* _p)
	: parent(_p)
	{
		SYNC1 = SYNC1_DEFAULT;
		SYNC2 = SYNC2_DEFAULT;
		reset();
	}


private: void reset()
	{
		msgStatus = UNINIT;
		msg.ID = 0;
		msg.FUN = 0;
		msg.Length = 0;
		msgIdx = 0;
		CheckSum = 0;
		cntFrame = cntByte = cntError = 0;
	}

public: void SetSync(uint8_t s1, uint8_t s2)
	{
		SYNC1 = s1;
		SYNC2 = s2;
		reset();
	}

public: void ParseByte(uint8_t newByte)
	{
		cntByte++;
		if (msgStatus < GOT_PAYLOAD)
			CheckSum += newByte;

		switch (msgStatus)
		{
		case UNINIT:
			if (newByte == SYNC1)
				msgStatus = GOT_SYNC1;
			break;

		case GOT_SYNC1:
			if (newByte != SYNC2)
				goto error;

			reset();
			msgStatus = GOT_SYNC2;
			break;

		case GOT_SYNC2:
			msg.ID = newByte;
			msgStatus = GOT_ID;
			break;

		case GOT_ID:
			msg.FUN = newByte;
			msgStatus = GOT_FUN;
			break;

		case GOT_FUN:
			msg.Length = newByte;
			msgStatus = GOT_LEN1;
			break;

		case GOT_LEN1:
			msg.Length |= (newByte << 8);

			if (msg.Length == 0)
				msgStatus = GOT_PAYLOAD;
			else
				msgStatus = GOT_LEN2;
			break;

		case GOT_LEN2:

			msg.Payload[msgIdx++] = newByte;

			if (msgIdx == msg.Length)
			{
				msgStatus = GOT_PAYLOAD;
			}
			break;

		case GOT_PAYLOAD:
			msgCheckSum = newByte;
			msgStatus = GOT_CHKSUM1;
			break;

		case GOT_CHKSUM1:
			msgCheckSum |= (newByte << 8);

			if (msgCheckSum == CheckSum)
			{
				cntFrame++;
				if (ParsePayload != NULL)
					ParsePayload(msg);

				goto restart;
			}
			else
				goto error;
		}

		return;
		error:
			cntError++;
			if (FramingErrorEvent != NULL)
				FramingErrorEvent();
		restart:
			msgStatus = UNINIT;
	}
};


class cSender
{
	// Definitions
//public: typedef  void  (*OnTransmitBytes)(void* _p, uint8_t *buffer, uint16_t length);
public: typedef  void  (*OnTransmitBytes)(uint8_t *buffer, uint16_t length);
public: typedef  void (*OnReceiveBytes)(uint8_t *buffer, uint16_t length);

public: OnTransmitBytes TransmitBytes;
public: OnReceiveBytes ReceiveBytes;

	// Properties
public: uint8_t SYNC1;
public: uint8_t SYNC2;

private: uint8_t buffer[MSG_PAYLOAD_SIZE];
private: void* parent;

	// Methods
public: cSender()
	{
		reset();
	}
public: void SetParent(void* _p)
	{
		parent = _p;
	}

private: void reset(void)
	{
		SYNC1 = SYNC1_DEFAULT;
		SYNC2 = SYNC2_DEFAULT;
		TransmitBytes = NULL;
		parent = NULL;
	}

public: void SetSync(uint8_t s1, uint8_t s2)
	{
		SYNC1 = s1;
		SYNC2 = s2;
		reset();
	}

public: uint8_t* SendMessage(uint8_t ID, uint8_t FUN, uint8_t* payload, uint16_t length)
	{
		buffer[0] = SYNC1;
		buffer[1] = SYNC2;
		buffer[2] = ID;
		buffer[3] = FUN;
		*(uint16_t*)(buffer + 4) = length;
		memcpy(buffer + 6, payload, length);
		uint16_t checkSum = summation(buffer + 2, length + 4);
		memcpy(buffer + length + 6, &checkSum, 2);

		//if (parent != NULL)
			if (TransmitBytes != NULL)
				TransmitBytes( buffer, length + 8);
                //TransmitBytes(parent, buffer, length + 8);
//        for (int i=0;i < length + 8; i++ ){
//            printf("%d \n",p[i]);
//		}
		return buffer;

	}
public: void puts(char* s)
	{
		int length = 0;
		while (s[length])  length++;

		if (TransmitBytes != NULL)
			//TransmitBytes(parent, (uint8_t*)s, length);
			TransmitBytes( (uint8_t*)s, length);
	}

public: void gets(char* s)
	{
		int length = 0;
		while (s[length])  length++;

		if (TransmitBytes != NULL)
			//TransmitBytes(parent, (uint8_t*)s, length);
			TransmitBytes( (uint8_t*)s, length);
	}

private: uint16_t summation(uint8_t* buffer, uint16_t length)
	{
		uint16_t bufferSum = 0;

		for (int bufferIdx = 0; bufferIdx < length; bufferIdx++)
			bufferSum += *(buffer + bufferIdx);

		return bufferSum;
	}
};

#endif
