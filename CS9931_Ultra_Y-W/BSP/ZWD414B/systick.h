#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "ModBus.h"


typedef enum {
    TIMER_EMPTY,
    TIMER_CONTINUE,
    TIMER_OVER,    
} TIMER_STATUS;



extern void SysTick_Init(void);
extern TIMER_STATUS Delay_ms_LED(int32_t nTime);
extern TIMER_STATUS get_ModBus_timer_status(void);
extern void SysTick_Dispose(void);
extern void ModBusServerRefreshOverTime(void);



#endif                 
