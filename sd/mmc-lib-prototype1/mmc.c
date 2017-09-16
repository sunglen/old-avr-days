/*********************************************
* Chip type           : ATmega16
* Clock frequency     : 4000000Hz

It works well.

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
vMOSI <--> DI (MOSI)
MISO <--> DO (MISO)
SCK <--> SCK
GND <--> GND
VTG <--> VCC (which is changed to 3.3v by dev board.)
note:
high status of DI, SCK, CS is changed to 3.3v by dev board.

4)
ft232bm bread board schematics see also the diagram on the wall.
5)
sd card dev board schematics see also the diagram on the wall.

*********************************************/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>

/* oscillator-frequency in Hz */
#define F_CPU 4000000UL
#include <util/delay.h>

/*
about uart, see datasheet for details.
atmega16:
when F_CPU = 4MHz, to generate a baud of 9600,
UBRR is set to 25. (4e6/(16*9600) -1)
*/
#define UART_BAUD_RATE 9600
/* must devided by '16L', or '16l', NOT only '16'*/
#define UART_BAUD_CALC(UART_BAUD_RATE,F_CPU) ((F_CPU)/((UART_BAUD_RATE)*16L)-1)

/* define the SPI port */
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define MISO_PIN PB6
#define MOSI_PIN PB5
#define SCK_PIN PB7

#define SS_DDR DDRB
#define SS_PORT PORTB
#define SS_PIN PB4

/* the command set define comes apart from proyon avrlib,
but read the sandisk sd card product manual rev1.9, 
CMD18(READ_MULTIPLE_BLOCK) and CMD25(WRITE_MULTIPLE_BLOCK) are not include.
*/
// MMC commands (taken from sandisk MMC reference), add a 'MMC_' prefix.

#define MMC_GO_IDLE_STATE			0		///< initialize card to SPI-type access
#define MMC_SEND_OP_COND			1		///< set card operational mode
#define MMC_SEND_CSD				9		///< get card's CSD
#define MMC_SEND_CID				10		///< get card's CID
#define MMC_SEND_STATUS				13
#define MMC_SET_BLOCKLEN			16		///< Set number of bytes to transfer per block
#define MMC_READ_SINGLE_BLOCK		17		///< read a block
#define MMC_WRITE_BLOCK				24		///< write a block
#define MMC_PROGRAM_CSD				27
#define MMC_SET_WRITE_PROT			28
#define MMC_CLR_WRITE_PROT			29
#define MMC_SEND_WRITE_PROT			30
#define MMC_TAG_SECTOR_START		        32
#define MMC_TAG_SECTOR_END			33
#define MMC_UNTAG_SECTOR			34
#define MMC_TAG_ERASE_GROUP_START 	35		///< Sets beginning of erase group (mass erase)
#define MMC_TAG_ERARE_GROUP_END		36		///< Sets end of erase group (mass erase)
#define MMC_UNTAG_ERASE_GROUP		37		///< Untag (unset) erase group (mass erase)
#define MMC_ERASE					38		///< Perform block/mass erase
#define MMC_CRC_ON_OFF				59		///< Turns CRC check on/off

// R1 Response bit-defines
#define MMC_R1_BUSY			        0x80	///< R1 response: bit indicates card is busy
#define MMC_R1_PARAMETER			0x40
#define MMC_R1_ADDRESS				0x20
#define MMC_R1_ERASE_SEQ			0x10
#define MMC_R1_COM_CRC				0x08
#define MMC_R1_ILLEGAL_COM			0x04
#define MMC_R1_ERASE_RESET			0x02
#define MMC_R1_IDLE_STATE			0x01

// Data Start tokens
#define MMC_STARTBLOCK_READ			0xFE	///< when received from card, indicates that a block of data will follow
#define MMC_STARTBLOCK_WRITE		        0xFE	///< when sent to card, indicates that a block of data will follow
#define MMC_STARTBLOCK_MWRITE	        	0xFC
// Data Stop tokens
#define MMC_STOPTRAN_WRITE			0xFD
// Data Error Token values
#define MMC_DE_MASK				0x1F
#define MMC_DE_ERROR				0x01
#define MMC_DE_CC_ERROR				0x02
#define MMC_DE_ECC_FAIL				0x04
#define MMC_DE_OUT_OF_RANGE			0x04
#define MMC_DE_CARD_LOCKED			0x04

