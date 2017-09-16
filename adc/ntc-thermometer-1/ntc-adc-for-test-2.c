/*
ntc-adc.c:

display the temperature on lcd.

see source code for connection details.
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

/* !! F_CPU must be defined BEFORE include delay.h */
/* use mega48 external crystal 8MHz/1=8MHz lfuse=0xe7*/
#define F_CPU 8000000UL
#include <util/delay.h>

#include "lcd.h"

#define Vref_mv 1100

volatile uint16_t analog_result;
volatile uint8_t analog_busy;

void init_adc_continuous(void)
{
  /* hardware connection: 
     VCC -->0.1uF--> GND
     mega48:
     27(ADC4)<--V_in
     22(GND)-->GND
     21(AREF)-->0.1uF(104)-->GND
     20(AVCC)-->VCC
     
     NTC connection:
     VCC(from stk500, 5.0V) -->/\/\/\/(R=10K)--->NTC-->GND
                                              |
					      |
					      V
				  V_in(to ADC pin of mega48)
				  
    
  */
     
  analog_busy = 1;
  /*改变通道及基准源*/
  /*use adc4 and internal 1.1v voltage.*/
  ADMUX = ((1<<REFS1)|(1<<REFS0)|0x04);
  
  /*adc control: prescaler, clear ADIF, interrupt enable, etc*/
  /* ADC prescaler: f_adc=f_osc/8=8MHz/8=1MHz */
  ADCSRA = ((1<<ADEN)|(1<<ADIF)|(1<<ADIE)|(1<<ADPS1)|(1<<ADPS0));

  /*自动触发使能*/
  ADCSRA |= (1<<ADATE);
  /*
    选择触发源:  
    连续转换模式:
    see datasheet_cn p231 for details
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
  int Vin_mv; // save value of V_in
  int r; //save resist of NTC
  int t; //temperature.
  
  lcd_init(LCD_DISP_ON_CURSOR);
  lcd_clrscr();
  
  init_adc_continuous();
  
  /* start first convertion*/
  ADCSRA |= (1<<ADSC);
  
  lcd_puts("R of NTC(Ohm):\n");
  
  for (;;) {

    /*
      resistance value of NTC:
      r(25)=500 Ohm
      r(t)=r(25)*e^(3300*(1/(t+273)-1/(25+273)))
      etc,
      r(-20)=3.58 KOhm
      r(60)=156 Ohm
      ...
      if R=10KOhm, then:
      V_in(t)=VCC*(r(t)/(r(t)+R))
      
      when V_in is known, 
      r=R/((VCC/V_in)-1)
      t=(1/((ln(r/500)/3300)+(1/(25+273))))-273
    */
    
    Vin_mv = analog_result*(Vref_mv/1024.0);
    
    r=10000.0/((5000.0/Vin_mv)-1.0);
    
    lcd_gotoxy(0,1);
    lcd_putc(r/1000+'0');
    lcd_gotoxy(1,1);
    lcd_putc((r%1000)/100+'0');
    lcd_gotoxy(2,1);
    lcd_putc((r%100)/10+'0');
    lcd_gotoxy(3,1);
    lcd_putc(r%10+'0');

    for(i=0;i<30;i++)
      _delay_ms(10);
  }
	
}
