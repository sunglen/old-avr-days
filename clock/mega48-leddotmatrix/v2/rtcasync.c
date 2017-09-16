/*
rtcasync.c:
library to use mega48 async RTC of TC2.
including ISR.
*/

#include "rtcasync.h"

/* test for leap year */
uint8_t not_leap(void)
{
  if (!(year%100))
    return (uint8_t)(year%400);
  else
    return (uint8_t)(year%4);
}

/* 
Start Timer/Counter2 in asynchronous operation using a 32.768kHz crystal.
*/
void RTC_init(void)
{
  uint8_t sreg;
	
  /* Save status register */
  sreg=SREG;
  /* disable global interrupt */
  cli();
	
  /* 0. Oscillator might take as long as one second to stabilize. */
  _delay_ms(1000);
    
  /* 1. Disable the Timer/Counter2 interrupts by clearing OCIE2 and TOIE2. */
  TIMSK2 &= ~((1<<OCIE2A)|(1<<OCIE2B)|(1<<TOIE2));
  
  /* 2. Select clock source by setting AS2 as appropriate. */
  ASSR = (1<<AS2);
  
  /* 3. Write new values to TCNT2, OCR2, and TCCR2B. */
  TCNT2 = 0;
  // select precaler: 32.768 kHz / 128 = 1 sec between each overflow:
  TCCR2B |= (1<<CS22) | (1<<CS20);            
	
  /* 4. To switch to asynchronous operation: Wait for clear: TCN2UB, OCR2AUB, OCR2BUB, TCR2AUB and
     TCR2BUB. (p138) */
  while(ASSR & 0x1F);
	
  /* 5. Clear the Timer/Counter2 Interrupt Flags. */
  TIFR2 |= ((1<<OCF2A)|(1<<OCF2B)|(1<<TOV2));
	
  /* 6. enable Timer2 Overflow interrupt */
  TIMSK2 |= (1<<TOIE2);
    
  /* restore status-register */
  SREG=sreg;
  
  sei(); /* ensure enable interrupts */
}


void RTC_set(uint8_t th, uint8_t tm, uint8_t ts, uint16_t yyyy, uint8_t mm, uint8_t dd, uint8_t w)
{
  uint8_t sreg;
	
  sreg=SREG;
  cli();
	
  RTC_h=th; 
  RTC_m=tm; 
  RTC_s=ts;
  year=yyyy;
  month=mm;
  date=dd;
  week=w;
  
  //restore SREG
  SREG=sreg;
}

/*
 Timer2-Overflow ISR called every second
*/
ISR(TIMER2_OVF_vect)
{
	
  RTC_s++;
	
  if (RTC_s == 60)
    {
      RTC_s = 0;
      RTC_m++;
		
      if (RTC_m > 59)
	{
	  RTC_m = 0;
	  RTC_h++;
			
	  if (RTC_h > 23)
	    {
	      RTC_h = 0;

	      week++;
	      if (week > 7)
		week=1;
	      
	      date++;
	      
	      if (date > 31)
		{
		  date = 1;
		  month++;
		}
	      else if ((date == 31) && (month == 4 || month == 6 || month == 9 || month == 11))
		{
		  date=1;
		  month++;
		}
	      else if (date == 30 && month == 2)
		{
		  date=1;
		  month++;
		}
	      else if (date == 29 && month == 2 && not_leap())
		{
		  date=1;
		  month++;
		}
	      
	      if (month == 13)
		{
		  year++;
		  month=1;
		}
	      
	    }
	}
    }
}

