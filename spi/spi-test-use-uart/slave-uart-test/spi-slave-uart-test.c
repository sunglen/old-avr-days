/*
spi-slave-uart-test.c :

an at90s8515 act as a spi slave, (through the interrupt routine),
receive characters from a spi master, then send it to serial port,
could be read by minicom.

connection:
90s8515 SPI_MASTER
SCK <-- SPI Master SCK
MISO --> SPI Master MISO
MOSI <-- SPI Master MOSI
SS <-- SPI Master Control Port(Normally /SS)

RxD,TxD <-->My ADM3202AN BOARD<--> minicom of pc, use uart lib.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

#include "uart.h"

#define F_CPU 4000000UL

/* 9600 baud */
#define UART_BAUD_RATE 9600

#include <util/delay.h>

/* define the SPI port */
#define DDR_SPI DDRB
#define SPI_PORT PORTB
#define MISO PB6
#define SS PB4
#define SS_PIN PINB

volatile unsigned char c;

/*
volatile uint8_t i=0;
volatile unsigned char buffer[32]={0};
*/

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
the ISR receive data from spi master,
and just put it to uart.
*/

ISR (SPI_STC_vect)
{
  // receive a character from master.
  c=SPDR;
  
  /*
  if(i>=31)
    i=0;
  buffer[i]=(unsigned char)c;
  buffer[i+1]=(unsigned char)'\0';
  i++;
  */
  
  uart_putc( (unsigned char)c );
}


int main (void)
{
  
  init_spi_slave();

  uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 

  sei();
  
  /* Display buffer that is approx. 4*3ms=12ms per frame.*/

  for (;;){
    //while(!(SPSR&(1<<SPIF)));
    
    //    while(!(SS_PIN & (1<<SS)));
    //uart_puts(buffer);
    ;
  }
}
