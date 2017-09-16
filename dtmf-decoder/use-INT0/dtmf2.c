/*
DTMF decoder.
see also connect.txt
Just show the detected DTMF signal in a 16x2 lcd.
set timebase=10us or 50us of oszifox (test the GS port of LC7385) will show the wave clearly.
*/

#include <inttypes.h>
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>

#include "lcd.h"

volatile uint8_t q;
const char q_table_Hex[] PROGMEM = {
  'D','1','2','3','4','5','6','7','8','9','0','*','#','A','B','C'
};

unsigned char buffer[33]={'\0'};
uint8_t i=0;

void decoder_init(void)
{
  DDRD &= ~((1<<PD2)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7));
  PORTD |= ((1<<PD2)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7));
  
  // step1:
  EIMSK = (1<<INT0);
  EICRA = ((1<<ISC00)|(1<<ISC01));
  EIFR = (1<<INTF0);
  
  //step2:
  sei();
  
}


ISR(INT0_vect)
{
  cli();
  q=(PIND>>4);
  buffer[i]=pgm_read_byte(q_table_Hex+q);
  i++;
  buffer[i]='\0';
  
  if(i>=32)
    i=0;
	
  lcd_clrscr();
  lcd_gotoxy(0,0); 
  lcd_puts(buffer);
  
  sei();
}


int main(void)
{
  
  decoder_init();

  lcd_init(LCD_DISP_ON);

  for(;;)
    ;
}  
