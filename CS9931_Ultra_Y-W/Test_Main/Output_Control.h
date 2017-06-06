#ifndef __OUTPUT_CONTROL__H__
#define __OUTPUT_CONTROL__H__


/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"


typedef  struct{
	uint16_t  AD_value;
	uint16_t  DA_value;
	uint16_t  Voltage;
}AC_DC_FACTER;


typedef  struct{
	uint16_t  AD_value;
	uint32_t  Current;
}CURRENT_FACTER;

extern CURRENT_FACTER  AC_Current_facter[6];
extern CURRENT_FACTER  DC_Current_facter[6]; 
extern AC_DC_FACTER AC_facter[40];
extern uint16_t const AC_CALIBRATE_POINT[40];
extern AC_DC_FACTER DC_facter[40];
extern uint16_t const DC_CALIBRATE_POINT[40];

extern void AC_SetVoltage(uint16_t dst_voltage,uint16_t out_rate);
extern void AC_Set_Output_DA(uint16_t da_value, uint16_t out_rate);
extern uint16_t AC_GetVoltage(void);
extern void DC_SetVoltage(uint16_t dst_voltage);
extern void DC_Set_Output_DA(uint16_t da_value);
extern uint16_t DC_GetVoltage(void);
extern uint32_t AC_GetCurrent(void);
extern uint32_t DC_GetCurrent(void);
extern void AC_Output_Enable(void);
extern void AC_Output_Disable(void);
extern void DC_Output_Enable(void);
extern void DC_Output_Disable(void);
extern void DC_Set_DAValue(uint16_t ADValue);
extern void AC_Set_DAValue(uint16_t ADValue,uint16_t out_rate);


extern void GR_Set_ouput_da(uint16_t m_da,uint16_t out_rate);

#endif



