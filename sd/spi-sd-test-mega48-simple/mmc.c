/*********************************************
* Chip type           : ATmega16
* Clock frequency     : 2457600Hz
*********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/signal.h>
#include <inttypes.h>
#include <avr/iom16.h>

#define F_OSC 2457600		           /* oscillator-frequency in Hz */
#define UART_BAUD_RATE 9600
#define UART_BAUD_CALC(UART_BAUD_RATE,F_OSC) ((F_OSC)/((UART_BAUD_RATE)*16l)-1)

#define SPIDI	6	// Port B bit 6 (pin7): data in (data from MMC)
#define SPIDO	5	// Port B bit 5 (pin6): data out (data to MMC)
#define SPICLK	7	// Port B bit 7 (pin8): clock
#define SPICS	4	// Port B bit 4 (pin5: chip select for MMC

char sector[512];

void delay_ms(unsigned short ms) {
	unsigned short outer1, outer2;
	outer1 = 200; 
	while (outer1) {
		outer2 = 1000;
		while (outer2) {
			while ( ms ) ms--;
			outer2--;
		}
		outer1--;
	}
}

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
	UBRRH = (uint8_t)(UART_BAUD_CALC(UART_BAUD_RATE,F_OSC)>>8);
	UBRRL = (uint8_t)UART_BAUD_CALC(UART_BAUD_RATE,F_OSC);
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
	char mystring[18] = "Captain was here! ";
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
	if (Command(0x58,0,512,0xFF) !=0) {
		uart_puts("MMC: write error 1 ");
		return 1;	
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
	if (Command(0x51,0,512,0xFF) != 0) {
		uart_puts("MMC: read error 1 ");
		return 1;
	}
	// wait for 0xFE - start of any transmission
	// ATT: typecast (char)0xFE is a must!
	while(SPI(0xFF) != (char)0xFE);

	for(i=0; i < 512; i++) {
		while(!(UCSRA & (1 << UDRE))); // wait for serial port
		UDR = SPI(0xFF);  // send character
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

	uart_puts("512 bytes sent");
	serialterminate();
	uart_puts("blinking LED now");
	serialterminate();

	// enable  PD5 as output
	DDRD |= (1<<PD5);
	while (1) {
		// PIN5 PORTD clear -> LED off
		PORTD &= ~(1<<PD5);
		delay_ms(500);
		// PIN5 PORTD set -> LED on
		PORTD |= (1<<PD5);
		delay_ms(500);	
	}
	return 0;
}

