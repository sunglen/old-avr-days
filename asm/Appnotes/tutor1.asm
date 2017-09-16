;***************************************************************************
;* A P P L I C A T I O N   N O T E   F O R   T H E   A V R   F A M I L Y
;* 
;* Number		:AVR900
;* File Name            :"tutor1.asm"
;* Title                :Assembler and Simulator Tutorial
;* Date                 :99.01.29
;* Version              :1.3
;* Support email        :avr@atmel.com
;* Target MCU           :AT90S8515
;*
;* DESCRIPTION
;*
;* Tutorial program for the AVR Assembler and Simulator.
;* The program cointains a simple routine which compares two 16 bit numbers
;* and returns the greater of the two. This routine is accessed three times
;* by the main program.
;* WARNING: The program contains intentional errors for tutorial purposes.
;*
;***************************************************************************
;.device	AT90S8515	;Prohibits use of non-implemented instructions

.include "8515def.inc"


;***** Global Register Variables


;***** Code

	rjmp	RESET		;Reset Handle

;***************************************************************************
;*
;* Subroutine "comp16s"
;*
;* This routine compares the two 16 bit signed values A (AH:AL) and 
;* B (BH:BL) and returns the greater value in (AH:AL)
;*
;* Number of words	:6
;* Number of cycles	:8/9 (min/max) 
;* Low registers used	:None
;* High registers used  :4 (AH,AL,BH,BL)	
;*
;***************************************************************************

;***** Subroutine Register Variables

.def	AL	=r16
.def	AH	=r17
.def	BL	=r18
.def	BH	=r32

;***** Code

comp16s:
	cp	BH,AH		;Compare low bytes
	cpc	BL,AL		;Compare high bytes
        brlt    c16s1           ;if B >= A
	mov	AH,BH		;    AH=BH
	mov	AL,BL		;    AL=BL
c16s1:	ret


;****************************************************************************
;*
;* Main Program
;*
;* This program calls the routine "comp16s" three times for comparison of
;* three 16 bit signed number pairs.
;*
;* 1. $-0fe3 and $2040 is compared and the greater value is moved to G1H,G1L
;* 2. $5000 and $4fbc is compared and the greater value is moved to G2H,G2L
;* 3. $-7f34 and $-3e12 is compared and the greater value is moved to G3H,G3L
;*
;***************************************************************************

;***** Main Program Register Variables

.def	G1L	=r0
.def	G1H	=r1
.def	G2L	=r2
.def	G2H	=r3
.def	G3L	=r4
.def	G3H	=r5

;***** Code

RESET:

	ldi 	R16,low(RAMEND)		; Load low byte address of end of RAM into register R16
	out	SPL,R16			; Initialize stack pointer to end of internal RAM
	ldi 	R16,high(RAMEND)	; Load high byte address of end of RAM into register R16	
	out 	SPH, R16		; Initialize high byte of stack pointer to end of internal RAM

	ldi	AL,low(-$fe3)	
	ldi	AH,high(-$fe3)	;A=-$fe3
	ldi	BL,$40	
	ldi	BH,$20		;B=$2040
	rcall   comp16s		;Compare
	mov	G1L,AL		;Move low byte
	mov	G1H,AH		;Move high byte	

	ldi	AL,$00	
	ldi	AH,$50		;A=$5000
	ldi	BL,$bc	
	ldi	BH,$4f		;B=$4fbc
	rcall   comp16s		;Compare
	mov	G2L,AL		;Move low byte
	mov	G2H,AH		;Move high byte	

	ldi	AL,low(-$7f34)	
	ldi	AH,high(-$7f34)	;A=-$7f34
	ldi	BL,low(-$3e12)	
	ldi	BH,high(-$3e12)	;B=-$3e12
	rcall   comp16s		;Compare
	mov	G3L,AL		;Move low byte
	mov	G3H,AH		;Move high byte	

forever:rjmp	forever		;Loop forever

