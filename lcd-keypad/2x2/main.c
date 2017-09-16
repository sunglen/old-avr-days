/*Hardware: see connect.txt */

#include <avr/io.h>
#include "lcd.h"
#include "KPDScan.h"

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}

#define demodelay 0xFF

int main(void)
{
  uint8_t key_val;
    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    for (;;) {
      /*
	lcd_puts("Hello, World!\n --mega48");
	mydelay(demodelay);
	lcd_clrscr();
	lcd_puts("go on now...\n");
	mydelay(demodelay);
      */
      key_val=KPDScan();
      /* output the key value (dec) to LCD */
      lcd_gotoxy(0,0);
      lcd_putc('0'+key_val/100);
      lcd_gotoxy(1,0);
      lcd_putc('0'+(key_val%100)/10);
      lcd_gotoxy(2,0);
      lcd_putc('0'+(key_val%100)%10);
    }
}

