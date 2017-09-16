;**** A P P L I C A T I O N   N O T E   A V R 236 ************************
;* 
;* Title:		CRC check of program memory
;* Version:		1.3 
;* Last updated:	11.11.2004
;* Target:		AT90Sxxx, ATtinyxxx, ATmegaxxx 
;*				(All AVR Devices with LPM instr, not AT90S1200)
;*
;* Support E-mail:	avr@atmel.com
;*
;* NOTE: Always check out Atmels web site, www.atmel.com for the latest and updated 
;* version of the software.
;*
;* DESCRIPTION
;* This application note describes how to perform CRC computation 
;* of code memory contents using a simple algoritm. 
;* To generate CRC checksum load the register "status" with 00 and call the routine 
;* "crc_gen". The resulting checksum is placed in the registers
;* byte_2(low byte) and byte_3(high byte).
;*
;* To check the CRC checksum load the register "status" with FF and call the routine 
;* "crc_gen". The resulting checksum is placed in the registers
;* byte_2(low byte) and byte_3(high byte). If the checksum is 00 the program code is
;* correct, if the checksum is non-zero an error has been introduced in the program code
;**************************************************************************


.include "8515def.inc"

;***** Constants

.equ		LAST_PROG_ADDR	= 0x1FFF	; Last program memory address(byte address)
.equ		CR	= 0x8005	; CRC divisor value

;**************************************************************************
;*
;*      PROGRAM START - EXECUTION STARTS HERE
;*
;**************************************************************************

;        .cseg

.org $0000
	rjmp RESET      ;Reset handle


;***************************************************************************
;*
;* "crc_gen" - Generation and checking of CRC checksum
;*
;* This subroutine generates the checksum for the program code.
;* 32 bits are loaded into 4 register, the upper 16 bits are XORed
;* with the divisor value each time a 1 is shifted into the carry flag 
;* from the MSB.
;*
;* If the status byte is 0x00,the routine will generate new checksum:
;* After the computing the code 16 zeros are
;* appended to the code and the checksum is calculated. 
;* 
;* If the status byte is different from 0X00, the routine will check if the current checksum is valid.
;* After the computing the code the original checksum are
;* appended to the code and calculated. The result is zero if no errors occurs
;* The result is placed in registers byte_2 and byte_3
;*  
;* Number of words	:44 + return
;* Number of cycles	:program memory size(word) * 175(depending on memory contens)
;* Low registers used	:6 (byte_0,byte_1,byte_2,byte_3)
;* High registers used  :7 (sizel,sizeh,crdivl,crdivh,count,status,zl,zh)	
;*
;***************************************************************************

;***** Subroutine Register Variables

.def	byte_0	= r0		; Lower byte of lower word
.def	byte_1	= r1		; Upper byte of lower word
.def	byte_2	= r2		; Lower byte of upper word
.def	byte_3	= r3		; Upper byte of upper word
.def	crc		= r4		; CRC checksum low byte
.def	crch		= r5		; CRC checksum high byte
.def	sizel 	= r17	; Program code size register
.def	sizeh 	= r18
.def	crdivl	= r19	; CRC divisor register
.def	crdivh	= r20
.def count	= r21	; Bit counter 
.def	status	= r22	; Status byte: generate(0) or check(1)

crc_gen:
	ldi 	sizel,low(LAST_PROG_ADDR) 	;Load last program memory address
	ldi 	sizeh,high(LAST_PROG_ADDR)	
	clr 	zl			;Clear Z pointer
	clr 	zh				
	ldi 	crdivh,high(CR);Load divisor value
	ldi	crdivl,low(CR)
	lpm				;Load first memory location
	mov	byte_3,byte_0	;Move to highest byte
	adiw	zl,0x01		;Increment Z pointer
	lpm				;Load second memory location
	mov	byte_2,byte_0		

next_byte:
	cp	zl,sizel		;Loop starts here
	cpc	zh,sizeh		;Check for end of code
	brge	end			;Jump if end of code
	adiw	zl,0x01			
	lpm				;Load high byte
	mov	byte_1,byte_0	;Move to upper byte
	adiw	zl,0x01		;Increment Z pointer
	lpm				;Load program memory location
	rcall 	rot_word	;Call the rotate routine
	rjmp	next_byte

end:	
	;ret	;uncomment this line if checksum is stored in last flash memory address.
	ldi	count,0x11		
	cpi	status,0x00		
	brne	check
	clr	byte_0		;Append 16 bits(0x0000) to
	clr	byte_1		;the end of the code for CRC generation
	rjmp	gen	
check:	
	mov	byte_0,crc		;Append the original checksum to
	mov	byte_1,crch    	;the end of the code for CRC checking
