OBJS:=main.o
TARGET:=tamaserver
LDFLAGS:=-lrt
CFLAGS:=-ggdb

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJS) $(TARGET)

.PHONEY: clean
