#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/signal.h>


volatile uint8_t RTC_h, RTC_m, RTC_s, RTC_secondchanged,
	RTC_minutechanged;


/* This delay is very rough - don't use it for precise delays */
void delay(unsigned int millisec)
{
	uint8_t i;
    
	while (millisec--)
 		for (i=0; i<125; i++) asm volatile ("nop"::);
}


/* 
Start Timer/Counter2 in asynchronous operation using a 32.768kHz crystal.
*/
void RTC_init(void)
{
	uint8_t sreg;
	
	/* Save status register and disable global interrupts */
	sreg=SREG;
	cli();
	
	/* 0. Oscillator might take as long as one second to stabilize. */
	delay(1000);
    
	/* 1. Disable the Timer/Counter2 interrupts by clearing OCIE2 and TOIE2. */
	TIMSK &= ~((1<<OCIE2)|(1<<TOIE2));
	
	/* 2. Select clock source by setting AS2 as appropriate. */
	ASSR = (1<<AS2);
	
	/* 3. Write new values to TCNT2, OCR2, and TCCR2. */
	TCNT2 = 0;	// clear TCNT2A
	// select precaler: 32.768 kHz / 128 = 1 sec between each overflow:
	TCCR2 |= (1<<CS22) | (1<<CS20);            
	
	/* 4. To switch to asynchronous operation: Wait for TCN2UB, OCR2UB, and
	TCR2UB. */
	while( (ASSR & (1<<TCN2UB)) | (ASSR & (1<<OCR2UB)) |
		(ASSR & (1<<TCR2UB)) ); 
	
	/* 5. Clear the Timer/Counter2 Interrupt Flags. */
	TIFR |= ((1<<OCF2)|(1<<TOV2));
	
	/* 6. enable Timer2 Overflow interrupt */
	TIMSK |= (1<<TOIE2);
    
	/* initial values */
	RTC_h=0;
	RTC_m=0;
	RTC_s=0;
	RTC_secondchanged = 1;
	RTC_minutechanged = 1;
	
	/* restore status-register */
	SREG=sreg;
}


void RTC_set(uint8_t th, uint8_t tm, uint8_t ts)
{
	uint8_t sreg;
	
	sreg=SREG;
	cli();
	
	RTC_h=th; 
	RTC_m=tm; 
	RTC_s=ts;
	RTC_secondchanged = 1;
	RTC_minutechanged = 1;
	
	SREG=sreg;
}


/*
 Timer2-Overflow ISR called every second
*/
SIGNAL(SIG_OVERFLOW2)
{
	/* set global second-changed flag */
	RTC_secondchanged = 1;
	
	RTC_s++;
	
	if (RTC_s == 60)
	{
		RTC_s = 0;
		RTC_m++;
		RTC_minutechanged = 1;
		
		if (RTC_m > 59)
		{
			RTC_m = 0;
			RTC_h++;
			
			if (RTC_h > 23) RTC_h = 0;
		}
	}
}

