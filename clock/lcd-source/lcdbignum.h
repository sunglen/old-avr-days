/*************************************************************************
Title:    LCD Big-Numbers
Author:   Martin Thomas <eversmith@heizung-thomas.de>
Date:     September 2004
Software: AVR-GCC 3.4.0/avr-lib 1.0.4
Hardware: - AVR connected to 4-line HD44780 compatible LCD text display
            any AVR with 7 free I/O pins if 4-bit IO port mode is used
Uses:     - LCD-Library from Peter Fleury (sightly extended)
          - Number-Patterns from www.mikrocontroller.net/forum
		  
- Please adapt the settings in lcd.h to your LCD (esp. LCD_START_LINEx).
**************************************************************************/

#ifndef lcdbignum_h
#define lcdbignum_h

/* Init has to be called before using the big numbers but
   after the LCD init-routine */
void LCDBigInit(void);

/* draws "num" in big size (all 4 lines) at column "col" */
void LCDBigNumWrite(uint8_t num, uint8_t col);

#endif