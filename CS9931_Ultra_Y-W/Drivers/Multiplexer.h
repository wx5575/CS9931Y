#ifndef __MULTIPLEXER__H__
#define __MULTIPLEXER__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"


#define   ADG509A_S1        (0)
#define   ADG509A_S2        (1)
#define   ADG509A_S3        (2)
#define   ADG509A_S4        (3)

#define   CD4053_D18_X0     (0)     
#define   CD4053_D18_X1     (1) 
#define   CD4053_D18_Y0     (0) 
#define   CD4053_D18_Y1     (2) 

#define   CD4053_D14_X0     (0)     
#define   CD4053_D14_X1     (1) 
#define   CD4053_D14_Y0     (0) 
#define   CD4053_D14_Y1     (2) 
#define   CD4053_D14_Z0     (0) 
#define   CD4053_D14_Z1     (4) 

#define   CD4053_D17_X0     (0)     
#define   CD4053_D17_X1     (1) 
#define   CD4053_D17_Y0     (0) 
#define   CD4053_D17_Y1     (2) 
#define   CD4053_D17_Z0     (0) 
#define   CD4053_D17_Z1     (4) 

//CD4051 D15状态字
#define   AC_VOL_FB         (0)
#define   DC_VOL_FB         (1)
#define   LC_VOL_FB         (2)
#define   GR_FB             (3)

extern void Multiplexer_Control_Init(void);
extern void ADG509A_State_Set(uint8_t state);
extern uint8_t ADG509A_State_Get(void);
extern void CD4053_D18_State_Set(uint8_t state);
extern uint8_t CD4053_D18_State_Get(void);
extern void CD4053_D17_State_Set(uint8_t state);
extern uint8_t CD4053_D17_State_Get(void);
extern void CD4053_D14_State_Set(uint8_t state);
extern uint8_t CD4053_D14_State_Get(void);
extern void CD4051_D15_State_Set(uint8_t state);
extern uint8_t CD4051_D15_State_Get(void);

#endif



