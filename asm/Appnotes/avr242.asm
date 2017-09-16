;**** A P P L I C A T I O N   N O T E   A V R ??? ************************
;* 
;* Title:		Multiplexing LED drive and 4x4 keypad sampling
;* Version:		1.0
;* Last Updated:	98.07.24
;* Target:		All AVR Devices 
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION
;* This Application note covers a program to provide a 24 hr Industrial
;* timer or real-time clock using I/O pins for dual functions.
;* With input via a 4 x 4 matrix keypad, output to a multiplexed
;* four digit LED display and two ON/OFF outputs to drive loads via additional 
;* interface circuitry.  LED loads are driven in this example but it could drive 
;* any load with the addition of suitable components. Tactile feedback is provided
;* on every key press by a piezo sounder which beeps when a key is pressed.
;* Included is a main program that allows clock setting via the keypad
;* and one ON/OFF time setting per 24 hours for each load, functions for the 
;* real time clock, key scanning, and adjustment routines. The example runs on 
;* the AT90S1200 to demonstrate how limited I/O  can be overcome, but can 
;* be any AVR with suitable changes in vectors, EEPROM and stack pointer. 
;* The timing assumes a 4.096 MHz crystal is employed (a 4 MHz crystal produces 
;* an error of -0.16% if 178 instead of 176 used in the timer load sequence, but this
;* could be adjusted in software at regular intervals). Look up tables are 
;* used in EEPROM to decode the display data, with additional characters provided 
;* for time and ON/OFF setting displays and a key pad conversion table.  
;* If the EEPROM is needed for your application the tables could be moved 
;* to ROM in the larger AVR devices.
;***************************************************************************

;***** Registers used by all programs
;******Global variables used by routines

.def	loset	=r1	;storage for timeset minutes
.def	hiset	=r2	;storage for timeset hours	
.def	ld1minon=r3	;storage for load on and off times
.def	ld1hron	=r4	;set from keypad entry
.def	ld1minoff=r5	;and tested in the housekeeping function
.def	ld1hroff=r6	;and stores on or off times for the loads
.def	ld2minon=r7
.def	ld2hron	=r8
.def	ld2minoff=r9
.def	ld2hroff=r10

.def	temp	=r16	;general scratch space
.def	second	=r17	;storage for RTC second count
.def 	minute	=r18	;storage for RTC minute count	
.def	hour	=r19	;storage for RTC hour count
.def	mask	=r20	;flash mask for  digits flashing
.def 	blink	=r21	;colon blink rate counter
.def	bounce	=r22	;keypad debounce counter
.def 	flash	=r23	;flash delay counter 
.def	lobyte	=r24	;storage for display function minutes digits
.def	hibyte	=r25	;storage for display function hours digits	
.def	key	=r26	;key number from scan

;***'key' values returned by 'keyscan'***************************
;VALUE	0   1	2   3	4   5	6   7	 8   9  10  11   12 13  14   15  16
;KEY	1   2   3   F   4   5   6   E    7   8   9   D    A  0  B    C   NONE  
;FUNC	1   2   3 LD1ON 4   5   6 LD1OFF 7   8   9 LD2ON SET 0 CLEAR LD2OFF

.def	tock	=r27	;5 ms pulse 
.def	flags	=r28	;flag byte for keypad command keys
			;  7     6     5    4      3      2     1    0
			;  5ms  keyok ld2off ld2on ld1off ld1on ld2 ld1 
			; tick      0 = off, 1 = on
.equ	ms5	=7	;ticks at 5 ms intervals for display time
.equ	keyok	=6	;sets when key is debounced, must be cleared again
.equ	ld2off	=5	;set by load ON/OFF key press and flags
.equ	ld2on	=4	;up the need for action
.equ 	ld1off	=3	;in the housekeeping routine
.equ	ld1on	=2
.equ	ld2	=1	;when set tells the housekeeping routine to
.equ	ld1	=0	;check load on/off times.

;***the T flag in the status register is used as a SET flag for time set
.equ	clear	=0	;RTC modification demand flag


;Port B pins

.equ	col1	=0	;LED a segment/keypad col 1
.equ	col2	=1	;LED b segment/keypad col 2
.equ	col3	=2	;LED c segment/keypad col 3
.equ	col4	=3	;LED d segment/keypad col 4
.equ	row1	=4	;LED e segment/keypad row 1
.equ	row2	=5	;LED f segment/keypad row 2
.equ	row3	=6	;LED g segment/keypad row 3
.equ	row4	=7	;LED decimal point/keypad row 4

