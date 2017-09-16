/*
glcd.c

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
//#include "bmp.h"

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
#define LCD_X_BYTES		122
#define LCD_Y_BYTES		4
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

void init_glcd_write(void)
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

int main (void)
{
  uint8_t i,j;
  
  init_glcd_write();
  
  /* display on/off: 0xaf=on, 0xae=off*/
  glcd_write_command(LCD_DISP_ON,3);


  for(i=0;i<=3;i++){
    for(j=0;j<=79;j++){
      /* pages */
      glcd_write_command(LCD_SET_PAGE|i, 3);
  
      /*cloumns*/
      glcd_write_command(LCD_SET_COL|j, 3);

      /*clear to white*/
      glcd_write_data(0x00, 3);
    }
  }

  /* 3rd page */
  glcd_write_command(LCD_SET_PAGE|2, 3);
  
  
  /* 31st cloumn*/
  glcd_write_command(LCD_SET_COL|31, 3);

  /* show a black column*/
  glcd_write_data(0xff, 3);
  
  for (;;) {

  }
}

