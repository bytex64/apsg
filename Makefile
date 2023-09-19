apsg: apsg.c
	cc $< -g -Wall -o $@

apsg.exe: apsg.c timer.c intr.asm
	tcc -mc -G -O -Z -DDOS -w apsg.c timer.c intr.asm
