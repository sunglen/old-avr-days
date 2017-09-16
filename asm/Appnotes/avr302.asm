;**** A P P L I C A T I O N   N O T E   A V R 3 0 2 ************************
;*
;* Title		: I2C Slave Implementation
;* Version		: 1.2
;* Last updated		: 97.07.17
;* Target		: AT90Sxxxx (All AVR Device)
;*			  Fast Mode: AT90S1200 only
;*
;* Support email	: avr@atmel.com
;*
;* Code Size		: 160 words
;* Low Register Usage	: 0
;* High Register Usage	: 5
;* Interrupt Usage	: External Interrupt0,
;*			  Timer/Counter0 overflow interrupt
;*
;* DESCRIPTION
;* 	This application note shows how to implement an AVR AT90S1200 device
;*	as an I2C slave peripheral. For use with other AT90S AVR devices, other
;*	I/O pins might have to be used, and the stack pointer might have to be
;*	initialized.
;*	Received data is stored in r0 and r0 is transmitted during a master
;*	read transfer. This simple protocol is implemented for demonstration
;*	purposes only.
;*
;*	Some features :
;*	* Interrupt based (using INT0 and TIM0).
;*	* The device can be given any 7-bit address. (Expandable to 10bit).
;*	* Supports normal and fast mode.
;*	* Easy insertion of "wait states".
;*	* Size and speed optimized.
;*	* Supports wake-up from idle mode.
;*
;*	Refers to the application note AVR302 documentation for more
;*	detailed description.
;*
;* USAGE
;*	Insert user code in the two locations marked "insert user code here".
;*
;* NOTES
;*	Minimum one instruction will be executed between each interrupt.
;*
;* STATISTICS
;*	Code Size	: 160
;*	Register Usage	: 5 High, 0 Low
;*	Interrupt Usage	: EXT_INT0 and TIM0_OVF
;*	Port Usage	: PD4(T0) and PD2(INT0)
;*	XTAL		: - I2C Fast Mode     : min 16.0MHz
;*			  - I2C Standard Mode : min  3.0MHz
;*
;***************************************************************************

;**** Includes ****

.include "1200def.inc"

;**** Global I2C Constants ****

.equ	devadr	= 0x50		; Slave device address
.equ	devadrm	= 0x7F		; Slave multi address mask

.equ	pinmask = 0x14		; <=> (1<<PD4)+(1<<PD2)

;**** Global Register Variables ****

.def	temp	= r16		; Temporary variable
.def	etemp	= r17		; Extra temporary variable
.def	i2cdata	= r18		; I2C data register
.def	i2cadr	= r19		; I2C address and direction register
.def	i2cstat	= r20		; I2C temporary SREG storage

;**** Interrupt Vectors ****

	rjmp	RESET		; Reset handle
	rjmp	EXT_INT0	; INT0 handle
	rjmp	TIM0_OVF	; Timer 0 overflow handle
;	( rjmp	ANA_COMP )	; ( Analog comparator handle )


;***************************************************************************
;*
;* INTERRUPT
;*	EXT_INT0 / i2c_wakeup
;*
;* DESCRIPTION
;*	Detects the start condition and handles the data transfer on 
;*	the I2C bus.
;*
;*	Received data is stored in the r0 register and r0 is transmitted 
;*	during a master read transfer.
;*
;* NOTE
;*	This interrupt code, in some cases, jumps to code parts in the 
;*	TIM0_OVF / i2c_skip section to reduce code size.
;*
;***************************************************************************

EXT_INT0:			; INT0 handle
	in	i2cstat,SREG	; store SREG


;*************************
;* Get I2C Slave Address *
;*************************

i2c_get_adr:
	ldi	i2cadr,1	; initialize address register
wlo_ga0:sbic	PIND,PIND4	; 	wait for SCL low
	rjmp	wlo_ga0
	rjmp	first_ga
do_ga:				; do
wlo_ga:	sbic	PIND,PIND4	; 	wait for SCL low
	rjmp	wlo_ga

	mov	temp,i2cadr	;	test address
	andi	temp,devadrm	; 	mask "register full" floating bit
	cpi	temp,devadr	; 	if device addressed
	in	temp,SREG	;		set T flag ("T = Z")
	bst	temp,1

first_ga:
	sec			; 	set carry
whi_ga:	sbis	PIND,PIND4	; 	wait for SCL high
	rjmp	whi_ga
	sbis	PIND,PIND2	; 	if SDA low
	clc			;		clear carry
	rol	i2cadr		; 	shift carry into address register

	brcc	do_ga		; while register not full

wlo_ca:	sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_ca

;**** Check Address ****
				; if T flag set (device addressed)
	brts	i2c_adr_ack	;	acknowledge address
				; else
	rjmp	i2c_adr_miss	; 	goto address miss handle

;**** Acknowledge Address ****

i2c_adr_ack:
	sbi	DDRD,DDD2	; assert acknowledge on SDA