// Data Response Token values
#define MMC_DR_MASK		      		0x1F
#define MMC_DR_ACCEPT				0x05
#define MMC_DR_REJECT_CRC			0x0B
#define MMC_DR_REJECT_WRITE_ERROR	        0x0D

char mysector[512];

void uart_init(void) {
  // set baud rate
  UBRRH = (uint8_t)(UART_BAUD_CALC(UART_BAUD_RATE,F_CPU)>>8);
  UBRRL = (uint8_t)UART_BAUD_CALC(UART_BAUD_RATE,F_CPU);

  // Enable receiver and transmitter; enable RX interrupt(for ISR(RX){echo})
  UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
  //asynchronous 8N1
  UCSRC = (1<<URSEL)|(3<<UCSZ0);
  sei();
}

void uart_putc(unsigned char c) 
{
  // wait until UDR ready
  while(!(UCSRA & (1 << UDRE)));
  UDR = c;    // send character
}

void uart_puts (char *s) 
{
  //  loop until *s != NULL
  while (*s) {
    uart_putc(*s);
    s++;
  }
}

void uart_put_CR_LF(void) 
{
  while(!(UCSRA & (1 << UDRE)));
  UDR = 0x0d;
  while(!(UCSRA & (1 << UDRE)));
  UDR = 0x0a;	
}


/* 
echo character through uart RX ISR, just for uart receiver test.
old style.
*/
SIGNAL (SIG_UART_RECV) {
  unsigned char c;
  c = UDR;
  //echo
  uart_putc(c);
}

void spi_master_init(void)
{
  volatile char IOReg;
  // configure /SS, MOSI, SCK as output pin, MISO is auto configured as input.
  SPI_DDR |= ((1<<MOSI_PIN)|(1<<SCK_PIN));
  SS_DDR |= (1<<SS_PIN);
  //F_sck=F_osc/128. attention: SPIE should NOT be enable.
  SPCR = ((1<<SPE)|(1<<MSTR)|(1<<SPR0)|(1<<SPR1));
  // clear SPI interrupt flag reading SPSR and SPDR.
  IOReg = SPSR;
  IOReg = SPDR;
  // pull down ss.
  SS_PORT &= ~(1<<SS_PIN);
}

/* return received byte.*/
uint8_t spi_transfer_byte(uint8_t c) 
{
  uint8_t rev;
  
  SPDR = c;
  while(!(SPSR & (1<<SPIF)));
  rev = SPDR;
  return rev;
}

// sends a command to the MMC
uint8_t mmc_send_cmd(uint8_t cmd, uint32_t arg)
{
  spi_transfer_byte(0xFF);
 
  spi_transfer_byte(0x40|cmd);
  
  spi_transfer_byte(arg>>24);
  spi_transfer_byte((uint8_t)(arg>>16));
  spi_transfer_byte((uint8_t)(arg>>8));
  spi_transfer_byte((uint8_t)arg);
  
  // crc is valid only for MMC_GO_IDLE_STATE
  spi_transfer_byte(0x95);
  
  spi_transfer_byte(0xFF);
  // return the last received character
  return spi_transfer_byte(0xFF);	
}

uint8_t mmc_init(void) 
{
  int i;
  
  spi_master_init();
  
  SS_PORT |= (1 << SS_PIN); // disable MMC
  
  // send dummy bytes with CS(SS) high before accessing
  for(i=0; i < 10; i++)
    spi_transfer_byte(0xFF); // send 10*8=80 clock pulses
  
  SS_PORT &= ~(1 << SS_PIN); // enable MMC
  
  // reset MMC
  if(mmc_send_cmd(MMC_GO_IDLE_STATE, 0) != 1)
    return 0;

  // initializing card for operation  
  // loop until r1 is 0.
  while(mmc_send_cmd(MMC_SEND_OP_COND, 0) != 0)
    ;
  
  // turn off CRC checking to simplify communication (seems default)
  mmc_send_cmd(MMC_CRC_ON_OFF, 0);
  
  // set block length to 512 bytes (default)
  mmc_send_cmd(MMC_SET_BLOCKLEN, 512);
  
  return 1;

}

