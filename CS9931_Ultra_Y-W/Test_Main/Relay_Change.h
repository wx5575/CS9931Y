#ifndef __RELAY_CHANGE__H__
#define __RELAY_CHANGE__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"

/*继电器位置宏定义*/

#define     DC_100mA         (0)
#define     DC_20mA          (1)
#define     DC_2mA           (2)
#define     DC_200uA         (3)
#define     DC_20uA          (4)
#define     DC_2uA           (5)
			
#define     IR_1M_10M        (6)			
#define     IR_10M_100M      (7)	
#define     IR_100M_1000M    (8)	
#define     IR_1G_10G        (9)	
#define     IR_10G_100G      (10)	







extern void Sampling_Relay_State_CHange(uint8_t state);
extern uint16_t Sampling_Relay_State_Get(void);


#endif