whi_aa:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_aa

;**** Check Transfer Direction ****

	lsr	i2cadr		; if master write
	brcc	i2c_master_write;	goto master write handle	


;*******************************
;* Transmit Data (master read) *
;*******************************

i2c_master_read:

;**** Load Data (handle outgoing data) ****

;! INSERT USER CODE HERE !
;!NOTE!	If the user code is more than ONE instruction then wait
;	states MUST be inserted as shown in description.

	mov	i2cdata,r0	; insert data in "serial"
				;  register (user code example)

	sec			; shift MSB out and "floating empty flag" in
	rol	i2cdata

wlo_mr:	sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_mr


;**** Transmitt data ****

	brcc	fb_low_mr	; if current data bit high
	cbi	DDRD,DDD2	;	release SDA
	rjmp	fb_mr		;	goto read loop
fb_low_mr:			; else
	sbi	DDRD,DDD2	;	force SDA
fb_mr:	lsl	i2cdata		; 	if data register not empty

loop_mr:
whi_mr:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_mr
wlo_mr2:sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_mr2

	brcc	b_low_mr	; if current data bit high

	cbi	DDRD,DDD2	;	release SDA
	lsl	i2cdata		; 	if data register not empty
	brne	loop_mr		;		loop
	rjmp	done_mr		;	done
b_low_mr:			; else
	sbi	DDRD,DDD2	;	force SDA
	lsl	i2cdata		; 	if data register not empty
	brne	loop_mr		;		loop

done_mr:
whi_mr2:sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_mr2
wlo_mr3:sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_mr3

	cbi	DDRD,DDD2	; release SDA

;**** Read Acknowledge from Master ****

whi_ra:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_ra

	sec			; read acknowledge
	sbis	PIND,PIND2
	clc
	brcc	i2c_master_read	; if ack transfer (next) data

wlo_ra:	sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_ra

	rjmp	i2c_wait_cond	; goto wait condition (rep. start or stop)


;*******************************
;* Receive Data (master write) *
;*******************************

i2c_master_write:

wlo_mw0:sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_mw0
	cbi	DDRD,DDD2	; remove acknowledge from SDA

whi_mw:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_mw

	in	temp,PIND	; sample SDA (first bit) and SCL
	andi	temp,pinmask	; mask out SDA and SCL
do_mw:				; do
	in	etemp,PIND	;	new sample
	andi	etemp,pinmask	;	mask out SDA and SCL
	cp	etemp,temp
	breq	do_mw		; while no change

	sbrs	etemp,PIND4	; if SCL changed to low
	rjmp	receive_data	;	goto receive data

	sbrs	etemp,PIND2	; if SDA changed to low
	rjmp	i2c_get_adr	;	goto repeated start
				; else
	rjmp	i2c_stop	;	goto transfer stop

receive_data:
	ldi	i2cdata,2	; set i2cdata MSB to zero
	sbrc	temp,PIND2	; if SDA sample is one
	ldi	i2cdata,3	;	set i2cdata MSB to one

do_rd:				; do
wlo_rd:	sbic	PIND,PIND4	;	wait for SCL low
	rjmp	wlo_rd
	sec			; 	set carry
whi_rd:	sbis	PIND,PIND4	;	wait for SCL high
	rjmp	whi_rd

	sbis	PIND,PIND2	; 	if SDA low
	clc			;		clear carry
	rol	i2cdata		; 	shift carry into data register

	brcc	do_rd		; while register not full

;**** Acknowledge Data ****

i2c_dat_ack:
wlo_da:	sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_da
	sbi	DDRD,DDD2	; assert acknowledge on SDA
whi_da:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_da
				; (acknowledge is removed later)

;**** Store Data (handle incoming data) ****

	; insert wait states here if nessesary
;	sbi	DDRD,DDD4	; (start wait state)

;! INSERT USER CODE HERE !
;!NOTE!	If the user code is more than TWO instruction then wait
;	states MUST be inserted as shown in description.

	mov	r0,i2cdata	; Store data receved in "serial"
				;  register (user code example)

;	cbi	DDRD,DDD4	; (end wait state)

	rjmp	i2c_master_write; Start on next transfer


;***************************************************************************
;*
;* INTERRUPT
;*	TIM0_OVF / i2c_skip
;*
;* DESCRIPTION
;*	This interrupt handles a "address miss". If the slave device is
;*	not addressed by a master, it will not acknowledge the address.
;*
;*	Instead of waiting for a new start condition in a "busy loop"
;*	the slave set up the counter to count 8 falling edges on SCL and 
;*	returns from the current interrupt. This "skipping" of data
;*	do not occupies any processor time. When 8 egdes are counted, a
;*	timer overflow interrupt handling routine (this one) will check
;*	the next condition that accure. If it's a stop condition
;*	the transfer ends, if it's a repeated start condition a jump
;*	to the i2c_wakeup interrupt will read the new address. If a new
;*	transfer is initiated the "skipping" process is repeated.
;*
;***************************************************************************

