;**** A P P L I C A T I O N   N O T E   A V R 2 4 0 ************************
;* 
;* Title:		4x4 keypad, wake-up on keypress
;* Version:		1.2
;* Last Updated:	2004.11.11
;* Target:		All AVR Devices 
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION
;* This Application note scans a 4 x 4 keypad and uses sleep mode
;* causing the AVR to wake up on keypress.  The design uses a minimum of
;* external components. Included is a test program that wakes up the AVR
;* and performs a scan when a key is pressed and flashes one of two LEDs 
;* the number of the key pressed.  The external interrupt line is used for
;* wake-up.  The example runs on the AT90S1200 but can be any AVR with suitable
;* changes in vectors, EEPROM and stack pointer. The timing assumes a 4 MHz clock.
;* A look up table is used in EEPROM to enable the same structure to be used
;* with more advanced programs e.g ASCII output to displays.
;***************************************************************************

;***** Register used by all programs
;******Global variable used by all routines

.def	temp	=r16	;general scratch space

;Port B pins

.equ	ROW1	=3	;keypad input rows
.equ	ROW2	=2
.equ	ROW3	=1
.equ	ROW4	=0
.equ	COL1	=7	;keypad output columns
.equ	COL2	=6
.equ	COL3	=5	
.equ	COL4	=4	

;Port D pins

.equ	GREEN	=0	;green LED
.equ	RED	=1	;red LED
.equ	INTR	=2	;interrupt input

.include "1200def.inc"

;***** Registers used by interrupt service routine


.def	key	=r17	;key pointer for EEPROM
.def	status	=r21	;preserve sreg here

;***** Registers used by delay subroutine
;***** as local variables

.def	fine	=r18	;loop delay counters
.def	medium	=r19	
.def 	coarse	=r20

;*****Look up table for key conversion******************************
.eseg					;EEPROM segment
.org 0

	.db	1,2,3,15,4,5,6,14,7,8,9,13,10,0,11,12
;****Source code***************************************************
.cseg					;CODE segment
.org 0
		rjmp reset		;Reset handler
		rjmp scan		;interrupt service routine	
		reti			;unused timer interrupt
		reti			;unused analogue interrupt

;*** Reset handler **************************************************
reset:		
		
		ldi temp,0xFB		;initialise port D as O/I
		out DDRD,temp		;all OUT except PD2 ext.int.
		ldi temp,0x30		;turn on sleep mode and power
		out MCUCR,temp		;down plus interrupt on low level.
		ldi temp,0x40		;enable external interrupts
		out GIMSK,temp
		sbi ACSR,ACD		;shut down comparator to save power
main:		cli			;disable global interrupts 
		ldi temp,0xF0		;initialise port B as I/O
		out DDRB,temp		; 4 OUT  4 IN
		ldi temp,0x0F		;key columns all low and
		out PORTB,temp		;active pull ups on rows enabled
		ldi temp,0x07		;enable pull up on PD2 and
		out PORTD,temp		;turn off LEDs
		sei			;enable global interrupts ready
		sleep			;fall asleep
		rcall flash		;flash LEDs for example usage
		ldi temp,0x40
		out GIMSK,temp		;enable external interrupt
		rjmp main		;go back to sleep after keyscan

;****Interrupt service routine***************************************
scan:		
		in status,SREG		;preserve status register
		sbis PINB,ROW1		;find row of keypress
		ldi key,0		;and set ROW pointer
		sbis PINB,ROW2
		ldi key,4
		sbis PINB,ROW3
		ldi key,8
		sbis PINB,ROW4
		ldi key,12
		ldi temp,0x0F		;change port B I/O to
		out DDRB,temp		;find column press
		ldi temp,0xF0		;enable pull ups and
		out PORTB,temp		;write 0s to rows
		rcall settle		;allow time for port to settle
		sbis PINB,COL1		;find column of keypress
		ldi temp,0		;and set COL pointer
		sbis PINB,COL2
		ldi temp,1
		sbis PINB,COL3
		ldi temp,2
		sbis PINB,COL4
		ldi temp,3
		add key,temp		;merge ROW and COL for pointer
		ldi temp,0xF0		;reinitialise port B as I/O
		out DDRB,temp		; 4 OUT  4 IN
		ldi temp,0x0F		;key columns all low and
		out PORTB,temp		;active pull ups on rows enabled
		out SREG,status		;restore status register

		ldi temp,0x00
		out GIMSK,temp 		;disable external interrupt
					;have to do this, because we're
					;using a level-triggered interrupt

		reti			;go back to main for example program

;***Example test program to flash LEDs using key press data************

flash:		out EEAR,key		;address EEPROM
		sbi EECR,EERE		;strobe EEPROM
		in temp,EEDR		;set number of flashes
		tst temp		;is it zero?
		breq zero		;do RED LED
green_flash:
			cbi PORTD,GREEN	;flash green LED 'temp' times
			rcall delay
			sbi PORTD,GREEN
			rcall delay
			dec temp
			brne green_flash
exit:		ret
zero:		ldi temp,10
flash_again:		cbi PORTD,RED	;flash red LED ten times
			rcall delay
			sbi PORTD,RED
			rcall delay
			dec temp
			brne flash_again
		rjmp exit

		
;****Time Delay Subroutine for LED flash*********************************
delay:
	ldi coarse,8			;triple nested FOR loop
cagain:		ldi medium,255		;giving about 1/2 second
magain:			ldi fine,255	;delay on 4 MHz clock
fagain:			dec fine
			brne fagain
		dec medium
		brne magain
	dec coarse
	brne cagain
	ret

;***Settling time delay for port to stabilise******************************	
settle:
	ldi temp,255
tagain:		dec temp
		brne tagain
	ret
