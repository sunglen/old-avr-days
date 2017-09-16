/*Hardware: HD44780 compatible LCD text display
  any AVR with 7 free I/O pins if 4-bit IO port mode is used*/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"

/*
#define F_CPU 1000000UL
#include <util/delay.h>
*/

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}

#define demodelay 100

 main(void)
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
	mydelay(demodelay);
	lcd_clrscr();
	lcd_puts("In v1!\n");
	mydelay(demodelay);
	lcd_clrscr();
	lcd_puts("go on now...\n");
	mydelay(demodelay);
    }
}

