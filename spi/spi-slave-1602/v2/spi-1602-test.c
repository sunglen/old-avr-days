/*
spi-1602-test.c

a mega48 act as a spi slave, through the interrupt routine,
receive 8-bit index value from a spi master,
get the character from flash array and store it in buffer.
then display it in 1602 lcd.

connection:

mega48 <--> mega64

SCK <-- SPI Master SCK
MISO --> SPI Master MISO
MOSI <-- SPI Master MOSI
SS <-- SPI Master Control Port(/SS of mega64)

see also #define for details

work well with "send-to-1602-slave/spi-master-test.c" !! 2006.3.1
*/

#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>
#include <stdlib.h>

#include "lcd.h"

/* use mega48 interal RC 8MHz/1=1MHz lfuse=0xe2*/
#define F_CPU 8000000UL
#include <util/delay.h>

/* define the SPI port */

#define DDR_SPI DDRB
#define SPI_PORT PORTB
#define MISO PB4



volatile uint8_t q;
const char q_table_Hex[] PROGMEM = {
  'D','1','2','3','4','5','6','7','8','9','0','*','#','A','B','C'
};

unsigned char buffer[33]={'\0'};
uint8_t i=0;



void init_spi_slave(void)
{
  volatile char IOReg;
  DDR_SPI |= (1<<MISO);
  SPCR = ((1<<SPE)|(1<<SPIE));
  // clear SPI interrupt flag reading SPSR and SPDR.
  IOReg = SPSR;
  IOReg = SPDR;
  // enable global interrupt.
  sei();
}

ISR(SPI_STC_vect)
{
  cli();
  
  // receive 8-bit index value from master.
  q=SPDR;

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
  //cli();
  
  lcd_init(LCD_DISP_ON);
  init_spi_slave();
  

  lcd_clrscr();
  lcd_puts("Waiting for Master...\n");
  
  for(;;)
    ;
  
}  
