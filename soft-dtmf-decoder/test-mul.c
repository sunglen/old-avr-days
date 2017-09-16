#include <avr/io.h>
//#include <util/delay.h>

int main(void)
{
float a=1.62;
float b=400.56;
float c;
//int i;


//DDRD=0xff;
c=a*b;
/*
for(i=0;i<3;i++)
{
PORTD=(((uint32_t)c>>(8*i))&0xff);
_delay_ms(10);
}
*/
return 0;
}
