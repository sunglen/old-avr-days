/*************************************************************************
test.c:
test access sd card through spi.

hardware:
spi dev board. <--> mega48 <-(same as uart test)-> 1602lcd
           
*************************************************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

#include "lcd.h"

#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>

/* define the SPI port */
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define MISO_PIN PB4
#define MOSI_PIN PB3
#define SCK_PIN PB5
#define SS_PIN PB2

char buffer[20]={'\0'};

void init_spi_master(void)
{
  volatile char IOReg;
  // configure /SS, MOSI, SCK as output pin, MISO is auto configured as input.
  SPI_DDR = ((1<<MOSI_PIN)|(1<<SCK_PIN)|(1<<SS_PIN));
  //F_sck=F_osc/16
  SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1)|(1<<SPIE));
  // clear SPI interrupt flag reading SPSR and SPDR.
  IOReg = SPSR;
  IOReg = SPDR;
  
  SPI_PORT &= ~(1<<SS_PIN);
  // enable global interrupt.
  //sei();
}


char SPI(char d) {  // send character over SPI
	char received = 0;
	SPDR = d;
	while(!(SPSR & (1<<SPIF)));
	received = SPDR;
	return (received);
}


char Command(char befF, uint16_t AdrH, uint16_t AdrL, char befH )
{	// sends a command to the MMC
	SPI(0xFF);
	SPI(befF);
	SPI((uint8_t)(AdrH >> 8));
	SPI((uint8_t)AdrH);
	SPI((uint8_t)(AdrL >> 8));
	SPI((uint8_t)AdrL);
	SPI(befH);
	SPI(0xFF);
	return SPI(0xFF);	// return the last received character
}

int MMC_Init(void) { // init SPI
	char i;
	SPI_PORT |= (1 << SS_PIN); // disable MMC
	// start MMC in SPI mode
	for(i=0; i < 10; i++) SPI(0xFF); // send 10*8=80 clock pulses
	SPI_PORT &= ~(1 << SS_PIN); // enable MMC

	if (Command(0x40,0,0,0x95) != 1) goto mmcerror; // reset MMC

st: // if there is no MMC, prg. loops here
	if (Command(0x41,0,0,0xFF) !=0) goto st;
	return 1;
mmcerror:
	return 0;
}

int writetommc(void) { // write test char to MMC
  int i;
  uint8_t c;
  // 512 byte-write-mode
  if (Command(0x58,0,512,0xFF) !=0) {
    lcd_clrscr();
    lcd_puts("MMC: write error 1 ");
    return 1;	
  }
  SPI(0xFF);
  SPI(0xFF);
  SPI(0xFE);
  //write:
  for (i=0;i<512;i++) {
    SPI('k');
  }
  // at the end, send 2 dummy bytes
  SPI(0xFF);
  SPI(0xFF);

  c = SPI(0xFF);
  c &= 0x1F; 	// 0x1F = 0b.0001.1111;
  if (c != 0x05) { // 0x05 = 0b.0000.0101
    lcd_clrscr();
    lcd_puts("MMC: write error 2 ");
    return 1;
  }
  // wait until MMC is not busy anymore
  while(SPI(0xFF) != (char)0xFF);
  return 0;
}

int readfrommmc(void) { // read
  int i;

  // 512 byte-read-mode 
  if (Command(0x51,0,512,0xFF) != 0) {
    lcd_clrscr();    
    lcd_puts("MMC: read error 1 ");
    return 1;
  }
  // wait for 0xFE - start of any transmission
  // ATT: typecast (char)0xFE is a must!
  while(SPI(0xFF) != (char)0xFE);

  for(i=0; i < 512; i++) {
    buffer[i%19]= SPI(0xFF);  // send character
  }
	
  // at the end, send 2 dummy bytes
  SPI(0xFF); // actually this returns the CRC/checksum byte
  SPI(0xFF);
  return 0;
}



int main(void)
{
  int i;
  init_spi_master();
  
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  lcd_puts("a sd-spi test.\n2006/04/23");
  for(i=0;i<40;i++)
    _delay_ms(100);
  
  MMC_Init();
  lcd_clrscr();
  lcd_puts("MMC online");

  for(i=0;i<40;i++)
    _delay_ms(100);

  sei();
  
  writetommc();
  readfrommmc();
  
  
  lcd_clrscr();
  lcd_putc(buffer[0]);


  for(i=0;i<40;i++)
    _delay_ms(100);
  
  while(1);
    
}
