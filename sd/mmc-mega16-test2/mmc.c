/*********************************************
* Chip type           : ATmega16
* Clock frequency     : 4000000Hz

It do works well.

hardware:
PC(minicom,ttyUSB1)<-usb->ft232bm board<-->atmega16L<-->sd card dev board
1)
PC(minicom,ttyUSB1,96008N1)<--usb--> ft232bm bread board.
2)
ft232bm bread board <--> atmega16L bread board
TXD(PIN25) <--> RXD(PD0)
RXD(PIN24) <--> TXD(PD1)
note:
ft232bm bread board use Vbus(+5v) from PC,
and atmega16L use VTG(+5V) from stk500.
there is *NOT* common GND.
3)
atmega16L bread board <--8 route Switch--> sd card dev board
SS <--> CS(SS)
MOSI <--> DI (MOSI)
MISO <--> DO (MISO)
SCK <--> SCK
GND <--> GND
VTG <--> VCC (which is changed to 3.3v by dev board.)
note:
The switch is needed because of programming error.

high status of DI, SCK, CS is changed to 3.3v by dev board.

4)
ft232bm bread board schematics see also the diagram on the wall.
5)
sd card dev board schematics see also the diagram on the wall.

This code needs rewrite.
references:
1. ProdManualSDCardv1.9.pdf
2. procyon avrlib: mmc.c mmc.h
3. ref-code.txt

and error handle code should be added.

*********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>

#define F_CPU 4000000		           /* oscillator-frequency in Hz */
#include <util/delay.h>

#define UART_BAUD_RATE 9600
#define UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) ((F_CPU)/((UART_BAUD_RATE)*16l)-1)

#define SPIDI	6	// Port B bit 6 (pin7): data in (data from MMC)
#define SPIDO	5	// Port B bit 5 (pin6): data out (data to MMC)
#define SPICLK	7	// Port B bit 7 (pin8): clock
#define SPICS	4	// Port B bit 4 (pin5: chip select for MMC)

char sector[512];

void usart_putc(unsigned char c) {
   // wait until UDR ready
	while(!(UCSRA & (1 << UDRE)));
	UDR = c;    // send character
}

void uart_puts (char *s) {
	//  loop until *s != NULL
	while (*s) {
		usart_putc(*s);
		s++;
	}
}

void init(void) {
	// set baud rate
	UBRRH = (uint8_t)(UART_BAUD_CALC(UART_BAUD_RATE,F_CPU)>>8);
	UBRRL = (uint8_t)UART_BAUD_CALC(UART_BAUD_RATE,F_CPU);
	// Enable receiver and transmitter; enable RX interrupt
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	//asynchronous 8N1
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
}

// INTERRUPT can be interrupted
// SIGNAL can't be interrupted
SIGNAL (SIG_UART_RECV) { // USART RX interrupt
	unsigned char c;
	c = UDR;
	usart_putc(c);
}

void serialterminate(void) { // terminate sent string!!!
	while(!(UCSRA & (1 << UDRE)));
	UDR = 0x0d;
	while(!(UCSRA & (1 << UDRE)));
	UDR = 0x0a;	
}

void SPIinit(void) {
	DDRB &= ~(1 << SPIDI);	// set port B SPI data input to input
	DDRB |= (1 << SPICLK);	// set port B SPI clock to output
	DDRB |= (1 << SPIDO);	// set port B SPI data out to output 
	DDRB |= (1 << SPICS);	// set port B SPI chip select to output
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
	PORTB &= ~(1 << SPICS);	// set chip select to low (MMC is selected)
}

char SPI(char d) {  // send character over SPI
	char received = 0;
	SPDR = d;
	while(!(SPSR & (1<<SPIF)));
	received = SPDR;
	return (received);
}

/*
should be rewrite:
use uint32_t for parameter.
and befH is fixed to 0x95
*/
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
	PORTB |= (1 << SPICS); // disable MMC
	// start MMC in SPI mode
	for(i=0; i < 10; i++) SPI(0xFF); // send 10*8=80 clock pulses
	PORTB &= ~(1 << SPICS); // enable MMC

	if (Command(0x40,0,0,0x95) != 1) goto mmcerror; // reset MMC

