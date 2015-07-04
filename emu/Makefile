LDFLAGS := 
CFLAGS := -ggdb -Wall
OBJS := main.o tamaemu.o

all: tamaemu

M6502/libM6502.a:
	make -C M6502

tamaemu: $(OBJS) M6502/libM6502.a
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) tamaemu