CFLAGS:=-ggdb

LCDFILES:=poopie1.lcd poopie2.lcd poopie3.lcd alert.lcd sick1.lcd sick2.lcd sleep1.lcd sleep2.lcd pooping1.lcd pooping2.lcd \
		dark1.lcd dark2.lcd dark3.lcd dark4.lcd stb1.lcd stb2.lcd stb3.lcd stb4.lcd gameend.lcd jump1.lcd jump2.lcd \
		doorsel.lcd irgame1.lcd irfail.lcd born1.lcd born2.lcd

../screens.c: compress $(LCDFILES)
	echo > $@.tmp
	for f in $(LCDFILES); do ./compress screen_$${f%.lcd} < $$f >>$@.tmp; done
	mv $@.tmp $@

compress: compress.o
	$(CC) -o $@ $^

clean:
	rm -rf compress *.o screens.inc
