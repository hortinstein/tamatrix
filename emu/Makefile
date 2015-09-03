LDFLAGS := -lgcc
CFLAGS := -ggdb -Wall -O2 -march=native
OBJS := main.o tamaemu.o i2c.o i2ceeprom.o lcd.o benevolentai.o lcdmatch.o screens.o \
		ir.o udp.o

ifeq ("$(PROF)","use")
LDFLAGS+= -fprofile-use
CFLAGS+= -fprofile-use
else
LDFLAGS+= -fprofile-generate
CFLAGS+= -fprofile-generate
endif

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