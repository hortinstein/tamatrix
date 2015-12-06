This contains all the software I've used/written for the Tamagotchi Matrix and viewer.

The directory structure:
/emu/ - The Tamagotchi emulator.
/server/ - The TamaServer, which connects the Tamagotchis and allows them to communicate.
/web/ - Sources for the web viewer as seen on http://tamahive.spritesserver.nl/
/viewer/ -Tamagotchi viewer
/viewer/esp12 - ESP12 part of Tamagotchi viewer
/viewer/remtama - Tama-Go figurine code to allow modifying display from buttons

Some notes:

***The Tamagotchi Emulator***
Run 'make' to make this. It shouldn't require anything special except gcc. Run the tamaemu binary to start
it up. You can control the Tamagotchi yourself by pressing the 1, 2 or 3 key followed by an enter. Control-C
will drop you into a debugging prompt, press control-C again to quit the emulator.

When you first start up the emulator, it may be wise to do it with the -n parameter. This disables the AI. The
very first thing that happens on an empty EEPROM is the Tamagotchi asking to set the time and date, and the
AI doesn't know how to do that. The trick is to start tamaemu with the -n parameter, then use the 1, 2 and 3
keys (followed by an enter!) to set the date and name your Tamagotchi. There's no need to set the true time 
and date, by the way, gameplay doesn't seem to be dependent on that. After you've done this and the Tamagotchi
has hatched, you can re-start tamaemu without the -n option and the AI should be able to do its thing.

***The Tamagotchi server***
Just run 'make' and start it up. The Tamagotchis running on the same server will automatically connect to it.

***The Tamagotchi webviewer***
You'll need a PHP-enabled webserver (I used Apache) to run this. The webserver should be on the same machine 
as the Tamaserver runs; they use non-networkable shared memory segments to communicate.

***The Tamagotchi viewer***
This consists of two subdirectories: one for the ESP12 module which communicates over WiFi to the Tamaserver
and one for the SPI flash chip which should be connected to the Tama-Go port of the Tamagotchi. To
assemble the latter, check out Natalies Egg-shell repo 
(git checkout https://github.com/natashenka/Egg-Shell.git), copy the remtama directory into the 
tASMgotchi directory, cd into the remtama dir and run 'make'. Use an Egg-Shell board or any flashrom-supported
programmer to flash the resulting remtama.bin. (Check the Makefile for an example.)

For the esp12 part, you need a working ESP8266 SDK and toolchain. If you have, just run 'make' in the ESP8266 
directory, and use 'make flash' to flash the code into an ESP12. The connections needed to control the
Tamagotchi are:
Button 0 - GPIO0
Button 1 - GPIO13
Button 2 - GPIO12
Make sure the other GPIOs have pullups/pulldowns as required to allow the ESP12 to boot from SPI flash. I 
suggest also wiring the reset input to the reset button of the Tamagotchi, to make resetting both easier.

With the thing assembled, to pick an access point to connect to, hold button1 of the Tamagotchi for >5 seconds.
The ESP12 should now go into access point mode. Connect to it and point a webbrowser at http://192.168.4.1 to
select an access point.




