/* This file should compile on your average Unix (tested on Linux and
 * FreeBSD) and in Borland Turbo C 2.0. The implementation is wildly
 * different for each - the Unix version prints values and delays for
 * validation, and the DOS version does actual timing and writes to the
 * SN76489. But this provides a good way to validate the logic without
 * having to wrangle emulators and disk images. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS
  #include "p_dos.h"

  /* Function pointer to the previous timer (int 56h) handler */
  interrupt_handler saved_timer_handler;
#else /* unix implementations */
  #include "p_unix.h"
#endif

/* This structure only contains the relevant I/O and delay commands. The
 * rest is filtered out in parse(). */
typedef struct vgm {
	uint8_t *data;
	uint16_t data_size;
} vgm_t;

/* Parse the file into a set of I/O and delay commands in a vgm struct.
 * Due to not wanting to mess with segmentation limitations, this will
 * load at most a little under 64K of data. If the song data is larger,
 * it will be truncated. */
vgm_t parse(FILE *f) {
	vgm_t vgm;
	uint32_t *wp; /* word pointer into buf */
	uint32_t v1;
	long end_of_data;
	uint16_t data_copied = 0, header_size = 0x40, version = 0;
	int n;
	uint8_t *d;
	uint8_t buf[0x100];
	uint8_t finish = 0;

	/* First, set end of data to actual file size. */
	n = fseek(f, 0, SEEK_END);
	if (n != 0) {
		perror("Seeking to end");
		exit(2);
	}
	end_of_data = ftell(f);
	/* Seek back to the beginning. */
	n = fseek(f, 0, SEEK_SET);
	if (n != 0) {
		perror("Seeking to beginning");
		exit(2);
	}

	n = fread(buf, 0x40, 1, f);
	if (n < 1) {
		perror("Failed to read header");
		exit(2);
	}

	if (memcmp(buf, "Vgm ", 4) != 0) {
		printf("Not an uncompressed VGM file\n");
		exit(2);
	}
	wp = (uint32_t *)buf;

	v1 = wp[1] + 4; /* EoF offset */
	if (v1 < end_of_data) {
		/* This uh, might not be totally reliable. But we're
		 * gonna trust it for now. */
		end_of_data = v1;
	} else if (v1 > end_of_data) {
		printf("File reports end of data beyond end of file. Ignoring.\n");
	}

	version = (uint16_t)wp[2]; /* version */
	printf("VGM version " U8_FS "." U8_FS "." U8_FS "\n",
		(uint8_t)((version >> 8) & 0xF),
		(uint8_t)((version >> 4) & 0xF),
		(uint8_t)(version & 0xF)
	);

	v1 = wp[3];
	if (v1 != 2000000ul) {
		int mhz = (int)(v1 / 1000);
		printf("This song's SN clock is %d.%02dMHz, not 2MHz. Pitch will be shifted.\n", mhz / 1000, mhz % 1000);
	}

	if (version > 0x151) {
		v1 = wp[0x34 >> 2]; /* data offset */
		if (v1 >= 0xc) {
			/* we have to read a bit more because this file
			   has a larger offset */
			int remainder = (int)(v1 - 0xc);
			if (remainder > 192) {
				printf("Header says there are %d more bytes in the header, but there can be at\n", remainder);
				printf("most 192\n");
				exit(2);
			}
			n = fread(buf + 0x40, remainder, 1, f);
			if (n < 1) {
				perror("Failed to read extra header");
				exit(2);
			}
			header_size += remainder;
			printf("Read %d extra header bytes\n", remainder);
		}
	}

	v1 = end_of_data - header_size; /* calculate data size */
	if (v1 > 0xFFFF) {
		/* Sizes larger than 64K get truncated for now */
		vgm.data_size = 0xFFFF;
	} else {
		vgm.data_size = (uint16_t) v1;
	}
	printf("Allocating " U16_FS " bytes\n", vgm.data_size);
	vgm.data = malloc(vgm.data_size);
	if (vgm.data == NULL) {
		printf("Could not allocate data space\n");
		exit(2);
	}

	d = vgm.data;
	/* Check for at most 0xFFFC copied bytes so we still have room
	 * to put a three-byte delay command. */
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
			*d = (uint8_t) cmd;
			d++;
			*d = (uint8_t) fgetc(f);
			d++;
			data_copied += 2;
			break;
		case 0x61:
			*d = (uint8_t) cmd;
			d++;
			*d = (uint8_t) fgetc(f);
			d++;
			*d = (uint8_t) fgetc(f);
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
			printf("data block of length " U32_FS " @ %ld\n", s, ftell(f));
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
	uint8_t * d = vgm->data;
	uint16_t delay;
	uint8_t cmd;

	while (d - vgm->data < vgm->data_size) {
		cmd = *d;
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

/* Sets the attentuation for all channels to 0xF, turning them off. */
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
	/* HACK - wait a couple of seconds for the FDD to stop.
	 * Otherwise, stealing the timer interrupt will cause the drive
	 * to keep spinning. */
	sleep(2);
	saved_timer_handler = install_timer_handler(timer);
	load_timer(82);
#endif

	play(&vgm);

	shutdown();

#ifdef DOS
	/* Restore the original timer handler */
	install_timer_handler(saved_timer_handler);
#endif
	return 0;
}
