#pragma once
#ifndef _TIMER_H
#define _TIMER_H

typedef void interrupt (far * interrupt_handler)(unsigned bp, unsigned di, unsigned si, unsigned ds, unsigned es, unsigned dx, unsigned cx, unsigned bx, unsigned ax);

#define load_timer(v) \
	outportb(0x58, v & 0xFF); \
	outportb(0x58, (v >> 8) & 0xFF);

interrupt_handler install_timer_handler(interrupt_handler next);

#endif /* _TIMER_H */
