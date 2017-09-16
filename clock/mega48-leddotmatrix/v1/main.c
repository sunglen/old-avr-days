/*************************************************************************
 mega48 led-dot-matrix RTC clock.
hardware:
1)8x8 led-dot-matrix connected. 
PORTD for column output(setting values),
and PORTB(0-5) and PINC4, PINC5 for row activate(scan).
2) PINC0 -- minus key
PINC1 -- plus key
PINC2 -- adjust select key.
3) PB6(TOSC1) and PB7(TOSC2) connected to a 32.768kHz crystal.

attention!
ABOUT Vtarg:
mega48 can work on 0-10MHz@2.7-5.5V, but to 8x8 led-dot-matrix,
when I don't connect resisters between row(or column) and the PINs of AVR,
5V voltage will make the led-dot-matrix works terrible, AND if SCK, MISO,
MOSI also connect to the led-dot-matrix, ISP program will fail.
through a test, 3.0V will work better.(set the vtarg of stk500 to 2.7V or
use two AA alakine batteries.), and ISP program will succeed.
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
#define KEYSELECT PINC2

#define KEYCNTMAX  0xf0
/* <5*10ms key-pressed is bounce */
#define KEYCNTMIN  5
/* 100*10ms key-pressed is "long" */
#define KEYCNTLONG 100
/* check every 50*10 ms for key-press to update time */
#define DTCHECKKEY 50

volatile uint8_t gHeartBeat10ms = 0;
volatile uint8_t gKeyPlusCnt = 0, gKeyMinusCnt = 0, gKeySelectCnt = 0;
volatile uint8_t adjust=0; /* adjust flag: time(0), date, month(1), year(2), week(3) */


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

	/* check if "select"-key is pressed (active low) */
	if ( !(KEYPIN & (1<<KEYSELECT)) ) {
		if (gKeySelectCnt < KEYCNTMAX) gKeySelectCnt++;
	}
	else gKeySelectCnt = 0;

	
	gHeartBeat10ms++;
}

void init_keys(void)
{
	uint8_t sreg;

	KEYDDR &= ~( (1<<KEYMINUS) | (1<<KEYPLUS) | (1<<KEYSELECT)); /* set pins as input */
	KEYPORT |= ( (1<<KEYMINUS) | (1<<KEYPLUS) | (1<<KEYSELECT)); /* enable int. pull-ups */
	
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
	uint8_t updaterequired, minute_update;
	static uint8_t tlastchange = 0;
	
	/* 
	   if no key is pressed "long" and the last update with
	   "shortly" pressed key has been done < DTCHECKKEY*10ms before
	   "now" do nothing and return 
	*/
	if ( ( gKeyPlusCnt < KEYCNTLONG ) && ( gKeyMinusCnt < KEYCNTLONG ) && (gKeySelectCnt < KEYCNTLONG)) {
		if ( (uint8_t)(gHeartBeat10ms - tlastchange) < DTCHECKKEY ) return;
	}
	
	sreg=SREG;
	
	updaterequired=0;
	minute_update=0;
	
	if (gKeySelectCnt > KEYCNTMIN)
	  {
	    cli();
	    updaterequired=1;
	    adjust++;
	  }

	/* increment time */
	if ( gKeyPlusCnt > KEYCNTMIN ) {
		cli();	/* disable interrupts to avoid side-effect with ISRs */
		updaterequired=1;
		if (adjust%4 == 0)
		  {
		    minute_update=1; /* minute  update */
		    RTC_m++;
		    if (RTC_m > 59)
		      {
			RTC_m = 0;
			RTC_h++;
			if (RTC_h > 23)
			  RTC_h = 0;
		      }
		  }
		else if (adjust%4 == 1)
		  {
		    date++;
		    if (date > 31)
		      {
			date = 1;
			month++;
		      }
		    if (month == 13)
		      month=1;
		  }
		else if ( adjust%4 == 2)
		  year++;
		else if ( adjust%4 == 3)
		  {
		    week++;
		    if (week > 7)
		      week=1;
		  }
	}
	/* decrement time */
	if ( gKeyMinusCnt > KEYCNTMIN ) {
		cli();
		updaterequired=1;
		if (adjust%4 == 0)
		  {
		    minute_update=1;
		    if (RTC_m==0) {
		      RTC_m=59;
		      if (RTC_h==0) RTC_h=23;
		      else RTC_h--;
		    }
		    else RTC_m--;
		  }
		else if (adjust%4 == 1)
		  {
		    if (date == 1)
		      {
			date=31;
			if (month == 1)
			  month=12;
			else
			  month--;
		      }
		    else
		      date--;
		  }
		else if (adjust%4 == 2)
		  year--;
		else if (adjust%4 == 3)
		  {
		    if (week == 1)
		      week=7;
		    else
		      week--;
		  }
	}
	
	if (updaterequired) {
	  if (minute_update)
	    RTC_s = 0; /* reset seconds */
	  tlastchange=gHeartBeat10ms; /* save change time */
	}

	SREG=sreg;
}


int main(void)
{
	init_keys(); /* init input-keys */
	
	RTC_init();
	sei(); /* enable interrupts */
	
	/****LED-dot-matrix I/O Init ****/
	DDRD = 0xFF;
	DDRB |= 0x3F;
	DDRC |= ((1<<PINC4)|(1<<PINC5));
	
	PORTB |= 0x3F; 
	PORTC |= ((1<<PINC4)|(1<<PINC5));
	
	RTC_set(22,6,33,2005,12,25,7);
	
	/* main loop */
	for (;;)                   
	{	
	  /* add LED Dot Matrix Output code here */
	  
	  PORTD = (RTC_h/10)|((year/1000)<<4);
	  PORTB = ~0x01;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;


	  PORTD = (RTC_h%10)|(((year%1000)/100)<<4);
	  PORTB = ~0x02;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;

	  PORTD = (RTC_m/10)|((((year%1000)%100)/10)<<4);
	  PORTB = ~0x04;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;

	  PORTD = (RTC_m%10)|((((year%1000)%100)%10)<<4);
	  PORTB = ~0x08;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;


	  PORTD = (RTC_s/10)|((month/10)<<4);
	  PORTB = ~0x10;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;

	  PORTD = (RTC_s%10)|((month%10)<<4);
	  PORTB = ~0x20;
	  _delay_ms(LIGHTED_TIME);
	  PORTB |= 0x3F;

	  PORTD = (0x00)|((date/10)<<4);
	  PORTC &= ~(1<<PINC4);
	  _delay_ms(LIGHTED_TIME);
	  PORTC |= (1<<PINC4);

	  PORTD = (week)|((date%10)<<4);
	  PORTC &= ~(1<<PINC5);
	  _delay_ms(LIGHTED_TIME);
	  PORTC |= (1<<PINC5);
	  
	  check_keys();
	}
	
	return 0; /* never reached */
}

