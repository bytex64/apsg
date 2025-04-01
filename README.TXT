                    ====== Apricot PSG Player ======

What is it?
-----------
APSG is a command-line DOS program for playing VGM files containing
SN76489 data on Apricot PC/Xi computers.


Huh?
----
Apricot PC/Xi was a line of computers made by the British company ACT.
They ran DOS, but they were not PC compatible, as they had many
architectural differences. One of those differences was the inclusion of
a TI SN76489 Digital Complex Sound Generator chip, also seen in the
TI-99/4A, BBC Micro, PCjr, and Tandy 1000; used in a number of arcade
games; and cloned into the Sega Master System and Game Gear. And since
it was a business computer without even CGA compatibility, it never saw
any games more advanced than Zork. Hilariously, the system only used it
to provide a keyboard click (using the noise channel) and a system beep.


How do I use it?
----------------
Run it with a VGM file like so:

apsg file.vgm

It does require that the VGM files are uncompressed. A lot of VGM files
are actually gzip compressed, so you will have to decompress them first
with something like:

zcat original.vgm > uncompressed.vgm

If the SN76489 frequency is not 2MHz (and it probably won't be - pretty
much every other system used a color clock signal), APSG will re-tune
the notes based on the difference. If you don't want it to do that, you
can use the /n switch before the filename.

On Unix, it will just print out a list of I/O writes and delays. This is
just for validating the logic without having to mess with emulators or
real hardware.


How do I build it?
--------------------
APSG compiles on Unix-like systems (tested on Linux and FreeBSD) and
Borland Turbo C 2.0/Borland Turbo Assembler 2.0.

To build on Unix, make sure you have a compiler available as `cc`, and
type:

make apsg

To build on DOS, make sure you have Borland Turbo C and Borland Turbo
Assembler installed, and both directories are in your path. Then type:

make apsg.exe

Yes. It's the same Makefile for GNU make, BSD make, and Borland make.
Make is very old.


What if I need help?
--------------------
You're on your own, buddy! I don't plan to improve this or maintain it,
so if it breaks, you get to keep both pieces. Good luck!


Did you use an old DOS text editor to write this?
-------------------------------------------------
I did! I used the same shareware version of Galaxy I used as a kid,
pulled off of an old floppy drive. You can even see the ^Z's at the end,
see!
