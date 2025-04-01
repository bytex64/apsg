#include <stdio.h>

#include "p_unix.h"

void rdelay(uint16_t d) {
	printf("delay %d (%d)\n", d, d << 4);
}

void outportb(uint16_t port, uint8_t byte) {
	printf("port write %02X: %02X\n", port, byte);
}

