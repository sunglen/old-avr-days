#include <avr/io.h>

int main( void )
{
unsigned char i, j, k,led=0;
DDRB=0xff;
PORTB=0xff;

while (1)
{
if(led)
PORTB|=0X01;
else
PORTB&=0XFE;
led=!led;
//延时
for (i=0; i<255; i++)
for(j=0; j<255;j++)
k++;
}
}
