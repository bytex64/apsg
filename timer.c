#include <stdio.h>
#include <dos.h>
#include "timer.h"

interrupt_handler install_timer_handler(interrupt_handler next) {
	interrupt_handler previous;
	__cli__();
	previous = getvect(0x56);
	setvect(0x56, next);
	__sti__();
	printf("Timer Vector %08lX => %08lX\n", previous, next);
	return previous;
}
