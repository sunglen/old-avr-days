#include <avr/io.h>
#include <inttypes.h>

uint8_t a=5, b=6, c;

int main(void)
{
  int i;
  DDRC=0xff;
  PORTC=0x00;

  c=a*b;

  if (c >= 30)
    PORTC=0x01;

  for(i=0;i<4000000UL;i++);
  
  PORTC=0x00;

  for(;;);

  return 0;
} 
