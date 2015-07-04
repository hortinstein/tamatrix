OBJS := ConDebug.o Debug.o M6502.o
TARGET := libM6502.a
CFLAGS := -ggdb

$(TARGET): $(OBJS)
	ar rcs $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
