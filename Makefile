include ../../Makefile.cfg

all:
	psx-gcc -o dumper.elf dumper.c crc32.c sio.c
	elf2exe dumper.elf dumper.exe

clean:
	rm -f dumper.elf dumper.exe cdimg

image:
	mkdir -p cd_root
	systemcnf dumper.exe > cd_root/system.cnf
	cp dumper.exe cd_root
	#cp DUMMY.3M cd_root
#	cp data/*.tim cd_root
	mkisofs -o dumper.hsf -V dumper -sysid PLAYSTATION cd_root
	mkpsxiso dumper.hsf dumper.bin $(TOOLCHAIN_PREFIX)/share/licenses/infousa.dat
	mv dumper.bin dumper.cue cdimg
	rm -f dumper.hsf
	rm -fr cd_root
