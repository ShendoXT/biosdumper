==================
 PS1 BIOS dumper:
==================

 Author: Shendo
 Version: 2.6
 Updated: 2013-XX-XX

========================
 About PS1 BIOS dumper:
========================

 PS1 BIOS dumper is a software for the PlayStation 1/PSone which
 enables you to dowload BIOS image from your console to your PC.

 There are two methods available:
 1. Using Memory Cards, and
 2. Using serial cable.

 Each method is fairly similar but has different hardware and software requirements.

 This BIOS dumper is written using open source Tails92's PSX libraries and as such is perfectly legal
 to distribute since it doesn't use _any_ part of the code from the Sony's original SDK.

=============
 Compiling :
=============
 Version 0.1 of PSXSDK is only guaranteed to work. Newer SDKs ditched BIOS routines
 and this dumper relies heavily on that code for gamepad and memory card handling.

 Run "make" for the exe and "make image" for the iso.

=============================
 Memory Card dumping method:
=============================

 Required hardware:
--------------------
* PlayStation console capable of booting burned CDs.
* PlayStation Memory Card with 15 free save slots.
* PlayStation Memory Card reader/MemCARDuino/DexDrive/homebrew enabled PlayStation 2.
* Cd-Writter.
* Blank Cd-R.

 Required Software:
--------------------
* BIOS dumper.
* BIOS merge application for your Operating System.
* Software to transfer saves from Memory Card to a PC:
	- Memory Card Captor Sakura (for PlayStation Memory Card reader) if you are using Windows.
	- PSX Memory Card Manager (for PlayStation Memory Card reader) if you are using Linux.
        - MemcardRex (for DexDrive and MemCARDuino).
        - uLaunchELF for PlayStation 2.

==============================
 Serial cable dumping method:
==============================

 Required hardware:
--------------------
* PlayStation console capable of booting burned CDs.
* Official Net Yaroze serial cable or Skywalker's home made serial cable.
* COM port on the PC (real port or USB adapter).
* Cd-Writter.
* Blank Cd-R.

 Required Software:
--------------------
* BIOS dumper.
* BIOS get application.

============
 ChangeLog:
============

 Version 2.6:
--------------
* Fixed error which displayed incorrect version and date on SCPH-1000 consoles.
* Added BIOS CRC-32 information.
* Added option to use serial cable.

 Version 2.5:
--------------
* App launchers are no longer needed. If you have a chipped PSX this one should boot and work just fine.
	- If you don't have a chipped PSX you still need a Boot Cd of course.
* Dumper now checks the status of the Memory Card and informs you if there are any errors.
* Reset after dumping is no longer required. If you have multiple cards at your disposal this will greatly save your time.
* Filename now contains part number.
* Rewritten from scratch using Tails92's PSX Libs (http://code.google.com/p/psxsdk).

 Version 2.0:
--------------
* Added BIOS date and version info on the screen.

 Version 1.0:
--------------
* Added console region info on the screen.

 Version 0.5:
--------------
* Changed font.
* Updated interface with menu screen.

 Version 0.1:
--------------
* Initial release.
* Written using Blade's Libs.
