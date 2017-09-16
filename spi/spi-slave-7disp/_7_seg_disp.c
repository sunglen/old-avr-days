/*
_7_seg_disp.c :

a mega48 act as a spi slave, (through the interrupt routine),
to receive 8bit instructiones from a spi master, decode it,
and store it in buffer. the main loop show the content of the 
buffer in four _7_segment_led_displays.

connection:
SCK <-- SPI Master SCK
MISO --> SPI Master MISO
MOSI <-- SPI Master MOSI
SS <-- SPI Master Control Port
see also #define for details
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 interal RC 8MHz/8=1MHz, default*/
//#define F_CPU 1000000UL

/* use mega48 interal RC 8MHz/1=1MHz lfuse=0xe2*/
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

/* define the SPI port */
#define DDR_SPI DDRB
#define SPI_PORT PORTB
#define MISO PB4

/*
INSTRUCTION DEFINITION
OF
THIS SPI SLAVE 7_SEGMENT_LED DISPLAY.
{
|bit7|bit6|...|bit0|

bit0 to bit3: to store data 0x00 to 0x0F,
is accordance with number 0,1,2,3..,E,F.

bit4: low bit of position
bit5: high bit of position
0b00 -- right most
0b01 -- 2nd from right
0b10 -- 2nd from left
0b11 -- left most

bit6: display with dot?
0 -- without dot
1 -- with dot

bit7: clear display(only clear the display set by bit4,5)
0 -- do nothing
1 -- clear display
}
*/
#define POS_L 4
#define POS_H 5
#define WITH_DOT 6
#define CLEAR_DISP 7

 
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



//volatile uint8_t buffer[12]={0};
//volatile uint8_t buffer[12]={0x05,0x07,0x03,0x01,0x10,0x11,0x11,0x11,0xFF,0xFF,0xFF,0xFF};

volatile uint8_t buffer[12]={0};



void init_disp(void)
{
  DDR_SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  DDR_SCAN_PORT |= ((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
  SEG_PORT |= ((1<<SEG_a)|(1<<SEG_b)|(1<<SEG_c)|(1<<SEG_d)|(1<<SEG_e)|(1<<SEG_f)|(1<<SEG_g)|(1<<SEG_dp));
  SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
}

void init_spi_slave(void)
{
  volatile char IOReg;
  DDR_SPI = (1<<MISO);
  SPCR = ((1<<SPE)|(1<<SPIE));
  // clear SPI interrupt flag reading SPSR and SPDR.
  IOReg = SPSR;
  IOReg = SPDR;
  // enable global interrupt.
  sei();
}

/*
add ISR code here.

the ISR 1)receive data from spi master,
2)decode it (since the data is 8bit, I must define a
set of instruction to work efficiently.)
and store to buffer.
*/

ISR (SPI_STC_vect)
{
  volatile uint8_t inst;  
  uint8_t pos;
  
  // receive 8bit instruction from master.
  inst=SPDR;
  // decode the instruction
  //extract position.
  pos = ((inst&((1<<POS_L)|(1<<POS_H)))>>4);
  //extract data
  buffer[pos] = inst&0x0F;
  //with dot?
  if(inst&(1<<WITH_DOT))
    buffer[pos+4]=0x10;
  else
    buffer[pos+4]=0x11;
  //clear display?
  if(inst&(1<<CLEAR_DISP))
    buffer[pos+8]=0x00;
  else
    buffer[pos+8]=0xFF;
}


int main (void)
{
  
  init_spi_slave();

  init_disp();

  /* for test:display 5.731  */
  buffer[0]=0x05;
  buffer[4]=0x10;
  buffer[8]=0xFF;
    
  buffer[1]=0x07;
  buffer[5]=0x11;
  buffer[9]=0xFF;
    
  buffer[2]=0x03;
  buffer[6]=0x11;
  buffer[10]=0xFF;

  buffer[3]=0x01;
  buffer[7]=0x11;
  buffer[11]=0xFF;

  /* Display buffer that is approx. 4*3ms=12ms per frame.*/
  for (;;) {

    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[0])|pgm_read_byte(_7_seg_font+buffer[4]))&buffer[8]);
    SCAN_PORT = (1<<DISP3);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[1])|pgm_read_byte(_7_seg_font+buffer[5]))&buffer[9]);
    SCAN_PORT = (1<<DISP2);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[2])|pgm_read_byte(_7_seg_font+buffer[6]))&buffer[10]);
    SCAN_PORT = (1<<DISP1);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
    SEG_PORT = ~((pgm_read_byte(_7_seg_font+buffer[3])|pgm_read_byte(_7_seg_font+buffer[7]))&buffer[11]);
    SCAN_PORT = (1<<DISP0);
    _delay_ms(LIGHTED_TIME_ms);
    SCAN_PORT &= ~((1<<DISP0)|(1<<DISP1)|(1<<DISP2)|(1<<DISP3));
    
  }
	
}
