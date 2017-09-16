/*
mmc.c:
see mmc.h for details.
*/

#include "mmc.h"

void uart_init(void)
{
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

void error(const char * mesg)
{
  uart_puts(mesg);
  uart_put_CR_LF();
}

void error_r1(uint8_t r1)
{
  error("R1:");
  
  if(r1 & MMC_R1_BUSY)
    error("MMC_R1_BUSY "); //not defined in prodmanualv1.9.
  if(r1 & MMC_R1_PARAMETER)
    error("Parameter Error");
  if(r1 & MMC_R1_ADDRESS)
    error("Address Error ");
  if(r1 & MMC_R1_ERASE_SEQ)
    error("Erase_Seq_Error ");
  if(r1 & MMC_R1_COM_CRC)
    error("Com CRC Error ");
  if(r1 & MMC_R1_ILLEGAL_COM)
    error("Illegal Command ");
  if(r1 & MMC_R1_ERASE_RESET)
    error("ERASE_RESET ");
  if(r1 & MMC_R1_IDLE_STATE)
    error("IN_IDLE_STATE ");
  
}

void error_dr(uint8_t dr)
{
  error("Data Response:");
  
  if((dr & MMC_DR_MASK) == MMC_DR_REJECT_CRC)
    error("Data rejected due to a CRC error.");
  else if((dr & MMC_DR_MASK) == MMC_DR_REJECT_WRITE_ERROR)
    error("Date rejected due to a Write Error.");
  else
    error("Unkown Error.");
  
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
  
  // reset MMC, r1 must be 0x01(in idle state).
  if(mmc_send_cmd(MMC_GO_IDLE_STATE, 0) != MMC_R1_IDLE_STATE)
    return 0; //error exit.

  // initializing card for operation  
  // loop until r1 is 0.
  while(mmc_send_cmd(MMC_SEND_OP_COND, 0) != 0)
    ;
  
  // turn off CRC checking to simplify communication (seems default)
  mmc_send_cmd(MMC_CRC_ON_OFF, 0);
  
  // set block length to 512 bytes (default)
  mmc_send_cmd(MMC_SET_BLOCKLEN, 512);

  SS_PORT |= (1 << SS_PIN); // disable MMC
  return 1;

}

uint8_t mmc_write_sector(uint32_t sect_addr, char *buf)
{
  int i;
  /* response format 1 or Data Response for write*/
  uint8_t resp;

  SS_PORT &= ~(1 << SS_PIN); // enable MMC
  
  /* the sector(block) length is always 512 bytes, byte address must 
     be aligned on a sector boundary.
  */
  // set block length to 512 byte in order to be avoid from other pre-setting.
  mmc_send_cmd(MMC_SET_BLOCKLEN, 512);

  if((resp=mmc_send_cmd(MMC_WRITE_BLOCK, sect_addr<<9)) != 0)
    {
      error_r1(resp);
      return resp;
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
  
  /*Every data block written to the card is acknowledged by a data response token.
    it is one byte long. see datasheet for details.
  */
  //read data response token.
  resp = spi_transfer_byte(0xFF);
  if((resp & MMC_DR_MASK) != MMC_DR_ACCEPT)
    { 
      error_dr(resp);
      return resp;
    }
  
  // wait until MMC is not busy anymore
  while(spi_transfer_byte(0xFF) != 0xFF)
    ;
  
  SS_PORT |= (1 << SS_PIN); // disable MMC
  return 0;
}

uint8_t mmc_read_block(uint32_t blk_addr, char *buf, int blk_size_byte)
{
  int i;
  uint8_t r1;

  SS_PORT &= ~(1 << SS_PIN); // enable MMC
  // set block length to blk_size.
  mmc_send_cmd(MMC_SET_BLOCKLEN, blk_size_byte);
	
  //problem:
  //on my 8M panasonic sd card, even read successfully, the r1 is 0xff just as fail.
  if((r1=mmc_send_cmd(MMC_READ_SINGLE_BLOCK, blk_addr*blk_size_byte)) != 0)
    {
      //error_r1(r1);
      //error? but maybe it does works.
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

  /*from datasheet 5.2.5:
    if a read operation fails and the card cannot provide the required data it will send
    a data error token, instead.
    so, a error handle routine(named error_det()) should be added here.
   */
  
  SS_PORT |= (1 << SS_PIN); // disable MMC
  return 0;
}

