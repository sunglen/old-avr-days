;**** A P P L I C A T I O N   N O T E   A V R 1 0 0  ************************
;*
;* Title:		Accessing the EEPROM
;* Version:		2.0
;* Last updated:	98.06.15
;* Target:		AT90Sxxxx (All AVR Devices)
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION
;* This Application note shows how to read data from and write data to the
;* EEPROM. Both random access and sequential access routines are listed.
;* The code is written for 8515, to modify for 4414,2313,2323... apply the 
;* following changes:
;*	- Remove all entries to EEARH
;*	- Rename EEARL to EEAR
;*
;* To modify for 1200, apply the changes above, and change the code
;* as commented inside the routines	
;*
;*
;***************************************************************************
.include "tn24def.inc"

	rjmp	RESET		;Reset Handle
	
;***************************************************************************
;* 
;* EEWrite
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* programs the EEPROM with register variable "EEdwr" at address "EEawr:EEawr"
;*
;* Number of words	:1200 ; 5 + return
;*			:8515 ; 7 + return
;* Number of cycles	:1200 ; 8 + return (if EEPROM is ready)
;*			:8515 ; 11 + return (if EEPROM is ready)
;* Low Registers used	:None
;* High Registers used:	;3 (EEdwr,EEawr,EEawrh)
;*
;***************************************************************************

;***** Subroutine register variables

.def	EEdwr	=r16		;data byte to write to EEPROM
.def	EEawr	=r17		;address low byte to write to
.def	EEawrh	=r18		;address high byte to write to

;***** Code

EEWrite:
	sbic	EECR,EEPE	;if EEPE not clear
	rjmp	EEWrite		;    wait more

;	out	EEAR,EEawr	;output address for 1200, commented out !

; the two following lines must be replaced with the line above if 1200 is used
	out 	EEARH,EEawrh	;output address high for 8515
	out	EEARL,EEawr	;output address low for 8515
		

	out	EEDR,EEdwr	;output data
	sbi 	EECR,EEMPE	;set master write enable, remove if 1200 is used	
	sbi	EECR,EEPE	;set EEPROM Write strobe
				;This instruction takes 4 clock cycles since
				;it halts the CPU for two clock cycles
	ret

;***************************************************************************
;* 
;* EERead
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* reads the register variable "EEdrd" from address "EEardh:EEard"
;*
;* Number of words	:1200 ; 5 + return
;*			:8515 ; 6 + return
;* Number of cycles	:1200 ; 8 + return (if EEPROM is ready)
;*			:8515 ; 9 + return (if EEPROM is ready)
;* Low Registers used	:1 (EEdrd)
;* High Registers used:	:2 (EEard,EEardh)
;*
;***************************************************************************

;***** Subroutine register variables

.def	EEdrd	=r0		;result data byte
.def	EEard	=r17		;address low to read from
.def	EEardh	=r18		;address high to read from

;***** Code

EERead:
	sbic	EECR,EEPE	;if EEPE not clear
	rjmp	EERead		;    wait more
;	out	EEAR,EEard	;output address for 1200, commented out !

; the two following lines must be replaced with the line above if 1200 is used
	out 	EEARH,EEardh	;output address high for 8515
	out	EEARL,EEard	;output address low for 8515


	sbi	EECR,EERE	;set EEPROM Read strobe
				;This instruction takes 4 clock cycles since
				;it halts the CPU for two clock cycles
	in	EEdrd,EEDR	;get data
	ret

		
;***************************************************************************
;* 
;* EEWrite_seq
;*
;* This subroutine increments the EEPROM address by one and waits until the 
;* EEPROM is ready for programming. It then programs the EEPROM with 
;* register variable "EEdwr_s".

;* Number of words	:1200 ; 7 + return
;*			:8515 ; 10 + return
;* Number of cycles	:1200 ; 10 + return (if EEPROM is ready)
;*			:8515 ; 15 + return (if EEPROM is ready)
;* Low Registers used	:None
;* High Registers used:	:3 (EEdwr_s,EEwtmp,EEwtmph)
;*
;***************************************************************************

;***** Subroutine register variables

.def	EEwtmp	=r24		;temporary storage of address low byte
.def	EEwtmph	=r25		;temporary storage of address high byte
.def	EEdwr_s	=r18		;data to write

;***** Code

EEWrite_seq:
	sbic	EECR,EEPE	;if EEPE not clear
	rjmp	EEWrite_seq	;   wait more

; Write sequence for 1200
;	in	EEwtmp,EEAR	;get address for 1200, commented out !
;	inc 	EEwtmp		;increment address 1200, commented out !
;	out	EEAR,EEwtmp	;output address 1200

