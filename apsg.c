#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS

#include <conio.h>
#include <dos.h>
#include "timer.h"

typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;

interrupt_handler saved_timer_handler;
uint16_t counter;

void interrupt timer(unsigned bp, unsigned di, unsigned si, unsigned ds, unsigned es, unsigned dx, unsigned cx, unsigned bx, unsigned ax);

void rdelay(uint16_t register d) {
	d >>= 4;
	while (counter < d) { }
	counter = 0;
}

#else

#include <stdint.h>

void rdelay(uint16_t d) {
	printf("delay %d\n", d);
}

void outportb(uint16_t port, uint8_t byte) {
	printf("port write %02X: %02X\n", port, byte);
}

#endif

#define TIMER_DELAY 90

typedef struct vgm {
	uint8_t *data;
	uint16_t data_size;
} vgm_t;

vgm_t parse(FILE *f) {
	vgm_t vgm;
	uint32_t * p;
	uint32_t v1;
	uint16_t data_copied = 0;
	int n;
	uint8_t buf[0x100];
	uint8_t *d;
	uint8_t finish = 0;

	n = fread(buf, 0x40, 1, f);
	if (n < 1) {
		perror("Failed to read header");
		exit(2);
	}

	p = (uint32_t *)buf;
	if (memcmp(p, "Vgm ", 4) != 0) {
		printf("Not an uncompressed VGM file\n");
		exit(2);
	}

	p++;
	v1 = *p - 0x3C; /* calculate data size from EoF offset */
	if (v1 > 0xFFFF) {
		/* Sizes larger than 64K get truncated for now */
		vgm.data_size = 0xFFFF;
	} else {
		vgm.data_size = (uint16_t) v1;
	}
	printf("Allocating %u bytes\n", vgm.data_size);
	vgm.data = malloc(vgm.data_size);
	if (vgm.data == NULL) {
		printf("Could not allocate data space\n");
		exit(2);
	}

	p++;
	v1 = *p; /* version */
	printf("VGM version %d.%d.%d\n", (char)((v1 >> 8) & 0xF), (char)((v1 >> 4) & 0xF), (char)(v1 & 0xF));

	if (v1 > 0x151) {
		p = (uint32_t *)(buf + 0x34);
		if (*p >= 0xc) {
			/* whoops, have to read a bit more because this file
			   has a larger offset */
			int m = (int)(*p - 0xc);
			if (m > (0x100 - 0x40)) {
				printf("Too much extra header\n");
				exit(2);
			}
			n = fread(buf + 0x40, m, 1, f);
			if (n < 1) {
				perror("Failed to read extra header");
				exit(2);
			}
			printf("Read %d extra header bytes\n", m);
		}
	}

	d = vgm.data;
	while (!finish && !feof(f) && data_copied < 0xFFFC) {
		uint32_t s = 0;
		int cmd = fgetc(f);
		switch (cmd) {
		case 0x4F:
			/* Game Gear PSG stereo (ignored) */
			fgetc(f);
			break;
		case 0x50:
			/* SN76489 write */
			*d = (unsigned char) cmd;
			d++;
			*d = (unsigned char) fgetc(f);
			d++;
			data_copied += 2;
			break;
		case 0x61:
			*d = (unsigned char) cmd;
			d++;
			*d = (unsigned char) fgetc(f);
			d++;
			*d = (unsigned char) fgetc(f);
			d++;
			data_copied += 3;
			break;
		case 0x62:
		case 0x63:
			*d = (uint8_t) cmd;
			d++;
			data_copied++;
			break;
		case 0x66:
			/* End of sound data */
			printf("End of sound data.\n");
			finish = 1;
			break;
		case 0x67:
			cmd = fgetc(f);
			if (cmd != 0x66) {
				printf("Corrupt data block: %02X\n", cmd);
				exit(2);
			}
			fgetc(f);
			n = fread(&s, 4, 1, f);
			if (n < 1) {
				perror("data block");
				exit(2);
			}
			printf("data block of length %u @ %ld\n", s, ftell(f));
			while (s > 0) {
				fgetc(f);
				s--;
			}
			break;
		case -1:
			perror("read data");
			exit(2);
		default:
			printf("Unknown command 0x%02X @ %lX\n", cmd, ftell(f) - 1);
			printf("Data copied: %04X\n", data_copied);
			exit(3);
		}
	}

	printf("Copied %u bytes of %u allocated\n", data_copied, vgm.data_size);
	vgm.data = realloc(vgm.data, data_copied);
	vgm.data_size = data_copied;
	return vgm;
}

void play(vgm_t * vgm) {
	unsigned char * d = vgm->data;
	unsigned int delay;

	while (d - vgm->data < vgm->data_size) {
		unsigned char cmd = *d;
		d++;
		switch (cmd) {
		case 0x50:
			outportb(0x50, *d);
			d++;
			break;
		case 0x61:
			delay = *(uint16_t *)d;
			d += 2;
			rdelay(delay);
			break;
		case 0x62:
			rdelay(735);
			break;
		case 0x63:
			rdelay(882);
			break;
		default:
			printf("unknown command %02X\n", cmd);
			exit(3);
		}
#ifdef DOS
		if (kbhit()) {
			getch();
			return;
		}
#endif
	}
}

void shutdown() {
	outportb(0x50, 0x9f);
	rdelay(10);
	outportb(0x50, 0xbf);
	rdelay(10);
	outportb(0x50, 0xdf);
	rdelay(10);
	outportb(0x50, 0xff);
}

int main(int argc, char * argv[]) {
	FILE * f;
	vgm_t vgm;

	if (argc < 2) {
		printf("Usage: apsg file.vgm\n");
		exit(1);
	}

	f = fopen(argv[1], "rb");
	vgm = parse(f);
	fclose(f);

#ifdef DOS
	/* HACK - wait a couple of seconds for the FDD to stop */
	sleep(2);
	saved_timer_handler = install_timer_handler(timer);
	load_timer(82);
#endif

	play(&vgm);

	shutdown();

#ifdef DOS
	install_timer_handler(saved_timer_handler);
#endif
	return 0;
}
