/*
a demo of mmc-lib prototype.

SD card was written in file 'hzk16.rotated', by command:
#dd if=hzk16.rotated of=/dev/sda bs=32
(hzk16.rotated contain gb-hz 16x16 fonts rotated 90 degree clockwise 
and with no initial offset, /dev/sda is a 8M SD card.
addtional information:
sd_offset can be set if use dd command. in this demo, sd_offset is 0.

this demo read a 32 bytes font from SD card by the given offset,
save it to buffer, then put it to uart.

the correct result read from SD card is:
0075040 00 00 ff fe 40 02 50 0a 50 8a 50 8a 50 8a 5f fa
0075056 50 8a 52 ca 54 8e 50 0a 40 02 ff ff 00 02 00 00
*/

#include "mmc.h"

int main(void) 
{
  int i;
  uint8_t gb_high, gb_low;
  uint16_t offset;
  
  uint8_t buffer[32];
  
  //gb-hz font: guo2(means nation in english), attentiion: ROTATED FORMAT!!
  gb_high=0xb9;
  gb_low=0xfa;

  offset=(gb_high-0xa1)*94+(gb_low-0xa1);
  
  uart_init();

  error("uart online");
  
  mmc_init();

  error("MMC online");

  mmc_read_block(offset, buffer, 32);
  
  error("read hanzi font...");
  
  for(i=0;i<32;i++)
    {
      //put high four bits in hex format.
      if((buffer[i]>>4) < 10)
	uart_putc((buffer[i]>>4)+'0');
      else
	uart_putc((buffer[i]>>4)-10+'a');
      
      //put low four bits in hex format.
      if((buffer[i]&0x0f) < 10)
	uart_putc((buffer[i]&0x0f)+'0');
      else
	uart_putc((buffer[i]&0x0f)-10+'a');

      if(i==15||i==31)
	uart_put_CR_LF();
      else
	uart_putc(' ');
    }
  
  error("The end");

  while(1)
    ;
  
  return 0;
}

