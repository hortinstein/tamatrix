

remtama.bin: remtama.asm images.txt
	../tASMgotchi.py -o remtama.bin remtama.asm images.txt

flash: remtama.bin
	cat $^ /dev/zero | dd of=flash.bin bs=1024 count=4096
	/usr/sbin/flashrom -l layout.txt -i tama -p ft2232_spi:type=tumpa,port=B,divisor=2 -w flash.bin
