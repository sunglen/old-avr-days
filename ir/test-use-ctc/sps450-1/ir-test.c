/*
ir-test.c :
use ctc mode of TC1 to generate 38KHz frequency.

OC1A(PB1) to generate continuous 38KHz carrier.(receiver module don't work)
but if OC1A generate a pause width, receiver module can also work.

PB2 to generate a code.

see also ouravr forum to get more information.

1)sender module:
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

2)receiver:

SPS-450-1 <-> oscilloscope
OUT <--> oszifox

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

/* define for TC1 and OC1A output */
#define DDR_OC1A DDRB
#define PORT_OC1A PORTB
#define PIN_OC1A PB1

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
  unsigned int i;
  
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
    
    /* generate a code on 38KHz carrier (PB2)
    Just like Figure 1 of sps-450-1 datasheet. 
    */
    
    cli();
    PORTB &= ~(1<<PB2);
    _delay_ms(3);
    
    // send leader. 9ms(38MHz carrier),4.5ms(low)
    sei();
    TCNT1=0x0000;
    _delay_ms(9);

    cli();
    PORTB &= ~(1<<PB2);
    for(i=0;i<45;i++)
      _delay_us(100);
    
    
    //TWL
    sei();
    TCNT1=0x0000;
    _delay_us(600);

    //TWH
    cli();
    PORTB &= ~(1<<PB2);
    _delay_us(600);
    
    //TWL
    sei();
    TCNT1=0x0000;
    _delay_us(600);
    
  }
}

