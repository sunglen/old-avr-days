/*
ir-test.c :
use ctc mode of TC1 to generate 38KHz frequency.

OC1A(PB1) to generate continuous 38KHz carrier.(receiver module don't work)
but if OC1A generate a pause width, receiver module can also work.

PB2 to generate a pause width(500~700us) that receiver module can decode.

see also ouravr forum to get more information.

IR led connection:


                      -----/\/\/\/(47Ohm)-->VTG(5V)
                      |
                      _
                      V   (+) (Infrared LED)
                     ---  (-)
		      |
                      |
                     /
PB2--->/\/\/\/(1K)--| (2SC1815)
                     \
     		      V
		      |
		      V
		     GND


*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 interal RC 8MHz/8=1MHz, default*/
//#define F_CPU 1000000UL

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


/* 7 segment led display font.
from first to last is:
'0','1','2','3','4','5','6','7',
'8','9','A','b','C','d','E','F','.'(dot),' '(disp off)
*/
const unsigned char _7_seg_font[18] PROGMEM = {
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

volatile unsigned int buffer[12]={0};
//volatile uint8_t buffer[12]={0x05,0x07,0x03,0x01,0x10,0x11,0x11,0x11,0xFF,0xFF,0xFF,0xFF};


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
  DDRB |= (1<<PB2);
  
  /* CTC mode, prescaler=N=8, f_clk_IO/N = 8MHz/8 = 1MHz*/
  TCCR1A=(1<<COM1A0);
  TCCR1B=((1<<WGM12)|(1<<CS11));
  
  /* Top = OCR1A = 13*/
  /* f_OC1A = f_clk_I0/(2*N*(1+OCR1A)) = 3.7449e4 Hz*/
  /* T_ir_base = 26.703 us */
     
  OCR1A=13;
  
  /*enable interrupt*/
  TIMSK1=((1<<TOIE1)|(1<<OCIE1A));
  sei();
  
  TCNT1=0x0000;
  TIFR1=(1<<OCF1A);
}

ISR (TIMER1_COMPA_vect)
{
  // use PB2 as another 38KHz output source.
  PORTB ^= (1<<PB2);
}

int main (void)
{
  
  init_ctc();

  for (;;) {

    /* generate a code on 38KHz carrier (OC1A)*/
    /* also can work */
    /*
    DDR_OC1A &= ~(1<<PIN_OC1A);
    
    _delay_ms(1);
    
    DDR_OC1A |= (1<<PIN_OC1A);
    
    _delay_us(600);
    
    */
    
    /* generate a code on 38KHz carrier (PB2)*/
    
    cli();
    PORTB &= ~(1<<PB2);
    
    _delay_ms(3);
    
    sei();
    TCNT1=0x0000;
    
    /* Output Pulse Width must be 500 ~ 700 us.
       according to the datasheet of PL-IRM0208-A358 recever module.
    */
    _delay_us(500);

  }
}