st: // if there is no MMC, prg. loops here
	if (Command(0x41,0,0,0xFF) !=0) goto st;
	return 1;
mmcerror:
	return 0;
}

void fillram(void)	 { // fill RAM sector with ASCII characters
	int i,c;
	char mystring[18] = "BAICHI was here!!!";
	c = 0;
	for (i=0;i<=512;i++) {
		sector[i] = mystring[c];
		c++;
		if (c > 17) { c = 0; }
	}
}

int writeramtommc(void) { // write RAM sector to MMC
	int i;
	uint8_t c;
	// 512 byte-write-mode
	// cmd24: 0x40|24=0x58
	//the unit of 32 bit address(as parameter) is Byte.
	// use uint32_t for 32bit parameter should be better.
	// the crc 0x95 is only needed by cmd0.
	if (Command(0x58,0,512*2,0x95) !=0) {
	  uart_puts("MMC: write error 1 ");
	  //report error 1, but it write card well. return value problem!
	  //return 1;	
	}
	SPI(0xFF);
	SPI(0xFF);
	SPI(0xFE);
	// write ram sectors to MMC
	for (i=0;i<512;i++) {
		SPI(sector[i]);
	}
	// at the end, send 2 dummy bytes
	SPI(0xFF);
	SPI(0xFF);

	c = SPI(0xFF);
	c &= 0x1F; 	// 0x1F = 0b.0001.1111;
	if (c != 0x05) { // 0x05 = 0b.0000.0101
		uart_puts("MMC: write error 2 ");
		return 1;
	}
	// wait until MMC is not busy anymore
	while(SPI(0xFF) != (char)0xFF);
	return 0;
}

int sendmmc(void) { // send 512 bytes from the MMC via the serial port
	int i;
	// 512 byte-read-mode 
	//cmd17: 0x40+17=0x51
	if (Command(0x51,0,512*2,0x95) != 0) {
		uart_puts("MMC: read error 1 ");
		// report read error 1, but it read well.
		//		return 1;
	}
	// wait for 0xFE - start of any transmission
	// ATT: typecast (char)0xFE is a must!
	while(SPI(0xFF) != (char)0xFE);

	for(i=0; i < 512; i++) {
		while(!(UCSRA & (1 << UDRE))); // wait for serial port
		UDR = SPI(0xFF);  // send character
		if(i%18 == 17)
		  serialterminate();
	}
	serialterminate();

	// at the end, send 2 dummy bytes
	SPI(0xFF); // actually this returns the CRC/checksum byte
	SPI(0xFF);
	return 0;
}

int sendmmc2(void) { // send 512 bytes from the MMC via the serial port
	int i;
	// 512 byte-read-mode 
	//cmd17: 0x40+17=0x51
	if (Command(0x51,0,512,0x95) != 0) {
		uart_puts("MMC: read error 1 ");
		// report read error 1, but it read well.
		//		return 1;
	}
	// wait for 0xFE - start of any transmission
	// ATT: typecast (char)0xFE is a must!
	while(SPI(0xFF) != (char)0xFE);

	for(i=0; i < 512; i++) {
		while(!(UCSRA & (1 << UDRE))); // wait for serial port
		UDR = SPI(0xFF);  // send character
		if(i%18 == 17)
		  serialterminate();
	}
	serialterminate();

	// at the end, send 2 dummy bytes
	SPI(0xFF); // actually this returns the CRC/checksum byte
	SPI(0xFF);
	return 0;
}

int main(void) {
	init();
	SPIinit();

	uart_puts("MCU online");
	serialterminate();

	MMC_Init();

	uart_puts("MMC online");
	serialterminate();

	sei(); // enable interrupts
	
	fillram();
	writeramtommc();
	sendmmc();
	sendmmc2();
	
	uart_puts("512 bytes sent");
	serialterminate();
	uart_puts("The end");
	serialterminate();

	while (1) {
	}
	return 0;
}