;Port D pins

.equ	A1	=0	;common anode drives (active low)
.equ	A2	=1	;
.equ	A3	=2	;
.equ	A4	=3	;
.equ	LOAD1	=4	;Load 1 output (active low)
.equ	LOAD2	=5	;Load 2 output (active low)
.equ	PZ	=6	;Piezo sounder output (active low)

.include "1200def.inc"

;***** Registers used by timer overflow interrupt service routine


.def	timer	=r31	;scratch space for timer loading
.def 	status 	=r0	;low register to preserve status register

;*****Look up table for LED display decoding **********************
.eseg					;EEPROM segment
.org 0

table1:	.db	0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90
	;digit	 0    1    2    3    4    5    6    7    8    9    
	
	.db	0x86,0x8E,0xA3,0xAB,0XFF,0XFF 
	;digit	 E     f   o    n       BLANK       special characters 

;****Look up table for key value conversion into useful numbers****

	;key	1  2  3  F  4  5  6  E  7  8  9   D    A  0   B   C
table2:	.db	1, 2, 3,15, 4, 5, 6,14, 7, 8, 9,  13, 10, 0, 11, 12
	;value	0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15
 
;****Source code***************************************************
.cseg					;CODE segment
.org 0
		rjmp reset		;Reset handler
		nop			;unused ext. interrupt
		rjmp tick		;timer counter overflow (5 ms)
		nop			;unused analogue interrupt

;*** Reset handler **************************************************
;*** to provide initial port, timer and interrupt setting up

reset:		
		ser temp		;  
		out DDRB,temp		;initialize port B as all Outputs
		out DDRD,temp		;initialize port D as all Outputs
		out PORTB,temp		;key columns all high/LEDs off 
		out PORTD,temp		;turn off LEDs and loads off 
		ldi temp,0x04		;timer prescalar /256 
		out TCCR0,temp
		ldi timer,176		;load timer for 5 ms
		out TCNT0,timer		;(256 - n)*256*0.2441 us
		ldi temp,0x02		;enable timer interrupts
		out TIMSK,temp
		clr flags		;clear control flags
		clr tock		;clear 5 ms tick
		clr bounce		;clear key bounce counter
		clr flash
		clr blink
		sei			;enable global interrupts 

;****Flash EEEE on LEDS as test and power down warning**************
;****repeats until SET key is pressed on keypad
		
		
timesetting:	ldi hibyte,0xaa		;show "EEEE" on LED
		ldi lobyte,0xaa		;display and
		ser mask		;set flashing display
notyet:		rcall display		;display until time set
		brtc notyet		;repeat until SET key pressed
		rcall setrtc		;and reset time		
		mov hour,hiset		;and reload hours	
		mov minute,loset	;and minutes	
		clt			;clear T flag 	
				
;*****Main clock house keeping loop*****************************

do:		
		clr mask		;do housekeeping
		cpi blink,100		;is half second up
		brne nohalf
		clr blink
		com flash		;invert flash 
nohalf:		
		cpi second,60		;is one minute up?
		brne nochange		;no
		clr second		;yes clear seconds and
		inc minute		;add one to minutes
		mov temp,minute
		andi temp,0x0f		;mask high minute
		cpi temp,10		;is it ten minutes?
		brne nochange		;no
		andi minute,0xf0	;clear low minutes
		ldi temp,0x10
		add minute,temp		;increment high minutes
		cpi minute,0x60		;is it 60 minutes?
		brne nochange		;no
		clr minute		;yes, clear minutes and
		inc hour		;add one to hours
		mov temp,hour
		andi temp,0x0f		;mask high hour
		cpi temp,10		;is 10 hours up?
		brne nochange		;no
		andi hour,0xf0		;yes, increment
		ldi temp,0x10
		add hour,temp		;high hours
nochange:
		cpi hour,0x24		;is it 24 hours?		
		brne sameday		;no,
		clr hour		;yes, clear time variables
		clr minute		;to start new day
		clr second		
sameday:				;update times
		mov lobyte,minute
		mov hibyte,hour
		rcall display		;show time for 20 ms
		brtc case1		;if not SET
		rcall setrtc		;and reset time		
		mov hour,hiset		;and reload hours	
		mov minute,loset	;and minutes	
		clt			;else, clear T flag  	
