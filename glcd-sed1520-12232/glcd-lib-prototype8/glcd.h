/*
glcd.h:
sed1520 glcd library prototype7:
can display the .xbm image file through the glcd_show_xbm routine.
can display 5x7 char through glcd_putc_5x7 and glcd_puts_5x7 routine.

hardware:
90s8515 <--> sc12232c

This file 
1)defined the hardware connection between microcontroller and 
sed1520 based 122x32 lcd.

at90s8515 <--> sc12232c
PC0~PC7 <--> DB0~DB7
RESET <--> RES
       |
      SW
       |
       V
      GND
PD2 <--> CS1
PD3 <--> CS2
PD4 <--> A0
PD5(OC1A) <--> CL
PD6 <--> E(68 series), RD(80 series)
PD7 <--> R/W(68 series), WR(80 series)

VTG <--> VDD
GND <--> VSS
GND <--> Vo
VTG <--> 100 Ohm <--> A
GND <--> K

2)defined command set of sed1520 and interface of this library.

*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

#include "font5x7.h"

/* use at90s8515 4MHz*/
#define F_CPU 4000000UL
#include <util/delay.h>

/* define for TC1 and OC1A output */
#define DDR_OC1A DDRD
#define PORT_OC1A PORTD
#define PIN_OC1A PD5

#define LCDDATAPORT  PORTC
#define LCDDATADDR   DDRC

#define LCDCTRLPORT  PORTD
#define LCDCTRLDDR   DDRD
#define LCDCMDPIN    PD4
#define LCDE1PIN     PD2
#define LCDE2PIN     PD3
#define PIN_E PD6
#define PIN_RW PD7

/* command set of SED1520 LCD Display Controller */
#define LCD_DISP_OFF 		0xAE	/* turn LCD panel OFF */
#define LCD_DISP_ON     	0xAF	/* turn LCD panel ON */
#define LCD_SET_LINE		0xC0	/* set line for COM0 (4 lsbs = ST3:ST2:ST1:ST0) */
#define LCD_SET_PAGE		0xB8	/* set page address (2 lsbs = P1:P0) */
#define LCD_SET_COL	        0x00	/* set column address (6 lsbs = Y4:Y4:Y3:Y2:Y1:Y0) */
#define LCD_SET_ADC_NOR		0xA0	/* ADC set for normal direction */
#define LCD_SET_ADC_REV		0xA1	/* ADC set for reverse direction */
#define LCD_STATIC_OFF		0xA4	/* normal drive */
#define LCD_STATIC_ON		0xA5	/* static drive (power save) */
#define LCD_DUTY_16	       	0xA8	/* driving duty 1/16 */
#define LCD_DUTY_32	       	0xA9	/* driving duty 1/32 */
#define LCD_SET_MODIFY		0xE0	/* start read-modify-write mode */
#define LCD_CLR_MODIFY		0xEE	/* end read-modify-write mode */
#define LCD_RESET	       	0xE2	/* soft reset command */

/* LCD screen and bitmap image array consants */
#define MAX_PAGE 3
#define MAX_COLUMN 60
#define SCRN_LEFT		0
#define SCRN_TOP		0
#define SCRN_RIGHT		121
#define SCRN_BOTTOM		31

/* control-lines hardware-interface (only "write") */
#define LCD_CMD_MODE()     LCDCTRLPORT &= ~(1<<LCDCMDPIN)
#define LCD_DATA_MODE()    LCDCTRLPORT |=  (1<<LCDCMDPIN)
#define LCD_ENABLE_E1()    LCDCTRLPORT &= ~(1<<LCDE1PIN)
#define LCD_DISABLE_E1()   LCDCTRLPORT |=  (1<<LCDE1PIN)
#define LCD_ENABLE_E2()    LCDCTRLPORT &= ~(1<<LCDE2PIN)
#define LCD_DISABLE_E2()   LCDCTRLPORT |=  (1<<LCDE2PIN)

/*
about CL:use ctc mode of TC1 to generate 2KHz frequency on PD5(OC1A) output to
PIN7(CL) of the glcd.
because f_OC1A=f_clk_IO/(2*N*(1+OCR1A)), when f_OC1A = 2e3 Hz,
f_clk_IO=4e6Hz, N=1, then OCR1A=999.
*/

#define _2KHz 999

/* generate clock on OC1A, parameter n is value of OCR1A. */
void gen_cl(uint16_t n);

/* call gen_cl() and put all pins output. */
void glcd_init_write(void);

/* chip act:
chip select:
cs=1: left; cs=2: right; cs=3: both.
read or write:
rw=0: write; else: read.
*/
void chip_act(uint8_t cs, uint8_t rw);

void glcd_write_command(uint8_t cmd, uint8_t cs);

void glcd_write_data(uint8_t data, uint8_t cs);

/* fill with c on cs */
void glcd_fill(uint8_t c, uint8_t cs);

/*display xxx.xbm (which is equal or less than 32x61)*/
void glcd_show_xbm(PGM_P pic, uint8_t width, uint8_t  height, uint8_t cs);


/* 
cement == 1: display text on 122x32 as a single displayer.
return the next column which can be used.
but, there is a bug to be solved:
think about a character jump from left screen to right screen.
*/
uint8_t glcd_putc_5x7(unsigned char c, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement);

/* return the next column which can be used.*/
uint8_t glcd_puts_5x7(unsigned char *s, int len, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement);

/*return the next column not write*/
uint8_t glcd_put_hzxbm_16x16(PGM_P hz, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement);

/* global: save current page number for text display*/
uint8_t current_page;
/* save current chip select for text display */
uint8_t current_cs;
