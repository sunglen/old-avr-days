/*************************************************************************
          -8x8 led-dot-matrix connected. PORTD for row output(setting values),
           and PORTB(0-5) and PINC4, PINC5 for column activate(scan).
**************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "rtcasync.h"

/* use interal RC osililator 8 MHz. prescale 8MHz/8, lfuse=0x62 (default)*/
#define F_CPU 1000000UL
#include <util/delay.h>
/* Keep activated for 2 ms per column */
#define LIGHTED_TIME 2


/* remember that PINB6 is TOSC1 and PINB7 is TOSC2 */
#define KEYPORT PORTC
#define KEYDDR  DDRC
#define KEYPIN  PINC
#define KEYMINUS PINC0
#define KEYPLUS  PINC1

#define KEYCNTMAX  0xf0
/* <5*10ms key-pressed is bounce */
#define KEYCNTMIN  5
/* 100*10ms key-pressed is "long" */
#define KEYCNTLONG 100
/* check every 50*10 ms for key-press to update time */
#define DTCHECKKEY 50

volatile uint8_t gHeartBeat10ms = 0;
volatile uint8_t gKeyPlusCnt = 0, gKeyMinusCnt = 0;

#warning hohoho

/* rough delay - only used for startup-messages and "gimmik" */
void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<2000;i++) asm volatile ("nop"::);
}

SIGNAL(SIG_OUTPUT_COMPARE1A)
{
	/* check if "minus"-key is pressed (active low) */
	if ( !(KEYPIN & (1<<KEYMINUS)) ) {
		if (gKeyMinusCnt < KEYCNTMAX) gKeyMinusCnt++;
	}
	else gKeyMinusCnt = 0;
	
	/* check if "plus"-key is pressed (active low) */
	if ( !(KEYPIN & (1<<KEYPLUS)) ) {
		if (gKeyPlusCnt < KEYCNTMAX) gKeyPlusCnt++;
	}
	else gKeyPlusCnt = 0;
	
	gHeartBeat10ms++;
}

void init_keys(void)
{
	uint8_t sreg;

	KEYDDR &= ~( (1<<KEYMINUS) | (1<<KEYPLUS) ); /* set pins as input */
	KEYPORT |= ( (1<<KEYMINUS) | (1<<KEYPLUS) ); /* enable int. pull-ups */
	
	sreg=SREG;
	cli();
	
	/* init Timer1 (ISR called ca. every 10ms @ 1 MHz Sys-Clock ) */
	OCR1A  = 4;     /* set top */
	/* CTC Mode 4 Prescaler 1024 */
	TCCR1B = (1<<WGM12)|(1<<CS12)|(1<<CS10); 
	TIMSK1 |= (1<<OCIE1A); /* enable output-compare int */
	TCNT1 = 0; /* reset counter */
	
	SREG=sreg;
}

void check_keys(void)
{
	uint8_t sreg;
	uint8_t updaterequired;
	static uint8_t tlastchange = 0;
	
	/* 
	   if no key is pressed "long" and the last update with
	   "shortly" pressed key has been done < DTCHECKKEY*10ms before
	   "now" do nothing and return 
	*/
	if ( ( gKeyPlusCnt < KEYCNTLONG ) && ( gKeyMinusCnt < KEYCNTLONG ) ) {
		if ( (uint8_t)(gHeartBeat10ms - tlastchange) < DTCHECKKEY ) return;
	}
	
	sreg=SREG;
	
	updaterequired=0;
	
	/* increment time */
	if ( gKeyPlusCnt > KEYCNTMIN ) {
		cli();	/* disable interrupts to avoid side-effect with ISRs */
		updaterequired=1; /* flag  update */
		RTC_m++;
		if (RTC_m==60) {
			RTC_m=0; 
			RTC_h++;
			if (RTC_h>23) RTC_h=0;
		}
	}
	
	/* decrement time */
	if ( gKeyMinusCnt > KEYCNTMIN ) {
		cli();
		updaterequired=1;
		if (RTC_m==0) {
			RTC_m=59;
			if (RTC_h==0) RTC_h=23;
			else RTC_h--;
		}
		else RTC_m--;
	}
	
	if (updaterequired) {
		RTC_s = 0; /* reset seconds */
		tlastchange=gHeartBeat10ms; /* save change time */
	}

	SREG=sreg;
}


int main(void)
{

	mydelay(0xff);			

	init_keys(); /* init input-keys */
	
	RTC_init();
	sei(); /* enable interrupts */
	
	/****LED-dot-matrix I/O Init ****/
	/* PORT A for row output, PORT D for column activate. */
	/* PD7(OC2) will be interfer if DDD7 set to 1. */
	/* so PD7 cannot be used now. I don't know HOW to disable OC2 ????*/
	/* use PINC2 instead of PD7 */
	/* DDA7 set to 0 or 1 is both OK. */
	DDRD = 0xFF;
	DDRB |= 0x3F;
	DDRC |= ((1<<PINC4)|(1<<PINC5));
	
	PORTB = ~0x00; 
	PORTC |= ((1<<PINC4)|(1<<PINC5));
	
	RTC_set(11,54,59); /* start-time, first LCD-update at 11:55:00 */
	
	/* main loop */
	for (;;)                            /* loop forever */
	{	
	  /* add LED Dot Matrix Output code here */
	  
	  PORTD = (RTC_s%10);
	  PORTB = ~0x01;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (RTC_s/10);
	  PORTB = ~0x02;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (RTC_m%10);
	  PORTB = ~0x04;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (RTC_m/10);
	  PORTB = ~0x08;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (RTC_h%10);
	  PORTB = ~0x10;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (RTC_h/10);
	  PORTB = ~0x20;
	  _delay_ms(LIGHTED_TIME);
	  PORTB = ~0x00;

	  PORTD = (255);
	  PORTC &= ~(1<<PINC4);
	  _delay_ms(LIGHTED_TIME);
	  PORTC |= (1<<PINC4);

	  PORTD = (255);
	  PORTC &= ~(1<<PINC5);
	  _delay_ms(LIGHTED_TIME);
	  PORTC |= (1<<PINC5);
	  
	  check_keys();
	}
	
	return 0; /* never reached */
}