case1:		sbrc flags,ld1		;is load 1 active?
		rjmp chkload1		;yes, check load 1
case2:		sbrc flags,ld2		;is load 2 active
		rjmp chkload2		;yes, check load 2
case3:		sbrc flags,ld1on	;is load 1 on time reset
		rjmp setld1on		;yes reset on time
case4:		sbrc flags,ld1off	;is load 1 off time reset
		rjmp setld1off		;yes reset off time
case5:		sbrc flags,ld2on	;is load 2 on time reset
		rjmp setld2on		;yes reset on time
case6:		sbrc flags,ld2off	;is load 2 on time reset
		rjmp setld2off		;yes reset on time
case7:		rjmp do			;repeat housekeeping loop

;****case routines to service load times and key presses********
		

chkload1:	cp hour,ld1hroff	;is load 1 off time reached?
		brne onload1
		cp minute,ld1minoff
		brne onload1
		sbi PORTD,LOAD1		;yes, turn load 1 off
onload1:		
		cp hour,ld1hron		;is load 1 on time reached?
		brne case2
		cp minute,ld1minon
		brne case2
		cbi PORTD,LOAD1		;yes,turn load 1 on
		rjmp case2		;repeat with load on

chkload2:	cp hour,ld2hroff	;is load 2 off time reached?
		brne onload2
		cp minute,ld2minoff
		brne onload2
		sbi PORTD,LOAD2		;yes, turn load 2 off
onload2:	
		cp hour,ld2hron		;is load 2 on time reached?
		brne case3
		cp minute,ld2minon
		brne case3
		cbi PORTD,LOAD2		;yes,turn load 2 on
		rjmp case3		;repeat with load on
setld1on:
		sbr flags,0x01		;make load 1 active
		rcall setrtc		;pickup new on time
		mov ld1hron,hiset	;and store
		mov ld1minon,loset
		cbr flags,0x04		;clear ld1on flag
		rjmp case4

setld1off:
		
		rcall setrtc		;pickup new off time
		mov ld1hroff,hiset	;and store
		mov ld1minoff,loset
		cbr flags,0x08		;clear ld1off flag
		rjmp case5
setld2on:
		sbr flags,0x02		;make load 2 active
		rcall setrtc		;pickup new on time
		mov ld2hron,hiset	;and store
		mov ld2minon,loset
		cbr flags,0x10		;clear ld2on flag
		rjmp case6
setld2off:
		
		rcall setrtc		;pickup new on time
		mov ld2hroff,hiset	;and store
		mov ld2minoff,loset
		cbr flags,0x20		;clear ld2off flag
		rjmp case7

		
;****Multiplexing routine to display time and scan keypad every*****
;****second pass,used by all routines taking digits from hibyte 
;****and lobyte locations with each digit on for 5 ms

display:	ser temp		;clear display
		out PORTB,temp
;****Keypad scanning routine to update key flags*******************

keyscan:	cbr flags,0x40		;clear keyok flag
		ldi key,0x10		;set no key pressed value
		ser temp		;set keypad port high prior to
		out PORTB,temp		;reinitializing the port
		in temp,PORTD		;turn off LEDs and leave loads
		ori temp,0x0f		;untouched prior to
		out PORTD,temp		;key scan
		ldi temp,0x0f		;set columns output and
		out DDRB,temp		;rows input with pull-ups
		ldi temp,0xf0		;enabled and all columns
		out PORTB,temp		;low ready for scan
		ldi temp,20		;short settling time
tagain1:	dec temp
		brne tagain1
		sbis PINB,ROW1		;find row of keypress
		ldi key,0		;and set ROW pointer
		sbis PINB,ROW2
		ldi key,4
		sbis PINB,ROW3
		ldi key,8
		sbis PINB,ROW4
		ldi key,12
		ldi temp,0xF0		;change port B I/O to
		out DDRB,temp		;find column press
		ldi temp,0x0F		;enable pull ups and
		out PORTB,temp		;write 0s to rows
		ldi temp,20		;short settling time
