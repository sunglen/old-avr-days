/*************************************************************************
Title:    LCD Big-Number Clock
Author:   Martin Thomas <eversmith@heizung-thomas.de>
Date:     June 2005
Version:  1.01
Software: AVR-GCC 3.4.3/avr-lib 1.2.3
Hardware: - AVR connected to 4-line HD44780 compatible LCD text display
            any AVR with 7 free I/O pins if 4-bit IO port mode is used
	  - 32 kHz Crystal connected to the TSOC-Pins 
          - AVR system clock should by around 1 MHz (internal R/C
            osc. 1MHz is factory default on ATmegas)
		  
Uses:     - LCD-Library from Peter Fleury (sightly extended)
          - Number-Patterns from www.mikrocontroller.net/forum
		  
- Please adapt the settings in lcd.h to your LCD (esp. LCD_START_LINEx).
- Please read the errata in the ATmega8-datasheet when using this 
  controller.

This code will work without modification with an ATmega16, 
32kHz at Pin28/29 and a Displaytech164-LCD connected according to
the definitions in lcd.h. Time-set keys are connected to 
PortB Pin 0 and 1 (low active).

License: main.c (this file) is distributed under the GNU Public License
which can be found at http://www.gnu.org/licenses/gpl.txt

The module lcdbignum.c is FREE - but no warranty.

**************************************************************************/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/signal.h>

#include "lcd.h"
#include "lcdbignum.h"
#include "rtcasync.h"

#define SPASS

#define POINTCHAR 'o' /* 0x6f */
#define BLANKCHAR ' ' /* 0x20 */

#define COL1 0
#define COL2 4
#define COL3 8
#define COL4 12
#define COLDOT 7
#define COLSEC 15

#define KEYPORT PORTA
#define KEYDDR  DDRA
#define KEYPIN  PINA
#define KEYMINUS PINA0
#define KEYPLUS  PINA1

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

#ifdef SPASS
/* rough delay - only used for startup-messages and "gimmik" */
void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<2000;i++) asm volatile ("nop"::);
}
#endif

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

void clockpoints(uint8_t sec)
{
	uint8_t c;
	lcd_gotoxy(COLDOT,1);
	c = ( !(sec & 0x01) ) ? POINTCHAR : BLANKCHAR;
	lcd_putc(c);
	lcd_gotoxy(COLDOT,2);
	/* c = ( sec & 0x01 ) ? POINTCHAR : BLANKCHAR; */
	lcd_putc(c);
}

void showseconds(uint8_t sec)
{
	lcd_gotoxy(COLSEC,1);
	lcd_putc(sec/10+'0');
	lcd_gotoxy(COLSEC,2);
	lcd_putc(sec%10+'0');
}

int main(void)
{
#ifdef SPASS
	uint8_t i;
#endif
			
	/* Disable JTAG by software (if LCD connected to PORTC on ATmega16/32) */
	MCUCSR|=_BV(JTD);
	MCUCSR|=_BV(JTD);

	lcd_init(LCD_DISP_ON); /* initialize display, cursor off */
	LCDBigInit(); /* upload CGRAM (user defined characters) */

	init_keys(); /* init input-keys */

	lcd_gotoxy(2,1);
	lcd_puts_P("RTC sync with");
	lcd_gotoxy(2,2);
	lcd_puts_P("32kHZ Crystal");
	RTC_init(); /* init async RTC with 32kHz-Crystal */

#ifdef SPASS
	/* startup "gimmik" */
	lcd_clrscr();
	lcd_gotoxy(4,0);
	lcd_puts_P("LCD Clock");
	lcd_gotoxy(3,1);
	lcd_puts_P("by M.Thomas");
	lcd_gotoxy(1,2);
	lcd_puts_P("Kaiserslautern");
	lcd_gotoxy(5,3);
	lcd_puts_P("Germany");
	mydelay(0xff);
	lcd_clrscr();
	clockpoints(0);
	i=10;
	do {
		i--;
		LCDBigNumWrite(i,COL1); /* params: number, column */
		LCDBigNumWrite(i,COL2); 
		LCDBigNumWrite(i,COL3); 
		LCDBigNumWrite(i,COL4); 
		mydelay(30);
	} while (i>0);
	lcd_clrscr();
	lcd_gotoxy(3,1);
	lcd_puts_P("Go! Go! GO!");
	mydelay(100);
#endif

	sei(); /* enable interrupts */
	
	lcd_clrscr();

	RTC_set(11,54,59); /* start-time, first LCD-update at 11:55:00 */
	
	/* main loop */
	for (;;)                            /* loop forever */
	{	
		if (RTC_secondchanged) {
			RTC_secondchanged=0;
			clockpoints(RTC_s);
			showseconds(RTC_s);
		}
			
		if (RTC_minutechanged) {
			RTC_minutechanged=0;
			LCDBigNumWrite(RTC_h/10,COL1); /* params: number, column */
			LCDBigNumWrite(RTC_h%10,COL2);
			LCDBigNumWrite(RTC_m/10,COL3);
			LCDBigNumWrite(RTC_m%10,COL4);
		}
		
		check_keys();
	}
	
	return 0; /* never reached */
}


