/*************************************************************************
Hardware: - AVR connected to 2-line HD44780 compatible LCD text display
            any AVR with 7 free I/O pins if 4-bit IO port mode is used
	  - 32 kHz Crystal connected to the TSOC-Pins 
          - AVR system clock should by around 1 MHz (internal R/C
            osc. 1MHz is factory default on ATmegas)
          -8x8 led-dot-matrix connected. PORTA for row output(setting values),
           and PORTD(0-6) and PINC2 for column activate(scan).
		  
Uses:     - LCD-Library from Peter Fleury (sightly extended)
          - Number-Patterns from www.mikrocontroller.net/forum
		  
- Please adapt the settings in lcd.h to your LCD (esp. LCD_START_LINEx).
- Please read the errata in the ATmega8-datasheet when using this 
  controller.

This code will work without modification with an ATmega16, 
32kHz at Pin28/29 and LCD connected according to
the definitions in lcd.h. Time-set keys are connected to 
PortC Pin 0 and 1 (low active).
**************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "lcd.h"

#include "rtcasync.h"

/* use interal RC osililator 1 MHz. lfuse=0xe1 */
#define F_CPU 1000000UL
#include <avr/delay.h>
/* Keep activated for 2 ms per column */
#define LIGHTED_TIME 2


/* remember that PINC6 is TOSC1 and PINC7 is TOSC2 */
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
	TIMSK |= (1<<OCIE1A); /* enable output-compare int */
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
		updaterequired=1; /* flag LCD update */
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
		RTC_secondchanged = 1; /* flag to update display */
		RTC_minutechanged = 1; /* flag to update display */
		tlastchange=gHeartBeat10ms; /* save change time */
	}

	SREG=sreg;
}

void showseconds(uint8_t sec)
{
	lcd_gotoxy(14,1);
	lcd_putc(sec/10+'0');
	lcd_gotoxy(15,1);
	lcd_putc(sec%10+'0');
}

int main(void)
{
			
	lcd_init(LCD_DISP_ON); /* initialize display, cursor off */

	init_keys(); /* init input-keys */

	lcd_gotoxy(0,0);
	lcd_puts("32kHZ Crystal: RTC-INIT...");
	mydelay(0xff);
	RTC_init(); /* init async RTC with 32kHz-Crystal */
	lcd_gotoxy(0,0);
	lcd_puts("initialed.");
	mydelay(0xff);

	sei(); /* enable interrupts */
	
	/****LED-dot-matrix I/O Init ****/
	/* PORT A for row output, PORT D for column activate. */
	/* PD7(OC2) will be interfer if DDD7 set to 1. */
	/* so PD7 cannot be used now. I don't know HOW to disable OC2 ????*/
	/* use PINC2 instead of PD7 */
	/* DDA7 set to 0 or 1 is both OK. */
	DDRA = 0xFF;
	DDRD = 0x7F;
	DDRC |= (1<<PINC2);
	
	PORTD = 0x00; 
	PORTC &= ~(1<<PINC2); /* all column unactivate */

	lcd_clrscr();

	RTC_set(11,54,59); /* start-time, first LCD-update at 11:55:00 */
	
	/* main loop */
	for (;;)                            /* loop forever */
	{	
	  /* add LED Dot Matrix Output code here */
	  
	  PORTA = ~(RTC_s%10);
	  PORTD = 0x01;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(RTC_s/10);
	  PORTD = 0x02;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(RTC_m%10);
	  PORTD = 0x04;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(RTC_m/10);
	  PORTD = 0x08;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(RTC_h%10);
	  PORTD = 0x10;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(RTC_h/10);
	  PORTD = 0x20;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(255);
	  PORTD = 0x40;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTA = ~(255);
	  PORTC |= (1<<PINC2);
	  _delay_ms(LIGHTED_TIME);
	  PORTC &= ~(1<<PINC2);
	  
	  /* there is LCD output code. */
		if (RTC_secondchanged) {
			RTC_secondchanged=0;

			showseconds(RTC_s);
		}
			
		if (RTC_minutechanged) {
			RTC_minutechanged=0;

			lcd_gotoxy(0,0);
			lcd_putc(RTC_h/10+'0');

			lcd_gotoxy(1,0);
			lcd_putc(RTC_h%10+'0');

			lcd_gotoxy(10,0);
			lcd_putc(RTC_m/10+'0');

			lcd_gotoxy(11,0);
			lcd_putc(RTC_m%10+'0');
		}
		
		check_keys();
	}
	
	return 0; /* never reached */
}

