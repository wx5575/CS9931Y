#ifndef __RELAY__H__
#define __RELAY__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"

/*继电器位置宏定义*/

#define    AMP_RELAY_4     		 (1<<0)
#define    AMP_RELAY_3     		 (1<<1)
#define    AMP_RELAY_2      	 (1<<2)
#define    AMP_RELAY_1     		 (1<<3)
#define    AMP_RELAY_5      	 (1<<4)
#define    AMP_RELAY_6			 (1<<5)
#define    AMP_RELAY_7			 (1<<6)
#define    GFI_GND_SELECT        (1<<7)

#define    EXT_DRIVER_O4       (1<<8)
#define    EXT_DRIVER_O3       (1<<9)
#define    EXT_DRIVER_O2       (1<<10)
#define    EXT_DRIVER_O1       (1<<11)
#define    EXT_DRIVER_O5       (1<<12)
#define    EXT_DRIVER_O6       (1<<13)
#define    EXT_DRIVER_O7       (1<<14)
#define    EXT_DRIVER_O8       (1<<15)

#define    W_IR_FILE6          (1<<16)
#define    W_IR_FILE1		   (1<<17)
#define    RET_GND_SELECT      (1<<18)
#define    AC_DC               (1<<19)
#define    W_IR_FILE2          (1<<20)
#define    W_IR_FILE3          (1<<21)
#define    W_IR_FILE4          (1<<22)
#define    W_IR_FILE5          (1<<23)

#define    ACW_DCW_IR          (1<<24)

#define RELAY_ON    1
#define RELAY_OFF   0

void ctrl_signal_dault_relay(uint8_t st);

extern void Relay_Control_Init(void);
extern void Relay_ON(uint32_t relay);
extern void Relay_OFF(uint32_t relay);
extern void ctrl_relay_EXT_DRIVE_O4_O5(uint8_t st);
extern void Relay_State_Set(uint32_t state);
extern uint16_t D23_Relay_State_Get(void);
extern uint8_t  D25_Relay_State_Get(void);
extern uint8_t Relay_State_Single_Get(uint32_t relay_index);
extern void PLC_OUT_C(uint8_t state);
extern void PLC_W_OUT_C(uint8_t state);
extern void PLC_METER_SOURCE_C(uint8_t state);















#endif



