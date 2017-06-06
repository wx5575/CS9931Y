#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f4xx_rtc.h"

void rt_hw_rtc_init(void);


uint16_t get_rtc_second(void);
uint16_t get_rtc_minute(void);
uint16_t get_rtc_hour(void);
uint16_t get_rtc_day(void);
uint16_t get_rtc_month(void);
uint16_t get_rtc_year(void);
	
#endif
