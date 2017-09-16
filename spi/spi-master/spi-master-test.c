/*
spi-master-test.c

a mega64 act as a spi master, send 8bit instructions to
control a spi-slave-7disp machine.

connection:
1)
attention:
mega64's ISP port:
mega64 -- STK500
PDI (PE0) -- MOSI
PDO (PE1) -- MISO

2)
spi-slave-machine -- mega64
Slave SCK <-- SPI Master SCK 
Slave MISO --> SPI Master MISO
Slave MOSI <-- SPI Master MOSI
Slave SS <-- SPI Master SS
see also #define for details
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega64 interal RC 1MHz */
#define F_CPU 1000000UL
#include <util/delay.h>

/* define the SPI port */
#define DDR_SPI DDRB
#define SPI_PORT PORTB
#define MISO_PIN PB3
#define MOSI_PIN PB2
#define SCK_PIN PB1
#define SS_PIN PB0

/*
INSTRUCTION DEFINITION
OF The 
SPI SLAVE 7_SEGMENT_LED DISPLAY.
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

 
void init_spi_master(void)
{
  volatile char IOReg;
  // configure /SS, MOSI, SCK as output pin, MISO is auto configured as input.
  DDR_SPI = ((1<<MOSI_PIN)|(1<<SCK_PIN)|(1<<SS_PIN));
  //F_sck=F_osc/16
  SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPIE));
  // clear SPI interrupt flag reading SPSR and SPDR.
  IOReg = SPSR;
  IOReg = SPDR;
  // enable global interrupt.
  //sei();
}

/*
add ISR code here.
*/

ISR (SPI_STC_vect)
{
;
}

int main (void)
{
  
  // send string "08.02"
  const unsigned char msg[4]={0x30,0x68,0x10,0x02};
  uint8_t i=0;
  
  init_spi_master();

  /* for test:
  */
  for(;;){
    //_delay_ms(1);
    // set /SS (link to /SS of slave) Low to active transmit.
    SPI_PORT &= ~(1<<SS_PIN);
    for (i=0;i<4;i++) {
      SPDR = *(msg+i);
      while (!(SPSR & (1<<SPIF)));    // wait until Char is sent 
    }
  
    // close transmit.
    SPI_PORT |= (1<<SS_PIN);
  }
  	
}
