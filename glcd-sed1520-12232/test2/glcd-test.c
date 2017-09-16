/*
glcd-test.c :
hardware:
90s8515 <--> sc12232c
see connect file for details.

The program just show something on the glcd simply for hardware test.
use ctc mode of TC1 to generate 2KHz frequency on PD5(OC1A) output to
PIN7(CL) of the glcd.
because f_OC1A=f_clk_IO/(2*N*(1+OCR1A)), when f_OC1A = 2e3 Hz,
f_clk_IO=4e6Hz, N=1, then OCR1A=999.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* use at90s8515 4MHz*/
#define F_CPU 4000000UL
#include <util/delay.h>

/* define for TC1 and OC1A output */
#define DDR_OC1A DDRD
#define PORT_OC1A PORTD
#define PIN_OC1A PD5

/*
at90s8515 <--> sc12232c
PC0~PC7 <--> DB0~DB7
PB0 <--> RES
PD2 <--> CS1
PD3 <--> CS2
PD4 <--> A0
PD5(OC1A) <--> CL
PD6(WR) <--> E(68 series), RD(80 series)
PD7(RD) <--> R/W(68 series), WR(80 series)

VTG <--> VDD
GND <--> VSS
GND <--> Vo
VTG <--> 100Ohm <--> A
GND <--> K
*/
#define DDR_DB DDRC
#define PORT_DB PORTC

#define DDR_CTRL DDRD
#define PORT_CTRL PORTD
#define PIN_CS1 PD2
#define PIN_CS2 PD3
#define PIN_A0 PD4
#define PIN_E PD6
#define PIN_RW PD7

void generate_cl(void)
{
  DDR_OC1A |= (1<<PIN_OC1A);
  
  TCCR1A=(1<<COM1A0); /* toggle the OC1A output line */
  /* CTC mode, prescaler(N)=1, f_clk_IO/N = 4MHz/1 = 4MHz*/
  TCCR1B=((1<<CTC1)|(1<<CS10));
  
  /* Top = OCR1A = 999*/
  /* f_OC1A = f_clk_I0/(2*N*(1+OCR1A)) = 2e3 Hz*/
  OCR1AL=999&0xff;
  OCR1AH=999>>8;
}

void init_glcd(void)
{
  DDR_DB = 0xff;
  DDR_CTRL |= ((1<<PIN_CS1)|(1<<PIN_CS2)|(1<<PIN_A0)|(1<<PIN_E)|(1<<PIN_RW));
}

int main (void)
{
  uint8_t i,j;
  generate_cl();
  
  init_glcd();
  
  /*select left half part*/
  PORT_CTRL &= ~(1<<PIN_CS1);
  PORT_CTRL |= (1<<PIN_CS2);
  
  /* display on/off: 0xaf=on, 0xae=off*/
  PORT_DB = 0xaf;
  PORT_CTRL &= ~((1<<PIN_A0)|(1<<PIN_RW));
  PORT_CTRL |= (1<<PIN_E);
  PORT_CTRL &= ~(1<<PIN_E);

  for(i=0;i<=3;i++){
    for(j=0;j<=79;j++){
      /* pages */
      PORT_DB = (0xb8|i);
      PORT_CTRL &= ~((1<<PIN_A0)|(1<<PIN_RW));
      PORT_CTRL |= (1<<PIN_E);
      PORT_CTRL &= ~(1<<PIN_E);
  
      /*cloumns*/
      PORT_DB = j;
      PORT_CTRL &= ~((1<<PIN_A0)|(1<<PIN_RW));
      PORT_CTRL |= (1<<PIN_E);
      PORT_CTRL &= ~(1<<PIN_E);

      /*clear to white*/
      PORT_DB = 0x00;
      PORT_CTRL |= (1<<PIN_A0);
      PORT_CTRL &= ~(1<<PIN_RW);
      PORT_CTRL |= (1<<PIN_E);
      PORT_CTRL &= ~(1<<PIN_E);
    }
  }

  /* 3nd page */
  PORT_DB = 0xb8|2;
  PORT_CTRL &= ~((1<<PIN_A0)|(1<<PIN_RW));
  PORT_CTRL |= (1<<PIN_E);
  PORT_CTRL &= ~(1<<PIN_E);
  
  /* 21st cloumn*/
  PORT_DB = 21;
  PORT_CTRL &= ~((1<<PIN_A0)|(1<<PIN_RW));
  PORT_CTRL |= (1<<PIN_E);
  PORT_CTRL &= ~(1<<PIN_E);

  /* show a black column*/
  PORT_DB = 0xff;
  PORT_CTRL |= (1<<PIN_A0);
  PORT_CTRL &= ~(1<<PIN_RW);
  PORT_CTRL |= (1<<PIN_E);
  PORT_CTRL &= ~(1<<PIN_E);
  
  for (;;) {
  }
}

