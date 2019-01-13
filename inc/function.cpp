#include <stdio.h>
#include <uuid/uuid.h>
#include <random>
#include <time.h>
#include "../inc/log.h"


void GenerateGUID(char *pGuid)
{

	uuid_t guid;
	uuid_generate(guid);
	for(int i=0;i<16;i++)
	{
		sprintf((pGuid + i*2),"%02X",guid[i]);
		//sprintf(pGuid + i*2),"%02X",guid[i]);
	}
}

//Generate a random data between 0-999999
unsigned int GetRandomData()
{
	srand(static_cast<unsigned int>(time(NULL)));
	unsigned int datH = static_cast<unsigned int>(rand())%1000*1000;	
	unsigned int datL = static_cast<unsigned int>(rand())%1000;
	//LogD("datH= %d,datL= %d\n",datH,datL);
	return datH+datL;
}

unsigned int GetRandomData(unsigned int min, unsigned int max)
{
	static std::uniform_int_distribution<unsigned>  u(min, max);
	static std::default_random_engine e;
	return u(e);
}



