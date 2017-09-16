/*
spi-master-test.c

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
  sei();
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
  const unsigned char buffer[] = "I am using SPI!";
  
  uint8_t i=0;
  
  init_spi_master();

  /* for test:
  */
  

  for(i=0;i<1;i++){
    // set /SS (link to /SS of slave) Low to active transmit.  
    SPI_PORT &= ~(1<<SS_PIN);

    
    SPDR='k';
    while (!(SPSR & (1<<SPIF)));    // wait until Char is sent 

      // close transmit.
    SPI_PORT |= (1<<SS_PIN);
  
  }
  
  return 0;
}
