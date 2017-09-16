;**** A P P L I C A T I O N   N O T E   A V R 1 2 8 ************************
;*
;* Title:		Setup and Use the Analog Comparator
;* Version:		1.1
;* Last updated:	97.07.04
;* Target:		AT90Sxxxx (Devices with Analog Comparator)
;*
;* Support E-mail:	avr@atmel.com
;*
;* DESCRIPTION:
;* This Application note shows how to enable and use some features of the 
;* AVR's on-board precision Analog Comparator. 
;*
;* The Application note is written as a program example performing the 
;* following tasks:
;*
;* - Wait for a positive output edge by polling the comparator output
;* - Wait for a positive output edge by polling the interrupt flag
;* - Enable interrupt on comparator output toggle. The interrupt routine
;*   increments a 16 bit register counter each time it is executed
;*
;***************************************************************************

.include "1200def.inc"

;***** Global Register Variables

.def	temp	=r16		;temporary storage register
.def	cntL	=r17		;register counter low byte
.def	cntH	=r18		;register counter high byte

;***** Interrupt Vectors

	rjmp	RESET		;Reset Handle

.org 	ACIaddr 		
	rjmp	ANA_COMP	;Analog Comparator Handle



;***************************************************************************
;*
;* "ANA_COMP"
;*
;* This interrupt routine is served each time ACI in the ACSR register is
;* set, provided that the Analog Comparator interrupt is enabled (ACIE is
;* set). The routine increments a 16-bit counter each time it is run
;*
;* Number of words	:5
;* Number of cycles	:8
;* Low registers used	:1 (ac_tmp)
;* High registers used	:2 (cntL,cntH)
;*
;***************************************************************************

;***** Interrupt Routine Register Variables

.def	ac_tmp	=r0		;temporary storage register for SREG

;***** Code

ANA_COMP:
		
	in	ac_tmp,SREG	;temporarily store the Status register	
	subi	cntL,low(-1)	
	sbci	cntH,high(-1)	;counter = counter + 1
	out	SREG,ac_tmp	;restore Status register
	reti



;***************************************************************************
;*
;* PROGRAM EXECUTION STARTS HERE
;*
;***************************************************************************

RESET:


;***** Include if used on device with RAM
;	ldi	temp,low(RAMEND)
;	out	SPL,temp
;	ldi	temp,high(RAMEND)
;	out	SPH,temp

;***************************************************************************
;*
;* "wait_edge1"
;*
;* This piece of code waits until the output of the comparator (the ACO-bit
;* in ACSR) goes high. This way of doing it requires no setup, however, 
;* extremely short pulses can be missed, since the program runs three clock 
;* cycles between each time the comparator is checked. Another disadvantage
;* is that the program has to wait for the output to be come negative first,
;* in case the output is positive when polling starts.
;*
;* Number of words	:4
;* Number of cycles	:4 per loop. Response time: 3 - 5 clock cycles
;* Low registers used	:None
;* High registers used	:None
;*
;***************************************************************************

;***** Code

wait_edge1:
	sbic	ACSR,ACO	;if output is high
	rjmp	wait_edge1	;    wait	
we1_1:	sbis	ACSR,ACO	;if output is low
	rjmp	we1_1		;    wait


;***************************************************************************
;*
;* "wait_edge2"
;*
;* This piece of code waits until the output of the comparator (the ACO-bit
;* in ACSR) goes high. This is a more secure solution, since the interrupt
;* flag is polled. This allows the user to insert code within the wait loop
;* because hardware "remembers" pulses of shorter duration than the polling
;* interval. Another positive feature is that there is no need to wait for
;* a preceeding negative edge.
;*
;* Number of words	:5
;* Number of cycles	:Inital setup :2
;*			 Flag clearing:1
;*			 Loop	      :4
;*			 Response time:3 - 5
;* Low registers used	:None
;* High registers used	:None
;*
;***************************************************************************

;***** Code

wait_edge2:
	
;***** Initial Hardware setup (assumes ACIE = 0 from reset)
	
	sbi	ACSR,ACIS0
	sbi	ACSR,ACIS1	;enable interrupt on rising output edge
	
;***** Wait

	sbi	ACSR,ACI	;write a "1" to the ACI flag to clear it
we2_1:	;----------------------- user code goes here
	sbis	ACSR,ACI	;if ACI is low
	rjmp	we2_1		;    wait more	



;***************************************************************************
;*
;* "ana_init"
;*
;* This code segment enables Analog Comparator Interrupt on output toggle.
;* The program then enters an infinite loop. 
;* The 16-bit counter is cleared prior to enabling the interrupt.
;*
;* Performance figures apply to interrupt initialization only.
;*
;* Number of words	:4
;* Number of cycles	:5
;* Low registers used	:None
;* High registers used	:1 (temp)
;*
;***************************************************************************

;***** Register Variables

.def	temp	=r16		;temporary register

;***** Code

ana_init:

;***** Clear 16-bit counter

	clr	cntL
	clr	cntH

;***** Enable Interrupt (assumes ACIE = 0 from reset)
	
	ldi	temp,(ACI<<1)	;clear interrupt flag and ACIS1/ACIS0...
	out	ACSR,temp	;...to select interrupt on toggle
	sei			;enable global interrupts
	sbi	ACSR,ACIE	;enable Analog Comparator interrupt
	
forever:rjmp	forever


	
