//PSX BIOS dumper, programmed using Tails92's PSX libs
//Shendo, Jan. 2011. - Jul. 2013.

#include <psx.h>
#include <stdio.h>
#include "include/fontspace.h"
#include "include/cardicon.h"

//Images
#include "include/font.h"
#include "include/selector.h"

//Defines
#define uchar unsigned char
#define ushort unsigned short
#define DescSW   	0xf4000000		//BIOS
#define SwCARD		(DescSW|0x01)	//Memory Card
#define EvSpIOE		0x0004			//End of I/O
#define EvSpTIMOUT      0x0100		//Time out occured
#define EvSpNEW		0x2000			//New device
#define EvSpERROR	0x8000			//Error occured
#define EvMdNOINTR	0x2000
#define O_WRONLY	2
#define O_CREAT		512

//Global variables
unsigned int GameDrawList[0x4000];	//128 kilobytes
GsDrawEnv GameDrawEnv;
GsDispEnv GameDispEnv;
GsRectangle GameRect;
GsImage TempImage;
GsSprite SelectorSprite;
uchar DumpBuffer[122880];		//Buffer for BIOS data
int ScreenFreeze = 0;			//Used for VBlank;
char BIOSver[] = "BIOS version: 0.0";
char BIOSdate[] = "Date: 00/00/00";
char FileName[] = "bu00:BEFFFFFFFFFFBIOSPT0";
unsigned int BIOSCRC32 = 0;		//Calculated CRC of the BIOS data
char BIOSCRC32Message[25];		//Message which will hold BIOS CRC32 value
int SelectorLight[2] = {0,0};
ushort PadBuffer = 0;
int PadFlag = 0;
int CurrentScreen = 0;
int CurrentSelection = 0;
int MemCardStatus = 0;
int VSyncCounter = 0;

//Memory Card events
static unsigned long ev0, ev1, ev2, ev3;

//Function prototypes
void GameSetup();
int GsGetMode();
void GsClearScreen(uchar r, uchar g, uchar b);
void GameVblankHandler();
void WaitVBlank();
void WaitFrames(int frames);
void GsFlipDisplay();
void GsPrintString(int x, int y, char *string);
void InitSelectorSprite();
void MainMenu();
void CheckMemcard();
void ShowCardError();
void DumpingMenu();
void DumpingComplete();
void SerialTransferMain();

//Init important stuff
void GameSetup()
{
	//Initialize PSX library
	PSX_Init();

	//Init graphics
	GsInit();

	//Clear VRAM
	GsClearMem();

	//Set up drawing environment
	GameDrawEnv.dither = 0;
	GameDrawEnv.draw_on_display = 1;
	GameDrawEnv.x = 0;
	GameDrawEnv.y = 0;
	GameDrawEnv.w = 320;
	GameDrawEnv.h = 240;
	GameDrawEnv.ignore_mask = 0;
	GameDrawEnv.set_mask = 0;

	//Apply drawing environment
	GsSetDrawEnv(&GameDrawEnv);

	//Set up display environment
	GameDispEnv.x = 0;
	GameDispEnv.y = 240;

	//Apply display environment
	GsSetDispEnv(&GameDispEnv);

	//Set drawing list
	GsSetList(GameDrawList);

	//Init Memory Card events
	ev0 = OpenEvent(SwCARD, EvSpIOE, EvMdNOINTR, NULL);
	ev1 = OpenEvent(SwCARD, EvSpERROR, EvMdNOINTR, NULL);
	ev2 = OpenEvent(SwCARD, EvSpTIMOUT, EvMdNOINTR, NULL);
	ev3 = OpenEvent(SwCARD, EvSpNEW, EvMdNOINTR, NULL);

	//Enable Memory Card events
	EnableEvent(ev0);
	EnableEvent(ev1);
	EnableEvent(ev2);
	EnableEvent(ev3);
}

//Get the native videomode of the console
int GsGetMode()
{
	if (*(char *)0xbfc7ff52 == 'E') return VMODE_PAL;
	else return VMODE_NTSC;
}

