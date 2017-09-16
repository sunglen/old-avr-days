/*
hz-gb.c:
read a gb2312 character, output it's hex internal code of gb2312.
*/

#include <stdio.h>
#include <inttypes.h>

int main(int argc, char** argv)
{
  unsigned char hz_low, hz_high;
  uint16_t hanzi;
  
  scanf("%c%c", &hz_high, &hz_low);

  hanzi=(hz_high<<8|hz_low);

  printf("0x%x\n",hanzi);

  return 0;
}
