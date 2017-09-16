/*
glcd.c
prototype6:
display the image file: pic1.xbm and q.xbm through the
glcd_show_xbm routine.
implement of 5x7 fonts display.

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

#include "pic1.xbm"
#include "q.xbm"

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

/* draw modes */
#define LCD_MODE_CLEAR     0
#define LCD_MODE_SET       1
#define LCD_MODE_XOR       2

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

void gen_cl(uint16_t n)
{
  DDR_OC1A |= (1<<PIN_OC1A);
  
  TCCR1A=(1<<COM1A0); /* toggle the OC1A output line */
  /* CTC mode, prescaler(N)=1, f_clk_IO/N = 4MHz/1 = 4MHz*/
  TCCR1B=((1<<CTC1)|(1<<CS10));
  
  OCR1AL=n&0xff;
  OCR1AH=n>>8;
}

void glcd_init_write(void)
{
  gen_cl(_2KHz);
  LCDDATADDR = 0xff;
  LCDCTRLDDR |= ((1<<LCDE1PIN)|(1<<LCDE2PIN)|(1<<LCDCMDPIN)
		 |(1<<PIN_E)|(1<<PIN_RW));
}

/* chip act:
chip select:
cs=1: left; cs=2: right; cs=3: both.
read or write:
rw=0: write; else: read.
*/
void chip_act(uint8_t cs, uint8_t rw)
{
  if ( cs & 0x01 ) 
    LCD_ENABLE_E1();

  if ( cs & 0x02 )
    LCD_ENABLE_E2();
  
  if (rw==0)
    LCDCTRLPORT &= ~(1<<PIN_RW);
  else
    LCDCTRLPORT |= (1<<PIN_RW);

  LCDCTRLPORT |= (1<<PIN_E);
  LCDCTRLPORT &= ~(1<<PIN_E);
	
  /*stand by*/
  LCD_DISABLE_E1();
  LCD_DISABLE_E2();
}

void glcd_write_command(uint8_t cmd, uint8_t cs)
{
  LCD_CMD_MODE();
  LCDDATAPORT = cmd;
  chip_act(cs,0);
}

void glcd_write_data(uint8_t data, uint8_t cs)
{
  LCD_DATA_MODE();
  LCDDATAPORT = data;
  chip_act(cs,0);
}

void glcd_fill(uint8_t c, uint8_t cs)
{
  uint8_t i,j;
  for(i=0;i<=MAX_PAGE;i++){
    for(j=0;j<=MAX_COLUMN;j++){

      /* pages */
      glcd_write_command(LCD_SET_PAGE|i, cs);
  
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|j, cs);

      /*clear to white*/
      glcd_write_data(c, cs);
    }
  }
}

/*display xxx.xbm (which is equal or less than 32x61)*/
void glcd_show_xbm(PGM_P pic, uint8_t width, uint8_t  height, uint8_t cs)
{
  uint8_t i,j,k;
  uint8_t old_byte, new_byte;
  
  for(i=0;i<=(width-1)/8;i++){
    for(j=0;j<=height-1;j++){
      /* pages */
      glcd_write_command(LCD_SET_PAGE|i, cs);
  
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|j, cs);

      /*
	the byte structure of xbm file is:
	picture pix: 1st_bit| 2nd_bit| ...| 7th_bit
	xbm byte: bit0(LSB)| bit1 |...|bit7(MSB)
	so, I should convert xbm byte:
	new byte <-- old byte
	bit0 <-- bit7
	bit1 <-- bit6
	.
	.
	.
	bit7 <-- bit0
       */
      
      old_byte=pgm_read_byte(pic+(j*4+3-i));
      new_byte = 0;
      for(k=0;k<=7;k++)
	new_byte |= ((old_byte>>k)&0x01)<<(7-k);

      glcd_write_data(new_byte, cs);
    }
  }
}


/* global: save current page number for text display*/
uint8_t current_page;
/* save current chip select for text display */
uint8_t current_cs;

/* cement == 1: display text on 122x32 as a single displayer.*/
/*
return current column value.

but, there is a bug to be solved:
think about a character jump from left screen to right screen.
*/
uint8_t glcd_putc_5x7(unsigned char c, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  uint8_t i, temp;
  
  if(col+4 > MAX_COLUMN)
    {
      if(cs==1 && cement){
	/* a problem should be solved here! */
	return glcd_putc_5x7(c, page, SCRN_LEFT, 2, 1);
	
      }else if(cs==2 && cement){
	return glcd_putc_5x7(c, page+1, SCRN_LEFT, 1, 1);
      }else{
	page++;
	col = SCRN_LEFT;
      }
    }
     
  if(page > MAX_PAGE)
    page = SCRN_TOP;

  /* pages */
  glcd_write_command(LCD_SET_PAGE|page, cs);
  
  /* write ascii character */
  for(i=col; i<=col+4; i++)
    {
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|i, cs);
      
      temp = pgm_read_byte(Font5x7 + (c-0x20)*5 + (i-col));
      
      glcd_write_data(temp, cs);
    }
  
  current_cs=cs;
  current_page=page;
  return i;
  
}


uint8_t glcd_puts_5x7(unsigned char *s, int len, uint8_t page, uint8_t col, uint8_t cs, uint8_t cement)
{
  int i;
  uint8_t current_col;
  
  for(i=0;i<len;i++){
    current_col=glcd_putc_5x7(s[i], page, col, cs, cement);
    col = current_col+1;
    page = current_page;
    cs=current_cs;
  }
  
  return current_col;
}


int main (void)
{
  int i;

  glcd_init_write();
  
  glcd_write_command(LCD_DISP_ON,3);

  glcd_fill(0, 3);

  glcd_show_xbm(pic1_bits, 32, 43, 1);

  //  glcd_putc_5x7('s', 2, 57, 2, 0);
  glcd_puts_5x7("WANTED!!! Name:Sun Ge Sex: Male", 31, 0, 0, 2, 0);
  
  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, 3);  
  glcd_puts_5x7("WHO is this man? ??Sun Ge????!!", 31, 0, 0, 1, 0);
  glcd_show_xbm(q_bits, 32, 61, 2);

  for(i=0;i<30;i++)
    _delay_ms(100);

  glcd_fill(0, 3);  
  glcd_puts_5x7("WHO is this man? ??Sun Ge????!! 234567890", 41, 0, 0, 1, 1);

  
  for (;;) {
  }

}