uint8_t mmc_write_sector(uint32_t sect_addr, char *buf)
{
  int i;
  /* response format 1 */
  uint8_t r1;
	
  /* the sector(block) length is always 512 bytes, byte address must 
     be aligned on a sector boundary.
  */
  // set block length to 512 byte in order to be avoid from other pre-setting.
  mmc_send_cmd(MMC_SET_BLOCKLEN, 512);

  if((r1=mmc_send_cmd(MMC_WRITE_BLOCK, sect_addr<<9)) != 0)
    {
      uart_puts("MMC_WRITE_BLOCK: r1 not zero:");
      uart_put_CR_LF();
      //uart_putc(r1);
      return r1;
    }
  spi_transfer_byte(0xFF);
  spi_transfer_byte(0xFF);
  
  /* send start block token 0xFE for:
     single block read, single block write, and multiple block read.
     see 5.2.4 of sd datasheet for details.
   */
  spi_transfer_byte(MMC_STARTBLOCK_WRITE);
  
  for (i=0;i<512;i++)
    spi_transfer_byte(buf[i]);
  
  // at the end, send 2 dummy bytes
  spi_transfer_byte(0xFF);
  spi_transfer_byte(0xFF);

  //read data response token.
  r1 = spi_transfer_byte(0xFF);
  if((r1 & MMC_DR_MASK) != MMC_DR_ACCEPT)
    { 
      uart_puts("r1&0x1F is NOT 0x05:");
      uart_put_CR_LF();
      return r1;
    }
  
  // wait until MMC is not busy anymore
  while(spi_transfer_byte(0xFF) != 0xFF)
    ;
  
  return 0;
}

uint8_t mmc_read_block(uint32_t blk_addr, char *buf, int blk_size_byte)
{
  int i;
  uint8_t r1;

  // set block length to blk_size.
  mmc_send_cmd(MMC_SET_BLOCKLEN, blk_size_byte);
	
  if((r1=mmc_send_cmd(MMC_READ_SINGLE_BLOCK, blk_addr*blk_size_byte)) != 0)
    {
      uart_puts("MMC_READ_SINGLE_BLOCK: r1 is not zero.");
      uart_put_CR_LF();
      //error? but it does works.
      //return r1;
    }
  
  // wait for start token: 0xFE 
  while(spi_transfer_byte(0xFF) != MMC_STARTBLOCK_READ)
    ;

  for(i=0; i < blk_size_byte; i++)
    buf[i] = spi_transfer_byte(0xFF);  // send character

  // at the end, send 2 dummy bytes
  // actually this returns 16 bit CRC/checksum.
  spi_transfer_byte(0xFF); 
  spi_transfer_byte(0xFF);

  return 0;
}


/* just for test */
void fillram(void)	 
{ 
  int i,c;
  char mystring[18] = "1234567890abcdefgh";
  c = 0;
  for (i=0;i<=512;i++) {
    mysector[i] = mystring[c];
    c++;
    if (c > 17) { c = 0; }
  }
}

/* show usage of this lib:
   read and write example.
 */
int main(void) {
  
  int i;
  char buffer[9]={'\0'};
  
  uart_init();

  uart_puts("uart online");
  uart_put_CR_LF();
  
  mmc_init();

  uart_puts("MMC online");
  uart_put_CR_LF();

  fillram();
	
  mmc_write_sector(0, mysector);
  
  for(i=0;i<64;i++)
    {
      mmc_read_block(i, buffer, 8);
      
      uart_puts(buffer);
      uart_put_CR_LF();
    }

  uart_puts("write sector 1...");
  uart_put_CR_LF();
  
  mmc_write_sector(1, mysector);
  
  mmc_read_block(65, buffer, 8);
      
  uart_puts(buffer);
  uart_put_CR_LF();
  
  
  uart_puts("The end");
  uart_put_CR_LF();

  while(1)
    ;
  
  return 0;
}

