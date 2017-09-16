/*
adc-test-7disp.c

Just show the ADC value on 7_seg_disp.
see source code for hardware connection.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

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



volatile uint16_t analog_result;
volatile uint8_t analog_busy;

void init_disp(void)
{
  DDR_SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  DDR_SCAN_PORT |= ((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
  SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
}

void init_adc_single(void)
{
  /* hardware: (mega48)
     27(ADC4)<--INPUT VOLTAGE (eg: Vout of LM35 temp sensor)
     22(GND)-->GND
     21(AREF)-->0.1uF(104)-->GND
     20(AVCC)-->VCC
  */
     
  analog_busy = 1;
  /*改变通道及基准源*/
  /*use adc4 and internal 1.1v voltage.*/
  ADMUX = ((1<<REFS1)|(1<<REFS0)|0x04);
  
  /*adc control: prescaler, clear ADIF, interrupt enable, etc*/
  /* ADC prescaler: f_adc=f_osc/8=8MHz/8=1MHz */
  ADCSRA = ((1<<ADEN)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS1)|(1<<ADPS0));

  /*连续转换模式:
    ADCSRA |= (1<<ADATE);
    选择触发源:
    ADCSRB = ...;
    see datasheet p231
  */
    
  /*enable interrupt*/
  sei();

  /*then, ADCSRA |= (1<<ADSC) can start convention.*/
}

ISR (ADC_vect)
{
  uint8_t adlow, adhigh;

  adlow = ADCL;
  adhigh = ADCH;
  analog_result = ((adhigh<<8)|adlow);
  analog_busy = 0;
}

int main (void)
{
  uint8_t buffer[12];
  
  init_disp();
  
  init_adc_single();
  
  /* init buffer for correct display*/
  buffer[0]=0x00;
  buffer[4]=0x11;
  buffer[8]=0xFF;
    
  buffer[1]=0x00;
  buffer[5]=0x11;
  buffer[9]=0xFF;
    
  buffer[2]=0x00;
  buffer[6]=0x11;
  buffer[10]=0xFF;

  buffer[3]=0x00;
  buffer[7]=0x11;
  buffer[11]=0xFF;

  /* Display buffer that is approx. 4*3ms=12ms per frame.*/
  for (;;) {
    /* start single convection*/
    ADCSRA |= (1<<ADSC);
    _delay_us(500);
    
    buffer[0] = analog_result/1000;
    buffer[1] = (analog_result%1000)/100;
    buffer[2] = (analog_result%100)/10;
    buffer[3] = analog_result%10;
  
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[0])|pgm_read_byte(_7_seg_font+buffer[4]))&buffer[8]);
    SCAN_PORT |= (1<<DISP3);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[1])|pgm_read_byte(_7_seg_font+buffer[5]))&buffer[9]);
    SCAN_PORT |= (1<<DISP2);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[2])|pgm_read_byte(_7_seg_font+buffer[6]))&buffer[10]);
    SCAN_PORT |= (1<<DISP1);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[3])|pgm_read_byte(_7_seg_font+buffer[7]))&buffer[11]);
    SCAN_PORT |= (1<<DISP0);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
  }
	
}