gen:	
	rcall	rot_word	;Call the rotate routine
	mov	crc,byte_2		
	mov	crch,byte_3
	ret				;Return to main prog
	
rot_word:
	ldi	count,0x11
rot_loop:
	dec 	count		;Decrement bit counter
	breq	stop			;Break if bit counter = 0
	lsl	byte_0		;Shift zero into lowest bit
	rol	byte_1		;Shift in carry from previous byte
	rol	byte_2		;Preceede shift
	rol	byte_3			
	brcc	rot_loop		;Loop if MSB = 0			
	eor 	byte_2,crdivl
	eor 	byte_3,crdivh	;XOR high word if MSB = 1
	rjmp	rot_loop
stop:	
	ret

;***************************************************************************
;* 
;* EERead_seq
;*
;* This routine reads the 
;* EEPROM into the global register variable "temp".
;*
;* Number of words	:4+ return
;* Number of cycles	:8 + return 
;* High Registers used	:4 (temp,eeadr,eeadrh,eedata)
;*
;***************************************************************************

.def	temp		= r16 
.def	eeadr	= r23
.def	eeadrh	= r24
.def	eedata	= r25

;***** Code

eeread:
	out	EEARH,eeadrh	;output address high byte
	out	EEARL,eeadr	;output address low byte
	sbi	EECR,EERE		;set EEPROM Read strobe
	in	eedata,EEDR	;get data
	ret

;***************************************************************************
;* 
;* EEWrite
;*
;* This subroutine waits until the EEPROM is ready to be programmed, then
;* programs the EEPROM with register variable "EEdwr" at address "EEawr"
;*
;* Number of words	:7 + return
;* Number of cycles	:13 + return (if EEPROM is ready)
;* Low Registers used	:None
;* High Registers used:	;4 (temp,eeadr,eeadr,eedata)
;*
;***************************************************************************

.def	temp		= r16 
.def	eeadr	= r23
.def	eeadrh	= r24
.def	eedata	= r25

eewrite:
	sbic	EECR,EEWE	;If EEWE not clear
	rjmp	EEWrite		;    Wait more
	out	EEARH,eeadrh	;Output address high byte
	out	EEARL,eeadr	;Output address low byte
	out	EEDR,eedata	;Output data
	sbi	EECR,EEMWE
	sbi	EECR,EEWE		;Set EEPROM Write strobe
	ret

;************************************************************************
;*
;*  Start Of Main Program
;*
.cseg
.def	crc		= r4		;Low byte of checksum to be returned
.def	crch		= r5		;High byte of checksum to be returned
.def	temp		= r16  
.def	status	= r22	;Status byte: generate(0) or check(1)
.def	eeadr	= r23
.def	eeadrh	= r24
.def	eedata	= r25

RESET:	
	ldi	r16,high(RAMEND)	;Initialize stack pointer	
	out	SPH,r16			;High byte only required if 
	ldi	r16,low(RAMEND)	;RAM is bigger than 256 Bytes
	out	SPL,r16

	ldi 	temp,0xff	
	out 	DDRB,temp		;Set PORTB as output
	out 	PORTB,temp	;Write 0xFF to PORTB
	clr 	status		;Clear status register, prepare for CRC generation
	rcall 	crc_gen
	
	ldi 	eeadr,0x01	;Set address low byte for EEPROM write
	ldi 	eeadrh,0x00	;Set address high byte for EEPROM write
	mov 	eedata,crc	;Set CRC low byte in EEPROM data
	rcall	eewrite		;Write EEPROM

	ldi 	eeadr,0x02	;Set address low byte for EEPROM write
	ldi 	eeadrh,0x00	;Set address high byte for EEPROM write
	mov 	eedata,crch	;Set CRC high byte in EEPROM data
	rcall	eewrite		;Write EEPROM

	out 	PORTB,crc	;Output CRC low value to PORTB
	
mainloop:
	sbic	EECR,EEWE	;If EEWE not clear
	rjmp	mainloop	;    Wait more

;********** Insert program code here *************

	ldi 	eeadr,0x01	;Set address low byte for EEPROM read
	ldi 	eeadrh,0x00	;Set address high byte for EEPROM read
	rcall	eeread		;Read EEPROM
	mov 	crc,eedata	;Read CRC low byte from EEPROM

	ldi 	eeadr,0x02	;Set address low byte for EEPROM read
	ldi 	eeadrh,0x00	;Set address high byte for EEPROM read
	rcall	eeread		;Read EEPROM
	mov 	crch,eedata	;Read CRC low byte from EEPROM

	ser	status		;Set status register, prepare for CRC checking
	rcall 	crc_gen

loop:	
	out	PORTB,crc	;Output CRC low value to PORTB
	rjmp loop

.exit
