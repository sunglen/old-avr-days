;**** A P P L I C A T I O N   N O T E   A V R 2 2 0 ************************
;*
;* Title:		Bubble Sort Algorithm
;* Version:		1.0
;* Last updated:	97.07.04
;* Target:		AT90Sxx1x (Devices with SRAM)
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION
;* This Application note shows how to sort a block of data in SRAM using	
;* the code efficient Bubble Sort Algorithm. The App. note contains a test
;* program which copies a 60-byte block of data from program memory to 
;* SRAM and sorts the data.
;*
;***************************************************************************

.include "8515def.inc"

.equ	SIZE	=60		;data block size
.equ	TABLE_L	=$60		;Low SRAM address of first data element
.equ	TABLE_H	=$00		;High SRAM address of first data element

	rjmp	RESET		;Reset Handle

;***************************************************************************
;*
;* "bubble"
;*
;* This subroutine bubble sorts the number of bytes found in "cnt1" + 1
;* with the last element in SRAM at location "last". 
;* This implementation sorts the data with the highest element at the 
;* lowest SRAM address. The sort order can be reversed by changing the 
;* "brlo" statement to "brsh". Signed sort can be obtained by using "brlt" 
;* or "brge"
;*
;* Number of words	:13 + return
;* Number of cycles	:6*(SIZE-1)+10*(SIZE(SIZE-1))+return (Min)
;*			 6*(SIZE-1)+13*(SIZE(SIZE-1))+return (Max)
;* Low registers used	:3 (A,B,cnt2)
;* High registers used	:3 (cnt1,endL,endH)
;* Pointers used	:Z
;*
;***************************************************************************

;***** Subroutine Register Variables

.def	A	=r13		;first value to be compared
.def	B	=r14		;second value to be compared
.def	cnt2	=r15		;inner loop counter
.def	cnt1	=r16		;outer loop counter
.def	endL	=r17		;end of data array low address
.def	endH	=r18		;end of data array high address

;***** Code

bubble:
	mov	ZL,endL
	mov	ZH,endH		;init Z pointer
	mov	cnt2,cnt1	;counter2 <- counter1
i_loop:	ld	A,Z		;get first byte, A (n)
	ld	B,-Z		;decrement Z and get second byte, B (n-1)
	cp	A,B		;compare A with B
	brlo	L1		;if A not lower 
	st	Z,A		;    store swapped
	std	Z+1,B
L1:	dec	cnt2
	brne	i_loop		;end inner loop
	dec	cnt1
	brne	bubble		;end outer loop		
	ret


;***************************************************************************
;*
;* Test Program
;*
;* This program copies 60 bytes of data from Program memory to SRAM. It 
;* then calls "bubble" to get the data sorted.
;*
;***************************************************************************

RESET:

;***** Main program Register variables

.def	temp	=r16

;***** Code

	ldi	temp,low(RAMEND)
	out	SPL,temp
	ldi	temp,high(RAMEND)
	out	SPH,temp	;init Stack Pointer

;***** Memory fill
	
	clr	ZH
	ldi	ZL,tableend*2+1	;Z-pointer <- ROM table end + 1
	ldi	YL,low(256*TABLE_H+TABLE_L+SIZE)
	ldi	YH,high(256*TABLE_H+TABLE_L+SIZE)	
				;Y pointer <- SRAM table end + 1
loop:	lpm			;get ROM constant
	st	-Y,r0		;store in SRAM and decrement Y-pointer
	sbiw	ZL,1		;decrement Z-pointer
	cpi	YL,TABLE_L	;if not done
	brne	loop		;    loop more
	cpi	YH,TABLE_H
	brne	loop

;***** Sort data

sort:	ldi	endL,low(TABLE_H*256+TABLE_L+SIZE-1)
	ldi	endH,high(TABLE_H*256+TABLE_L+SIZE-1)
				;Z <- end of array address
	ldi	cnt1,SIZE-1	;cnt1 <- size of array - 1
	rcall	bubble

forever:rjmp	forever	



;***** 60 ROM Constants

table:
.db	120,196
.db	78,216
.db	78,100
.db	43,39
.db	241,43
.db	62,172
.db	109,69
.db	48,184
.db	215,231
.db	63,133
.db	216,8
.db	121,126
.db	188,98
.db	168,205
.db	157,172
.db	108,233
.db	80,255
.db	252,102
.db	198,0
.db	171,239
.db	107,114
.db	172,170
.db	17,45
.db	42,55
.db	34,174
.db	229,250
.db	12,179
.db	187,243
.db	44,231
tableend:
.db	76,48

