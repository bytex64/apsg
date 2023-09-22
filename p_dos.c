#include <stdio.h>

#include "p_dos.h"

/* Tick counter. Incremented from the interrupt handler and read in
 * rdelay(). */
uint16_t counter;

interrupt_handler install_timer_handler(interrupt_handler next) {
	interrupt_handler * timer_vector = MK_FP(0, 0x56 << 2);
	interrupt_handler previous;
	__cli__();
	previous = *timer_vector;
	*timer_vector = next;
	__sti__();
	/* printf("Timer Vector %08lX => %08lX\n", previous, next); */
	return previous;
}

/* Wait until the counter reaches our target value. */
void rdelay(uint16_t register d) {
	d >>= 4;
	while (counter < d) { }
	counter = 0;
}

