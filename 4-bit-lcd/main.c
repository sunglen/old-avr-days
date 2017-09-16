/*Hardware: HD44780 compatible LCD text display
  any AVR with 7 free I/O pins if 4-bit IO port mode is used*/

#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd.h"


/*
** constant definitions
*/


/*
** function prototypes
*/ 
void wait_until_key_pressed(void);


void wait_until_key_pressed(void)
{
    unsigned char i, j;
    

    do {
        loop_until_bit_is_clear(PINA,PINA7);     /* wait until key is pressed */
        
        for (i=0;i<255;i++) for (j=0;j<255;j++); /* debounce */
            
        if (bit_is_clear(PINA,PINA7)) break;
    } while (1);
    loop_until_bit_is_set(PINA,PINA7);            /* wait until key is released */
 
}

void mydelay(unsigned char v)
{
	unsigned char j;
	unsigned int i;
	for (j=0;j<v;j++) for (i=0;i<8000;i++) asm volatile ("nop"::);
}

#define demodelay 0xff

int main(void)
{
	////unsigned char pos; 
	unsigned char i;
	char buffer[5];
	
	DDRA &= ~(1<<PA7); // cbi(DDRA,PA7); /* Pin PA7 input */
	PORTA |= (1<<PA7); // sbi(PORTA,PA7);/* Pin PA7 pull-up enabled */

	DDRC=0xFF;	// dbg only
	PORTC=0x7F; // dbg only

    /* initialize display, cursor off */
    lcd_init(LCD_DISP_ON);

    for (;;) {                           /* loop forever */
        /* 
         * Test 1:  write text to display
         */
    
        /* clear display and home cursor */
        lcd_clrscr();
        
        /* put string to display (line 1) with linefeed */
	lcd_puts("Hello, World!\n --mega48");
	mydelay(demodelay);
	lcd_clrscr();
	lcd_puts("In other application!\n");
	mydelay(demodelay);
	lcd_clrscr();
	lcd_puts("Press SW7 to continue...");
	wait_until_key_pressed();
        lcd_clrscr();
	lcd_puts("go on now...\n");
	mydelay(demodelay);
	
	lcd_scrollup();
	mydelay(demodelay);
	
	lcd_clrscr();
		//put Standard Character Pattern(S0) 0x21~0xfd
		//which described by SC1602BS*B manual.
		for (i=0x21;i<=0xFD;i++) {
			lcd_putc(i);
			mydelay(20);
		}
		mydelay(demodelay);

		/*
		for (i=0;i<40;i++) {
			lcd_command(LCD_MOVE_DISP_LEFT);
			// mydelay(40);
			wait_until_key_pressed();
		}
		wait_until_key_pressed();
		*/
		
		
		lcd_clrscr();
		lcd_gotoxy(0,0);
		for (i=0;i<LCD_LINES*3;i++) {
			itoa(i+1, buffer, 10);
			lcd_puts(buffer);
			lcd_puts(".line\n");
			mydelay(200);
		}
		mydelay(demodelay);
		
		lcd_clrscr();
		lcd_puts("Line 1\n");
		lcd_puts("Line 2\n");
		lcd_puts("Line 3\n");
		lcd_puts("Line 4");
		mydelay(demodelay);
		lcd_puts("Line 12345678901234567890");
		mydelay(demodelay);
		mydelay(demodelay);
		
		for (i=0;i<LCD_LINES-1;i++) {
			lcd_scrollup();
			mydelay(100);
		}
		
		lcd_scrollup();
		lcd_command(LCD_DISP_ON_CURSOR_BLINK);
		
		mydelay(demodelay);
				
		lcd_clrscr();
		lcd_gotoxy(0,0);
		lcd_puts("Line 1");
		mydelay(demodelay);
		lcd_gotoxy(0,1);
		lcd_puts("Line 2");
		
		mydelay(demodelay);
		lcd_gotoxy(0,3);
		mydelay(demodelay);
		lcd_puts("Line 4");
		mydelay(demodelay);
		lcd_gotoxy(0,2);
		lcd_puts("Line 3");
		mydelay(demodelay);
		
		 /* move cursor to position 7 on line 2 */
        lcd_gotoxy(7,3);  
        
        /* write single char to display */
        lcd_putc(':');
		
        /* wait until push button PD2 (INT0) is pressed */
        mydelay(demodelay);
        
        
        /*
         * Test 2: use lcd_command() to turn on cursor
         */
        
        /* turn on cursor */
        //lcd_command(LCD_DISP_ON_CURSOR);
		lcd_command(LCD_DISP_ON_CURSOR_BLINK);

        /* put string */
        lcd_puts( "CurOn");
		lcd_gotoxy(8,3);  
        
        /* wait until push button PD2 (INT0) is pressed */
        mydelay(demodelay);// wait_until_key_pressed();


        /*
         * Test 3: display shift
         */
        
        lcd_clrscr();     /* clear display home cursor */

        /* put string from program memory to display */
        lcd_puts_P( "Line 1 longer than 14 characters" );
		mydelay(demodelay);
        lcd_puts_P( "and also Line 2 longer than 14 characters and even more" );
        mydelay(demodelay);
		
        /* move BOTH lines one position to the left */
        //lcd_command(LCD_MOVE_DISP_LEFT);
        
        /* wait until push button PD2 (INT0) is pressed */
        mydelay(demodelay);

		/* 
		lcd_clrscr();
		lcd_puts_P( "Hey Thomas!     Drueck mal auf  den SW7 Knopf!" );
		wait_until_key_pressed();
		lcd_clrscr();
		lcd_puts_P( "Machst auch alles was man Dir sagt he?");
		wait_until_key_pressed();
		*/

        /* turn off cursor */
        lcd_command(LCD_DISP_ON);
        
    }
}

