;;; test at90s8515 with external sram.
;;; connection:
;;; at90s8515 <--> 74F373 (/OE -->GND)
;;; ALE <--> LE
;;; AD0~AD7 <--> D0~D7
;;; 
;;; 74F373 <--> SRM2B256SLMX55(32KB SRAM)
;;; O0~O7 <--> A0~A7
;;;
;;; at90s8515 <--> SRM2B256SLMX55
;;; /WR <--> /WR
;;; /RD <--> /OE
;;; A15 <--> /CS
;;; AD0~AD7 <--> I/O1~I/O8
;;; A8~A14 <--> A8~A14

#include <avr/io.h>

	.section .init1,"ax",@progbits
	
	;;enable external sram and external sram wait state(SRW)
	ldi r16,_BV(SRE)|_BV(SRW)
	out _SFR_IO_ADDR(MCUCR),r16