tagain2:	dec temp
		brne tagain2		;allow time for port to settle
		clr temp
		sbis PINB,COL1		;find column of keypress
		ldi temp,0		;and set COL pointer
		sbis PINB,COL2
		ldi temp,1
		sbis PINB,COL3
		ldi temp,2
		sbis PINB,COL4
		ldi temp,3
		add key,temp		;merge ROW and COL for pointer
		cpi key,0x10		;if no key pressed 
		breq nokey		;escape routine, else
		ldi temp,0x10
		add key,temp		;change to table 2
		out EEAR,key		;send address to EEPROM (0 - 15)
		sbi EECR,EERE		;strobe EEPROM
		in key,EEDR		;read decoded number for true key
convert:	cpi key,10		;is it SET key ?
		brne notset		;no check next key
		set			;yes set T flag in status register
notset:		cpi key,11		;is key CLEAR?
		brne notclear		;no, check next key
		sbi PORTD,LOAD1		;yes, shut down all loads
		sbi PORTD,LOAD2
		cbr flags,0x03		;deactivate both loads
notclear:	cpi key,15		;is key LD1ON?
		brne notld1on		;no, check next key
		sbr flags,0x04		;yes, set LD1ON flag
notld1on:	cpi key,14		;is key LD1OFF?
		brne notld1off		;no, check next key
		sbr flags,0x08		;yes, set LD1OFF flag
notld1off:	cpi key,13		;is key LD2ON?
		brne notld2on		;no, check next key
		sbr flags,0x10		;yes, set LD2ON flag
notld2on:	cpi key,12		;is key LD2OFF?
		brne notld2off		;no, check next key
		sbr flags,0x20		;yes, set LD2OFF flag
notld2off:
	
;***Tactile feedback note generation routine*****************
;***provides a 4 kHz TONE to the piezo sounder for 5 ms*****

tactile:	cbr flags,0x80
		cbi PORTD,PZ		;turn on piezo
		ldi temp,125		;for a short time
t1again:	dec temp
		brne t1again
		sbi PORTD,PZ		;turn on piezo
		ldi temp,125		;for a short time
t2again:	dec temp
		brne t2again
		sbrs flags,ms5		;repeat for 5ms
		rjmp tactile	
notok:		cpi bounce,40
		brlo nokey
		sbr flags,0x40		;set bounce flag
nokey:		ser temp	
		out DDRB,temp		;reinitialize port B as all Outputs
		out PORTB,temp		;and clear LEDs

;***Display routine to multiplex all four LED digits****************

		cbi PORTD,A1		;turn digit 1 on
		mov temp,lobyte		;find low minute
digit1:	
		cbr flags,0x80		;clear 5 ms tick flag
		andi temp,0x0f		;mask high nibble of digit
		out EEAR,temp		;send address to EEPROM (0 - 15)
		sbi EECR,EERE		;strobe EEPROM
		in temp,EEDR		;read decoded number
		sbrs flash,clear	;flash every 1/2 second
		or temp,mask		;flash digit if needed
		out PORTB,temp		;write to LED for 5 ms
led1:		sbrs flags,ms5		;5 ms finished?
		rjmp led1		;no, check again
		sbi PORTD,A1		;turn digit 1 off
		ser temp		;clear display
		out PORTB,temp
		cbi PORTD,A2		;
		mov temp,lobyte		;find high minute
		swap temp
digit2:	
		cbr flags,0x80		;clear 5 ms tick flag
		andi temp,0x0f		;mask high nibble of digit
		out EEAR,temp		;send address to EEPROM (0 - 15)
		sbi EECR,EERE		;strobe EEPROM
		in temp,EEDR		;read decoded number
		sbrs flash,clear	;flash every 1/2 second
		or temp,mask		;flash digit if needed
		out PORTB,temp		;write to LED for 5 ms
led2:		sbrs flags,ms5		;5 ms finished?
		rjmp led2		;no, check again
		sbi PORTD,A2		;
		ser temp		;clear display
		out PORTB,temp
		cbi PORTD,A3		;
		mov temp,hibyte
digit3:	
		cbr flags,0x80		;clear 5 ms tick flag
		andi temp,0x0f		;mask high nibble of digit
		out EEAR,temp		;send address to EEPROM (0 - 15)
		sbi EECR,EERE		;strobe EEPROM
		in temp,EEDR		;read decoded number
		sbrs second,clear	;flash colon
		andi temp,0x7f
		sbrs flash,clear	;flash every 1/2 second
		or temp,mask		;flash digit if needed
		out PORTB,temp		;write to LED for 5 ms
