apsg: apsg.c p_unix.c
	cc -g -Wall $^ -o $@

apsg.exe: apsg.c p_dos.c intr.asm
	tcc -mc -G -O -Z -DDOS -w apsg.c p_dos.c intr.asm
