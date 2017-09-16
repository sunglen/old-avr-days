//#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>


uint8_t a=5, b=6, c;

int main(void)
{

 // DDRC=0xff;
  //PORTC=0x00;

  c=a*b;
  if (c >= 30)
    printf("%d*%d>=30", a, b);
    //PORTC=0x01;

  //for(;;);

  return 0;
} 
