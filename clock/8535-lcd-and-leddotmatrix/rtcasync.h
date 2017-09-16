#ifndef rtcasync_h_
#define rtcasync_h_

#include <inttypes.h>

extern volatile uint8_t RTC_h, RTC_m, RTC_s, RTC_secondchanged,
	RTC_minutechanged;

void RTC_set(uint8_t th, uint8_t tm, uint8_t ts);
void RTC_init(void);

#endif