#include <inttypes.h>
#include <avr/pgmspace.h>

#include "lcdbignum.h"
#include "lcd.h"

#define ORGFOUR

// Patterns from www.mikrocontroller.net/forum -> Codesammlung
const uint8_t CGRamImg[] PROGMEM = {
	0,0,0,0,1,3,7,15,
	0,0,0,0,31,31,31,31,
	0,0,0,0,16,24,28,30,
	31,31,31,31,30,28,24,16,
	16,24,28,30,31,31,31,31,
	15,7,3,1,0,0,0,0,
	31,31,31,31,0,0,0,0,
	30,28,24,16,0,0,0,0
};

const uint8_t Big0[] PROGMEM = {
    0,1,2,
    255,32,255,
    255,32,255,
    5,6,7
  };
  
const uint8_t Big1[] PROGMEM = {
    0,1,32,
    32,255,32,
    32,255,32,
    6,6,6
	};

const uint8_t Big2[] PROGMEM = {
    0,1,2,
    0,1,3,
    255,32,32,
    6,6,6
	};

const uint8_t Big3[] PROGMEM = {
    0,1,2,
    32,0,3,
    32,5,4,
    5,6,7
	};

const uint8_t Big4[] PROGMEM = {
#ifndef ORGFOUR
    0,32,1,
	255,32,255,
	6,6,255,
	32,32,6
#else
    1,32,32,
    255,1,255,
    32,32,255,
    32,32,6
#endif
	};
 
const uint8_t Big5[] PROGMEM = {
    1,1,1,
    255,1,2,
    32,32,255,
    6,6,7
	};

const uint8_t Big6[] PROGMEM = {
    0,1,2,
    255,1,2,
    255,32,255,
    5,6,7
	};

const uint8_t Big7[] PROGMEM = {
    1,1,1,
    32,0,3,
    32,255,32,
    32,6,32
	};
  
const uint8_t Big8[] PROGMEM = {
    0,1,2,
    255,1,255,
    255,32,255,
    5,6,7,
	};

const uint8_t Big9[] PROGMEM = {
    0,1,2,
    255,1,255,
    32,32,255,
    5,6,7
	};

const uint8_t *BigNums[] PROGMEM = {
	Big0, Big1, Big2, Big3, Big4, Big5, 
	Big6, Big7, Big8, Big9
};

/* 
  LCDBigInit copies the patterns of the used-defined
  characters from the AVR flash-memory into the LCD's 
  character-ram (CGRAM).
  
  The LCD has to be initialized (lcd_init from lcd.h/.c)
  before calling this routine.
*/ 
void LCDBigInit(void)
{
	uint8_t i;

	/* set CGRAM Address-Pointer to 0 */
	lcd_command( (1<<LCD_CGRAM)+0 ); 
	
	/* copy CGRAM image from flash to display */
	/* LCD increments pointer automagicly after write */
	for (i = 0; i < sizeof(CGRamImg); i++) 
		lcd_putc( pgm_read_byte(&CGRamImg[i]) );
}

/* 
  LCDBigNumWrite draws a "large" number
  
  The LCD has to be initialized (lcd_init from lcd.h/.c)
  and the user-defined characters have to be copied
  into the LCD (LCDBigInit) before calling this routine.
  
  Parameters: num: number to draw (0-9), col: lcd-column
*/ 
void LCDBigNumWrite(uint8_t num, uint8_t col)
{
	uint8_t c,r,i;
	uint8_t *pChar;
	
	/* get pointer to Big(num)-array in flash */
	pChar=(uint8_t*)(pgm_read_word(&BigNums[num]));
	
	i=0;
	for (r=0;r<4;r++) /* row-loop */
	{
		lcd_gotoxy(col,r); /* set row */
		for (c=0;c<3;c++) /* column-loop */
		{
			/* set character at pos (col+c,r) according to
			   pattern in BigX[i] where X=num */
			lcd_putc(pgm_read_byte(pChar+i));
			i++;
		}
	}
}
