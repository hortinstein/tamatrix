LDFLAGS := 
CFLAGS := -ggdb -Wall -O2
OBJS := main.o tamaemu.o i2c.o i2ceeprom.o

all: tamaemu

M6502/libM6502.a:
	make -C M6502

tamaemu: $(OBJS) M6502/libM6502.a
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) tamaemu