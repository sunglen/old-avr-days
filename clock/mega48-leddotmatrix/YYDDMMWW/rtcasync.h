#ifndef rtcasync_h_
#define rtcasync_h_

#include <inttypes.h>

extern volatile uint8_t RTC_h, RTC_m, RTC_s, month, date, week;
extern volatile uint16_t year;


void RTC_set(uint8_t th, uint8_t tm, uint8_t ts, uint16_t yyyy, uint8_t mm, uint8_t dd, uint8_t w);
void RTC_init(void);

#endif
