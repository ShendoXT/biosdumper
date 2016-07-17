//Simple CRC-32 implementation
//Based on the public domain examples by Damien Guard

#include "include/crc32.h"

void CreateTable()
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int entry = 0;

	for(i = 0; i < 256; i++)
	{
		entry = i;
		for (j = 0; j < 8; j++)
		{
			if ((entry & 1) == 1)
				entry = (entry >> 1) ^ Polynomial;
			else
				entry = entry >> 1;
		}

		CRC32Table[i] = entry;
	}
}

unsigned int CalculateCRC32(unsigned char *buffer, int start, int size)
{
	unsigned int CRC = Seed;
	int i = 0;

	for(i = start; i < size; i++)
	{
		CRC = (CRC >> 8) ^ CRC32Table[buffer[i] ^ CRC & 0xff];
	}

	return CRC ^ Seed;
}