//Clear screen with a desired color
void GsClearScreen(uchar r, uchar g, uchar b)
{
	//Set up rectangle
	GameRect.x = 0;
	GameRect.y = 0;
	GameRect.w = 320;
	GameRect.h = 240;
	GameRect.r = r;
	GameRect.g = g;
	GameRect.b = b;
	GameRect.attribute = 0;

	//Push rectangle onto a list
	GsSortRectangle(&GameRect);
}

//Handler for the VBlank event
void GameVblankHandler()
{
	//Unfreeze the screen
	ScreenFreeze = 0;
}

//Wait for vertical blank
void WaitVBlank()
{
	//Wait for the VBlank event
	while(ScreenFreeze);

	//Freeze the screen for the next round
	ScreenFreeze = 1;
}

//Wait for specified number of frames
void WaitFrames(int frames)
{
	while(frames > 0)
	{
		WaitVBlank();
		frames--;
	}
}

//Change front and back buffer
void GsFlipDisplay()
{
	if(GameDispEnv.y == 0)
	{
		GameDispEnv.y = 240;
		GameDrawEnv.y = 0;
	}
	else
	{
		GameDispEnv.y = 0;
		GameDrawEnv.y = 240;
	}

	//Apply new display and drawing environments
	GsSetDispEnv(&GameDispEnv);
	GsSetDrawEnv(&GameDrawEnv);
}

//Print a string at the specified coordinates
void GsPrintString(int x, int y, char *string)
{
	GsSprite CharSprite;
	char CharOffset;

	//Set up character sprite
	CharSprite.x = x;
	CharSprite.y = y;
	CharSprite.w = 8;
	CharSprite.h = 8;
	CharSprite.r = 128;
	CharSprite.g = 128;
	CharSprite.b = 128;
	CharSprite.cx = 320;		//Clut X
	CharSprite.cy = 240;		//Clut Y
	CharSprite.tpage = 5;		//64px = 1 tpage
	CharSprite.attribute = COLORMODE(COLORMODE_8BPP);

	while(*string)
	{
		//Check if this is a printable character
		if(*string >= 0x20 && *string <= 0x7F)
		{
			//Get char offset
			CharOffset = *string - 0x20;

			CharSprite.u = (CharOffset%0x20) * 8;
			CharSprite.v = (CharOffset/0x20) * 8;

			//Place sprite in the drawing list
			GsSortSimpleSprite(&CharSprite);
		}

		//Point to the next character
		string++;

		//Increase X offset
		CharSprite.x += (FontSpace[CharOffset] + 1);
	}
}

//Initialise selector sprite
void InitSelectorSprite()
{
	SelectorSprite.x = 18;
	SelectorSprite.y = 109 + CurrentSelection * 10;
	SelectorSprite.w = 80;
	SelectorSprite.h = 9;
	SelectorSprite.u = 0;
	SelectorSprite.v = 24;
	SelectorSprite.r = SelectorLight[0] + 45;
	SelectorSprite.g = SelectorLight[0];
	SelectorSprite.b = SelectorLight[0];
	SelectorSprite.cx = 320;		//Clut X
	SelectorSprite.cy = 241;		//Clut Y
	SelectorSprite.tpage = 5;		//64px = 1 tpage
	SelectorSprite.attribute = COLORMODE(COLORMODE_8BPP);
}

