OBJS := ConDebug.o Debug.o M6502.o
TARGET := libM6502.a
CFLAGS := -ggdb -O2 -fprofile-use -march=native

ifeq ("$(PROF)","use")
LDFLAGS+= -fprofile-use
CFLAGS+= -fprofile-use
else
LDFLAGS+= -fprofile-generate
CFLAGS+= -fprofile-generate
endif


$(TARGET): $(OBJS)
	ar rcs $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
