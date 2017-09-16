/*********************************************
mmc.h:

It works well on following env:

* Chip type           : ATmega16
* Clock frequency     : 4000000Hz

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

#define MMC_GO_IDLE_STATE  0  //initialize card to SPI-type access
#define MMC_SEND_OP_COND   1  //set card operational mode
#define MMC_SEND_CSD	   9  //get card's CSD
#define MMC_SEND_CID	   10 //get card's CID
#define MMC_SEND_STATUS	   13
#define MMC_SET_BLOCKLEN   16 //Set number of bytes to transfer per block
#define MMC_READ_SINGLE_BLOCK  17  //read a block
#define MMC_READ_MULTIPLE_BLOCK 18 //read multiple block
#define MMC_WRITE_BLOCK	       24  //write a block
#define MMC_WRITE_MULTIPLE_BLOCK 25 //write multiple block
#define MMC_PROGRAM_CSD			        27
#define MMC_SET_WRITE_PROT			28
#define MMC_CLR_WRITE_PROT			29
#define MMC_SEND_WRITE_PROT			30
#define MMC_TAG_SECTOR_START		        32
#define MMC_TAG_SECTOR_END			33
#define MMC_UNTAG_SECTOR			34
#define MMC_TAG_ERASE_GROUP_START  35  //Sets beginning of erase group (mass erase)
#define MMC_TAG_ERARE_GROUP_END	   36  //Sets end of erase group (mass erase)
#define MMC_UNTAG_ERASE_GROUP	   37  //Untag (unset) erase group (mass erase)
#define MMC_ERASE		   38  //Perform block/mass erase
#define MMC_CRC_ON_OFF		   59  //Turns CRC check on/off

// R1 Response bit-defines (response to command)
//R1 response: bit indicates card is busy
#define MMC_R1_BUSY			        0x80	
#define MMC_R1_PARAMETER			0x40
#define MMC_R1_ADDRESS				0x20
#define MMC_R1_ERASE_SEQ			0x10
#define MMC_R1_COM_CRC				0x08
#define MMC_R1_ILLEGAL_COM			0x04
#define MMC_R1_ERASE_RESET			0x02
#define MMC_R1_IDLE_STATE			0x01

// Data Start tokens for single block read, write and multiple block read.
//when received from card, indicates that a block of data will follow
#define MMC_STARTBLOCK_READ			0xFE	
//when sent to card, indicates that a block of data will follow
#define MMC_STARTBLOCK_WRITE		        0xFE	

// For multiple block write operation:
// first byte of each block. instead of MMC_STARTBLOCK_WRITE(0xFE)
#define MMC_STARTBLOCK_MWRITE	        	0xFC
// Data Stop tokens
#define MMC_STOPTRAN_WRITE			0xFD

// Data Error Token values (read error)
#define MMC_DE_MASK				0x1F
#define MMC_DE_ERROR				0x01
#define MMC_DE_CC_ERROR				0x02
#define MMC_DE_ECC_FAIL				0x04
#define MMC_DE_OUT_OF_RANGE			0x04
#define MMC_DE_CARD_LOCKED			0x04

// Data Response Token values (write error)
#define MMC_DR_MASK		      		0x1F
#define MMC_DR_ACCEPT				0x05
#define MMC_DR_REJECT_CRC			0x0B
#define MMC_DR_REJECT_WRITE_ERROR	        0x0D

//uart debug
void uart_init(void);
void uart_putc(unsigned char c);
void uart_puts (char *s);
void uart_put_CR_LF(void);

//spi and mmc operation
void error(const char * mesg);
void error_r1(uint8_t r1);
void error_dr(uint8_t dr);
void spi_master_init(void);
uint8_t spi_transfer_byte(uint8_t c);
uint8_t mmc_send_cmd(uint8_t cmd, uint32_t arg);
uint8_t mmc_init(void);
uint8_t mmc_write_sector(uint32_t sect_addr, char *buf);
uint8_t mmc_read_block(uint32_t blk_addr, char *buf, int blk_size_byte);
