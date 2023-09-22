#ifndef _P_DOS_H
#define _P_DOS_H

#include <conio.h>
#include <dos.h>

typedef unsigned char uint8_t;
typedef unsigned int  uint16_t;
typedef unsigned long uint32_t;
#define U8_FS  "%hu"
#define U16_FS "%u"
#define U32_FS "%lu"

typedef void interrupt (far * interrupt_handler)(unsigned bp, unsigned di, unsigned si, unsigned ds, unsigned es, unsigned dx, unsigned cx, unsigned bx, unsigned ax);

#define load_timer(v) \
	outportb(0x58, v & 0xFF); \
	outportb(0x58, (v >> 8) & 0xFF);

interrupt_handler install_timer_handler(interrupt_handler next);
void rdelay(uint16_t register d);

/* Prototype declaration for the timer handler implemented in intr.asm. */
void interrupt timer();

#endif /* _P_DOS_H */
