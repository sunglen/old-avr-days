/*
spi-master-test.c

a mega64 act as a spi master, send 8-bit index value to
control a spi-slave-1602-lcd display machine.

!!NOTE:
The test of this source code is with the following slave code:
devel/spi-slave-1602/spi-1602-test.c

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
  // enable global interrupt.(if want to use ISR)
  //  sei();
}

/*
add ISR code here.

ISR (SPI_STC_vect)
{
;
}

*/

int main (void)
{
  uint8_t i;
  init_spi_master();

  /* for test:
  */

  for(;;){
    
    // delay 1s just for test.
    for(i=0;i<100;i++)
      _delay_ms(10);

    // set /SS (link to /SS of slave) Low to active transmit.
    SPI_PORT &= ~(1<<SS_PIN);
    
    // can also work without delay!
    //_delay_us(10);

    for(i=0;i<16;i++){
      SPDR = i;
      while (!(SPSR & (1<<SPIF)));    // wait until Char is sent 
      //if delay = 10 us the slave can only display(received?) C and D,
      // _delay_us(10);
      //if delay = 1 ms, the slave can display(received?) half characters which
      //the master sent. (that means about half data lost).
      //_delay_ms(1);
      //if delay=3ms, the slave can display(received?) 80% part of characters
      //which the master sent.
      //_delay_ms(3);
      
      // delay = 5ms can work fine.
      // "devel/spi-slave-1602/spi-1602-test.c" can received all characters.
      // 可能是slave里:
      /*
      lcd_clrscr();
      lcd_gotoxy(0,0); 
      lcd_puts(buffer);
      */
      // 这段代码比较费时间。
      _delay_ms(5);
    }
    
    // close transmit.
    SPI_PORT |= (1<<SS_PIN);
    
  }
  	
}
