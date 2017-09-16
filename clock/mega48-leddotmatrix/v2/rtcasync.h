#include <inttypes.h>
#include <avr/interrupt.h>

#define F_CPU 1000000UL
#include <util/delay.h>

volatile uint8_t RTC_h, RTC_m, RTC_s, month, date, week;
volatile uint16_t year;

/*if year is leap year, then return 0; else(not leap year) return non-zero*/
uint8_t not_leap(void);

/*set initial global value for start runing time.*/
void RTC_set(uint8_t th, uint8_t tm, uint8_t ts, uint16_t yyyy, uint8_t mm, uint8_t dd, uint8_t w);

/*Start Timer/Counter2 in asynchronous operation using a 32.768kHz crystal.*/
void RTC_init(void);

