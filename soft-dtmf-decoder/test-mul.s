	.file	"test-mul.c"
	.arch atmega48
__SREG__ = 0x3f
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__tmp_reg__ = 0
__zero_reg__ = 1
	.global __do_copy_data
	.global __do_clear_bss
	.text
.global	main
	.type	main, @function
main:
/* prologue: frame size=12 */
	ldi r28,lo8(__stack - 12)
	ldi r29,hi8(__stack - 12)
	out __SP_H__,r29
	out __SP_L__,r28
/* prologue end (size=4) */
	ldi r24,lo8(0x3fcf5c29)
	ldi r25,hi8(0x3fcf5c29)
	ldi r26,hlo8(0x3fcf5c29)
	ldi r27,hhi8(0x3fcf5c29)
	std Y+1,r24
	std Y+2,r25
	std Y+3,r26
	std Y+4,r27
	ldi r24,lo8(0x43c847ae)
	ldi r25,hi8(0x43c847ae)
	ldi r26,hlo8(0x43c847ae)
	ldi r27,hhi8(0x43c847ae)
	std Y+5,r24
	std Y+6,r25
	std Y+7,r26
	std Y+8,r27
	ldd r18,Y+5
	ldd r19,Y+6
	ldd r20,Y+7
	ldd r21,Y+8
	ldd r22,Y+1
	ldd r23,Y+2
	ldd r24,Y+3
	ldd r25,Y+4
	rcall __mulsf3
	movw r26,r24
	movw r24,r22
	std Y+9,r24
	std Y+10,r25
	std Y+11,r26
	std Y+12,r27
	ldi r24,lo8(0)
	ldi r25,hi8(0)
/* epilogue: frame size=12 */
	rjmp exit
/* epilogue end (size=1) */
/* function main size 38 (33) */
	.size	main, .-main
/* File "test-mul.c": code   38 = 0x0026 (  33), prologues   4, epilogues   1 */
