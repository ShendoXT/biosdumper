/*
 * SIO communication library for PSXSDK.
 * Shendo 2013/07/28.
 * Thanks to Martin Korth of the NO$PSX for documentation.
 * 
 * This library is accessing SIO registers directly, no BIOS routines are used.
 */

#include <psx.h>
#include <stdio.h>
#include "include/psxsio.h"

void StartSIO(int bitrate)
{
	/*Set to 8N1 mode with desired bitrate*/
	StartSIOEx(bitrate, SIO_DATA_LEN_8, SIO_PARITY_NONE, SIO_STOP_BIT_1);
}

void StartSIOEx(int bitrate, int datalenght, int parity, int stopbit)
{
	/*Set SIO_MODE register, bitrate reload factor set to MUL16 by default*/
	*SIO_MODE = SIO_REL_MUL16 | (datalenght << 2) | (parity << 4) | (stopbit << 6);

	/*Reset SIO_CTRL register.*/
	*SIO_CTRL = 0;

	/*Set TX and RT to enabled, no handshaking signals.*/
	*SIO_CTRL = 1 | (1 << 2);

	/*Calculate bitrate reload value based on the given bitrate
	 * Reload = SystemClock (33 Mhz) / (Factor (MULI16) * bitrate)*/
	*SIO_BPSV = 0x204CC00 / (16 * bitrate);
}

void StopSIO()
{
	/*Set all SIO related registers to zero*/
	*SIO_MODE = 0;
	*SIO_CTRL = 0;
	*SIO_BPSV = 0;
}

unsigned char ReadByteSIO()
{
	return *(unsigned char*)SIO_TX_RX;
}

void SendByteSIO(unsigned char data)
{
	*SIO_TX_RX = data;
}

int CheckSIOInBuffer()
{
	/*Return status of RX FIFO*/
	return (*SIO_STAT & 0x2)>>1;
}

int CheckSIOOutBuffer()
{
	/*Return status of TX Ready flag*/
	return (*SIO_STAT & 0x4)>>2;
}