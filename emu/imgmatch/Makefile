CFLAGS:=-ggdb

LCDFILES:=poopie1.lcd poopie2.lcd

../screens.c: compress $(LCDFILES)
	echo > $@.tmp
	for f in $(LCDFILES); do ./compress screen_$${f%.lcd} < $$f >>$@.tmp; done
	mv $@.tmp $@

compress: compress.o
	$(CC) -o $@ $^

clean:
	rm -rf compress *.o screens.inc
