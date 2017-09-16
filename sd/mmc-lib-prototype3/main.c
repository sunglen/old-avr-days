/*
a simple demo of mmc-lib prototype.
write mysector[] to the frist and second sector of SD card,
read from SD card and put it to uart. (read sector size is set to 8 byte.)

it can also read the sectors from pc:
(insert SD card to Digital Camera)
#dd if=/dev/sda of=sd.rawdata bs=512 count=2
the file sd.rawdata shows data the avr write to it.
*/

#include "mmc.h"

char mysector[512];
/* just for test */
void fillram(void)	 
{ 
  int i,c;
  char mystring[18] = "123456ghijklmnopqr";
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

  error("uart online");
  
  mmc_init();

  error("MMC online");

  fillram();
	
  mmc_write_sector(0, mysector);
  
  for(i=0;i<64;i++)
    {
      mmc_read_block(i, buffer, 8);
      
      error(buffer);

    }

  error("write sector 1...");
  
  mmc_write_sector(1, mysector);
  
  mmc_read_block(65, buffer, 8);
      
  error(buffer);
  
  error("The end");

  while(1)
    ;
  
  return 0;
}

