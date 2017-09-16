/*Hardware: HD44780 compatible LCD text display
  any AVR with 7 free I/O pins if 4-bit IO port mode is used*/
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"

/*
void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}
*/

// #define demodelay 100


/* use mega48 interal RC 8MHz/8=1MHz lfuse=0x62*/
#define F_CPU 1000000UL
#include <util/delay.h>

int main(void)
{

	/*
	CLKPR = (1<<CLKPCE);
	CLKPR |= (1<<CLKPS0);
	CLKPR &= ~(1<<CLKPCE);
	*/
    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);

    for (;;) {                           /* loop forever */
        lcd_clrscr();
        
	lcd_puts("Hello, World!\n --mega48");
	_delay_ms(500);
	lcd_clrscr();
	lcd_puts("In v2!\n");
	_delay_ms(500);
	lcd_clrscr();
	lcd_puts("go on now...\n");
	_delay_ms(500);
    }
}