//Main menu of the program
void MainMenu()
{
	int i = 0;

	//Check if DOWN is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_DOWN) && CurrentSelection < 5)
	{
		CurrentSelection++;
		PadFlag = 1;
	}

	//Check if UP is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_UP) && CurrentSelection > 0)
	{
		CurrentSelection--;
		PadFlag = 1;
	}

	//Update selector
	InitSelectorSprite();

	//Place selector sprite in the drawing list
	GsSortSimpleSprite(&SelectorSprite);

	//Print copyright and other info
	GsPrintString(20,20,"PSX BIOS dumper 2.6, Shendo 2013");
	GsPrintString(20,30,"Coded using PSXSDK.");
	GsPrintString(20,100,"Transfer to Memory Card:");
	GsPrintString(20,110,"Part 1");
	GsPrintString(20,120,"Part 2");
	GsPrintString(20,130,"Part 3");
	GsPrintString(20,140,"Part 4");
	GsPrintString(20,150,"Part 5");
	GsPrintString(20,160,"Serial cable");
	GsPrintString(20,210,"Press } to proceed");
		
	//Show console region
	if (*(char *)0xbfc7ff52 == 'E') GsPrintString(20, 50, "Console region: PAL");
	if (*(char *)0xbfc7ff52 == 'A') GsPrintString(20, 50, "Console region: NTSC U/C");
	if (*(char *)0xbfc7ff52 == 'J') GsPrintString(20, 50, "Console region: NTSC J");

	//Show BIOS CRC-32
	GsPrintString(20,60,BIOSCRC32Message);

	//Show BIOS version and date(SCPH-1000 doesn't have that info)
	if (*(char *)0xbfc7ff46 == 0x2E)
	{
		GsPrintString(20, 70, BIOSver);
		GsPrintString(20, 80, BIOSdate);
	}

	//Animate selector
	if(SelectorLight[1] == 0)
		SelectorLight[0]+=3;
	else
		SelectorLight[0]-=3;

	//Reverse direction
	if(SelectorLight[0] >= 55)
		SelectorLight[1] = 1;

	if(SelectorLight[0] <= 0)
		SelectorLight[1] = 0;

	//Check if X is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_CROSS))
	{
		//Check if this is the serial cable selection
		if(CurrentSelection == 5)
		{
			//Clear serial input buffer
			for(i = 0; i < 8; i++)ReadByteSIO();

			CurrentScreen = 5;
		}
		else
		{
			GsClearScreen(0,0,0);
			GsPrintString(20,20,"Checking Memory Card...");
			CurrentScreen++;
		}
		
		PadFlag = 1;
	}
}

//Check Memory Card status
void CheckMemcard()
{
	//Buffer for reading data
	uchar buffer[128];

	//Slot counter
	int counter = 0;

	//Clear Memory Card events
	TestEvent (ev0);
	TestEvent (ev1);
	TestEvent (ev2);
	TestEvent (ev3);

	//Update info for Memory Card in slot 1
	_card_info(0x00);

	//Loop untill an event occures
	while(1)
	{
		//Everything went well
		if(TestEvent (ev0) == 1)
		{
			MemCardStatus = 0;
			break;
		}
		
		//Error
		if(TestEvent (ev1) == 1)
		{
			MemCardStatus = 1;
			break;
		}

		//No memory card
		if(TestEvent (ev2) == 1)
		{
			MemCardStatus = 2;
			break;
		}

		//New card connected
		if(TestEvent (ev3) == 1)
		{
			_new_card();
			MemCardStatus = 0;
			break;
		}
	}

	//Check if card is formatted
	if(MemCardStatus == 0)
	{
		//Clear first item in the buffer
		buffer[0] = 0;

		//Read first block of the Memory Card
		_card_read(0x00,0,&buffer[0]);

		//Wait 20 frames (waiting for card to finish reading)
		WaitFrames(20);

		if(buffer[0] == 'M' && buffer[1] == 'C')
		{
			//Check if all 15 slots are free
			for(counter = 1; counter<16; counter++)
			{
				//Clear first item in the buffer
				buffer[0] = 0;

				_new_card();
				_card_read(0x00,counter,&buffer[0]);

				//Wait 20 frames (waiting for card to finish reading)
				WaitFrames(20);

				//Check if slot is free
				if(buffer[0] != 0xA0 && buffer[0] != 0xA1 && buffer[0] != 0xA2 && buffer[0] != 0xA3)
				{
					MemCardStatus = 4;
					break;
				}
			}
		}
		else
		{
			//Card is not formatted
			MemCardStatus = 3;
		}
	}

	//Wait a while
	WaitFrames(20);

	CurrentScreen++;
}

