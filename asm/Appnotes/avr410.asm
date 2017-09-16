;***************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number		: AVR410
;* File Name            :"rc5.asm"
;* Title                :RC5 IR Remote Control Decoder
;* Date                 :97.08.15
;* Version              :1.0
;* Target MCU           :AT90S1200
;*
;* DESCRIPTION
;* This Application note describes how to decode the frequently used
;* RC5 IR remote control protocol.
;*
;* The timing is adapted for 4 MHz crystal
;*
;***************************************************************************
.include "1200def.inc"
.device AT90S1200


.equ	INPUT		=2	;PD2
.equ	SYS_ADDR	=0	;The system address


.def	S	=R0
.def	inttemp	=R1
.def	ref1	=R2
.def	ref2	=R3

.def	temp	=R16

.def	timerL	=R17
.def	timerH	=R16

.def	system	=R19
.def	command	=R20

.def	bitcnt	=R21


.cseg
.org 0
		rjmp	reset

;********************************************************************
;* "TIM0_OVF" - Timer/counter overflow interrupt handler
;*
;* The overflow interrupt increments the "timerL" and "timerH"
;* every 64us and 16,384us.
;*
;* Number of words:	7
;* Number of cycles:	6 + reti
;* Low registers used:	1
;* High registers used: 3
;* Pointers used:	0
;********************************************************************
.org OVF0addr
TIM0_OVF:	in	S,sreg
		inc	timerL		;Updated every 64us
		inc	inttemp
		brne	TIM0_OVF_exit

		inc	timerH

TIM0_OVF_exit:	out	sreg,S
		reti




;********************************************************************
;* Example program
;*
;* Initializes timer, ports and interrupts.
;*
;* Calls "detect" in an endless loop and puts the result out on
;* port B.
;*
;* Number of words:	16
;* Low registers used:	0
;* High registers used: 3
;* Pointers used:	0
;********************************************************************

reset:		;ldi  	temp,low(RAMEND)	;Initialize stackpointer
		;out	SPL,temp			
		;ldi  	temp,high(RAMEND)	; Commented out since 1200 does not hae SRAM
		;out	SPH,temp
	
		ldi	temp,1		;Timer/Counter 0 clocked at CK
		out	TCCR0,temp

		ldi	temp,1<<TOIE0	;Enable Timer0 overflow interrupt
		out	TIMSK,temp

		ser	temp		;PORTB as output
		out	DDRB,temp

		sei			;Enable gobal iterrupt



main:		rcall	detect		;Call RC5 detect routine

		cpi	system,SYS_ADDR	;Respponds only at the specified address
		brne	release

		andi	command,0x3F	;Remove control bit
		out	PORTB,command

		rjmp	main


release:	clr	command		;Clear PORTB
		out	PORTB,command
		rjmp	main




;********************************************************************
;* "detect" - RC5 decode routine
;*
;* This subroutine decodes the RC5 bit stream applied on PORTD
;* pin "INPUT". 
;*
;* If successe: The command and system address are
;*		returned in "command" and "system".
;*		Bit 6 of "command" holds the toggle bit.
;*
;* If failed: 	$FF in both "system" and "command"
;*
;* Crystal frequency is 4MHz
;*
;* Number of words:	72
;* Low registers used:	 3
;* High registers used:  6
;* Pointers used:	 0
;********************************************************************
detect:		clr	inttemp
		clr	timerH

detect1:	clr	timerL

detect2:	cpi	timerH,8	;If line not idle within 131ms
		brlo	dl1
		rjmp	fault		;  then exit

dl1:		cpi	timerL,55	;If line low for 3.5ms
		brge	start1		;   then wait for start bit

		sbis	PIND,INPUT	;If line is
		rjmp	detect1		;   low  - jump to detect1
		rjmp	detect2		;   high - jump to detect2


start1:		cpi	timerH,8	;If no start bit detected
		brge	fault		;within 130ms then exit

		sbic	PIND,INPUT	;Wait for start bit
		rjmp	start1


		clr	timerL		;Measure length of start bit
		
start2:		cpi	timerL,17	;If startbit longer than 1.1ms,
		brge	fault		;   exit

		sbis	PIND,INPUT
		rjmp	start2
					;Positive edge of 1st start bit

		mov	temp,timerL	;timer is 1/2 bit time
		clr	timerL

		mov	ref1,temp
		lsr	ref1
		mov	ref2,ref1
		add	ref1,temp	;ref1 = 3/4 bit time
		lsl	temp
		add	ref2,temp	;ref2 = 5/4 bit time


start3:		cp	timerL,ref1	;If high periode St2 > 3/4 bit time
		brge	fault		;   exit

		sbic	PIND,INPUT	;Wait for falling edge start bit 2
		rjmp	start3

		clr	timerL
		ldi	bitcnt,12	;Receive 12 bits
		clr	command
		clr	system


sample:		cp	timerL,ref1	;Sample INPUT at 1/4 bit time
		brlo	sample

		sbic	PIND,INPUT
		rjmp	bit_is_a_1	;Jump if line high


bit_is_a_0:	clc			;Store a '0'
		rol	command
		rol	system

					;Synchronize timing
bit_is_a_0a:	cp	timerL,ref2	;If no edge within 3/4 bit time
		brge	fault		;   exit
		sbis	PIND,INPUT	;Wait for rising edge
		rjmp	bit_is_a_0a	;in the middle of the bit

		clr	timerL
		rjmp	nextbit

bit_is_a_1:	sec			;Store a '1'
		rol	command
		rol	system
					;Synchronize timing
bit_is_a_1a:	cp	timerL,ref2	;If no edge within 3/4 bit time
		brge	fault		;   exit
		sbic	PIND,INPUT	;Wait for falling edge
		rjmp	bit_is_a_1a	;in the middle of the bit

		clr	timerL

nextbit:	dec	bitcnt		;If bitcnt > 0
		brne	sample		;   get next bit


;All bits sucessfully received!
		mov	temp,command	;Place system bits in "system"
		rol	temp
		rol	system
		rol	temp
		rol	system

		bst	system,5	;Move toggle bit
		bld	command,6	;to "command"

					;Clear remaining bits
		andi	command,0b01111111
		andi	system,0x1F

		ret

fault:		ser	command		;Both "command" and "system"
		ser	system		;0xFF indicates failure
		ret
