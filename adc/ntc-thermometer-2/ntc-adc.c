/*
ntc-adc.c:
(version 2)

display the temperature on lcd.
choose R1=5K6 other than 10K. Vref=1.1V
it seems suitable for -3 to 100 degree.

see hardware.txt for connection details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 external crystal 8MHz/1=8MHz lfuse=0xe7*/
#define F_CPU 8000000UL
#include <util/delay.h>

#include <math.h>

#include "lcd.h"

#define AVCC_mv 5000.0
#define Vref_mv 1100.0

//#define R1 5600.0
//Infact, This 5K6 resistor is 5520 Ohm.(in approx. 25 degree)
#define R1 5520.0

volatile uint16_t analog_result;
volatile uint8_t analog_busy;

void init_adc_continuous(void)
{
     
  analog_busy = 1;
  /*choose the ADC chanel and standard voltage source*/
  /*use adc4 and internal 1.1v voltage.*/
  ADMUX = ((1<<REFS1)|(1<<REFS0)|0x04);
  
  /*adc control: prescaler, clear ADIF, interrupt enable, etc*/
  /* ADC prescaler: 
     f_adc=f_osc/128=8e6/128 ~= 60KHz
  */
  ADCSRA = ((1<<ADEN)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

  /*auto trigger enable*/
  ADCSRA |= (1<<ADATE);
  /*
    choose trigger source:
    continuous mode.
    see datasheet p231(Chinese version) for details.
  */
  ADCSRB &= ~((1<<ADTS2)|(1<<ADTS1)|(1<<ADTS0));
    
  /*enable interrupt*/
  sei();

  /*then, ADCSRA |= (1<<ADSC) can start convention.*/
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
  double Vin_mv; // save value of V_in
  double r; //save resist of NTC
  double t; //temperature.
  int t_temp; //save temporary t_temp=t*10 for display.
  
  lcd_init(LCD_DISP_ON);
  lcd_clrscr();
  
  init_adc_continuous();
  
  /* start first convertion*/
  ADCSRA |= (1<<ADSC);
  
  lcd_puts("Temperature:\n");
  lcd_gotoxy(5,1);
  lcd_putc(0xDF);
  lcd_gotoxy(6,1);
  lcd_putc('C');
  
  for (;;) {

    /*
      resistance value of NTC:
      r(25)=500 Ohm
      r(t)=r(25)*e^(3300*(1/(t+273)-1/(25+273)))
      etc,
      r(-20)=3.58 KOhm
      r(-3)=1576 KOhm
      r(60)=156 Ohm
      r(100)=54 Ohm
      ...
      if R1=5.6KOhm, then:
      V_in(t)=AVCC*(r(t)/(r(t)+R1))
      etc,
      V_in(-3)=1098 mv
      V_in(100)=47.7 mv
      ...
      
      when V_in is known, 
      r=R1/((AVCC/V_in)-1)
      because of B=3300K, r(25)=500 Ohm:
      t=(1/((ln(r/500)/3300)+(1/(25+273))))-273
    */
    
    Vin_mv = analog_result*(Vref_mv/1024.0);
    
    r=R1/((AVCC_mv/Vin_mv)-1.0);
    
    t=1.0/(log(r/500.0)/3300.0+(1.0/(25.0+273.0))) - 273;
    
    /*
      display xxx.x degree in lcd.
     */
    t_temp=t*10.0;
    
    lcd_gotoxy(0,1);
    lcd_putc(t_temp/1000+'0');
    lcd_gotoxy(1,1);
    lcd_putc((t_temp%1000)/100+'0');
    lcd_gotoxy(2,1);
    lcd_putc((t_temp%100)/10+'0');
    lcd_gotoxy(3,1);
    lcd_putc('.');
    lcd_gotoxy(4,1);
    lcd_putc(t_temp%10+'0');

    for(i=0;i<20;i++)
      _delay_ms(10);
  }
	
}