//Show error if it occured while checking the Memory Card
void ShowCardError()
{
	//Check if triangle is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_TRIANGLE))
	{
		//Return to main menu
		CurrentScreen = 0;
		PadFlag = 1;
	}

	GsPrintString(20,210,"Press | to return to main menu");

	switch(MemCardStatus)
	{
		default:
			//No errors, proceed to dumping
			GsClearScreen(0,0,0);
			GsPrintString(20,20,"Dumping...");
			GsPrintString(20,30,"Please wait");
			CurrentScreen++;
			break;

		case 1:
			GsPrintString(20,20,"Error occured while reading");
			GsPrintString(20,30,"Memory Card in slot 1");
			break;

		case 2:
			GsPrintString(20,20,"No Memory Card in slot 1");
			break;

		case 3:
			GsPrintString(20,20,"Memory Card in slot 1 is not formatted");
			break;

		case 4:
			GsPrintString(20,20,"Memory Card in slot 1 is not empty");
			GsPrintString(20,40,"All 15 slots must be empty");
			GsPrintString(20,50,"in order to dump BIOS data");
			break;
	}
}

//The actuall dumping process
void DumpingMenu()
{
	int f;
	int i;
	int MaxOffset = 114688;

	//Update part name
	FileName[23] = '1' + CurrentSelection;		//ASCII
	CardIcon[35] = 0x50 + CurrentSelection;		//SJIS

	//Check if this is the fifth part (Which is smaller then others)
	if(CurrentSelection == 4)MaxOffset = 65536;

	//Copy icon data to BIOS buffer (memcpy is buggy)
	for(i = 0; i < 8192; i++)
		DumpBuffer[i] = CardIcon[i];

	//Copy BIOS data to BIOS buffer
	for(i = 0; i < MaxOffset; i++)
		DumpBuffer[i+8192] = (*(uchar *)(0x0bfc00000 + i + (114688*CurrentSelection)));

	//Create a 15 slot save on memorycard
	f = open(FileName, O_CREAT | 15 << 16);
	
	//Close file
	close(f);
	
	//Open file for writing
	f = open(FileName, O_WRONLY);

	//Write data to Memory Card
	write(f,&DumpBuffer[0], 122880);
	
	//Close file
	close(f);

	CurrentScreen++;
}

//Dumping process complete, show info
void DumpingComplete()
{
	//Check if triangle is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_TRIANGLE))
	{
		//Return to main menu
		CurrentScreen = 0;
		PadFlag = 1;
	}

	//Check if circle is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_CIRCLE))
	{
		//Reset the console
		run_bios();
	}

	GsPrintString(20,20,"Dumping complete");
	GsPrintString(20,200,"Press > to reset the console");
	GsPrintString(20,210,"Press | to return to main menu");
}

//Main Menu of the serial transfer section
void SerialTransferMain()
{
	//Check if triangle is pressed
	if (PadFlag == 0 && (PadBuffer & PAD_TRIANGLE))
	{
		//Return to main menu
		CurrentScreen = 0;
		PadFlag = 1;
	}

	//Check if there are any bytes in the input buffer
	if(CheckSIOInBuffer())
	{
		//Check if this is the sync byte
		if(ReadByteSIO() == 'S')
		{
			CurrentScreen++;
			VSyncCounter = 0;
		}
	}

	GsPrintString(20,20,"Serial cable transfer");
	GsPrintString(20,40,"Status:");
	GsPrintString(20,50,"Waiting for PC");
	GsPrintString(20,210,"Press | to return to main menu");
}

