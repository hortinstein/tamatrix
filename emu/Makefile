LDFLAGS := 
CFLAGS := -ggdb -Wall -O2
OBJS := main.o tamaemu.o i2c.o i2ceeprom.o lcd.o benevolentai.o lcdmatch.o screens.o

all: tamaemu

screens.c: dummy
	make -C imgmatch


M6502/libM6502.a: dummy
	make -C M6502

tamaemu: $(OBJS) M6502/libM6502.a
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	make -C M6502 clean
	make -C imgmatch clean
	rm -f $(OBJS) tamaemu

.PHONY: clean dummy