led3:		sbrs flags,ms5		;5 ms finished?
		rjmp led3		;no, check again
		sbi PORTD,A3
		ser temp		;clear display
		out PORTB,temp
		cbi PORTD,A4		;
		mov temp,hibyte
		swap temp
		andi temp,0x0f		;is hi hour zero?
		brne digit4
		ldi temp,0xff		;yes,blank hi hour
digit4:	
		cbr flags,0x80		;clear 5 ms tick flag
		andi temp,0x0f		;mask high nibble of digit
		out EEAR,temp		;send address to EEPROM (0 - 15)
		sbi EECR,EERE		;strobe EEPROM
		in temp,EEDR		;read decoded number
		sbrs flash,clear	;flash every 1/2 second
		or temp,mask		;flash digit if needed
		out PORTB,temp		;write to LED for 5 ms
led4:		sbrs flags,ms5		;5 ms finished?
		rjmp led4		;no, check again
		sbi PORTD,A4
		ser temp		;clear display
		out PORTB,temp
		tst mask		;is flash complete?
		breq outled		;yes, exit
		cpi blink,50		;is blink time done?
		brlo outled		;no, exit 
		clr blink		;yes, clear blink rate counter
		com flash		;and invert flash byte
outled:		ret



		
;****Function to Set RTC/on-off hours and minutes from keypad
;****returns with minutes in 'loset' and hours in'hiset'
		
setrtc:		ser mask		;set flashing display
		ldi hibyte,0xdf		;place 'n' in hi hour
		ser lobyte		;and blank in lo hr & minutes
hihrus:		clr bounce
bounce1:	rcall display		;display and check keypad
		sbrs flags,keyok
		rjmp bounce1
		cbr flags,0x40		;clear keyok flag
		cpi key,0x03		;is high hour > 2
		brsh hihrus		;yes, read key again
hihrok:					;no, valid entry
		swap key		;move hihour to hi nibble
		mov hiset,key		;and store in hours
		ldi hibyte,0x0d		;place 'n' in lo hour
		add hibyte,hiset	;merge hihour and 'n'
lohrus:		clr bounce
bounce2:	rcall display		;display and check keypad
		sbrs flags,keyok	;is key stable?
		rjmp bounce2		;no try again
		cbr flags,0x40		;yes, clear keyok flag
		mov temp,hibyte		;check that total hours
		andi temp,0xf0		;are not > 24
		add temp,key
		cpi temp,0x24		;is hour>24?
		brsh lohrus		;yes, read key again
		add hiset,key		;no, merge hi and lo hours
lohrok:		
		mov hibyte,hiset	;display hours as set
		ldi lobyte,0xdf		;place 'n' in hi minutes
himinus:	clr bounce
bounce3:	rcall display		;display and check keypad
		sbrs flags,keyok
		rjmp bounce3
		cbr flags,0x40		;clear keyok flag
		cpi key,6		;is hi minutes >5
		brsh himinus		;no, read key again
lominok:	
		swap key		;move himin to hi nibble
		mov loset,key		;and store in minutes
		ldi lobyte,0x0d		;place 'n' in lo minutes
		add lobyte,loset	;merge with hi minute
lominus:	clr bounce
bounce4:	rcall display		;display and check keypad
		sbrs flags,keyok
		rjmp bounce4
		cbr flags,0x40		;clear keyok flag
		cpi key,10		;is key >9
		brsh lominus		;no, read key again
		add loset,key		;yes, merge hi and lo minutes
		clr mask		;clear digits flash
		ret			;and return with time set

;****Timer Overflow Interrupt service routine******************************
;****Updates 5 ms, flash and debounce counter to provide RTC time reference

tick:		
		in status,SREG		;preserve status register
		inc tock		;add one to 5 ms 'tock' counter
		inc blink		;and blink rate counter
		inc bounce		;and bounce rate delay
		sbr flags,0x80		;set 5 ms flag for display time
		cpi tock,200		;is one second up?
		breq onesec		;yes, add one to seconds
		nop			;balance interrupt time
		rjmp nosecond		;no, escape
onesec:		inc second		;add one to seconds
		clr tock		;clear 5 ms counter
nosecond:	ldi timer,176		;reload timer 
		out TCNT0,timer
		out SREG,status		;restore status register
		reti			;return to main

