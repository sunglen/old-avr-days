/*
hanzi-test.c:
input a gb2312 character, output it's dot-matrix font.
*/

#include <stdio.h>
#include <inttypes.h>

#define HZK_PATH "hzk16"

#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_NBYTES (((FONT_WIDTH)*(FONT_HEIGHT))/8)

unsigned char hzk_font[FONT_NBYTES];

uint32_t hzk_ex_font(const char *hzk_path, unsigned char hz_high, unsigned char hz_low)
{
  uint32_t hzk_offset;
  FILE *hzk_fp;
  
  /*
    qu_wei_ma = hz(gb)_ma - 0xa0a0
    hzk_offset = (((qu_wei_ma_high_byte - 1)*94 + (qu_wei_ma_low_byte-1))*
                 FONT_NBYTES)
  */    
  hzk_offset=((hz_high-0xa1)*94+(hz_low-0xa1))*FONT_NBYTES;
  
  if((hzk_fp=fopen(hzk_path, "r")) == NULL)
    {
      fprintf(stderr, "hzk open error.");
      return -1;
    }
  
  fseek(hzk_fp, hzk_offset, SEEK_SET);
  
  fread(hzk_font, 1, FONT_NBYTES, hzk_fp);
  
  fclose(hzk_fp);

  return hzk_offset;
}

int hzk2xbm_printf(const char *xbm_path, const char *xbm_name)
{
  FILE *xbm_fp;
  uint8_t old_byte, new_byte;
  
  int j, k;
  
  /*
    the byte structure of xbm file is:
    picture pix: 1st_bit| 2nd_bit| ...| 7th_bit
    xbm byte: bit0(LSB)| bit1 |...|bit7(MSB)
    so, I should convert xbm byte:
    new byte <-- old byte
    bit0 <-- bit7
    bit1 <-- bit6
    .
    .
    .
    bit7 <-- bit0
  */
  if((xbm_fp=fopen(xbm_path, "w")) == NULL)
    {
      fprintf(stderr, "xbm open error.");
      return -1;
    }
  
  fprintf(xbm_fp, "#define %s_width %u\n", xbm_name, FONT_WIDTH);
  fprintf(xbm_fp, "#define %s_height %u\n", xbm_name, FONT_HEIGHT);
  fprintf(xbm_fp, "static unsigned char %s_bits[] PROGMEM = {\n", xbm_name);
  
  for(j=0;j<FONT_NBYTES;j++)
    {    
      old_byte=hzk_font[j];
      new_byte = 0;
      for(k=0;k<=7;k++)
	new_byte |= ((old_byte>>k)&0x01)<<(7-k);
      
      if(j == (FONT_NBYTES-1))
	fprintf(xbm_fp, "0x%.2x", new_byte);
      else
	fprintf(xbm_fp, "0x%.2x, ", new_byte);
    }
  
  fprintf(xbm_fp, "};");
  
  fclose(xbm_fp);
  
  return 0;
}

int main(int argc, char **argv)
{
  int i;
  unsigned char hz_low, hz_high;

  scanf("%c%c", &hz_high, &hz_low);

  hzk_ex_font(HZK_PATH, hz_high, hz_low);
  
  printf("the font is:\n");
  
  for(i=0;i<FONT_NBYTES;i++)
    printf("0x%.2x ", hzk_font[i]);
  
  printf("\n");

  hzk2xbm_printf(argv[1], argv[2]);
  
  return 0;
}
