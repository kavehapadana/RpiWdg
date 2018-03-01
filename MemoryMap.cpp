
#include "MemoryMap.h"


table_struct MemMAP[NUM_MemMAP];

uint8_t typeRegMAP[NUM_MemMAP] = {1};//{1,1,1,1,1,1,1,1,1,1} ;

void Init_MemMAP()
{
	for(int i = 0 ; i< NUM_MemMAP; i++)
	{
		MemMAP[i].Type= typeRegMAP[i];
	}
}


