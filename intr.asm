; void interrupt timer() {
; 	load_timer(91);
; 	counter++;
; 	/* Acknowledge interrupt */
; 	outportb(0x0, 0x20);
; }

; VGM specifies a 44100Hz "sample rate" upon which all operations are timed.
; But since our PIT is running at 250KHz, we instead count time at 1/16th that
; rate. The reload value is chosen to try to meet that timing, but due to
; factors I don't totally understand (but are probably related to how long it
; takes to enter an interrupt), we use a smaller value than the expected 91.
;
; This would all be unnecessary if we could use the rate generator or square
; wave mode to generate precise periodic timing, but the PIC is set to level
; sensitive mode.

RELOAD	EQU 80

_TEXT	segment	byte public 'CODE'
	assume	cs:_TEXT,ds:DGROUP,ss:DGROUP

	public _timer
_timer	proc	far
	push	ax
	; reset counter - low, then high
	mov	al, RELOAD
	out	58h, al
	mov	al, 0
	out	58h, al
	; increment counter
	push	ds
	mov	ax, DGROUP
	mov	ds, ax
	inc	word ptr DGROUP:_counter
	; acknowledge interrupt
	mov	al, 20h
	out	0, al
	pop	ds
	pop	ax
	iret	
_timer	endp

_TEXT	ends

DGROUP	group	_DATA,_BSS

_DATA	segment word public 'DATA'
extrn	_counter:word
_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

	end