TIM0_OVF:			; ( Timer 0 overflow handle )
	in	i2cstat,SREG	; store SREG

i2c_adr_miss:
	
;**** Drop Acknowledge ****

whi_dac:sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_dac
wlo_dac:sbic	PIND,PIND4	; wait for SCL low
	rjmp	wlo_dac

	; disable timer 0 overflow interrupt
	ldi	temp,(0<<TOIE0)
	out	TIMSK,temp
	; enable external interrupt request 0
	ldi	temp,(1<<INT0)
	out	GIMSK,temp


;************************
;* Wait for a Condition *
;************************

i2c_wait_cond:
whi_wc:	sbis	PIND,PIND4	; wait for SCL high
	rjmp	whi_wc

	in	temp,PIND	; sample SDA (first bit) and SCL
	andi	temp,pinmask	; mask out SDA and SCL
do_wc:				; do
	in	etemp,PIND	;	new sample
	andi	etemp,pinmask	;	mask out SDA and SCL
	cp	etemp,temp
	breq	do_wc		; while no change

	sbrs	etemp,PIND4	; if SCL changed to low
	rjmp	i2c_skip_byte	;	goto skip byte

	sbrs	etemp,PIND2	; if SDA changed to low
	rjmp	i2c_get_adr	;	goto repeated start
				; else
				;	goto transfer stop


;*************************
;* Handle Stop Condition *
;*************************

i2c_stop:
	;! Set INT0 to generate an interrupt on low level,
	;! then set INT0 to generate an interrupt on falling edge.
	;! This will clear the EXT_INT0 request flag.
	ldi	temp,(0<<ISC01)+(0<<ISC00)
	out	MCUCR,temp
	ldi	temp,(1<<ISC01)+(0<<ISC00)
	out	MCUCR,temp

	out	SREG,i2cstat	; restore SREG
	reti			; return from interrupt


;****************
;* Skip byte(s) *
;****************

i2c_skip_byte:
	; set counter initial value
	ldi	temp,-7
	out	TCNT0,temp
	; enable timer 0 overflow interrupt
	ldi	temp,(1<<TOIE0)
	out	TIMSK,temp
	; disable external interrupt request 0
	ldi	temp,(0<<INT0)
	out	GIMSK,temp

	out	SREG,i2cstat	; restore SREG
	reti


;***************************************************************************
;*
;* FUNCTION
;*	i2c_init
;*
;* DESCRIPTION
;*	Initialization of interrupts and port used by the I2C interface
;*	and waits for the first start condition.
;*
;* USAGE
;*	Jump to this code directly after a reset.
;*
;* RETURN
;*	none
;*
;* NOTE
;*	Code size can be reduced by 12 instructions (words) if no transfer
;*	is started on the I2C bus (bus free), before the interrupt
;*	initialization is finished. If this can be ensured, remove the
;*	"Wait for I2C Start Condition" part and replace it with a "sei"
;*	instruction.
;*
;***************************************************************************

RESET:
i2c_init:

;**** PORT Initialization ****

	; Initialize PD2 (INT0) for open colector operation (SDA in/out)
	ldi	temp,(0<<DDD4)+(0<<DDD2)
	out	DDRD,temp

	; Initialize PD4 (T0) for open colector operation (SCL in/out)
	ldi	temp,(0<<PD4)+(0<<PD2)
	out	PORTD,temp

;**** Interrupt Initialization ****

	; Set INT0 to generate an interrupt on falling edge
	ldi	temp,(1<<ISC01)+(0<<ISC00)
	out	MCUCR,temp

	; Enable INT0
	ldi	temp,(1<<INT0)
	out	GIMSK,temp

	; Set clock to count on falling edge of T0
	ldi	temp,(1<<CS02)+(1<<CS01)+(0<<CS00)
	out	TCCR0,temp


;***************************************************
;* Wait for I2C Start Condition (13 intstructions) *
;***************************************************

do_start_w:			; do
	in	temp,PIND	; 	sample SDA & SCL
	com	temp		;	invert
	andi	temp,pinmask	;	mask SDA and SCL
	brne	do_start_w	; while (!(SDA && SCL))

	in	temp,PIND	; sample SDA & SCL
	andi	temp,pinmask	; mask out SDA and SCL
do_start_w2:			; do
	in	etemp,PIND	;	new sample
	andi	etemp,pinmask	;	mask out SDA and SCL
	cp	etemp,temp
	breq	do_start_w2	; while no change

	sbrc	etemp,PIND2	; if SDA no changed to low
	rjmp	do_start_w	;	repeat

	rcall	i2c_get_adr	; call(!) interrupt handle (New transfer)


;***************************************************************************
;*
;* PROGRAM
;*	main - Test of I2C slave implementation
;*
;***************************************************************************

main:	rjmp	main		; loop forever


;**** End of File ****

