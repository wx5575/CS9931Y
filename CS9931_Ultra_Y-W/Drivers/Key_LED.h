#ifndef __KEY_LED__H__
#define __KEY_LED__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"


#define   ON     (1)
#define   OFF    (0)



extern void Key_LED_Control_Init(void);
extern void Set_LED_PASSLED(uint8_t state);
extern void Set_LED_FAILLED(uint8_t state);
extern void Set_LED_TESTLED(uint8_t state);
extern void Set_BUFFER(uint8_t state);
extern uint32_t KeyValue_Read(void);



#endif