//Command listen mode of the serial transfer
void SerialTransferListen()
{
	int i = 0;
	unsigned int MemPoint = 0;

	//Wait for command from the PC
	if(CheckSIOInBuffer())
	{
		switch(ReadByteSIO())
		{
			default:
				break;

			case 'R':			//Read BIOS
				//Get part of the BIOS to send (0 - 255)
				while(1)if(CheckSIOInBuffer())break;
				MemPoint = ReadByteSIO();

				//Get offset in 2048 byte segments
				MemPoint = (MemPoint * 2048);

				for (i = 0; i < 2048; i++)
				{
					while(1)if(CheckSIOOutBuffer())break;
					SendByteSIO(*(uchar *)(0x0bfc00000 + i + MemPoint));
				}
				VSyncCounter = 0;
				break;

			case 'C':			//Send BIOS CRC-32

				while(1)if(CheckSIOOutBuffer())break;
				SendByteSIO((uchar)(BIOSCRC32 & 0xFF));

				while(1)if(CheckSIOOutBuffer())break;
				SendByteSIO((uchar)(BIOSCRC32 >> 8));

				while(1)if(CheckSIOOutBuffer())break;
				SendByteSIO((uchar)(BIOSCRC32 >> 16));

				while(1)if(CheckSIOOutBuffer())break;
				SendByteSIO((uchar)(BIOSCRC32 >> 24));

				VSyncCounter = 0;
				break;
		}
	}

	VSyncCounter++;

	//If half a second passed from the last transmission, cancel communication
	if(VSyncCounter > 30)CurrentScreen--;

	GsPrintString(20,20,"Serial cable transfer");
	GsPrintString(20,40,"Status:");
	GsPrintString(20,50,"Transferring data");
}

int main()
{
	//Init PSX
	GameSetup();

	//Set proper video mode
	GsSetVideoMode(320, 240, GsGetMode());
	//GsSetVideoMode(320, 240, VMODE_NTSC);

	//Set a handler to be executed each VBlank
	SetVBlankHandler(GameVblankHandler);

	//Load a custom font
	GsImageFromTim(&TempImage, FontTimData);

	//Upload it to VRAM
	GsUploadImage(&TempImage);

	//Load selector image
	GsImageFromTim(&TempImage, SelectorTimData);

	//Upload it to VRAM
	GsUploadImage(&TempImage);

	//Make a sprite out of it
	InitSelectorSprite();

	//Load debug font
	GsLoadFont(640,10,640,0);

	//Get BIOS version
	BIOSver[14] = *(char *)0xbfc7ff45;
	BIOSver[16] = *(char *)0xbfc7ff47;

	//Get BIOS date
	BIOSdate[6] = *(char *)0xbfc7ff49;
	BIOSdate[7] = *(char *)0xbfc7ff4A;
	BIOSdate[9] = *(char *)0xbfc7ff4C;
	BIOSdate[10] = *(char *)0xbfc7ff4D;
	BIOSdate[12] = *(char *)0xbfc7ff4F;
	BIOSdate[13] = *(char *)0xbfc7ff50;

	//Calculate BIOS CRC32
	BIOSCRC32 = CalculateCRC32((unsigned int)0x0bfc00000, 0, 524288);

	//Create BIOS CRC32 message
	sprintf(BIOSCRC32Message, "BIOS CRC-32: %x", BIOSCRC32);

	//Start SIO communication
	StartSIO(115200);

	//Main game loop
	while(1)
	{
		//Clear screen
		GsClearScreen(0,0,0);
		
		//Get gamepad status
		PSX_ReadPad(&PadBuffer, NULL);

		//Check if pad flag should be cleared
		if(PadFlag == 1 && PadBuffer == 0)
			PadFlag = 0;

		//Return to loader if select is pressed
		//if(PadBuffer & PAD_SELECT)break;

		//Select a scene to be shown
		switch(CurrentScreen)
		{
			default:
				break;

			case 0:
				MainMenu();
				break;

			case 1:
				CheckMemcard();
				break;

			case 2:
				ShowCardError();
				break;

			case 3:
				DumpingMenu();
				break;

			case 4:
				DumpingComplete();
				break;

			case 5:
				SerialTransferMain();
				break;

			case 6:
				SerialTransferListen();
				break;
		}

		//Draw data from the draw list
		GsDrawList();

		//Wait for VBlank
		WaitVBlank();
	
		//Flip display
		GsFlipDisplay();
	}

	//Cleanly close everything
	StopSIO();
	StopPAD();
	RemoveVBlankHandler();

	//__asm__("j 0x801e5684");

	//Perform a warm boot
	__asm__("li $9, 0xa0");
	__asm__("j 0xa0");
	__asm__("jr $ra");

	//Everything went well
	return 0;
}