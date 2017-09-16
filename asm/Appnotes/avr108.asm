;**** A P P L I C A T I O N   N O T E   A V R 1 0 8 ************************
;*
;* Title:		Load Program Memory
;* Version:		1.0
;* Last updated:	98.02.27
;* Target:		AT90Sxx1x and higher (Devices with SRAM)
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION
;* This Application note shows how to use the Load Program Memory (LPM)
;* instruction. The App. note loads the string "Hello World" from 
;* program memory byte by byte, and puts it onto port B.
;*
;***************************************************************************


.include "8515def.inc"

.device AT90S8515			; Specify device

.def	temp	=r16			; Define temporary variable


start:	ldi	temp,low(RAMEND)
	out	SPL,temp		; Set stack pointer to last internal RAM location
	ldi	temp,high(RAMEND)
	out	SPH,temp

	ldi	temp,$ff
	out	PORTB,temp		; Set all pins at port B high
	out	DDRB,temp		; Set port B as output

	; Load the address of 'message' into the Z register. Multiplies
	; word address with 2 to achieve the byte address, and uses the
	; functions high() and low() to calculate high and low address byte.

	ldi	ZH,high(2*message)	; Load high part of byte address into ZH
	ldi	ZL,low(2*message)	; Load low part of byte address into ZL

loadbyte:
	lpm				; Load byte from program memory into r0

	tst	r0			; Check if we've reached the end of the message
	breq	quit			; If so, quit

	out	PORTB,r0		; Put the character onto Port B

	rcall	one_sec_delay		; A short delay

	adiw	ZL,1			; Increase Z registers
	rjmp	loadbyte


quit:	rjmp quit


one_sec_delay:
	ldi	r20, 20
	ldi	r21, 255
	ldi	r22, 255
delay:	dec	r22
	brne	delay
	dec	r21
	brne	delay
	dec	r20
	brne	delay
	ret


message:
.db	"Hello World"
.db	0

