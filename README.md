# PS1 BIOS dumper:

![biosdumper](https://cloud.githubusercontent.com/assets/8411572/25516011/38a83c36-2be8-11e7-9a16-1026c00cd5e7.png)

<b>About:</b>
<br>PS1 BIOS dumper is a software for the PlayStation 1/PSone which
<br>enables you to dowload BIOS image from your console to your PC.

 There are two methods available:
 1. Using Memory Cards, and
 2. Using serial cable.

 Each method is fairly similar but has different hardware and software requirements.

 This BIOS dumper is written using open source Tails92's PSX libraries and as such is perfectly legal
 <br>to distribute since it doesn't use <b>any</b> part of the code from the Sony's original SDK.

<b>Compiling:</b>
<br>Version 0.1 of PSXSDK is only guaranteed to work. Newer SDKs ditched BIOS routines
<br>and this dumper relies heavily on that code for gamepad and memory card handling.

Run "make" for the exe and "make image" for the iso.

<b>Memory Card dumping method:</b>
-
Required hardware:
* PlayStation console capable of booting burned CDs.
* PlayStation Memory Card with 15 free save slots.
* PlayStation Memory Card reader/MemCARDuino/DexDrive/homebrew enabled PlayStation 2.
* Cd-Writter.
* Blank Cd-R.

Required Software:
* BIOS dumper.
* BIOS merge application for your Operating System.
* Software to transfer saves from Memory Card to a PC:
	- Memory Card Captor Sakura (for PlayStation Memory Card reader) if you are using Windows.
	- PSX Memory Card Manager (for PlayStation Memory Card reader) if you are using Linux.
        - MemcardRex (for DexDrive and MemCARDuino).
        - uLaunchELF for PlayStation 2.

<b>Serial cable dumping method:</b>
-
Required hardware:
* PlayStation console capable of booting burned CDs.
* Official Net Yaroze serial cable or Skywalker's home made serial cable.
* COM port on the PC (real port or USB adapter).
* Cd-Writter.
* Blank Cd-R.

Required Software:
* BIOS dumper.
* BIOS get application.
