#ifndef _PSXSDK_SIO_H
#define _PSXSDK_SIO_H

/*Bitrate reload factor*/
#define SIO_REL_STOP	0
#define SIO_REL_MUL1	1
#define SIO_REL_MUL16	2
#define SIO_REL_MUL64	3

/*Bit lenght*/
#define SIO_DATA_LEN_5	0
#define SIO_DATA_LEN_6	1
#define SIO_DATA_LEN_7	2
#define SIO_DATA_LEN_8	3

/*Stop bit*/
#define SIO_STOP_BIT_1	1
#define SIO_STOP_BIT_15	2	/*1.5*/
#define SIO_STOP_BIT_2	3

/*Parity*/
#define SIO_PARITY_NONE	0
#define SIO_PARITY_ODD	1	/*ODD/EVEN are set to vice-versa according to Martin's docs*/
#define SIO_PARITY_EVEN	3

/*Pointers to SIO registers*/
static unsigned char *SIO_TX_RX = (unsigned char*)0x1F801050;	/*Read/Write*/
static unsigned short *SIO_STAT = (unsigned short*)0x1F801054;	/*Read only*/
static unsigned short *SIO_MODE = (unsigned short*)0x1F801058;	/*Read/Write*/
static unsigned short *SIO_CTRL = (unsigned short*)0x1F80105A;	/*Read/Write*/
static unsigned short *SIO_BPSV = (unsigned short*)0x1F80105E;	/*Read/Write*/

/*
 * Initialize SIO communication at the specified bitrate.
 * Mode is 8N1.
 */
void StartSIO(int bitrate);

/*
 * Same as StartSIO but with more control.
 * IMPORTANT: Must use defined macros.
 * For example setting datalenght to 5 should be done with "SIO_DATA_LEN_5"
 * and not simply passing 5 as an argument.
 */
void StartSIOEx(int bitrate, int datalenght, int parity, int stopbit);

/*
 * Shuts down SIO communication.
 */
void StopSIO();

/*
 * Read a single byte from the input buffer.
 * Direct access to register, doesn't use BIOS routines.
 */
unsigned char ReadByteSIO();

/*
 * Send a single byte to the output buffer.
 * Direct access to register, doesn't use BIOS routines.
 */
void SendByteSIO(unsigned char data);

/*
 * Check if any data is waiting in the input buffer.
 * Must be used when fetching data otherwise incorrect data could be read (usually 0x00).
 * 0 - No, 1 - Yes
 */
int CheckSIOInBuffer();

/*
 * Check if register is ready to send data (previos operation finished).
 * Must be used when sending data as output buffer is only 2 bytes long.
 * 0 - No, 1 - Yes.
 */
int CheckSIOOutBuffer();

#endif
