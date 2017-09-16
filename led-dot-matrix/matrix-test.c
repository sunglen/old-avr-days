/*
*/
#include <avr/io.h>

/* !! F_CPU must be defined BEFORE include delay.h */
#define F_CPU 4000000UL
#include <avr/delay.h>

/* Keep activated for 2 ms per column */
#define LIGHTED_TIME 2

int main (void)
{
  unsigned int n;
	/**** I/O Init ****/
	/* PORT A for row output, PORT C for column activate. */
        DDRA = 0xFF;
	DDRC = 0xFF;

	PORTC = 0x00; /* all column unactivate */
	
	/* Display 'A' for a few seconds (
	   when n=400 approximate 9.5s), that
	   is 42 frames per second. */
	for (n=400;n>0;n--) {
	  PORTA = ~0x1F;
	  PORTC = 0x01;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x30;
	  PORTC = 0x02;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x50;
	  PORTC = 0x04;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x90;
	  PORTC = 0x08;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x90;
	  PORTC = 0x10;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x50;
	  PORTC = 0x20;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x30;
	  PORTC = 0x40;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	  PORTA = ~0x1F;
	  PORTC = 0x80;
	  _delay_ms(LIGHTED_TIME);
	  PORTC = 0x00;

	}
	
}
