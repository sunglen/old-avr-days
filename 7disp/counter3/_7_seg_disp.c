/*
_7_seg_disp.c :

Just test for 7 segment led display.
see also #define for details

FAIL TO DISPLAY!!!!
geek problem.
seems have some relation with "volatile".
*/

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <avr/pgmspace.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 interal RC 8MHz/1=8MHz lfuse=0xe2*/
#define F_CPU 8000000UL
#include <util/delay.h>

/* Keep activated for 3 ms */
#define LIGHTED_TIME_ms 3

/* define the connection of each segment */
#define SEG_PORT PORTD
#define DDR_SEG_PORT DDRD
#define SEG_a PD0 //disp pin7
#define SEG_b PD1 //disp pin6
#define SEG_c PD2 //disp pin4
#define SEG_d PD3 //disp pin2
#define SEG_e PD4 //disp pin1
#define SEG_f PD5 //disp pin9
#define SEG_g PD6 //disp pin10
#define SEG_dp PD7 //disp pin5

/* define the common Anode scan port */
#define SCAN_PORT PORTC //to pin3 of each disp.
#define DDR_SCAN_PORT DDRC
#define DISP0 PC0
#define DISP1 PC1
#define DISP2 PC2
#define DISP3 PC3

/* define for TC1 and OC1A output */
#define DDR_OC1A DDRB
#define PORT_OC1A PORTB
#define PIN_OC1A PB1

volatile static uint8_t buffer[12];

/* 7 segment led display font.
from first to last is:
'0','1','2','3','4','5','6','7',
'8','9','A','b','C','d','E','F','.'(dot),' '(disp off)
*/
const unsigned char _7_seg_font[18] = {
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f),
  (1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_e)|(1<<SEG_d),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_g)|(1<<SEG_c)|(1<<SEG_d),
  (1<<SEG_f)|(1<<SEG_g)|(1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_c)|(1<<SEG_d),
  (1<<SEG_a)|(1<<SEG_f)|(1<<SEG_e)|(1<<SEG_d)|(1<<SEG_c)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f),
  (1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_a)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g),
  (1<<SEG_dp),
  0x00
};

/*
buffer structure:
buffer[m] (m=0,1,2,3) have contents of 0x00 to 0x0F;
buffer[n] (n=m+4, is 4,5,6,7) have content of 0x10 (with dot) 
or 0x11 (without dot).
buffer[t] (t=m+8, is 8,9,10,11) is not the index of fonts,
It is 0x00(disp off) or 0xFF(disp on).
*/

//uint8_t buffer[12];

void init_disp(void)
{
  DDR_SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  DDR_SCAN_PORT |= ((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
  SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
}

void init_ctc(void)
{
  DDR_OC1A |= (1<<PIN_OC1A);
  PORT_OC1A &= ~(1<<PIN_OC1A);
  
  /* CTC mode, prescaler=8 */
  TCCR1A=(1<<COM1A0);
  TCCR1B=((1<<WGM12)|(1<<CS11));
  /* Top = OCR1A */
  OCR1A=0xFFFF;

  
  /*enable interrupt*/
  TIMSK1=((1<<TOIE1)|(1<<OCIE1A));
  sei();
  
  TCNT1=0x0000;
  TIFR1=(1<<OCF1A);
}

ISR (TIMER1_COMPA_vect)
{
  // just for debug.
  PORTB ^= (1<<PB2);

  /* show a simple counter */
      if(buffer[3]>=0x0F){
	buffer[3]=0x00;
	if(buffer[2]>=0x0F){
	  buffer[2]=0x00;
	  if(buffer[1]>=0x0F){
	    buffer[1]=0x00;
	    if(buffer[0]>=0x0F)
	      buffer[0]=0x00;
	    else
	      buffer[0]++;
	  }else
	  buffer[1]++;
	}else
	buffer[2]++;
      }else
      buffer[3]++;
}

int main (void)
{

  init_ctc();
  
  init_disp();

  /* for test:display 5.734  */

  buffer[0]=0x05;
  buffer[4]=0x10;
  buffer[8]=0xFF;
    
  buffer[1]=0x07;
  buffer[5]=0x11;
  buffer[9]=0xFF;
    
  buffer[2]=0x03;
  buffer[6]=0x11;
  buffer[10]=0xFF;

  buffer[3]=0x04;
  buffer[7]=0x11;
  buffer[11]=0xFF;

  
  /* Display buffer that is approx. 4*3ms=12ms per frame.*/
  for (;;) {
    
    SEG_PORT = ~((_7_seg_font[buffer[0]]|_7_seg_font[buffer[4]])&buffer[8]);
    SCAN_PORT |= (1<<DISP3);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((_7_seg_font[buffer[1]]|_7_seg_font[buffer[5]])&buffer[9]);
    SCAN_PORT |= (1<<DISP2);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((_7_seg_font[buffer[2]]|_7_seg_font[buffer[6]])&buffer[10]);
    SCAN_PORT |= (1<<DISP1);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((_7_seg_font[buffer[3]]|_7_seg_font[buffer[7]])&buffer[11]);
    SCAN_PORT |= (1<<DISP0);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
  }
	
}
