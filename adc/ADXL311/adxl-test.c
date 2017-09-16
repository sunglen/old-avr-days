/*
adxl-test.c:

see source code for connection details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* use mega48 external crystal 8MHz/1=8MHz lfuse=0xe7*/
#define F_CPU 8000000UL
#include <util/delay.h>

#include <math.h>

#include "lcd.h"

#define AVCC_mv 5000
#define Vref_mv AVCC_mv

volatile uint16_t analog_result;
volatile uint8_t analog_busy;

  /* hardware connection: 
     VCC -->0.1uF--> GND
     mega48:
     28(ADC5)<--Yout
     27(ADC4)<--Xout
     22(GND)-->GND
     21(AREF)-->0.1uF(104)-->GND
     20(AVCC)-->VCC
     
     ADXL311 connection:
     VCC(from stk500, 5.0V) --> Vdd
     GND <-- COM
     NC <--> ST
     ADC4<--Xout
     ADC5<--Yout
  */
void adc_init_single(void)
{
  analog_busy = 1;
  
  /*adc control: prescaler, clear ADIF, interrupt enable, etc*/
  /* ADC prescaler: 
     f_adc=f_osc/128=8e6/128 ~= 60KHz
  */
  ADCSRA = ((1<<ADEN)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

  /*enable interrupt*/
  sei();
}

ISR (ADC_vect)
{
  uint8_t adlow, adhigh;

  adlow = ADCL;
  adhigh = ADCH;
  analog_result = ((adhigh<<8)|adlow);
  analog_busy = 0;
}

int main (void)
{
  int i;
  uint16_t Xout_mv, Yout_mv;
  
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  
  adc_init_single();
  
  lcd_puts("Xout_mv:");
  lcd_gotoxy(0,1);
  lcd_puts("Yout_mv:");
  
  for (;;) {

    //get Xout
    analog_busy=1;    
    /*select channel 4 (Xout) and use Aref=Avcc*/
    ADMUX = ((1<<REFS0)|0x04);
    
    /* start convertion*/
    ADCSRA |= (1<<ADSC);
    
    while(analog_busy)
      ;
    Xout_mv=analog_result/1024.0*Vref_mv;
    
    //get Yout
    analog_busy=1;
    /*select channel 5 (Yout) and use Aref=Avcc*/
    ADMUX = ((1<<REFS0)|0x05);
        
    /* start convertion*/
    ADCSRA |= (1<<ADSC);
    
    while(analog_busy)
      ;
    Yout_mv=analog_result/1024.0*Vref_mv;
    
    // display Xout and Yout in mv.
    lcd_gotoxy(10,0);
    lcd_putc(Xout_mv/1000+'0');
    lcd_gotoxy(11,0);
    lcd_putc((Xout_mv%1000)/100+'0');
    lcd_gotoxy(12,0);
    lcd_putc((Xout_mv%100)/10+'0');
    lcd_gotoxy(13,0);
    lcd_putc((Xout_mv%10)+'0');


    lcd_gotoxy(10,1);
    lcd_putc(Yout_mv/1000+'0');
    lcd_gotoxy(11,1);
    lcd_putc((Yout_mv%1000)/100+'0');
    lcd_gotoxy(12,1);
    lcd_putc((Yout_mv%100)/10+'0');
    lcd_gotoxy(13,1);
    lcd_putc((Yout_mv%10)+'0');
    
    for(i=0;i<10;i++)
      _delay_ms(50);
  }
	
}
