; void interrupt timer() {
; 	load_timer(91);
; 	counter++;
; 	/* Acknowledge interrupt */
; 	outportb(0x0, 0x20);
; }

_TEXT	segment	byte public 'CODE'
	assume	cs:_TEXT,ds:DGROUP,ss:DGROUP

	public _timer
_timer	proc	far
	push	ax
	; reset counter - low, then high
	mov	al, 82
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
