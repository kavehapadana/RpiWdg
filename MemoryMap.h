#ifndef _MemoryMap_H_
#define _MemoryMap_H_

//#include "stm32f10x.h"
#include <stdint.h>
#include <string.h>


/*
 ----------------------
| Name Of Value | Type |
 ----------------------
|     U8        |   1  |
 ----------------------
|     I8        |   2  |
 ----------------------
|     U16       |   3  |
 ----------------------
|     I16       |   4  |
 ----------------------
|     U32       |   5  |
 ----------------------
|     I32       |   6  |
 ----------------------
|     f         |   7  |
 ----------------------

*/

#define NUM_MemMAP 1//50

typedef struct
{
	uint8_t  Type;
	uint16_t Reg;
	union
	{
		uint32_t U32;
		uint32_t I32;
		float    f;
		uint16_t U16,ReserveU16;
		int16_t  I16,ReserveI16;
		uint8_t  U8,ReserveU8[3];
		int8_t  I8,ReserveI8[3];
		uint8_t	c[4];
	}Value;


}table_struct;

typedef union
{
		uint32_t U32;
		uint32_t I32;
		float    f;
		uint16_t U16,ReserveU16;
		int16_t  I16,ReserveI16;
		uint8_t  U8,ReserveU8[3];
		int8_t  I8,ReserveI8[3];
		uint8_t	c[4];
}typedef_valueStruct;

typedef union
{
	uint16_t  U16;
	uint8_t	c[2];
}typedef_regStruct;

extern table_struct MemMAP[NUM_MemMAP];

extern void Init_MemMAP();

#endif
