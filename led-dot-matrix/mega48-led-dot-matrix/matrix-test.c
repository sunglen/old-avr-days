/*
*/
#include <avr/io.h>

/* !! F_CPU must be defined BEFORE include delay.h */
#define F_CPU 1000000UL
#include <util/delay.h>

/* Keep activated for 3 ms per column */
#define LIGHTED_TIME 3

int main (void)
{
  unsigned int n;
	/**** I/O Init ****/
	/* PORT B(0-7) for row output, PORT D for column activate. */
        DDRB = 0xFF;
	DDRD = 0xFF;

	PORTD = 0x00; /* all column unactivate */
	
	/* Display 'A' for a few seconds (
	   when n=400 approximate 10s), that
	   is 40 frames per second. (use mega48 interal RC 8MHz/8=1MHz) 
           set LIGHTED_TIME=2 will be better.(approx. 61 frames per second) */
	for (n=400;n>0;n--) {
	  PORTB = ~0x1F;
	  PORTD = 0x01;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x30;
	  PORTD = 0x02;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x50;
	  PORTD = 0x04;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x90;
	  PORTD = 0x08;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x90;
	  PORTD = 0x10;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x50;
	  PORTD = 0x20;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x30;
	  PORTD = 0x40;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	  PORTB = ~0x1F;
	  PORTD = 0x80;
	  _delay_ms(LIGHTED_TIME);
	  PORTD = 0x00;

	}
	
}
