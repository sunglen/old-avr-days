/*
hzk-rotate.c:
This program read a HZK file, rotate +90 degree(clockwise),
store rotated font to a new HZK file.

It support hzk16 from ucdos, and hzk16.bpsf from zhcon.

usage: 
eg: hzk-rotate in_hzk_file rotated_hzk_file
*/

#include <stdio.h>
#include <inttypes.h>

//hanziku file path
//#define HZK_PATH "hzk16"
//#define HZK_PATH "gb-24-kai.bpsf"
#define HZK_PATH "hzk16.bpsf"

/*initial offset of hanziku,
eg:
for /usr/lib/zhcon/font/hzk16.bpsf , is 9.
for /usr/lib/zhcon/font/gb-16.bpsf , is 9.
for /usr/lib/zhcon/font/gb-24-kai.bpsf , is still 9.
It seems the first 9 byte tell the file is a Linux/i386 PC Screen Font data.
*/
//#define HZK_INIT_OFFSET 0
#define HZK_INIT_OFFSET 9

//16x16, or 24x24, or others.
#define FONT_WIDTH 16
#define FONT_HEIGHT 16
//#define FONT_WIDTH 24
//#define FONT_HEIGHT 24

#define FONT_NBYTES (((FONT_WIDTH)*(FONT_HEIGHT))/8)

//store the row hanzi font (source format from any HZK)
unsigned char hzk_font[FONT_NBYTES];

//rotate hzk_font[] 90 degree clockwise.
void hzk_rotate_16x16(void)
{
  unsigned char temp[32];
  uint8_t new_byte, offset_old, offset_new;
  int i,j;
  
  //1st step: construct new byte: [0:2:14]
  //i: new byte no.
  for(i=0;i<=14;i=i+2)
    {
      offset_old=((14-i)/2);
      
      new_byte=0;
      
      //j:old byte no.
      for(j=16;j<=30;j=j+2)
	{
	  offset_new=(j-16)/2;
	  new_byte|=((hzk_font[j]>>offset_old)&0x01)<<offset_new;
	}
      
      temp[i]=new_byte;
    }

  //2nd step: construct new byte: [1:2:15]
  //i: new byte no.
  for(i=1;i<=15;i=i+2)
    {
      offset_old=((15-i)/2);
      
      new_byte=0;
      
      //j:old byte no.
      for(j=0;j<=14;j=j+2)
	{
	  offset_new=j/2;
	  new_byte|=((hzk_font[j]>>offset_old)&0x01)<<offset_new;
	}
      
      temp[i]=new_byte;
    }

  //3rd step: construct new byte: [16:2:30]
  //i: new byte no.
  for(i=16;i<=30;i=i+2)
    {
      offset_old=((30-i)/2);
      
      new_byte=0;
      
      //j:old byte no.
      for(j=17;j<=31;j=j+2)
	{
	  offset_new=(j-17)/2;
	  new_byte|=((hzk_font[j]>>offset_old)&0x01)<<offset_new;
	}
      
      temp[i]=new_byte;
    }

  //4th step: construct new byte: [17:2:31]
  //i: new byte no.
  for(i=17;i<=31;i=i+2)
    {
      offset_old=((31-i)/2);
      
      new_byte=0;
      
      //j:old byte no.
      for(j=1;j<=15;j=j+2)
	{
	  offset_new=(j-1)/2;
	  new_byte|=((hzk_font[j]>>offset_old)&0x01)<<offset_new;
	}
      
      temp[i]=new_byte;
    }

  for(i=0;i<32;i++)
    hzk_font[i]=temp[i];
  
}

//argv[1] is in_hzk_file, argv[2] is rotated_hzk_file
int main(int argc, char **argv)
{
  int i;
  FILE *in_hzk_fp, *rotated_hzk_fp;

  if((in_hzk_fp=fopen(argv[1], "r")) == NULL)
    {
      fprintf(stderr, "in_hzk open error.");
      return -1;
    }

  if((rotated_hzk_fp=fopen(argv[2], "w")) == NULL)
    {
      fprintf(stderr, "rotated_hzk open error.");
      return -1;
    }
  
  fseek(in_hzk_fp, HZK_INIT_OFFSET, SEEK_SET);
  
  while(fread(hzk_font, 1, FONT_NBYTES, in_hzk_fp) != 0)
    {
      hzk_rotate_16x16();
      fwrite(hzk_font, 1, FONT_NBYTES, rotated_hzk_fp);
    }
    
  fclose(in_hzk_fp);
  
  fclose(rotated_hzk_fp);

  return 0;
}
