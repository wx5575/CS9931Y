#ifndef __GR__H__
#define __GR__H__


/*包含的头文件*/
#include <stdint.h>


/*数据类型定义*/
/*数据类型定义*/
typedef  struct{
	uint16_t  I_DA_value;
	uint16_t  I_AD_value;
	uint16_t  V_AD_value;
	uint16_t  I_Value;
}GR_CURRENT_FACTER;


extern void GR_Output_Enable(void);
extern void GR_Output_Disable(void);
extern uint32_t GR_Get_Voltage(void);
extern void GR_Set_Current(uint16_t m_ampere,uint16_t out_rate);
extern uint32_t GR_Get_Current(void);
extern uint32_t GR_Get_RS(void);
extern void GR_Set_DA_Value(uint16_t DA_Value,uint16_t out_rate);

#endif