; Write sequence for 8515, must be replaced with the lines above if 1200 is used
	in	EEwtmp,EEARL	;get address low 8515
	in	EEwtmph,EEARH	;get address high 8515	
 	adiw	EEwtmp,0x01	;increment address 8515
	out	EEARL,EEwtmp	;output address 8515
	out	EEARH,EEwtmph	;output address 8515
	

	out	EEDR,EEdwr_s	;output data
	sbi 	EECR,EEMPE	;set master write enable, remove if 1200 is used	
	sbi	EECR,EEPE	;set EEPROM Write strobe
				;This instruction takes 4 clock cycles since
				;it halts the CPU for two clock cycles
	ret

;***************************************************************************
;* 
;* EERead_seq
;*
;* This subroutine increments the address stored in EEAR and reads the 
;* EEPROM into the register variable "EEdrd_s".

;* Number of words	:1200 ; 5 + return
;*			:8515 ; 7 + return
;* Number of cycles	:1200 ; 8 + return (if EEPROM is ready)
;*			:8515 ; 11 + return (if EEPROM is ready)
;* Low Registers used	:1 (EEdrd_s)
;* High Registers used:	:2 (EErtmp,EErtmph)
;*
;***************************************************************************

;***** Subroutine register variables

.def	EErtmp	=r24		;temporary storage of low address
.def	EErtmph	=r25		;temporary storage of high address
.def	EEdrd_s	=r0		;result data byte

;***** Code

EERead_seq:
	sbic	EECR,EEPE	;if EEPE not clear
	rjmp	EERead_seq	;   wait more
; The above sequence for EEPE = 0 can be skipped if no write is initiated.

; Read sequence for 1200
;	in	EErtmp,EEAR	;get address for 1200, commented out !
;	inc 	EErtmp		;increment address 1200, commented out !
;	out	EEAR,EErtmp	;output address 1200

; Read sequence for 8515, must be replaced with the lines above if 1200 is used
	in	EErtmp,EEARL	;get address low 8515
	in	EErtmph,EEARH	;get address high 8515	
	adiw	EErtmp,0x01	;increment address 8515
	out	EEARL,EErtmp	;output address 8515
	out	EEARH,EErtmph	;output address 8515


	sbi	EECR,EERE	;set EEPROM Read strobe
				;This instruction takes 4 clock cycles since
				;it halts the CPU for two clock cycles
	in	EEdrd_s,EEDR	;get data
	ret



;****************************************************************************
;*
;* Test/Example Program
;*
;****************************************************************************

;***** Main Program Register variables

.def	counter	=r19
.def	temp	=r20

;***** Code

RESET:
;***** Initialize stack pointer
;* Initialize stack pointer to highest address in internal SRAM
;* Comment out for devices without SRAM

	ldi	r16,high(RAMEND) ;High byte only required if 
	out	SPH,r16	         ;RAM is bigger than 256 Bytes
	ldi	r16,low(RAMEND)	 
	out	SPL,r16


;***** Program a random location

	ldi	EEdwr,$aa
	ldi	EEawrh,$00
	ldi	EEawr,$10
	rcall	EEWrite		;store $aa in EEPROM location $0010

;***** Read from a random locations

	ldi	EEardh,$00
	ldi	EEard,$10
	rcall	EERead		;read address $10
	out	PORTB,EEdrd	;output value to Port B

;***** Fill the EEPROM address 1..64 with bit pattern $55,$aa,$55,$aa,...

	ldi	counter,63	;init loop counter
	clr 	temp
	out	EEARH,temp	;EEARH <- $00
	clr	temp		
	out	EEARL,temp	;EEAR <- $00 (start address - 1)

loop1:	ldi	EEdwr_s,$55	
	rcall	EEWrite_seq	;program EEPROM with $55
	ldi	EEdwr_s,$aa		
	rcall	EEWrite_seq	;program EEPROM with $aa
	dec	counter		;decrement counter
	brne	loop1		;and loop more if not done

;***** Copy 10 first bytes of EEPROM to r1-r11

	clr 	temp
	out	EEARH,temp	;EEARH <- $00
	ldi	temp,$00	
	out	EEARL,temp	;EEAR <- $00 (start address - 1)

	clr 	ZH
	ldi	ZL,1		;Z-pointer points to r1

loop2:	rcall	EERead_seq	;get EEPROM data
	st	Z,EEdrd_s	;store to SRAM
	inc	ZL
	cpi	ZL,12		;reached the end?
	brne	loop2		;if not, loop more

forever:rjmp	forever		;eternal loop
