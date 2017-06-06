#include "Relay.h"
#include "Output_Control.h"
#include "Relay.h"
#include "AD_DA.h"
#include "spi_cpld.h"
#include "Multiplexer.h"
#include "Relay_Change.h"
#include "Key_LED.h"
#include "Cal.h"

#if defined(__cplusplus)

extern "C" {     /* Make sure we have C-declarations in C++ programs */

#endif

/**************************************************************************
 *                           函数内联关键字宏定义 -- 编译器相关
***************************************************************************/

#define     INLINE                                   __inline


static void Delay_ms(unsigned int dly_ms)
{
	unsigned int dly_i;
	
	while(dly_ms--)
		for(dly_i = 0; dly_i < 18714; dly_i++);
}


static uint16_t AC_Get_DA_Value(uint16_t dst_voltage)
{
	uint8_t i = 0;
	float k;
	
	if(dst_voltage <= Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].Voltage)
		return ((float)dst_voltage / (float)Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].Voltage) * Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].DA_value;
		
	for(; i < Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num; i++)
	{
		if(dst_voltage < Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].Voltage)break;
	}
	
	if(i == Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	k = (float)(Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i + 1].DA_value - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].DA_value) /     \
	    (float)(Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i + 1].Voltage - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].Voltage);
	return Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].DA_value + k * (dst_voltage - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].Voltage);
	
}


void AC_Output_Enable(void)
{
	//	CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
	Relay_ON(AMP_RELAY_5);
	CPLD_GPIO_Control(W_OUT_C, 1);
}

void AC_Output_Disable(void)
{
	AC_SetVoltage(0, 0);
	Delay_ms(10);
	CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
//	CPLD_GPIO_Control(W_OUT_C, 0);
	Delay_ms(10);
	Relay_OFF(AMP_RELAY_5);
}

void AC_SetVoltage(uint16_t dst_voltage, uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	
	if(dst_voltage)
	{
		if(IsRelayDone == 0)
		{
			// 			CPLD_Sine_Control(ADCW_SINE | GR_SINE,OFF);
			// 			DAC_SetValue(W_VREF,0);
			// 			Delay_ms(15);
			Relay_OFF(AC_DC);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_6);
			Relay_ON(AMP_RELAY_7);
			Delay_ms(15);
			Relay_OFF(AMP_RELAY_3);
			Delay_ms(15);
			IsRelayDone = 1;
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(AC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE, out_rate);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE, ON);
		}
		
		DAC_SetValue(W_VREF, AC_Get_DA_Value(dst_voltage));
	}
	
	else
	{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE, OFF);
		DAC_SetValue(W_VREF, 0);
		Delay_ms(30);
		Relay_OFF(AC_DC);
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_6);
		Relay_OFF(AMP_RELAY_7);
		Relay_OFF(AMP_RELAY_3);
	}
	
}

void AC_Set_Output_DA(uint16_t da_value, uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	
	if(da_value)
	{
		if(IsRelayDone == 0)
		{
			// 			CPLD_Sine_Control(ADCW_SINE | GR_SINE,OFF);
			// 			DAC_SetValue(W_VREF,0);
			// 			Delay_ms(15);
			Relay_OFF(AC_DC);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_6);
			Relay_ON(AMP_RELAY_7);
			Delay_ms(15);
			Relay_OFF(AMP_RELAY_3);
			Delay_ms(15);
			IsRelayDone = 1;
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(AC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE, out_rate);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE, ON);
		}
		
		DAC_SetValue(W_VREF, da_value);
	}
	
	else
	{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE, OFF);
		DAC_SetValue(W_VREF, 0);
		Delay_ms(30);
		Relay_OFF(AC_DC);
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_6);
		Relay_OFF(AMP_RELAY_7);
		Relay_OFF(AMP_RELAY_3);
	}
	
}

void AC_Set_DAValue(uint16_t ADValue, uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	
	if(ADValue)
	{
		AC_Output_Enable();
		
		if(IsRelayDone == 0)
		{
			Relay_ON(AMP_RELAY_6);
			Relay_ON(AMP_RELAY_7);
			//			Delay_ms(10);
			Relay_ON(AMP_RELAY_3);
			IsRelayDone = 1;
			Delay_ms(10);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(AC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE, out_rate);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE, ON);
		}
		
		DAC_SetValue(W_VREF, ADValue);
	}
	
	else
	{
		AC_Output_Disable();
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE, OFF);
		DAC_SetValue(W_VREF, 0);
		Delay_ms(30);
		Relay_OFF(AC_DC);
		Relay_OFF(AMP_RELAY_6);
		Relay_OFF(AMP_RELAY_7);
		Relay_OFF(AMP_RELAY_3);
	}
	
}




uint16_t AC_GetVoltage(void)
{
	uint8_t i = 0;
	uint16_t AD_Value;
	float k;
	
	for(; i < 10; i++)Read_AD_Value(W_V_AD_IN);
	
	AD_Value = Read_AD_Value(W_V_AD_IN);
	
	if(AD_Value <= Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].AD_value)
		return ((float)AD_Value / (float)Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].AD_value) * Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[0].Voltage;
		
	for(i = 0; i < Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num; i++)
	{
		if(AD_Value < Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].AD_value)break;
	}
	
	if(i == Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	
	k = (float)(Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i + 1].Voltage - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].Voltage) / \
	    (float)(Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i + 1].AD_value - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].AD_value);
	return Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].Voltage + k * (AD_Value - Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[i].AD_value);
}


static uint16_t DC_Get_DA_Value(uint16_t dst_voltage)
{

	uint8_t i = 0;
	float k;
	
	if(dst_voltage <= Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].Voltage)
		return ((float)dst_voltage / (float)Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].Voltage) * Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].DA_value;
		
	for(; i < Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num; i++)
	{
		if(dst_voltage < Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].Voltage)break;
	}
	
	if(i == Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	k = (float)(Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i + 1].DA_value - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].DA_value) /     \
	    (float)(Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i + 1].Voltage - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].Voltage);
	return Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].DA_value + k * (dst_voltage - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].Voltage);
	
}

void DC_Output_Enable(void)
{
	//	CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
	Relay_ON(AMP_RELAY_5 | AC_DC);
	CPLD_GPIO_Control(W_OUT_C, 1);
}

void DC_Output_Disable(void)
{
	DC_SetVoltage(0);
	Delay_ms(10);
	CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
//	CPLD_GPIO_Control(W_OUT_C, 0);
	Delay_ms(10);
	Relay_OFF(AMP_RELAY_5 | AC_DC);
}

void DC_SetVoltage(uint16_t dst_voltage)
{
	static uint8_t IsRelayDone = 0;
	
	if(dst_voltage)
	{
		DAC_SetValue(W_VREF, DC_Get_DA_Value(dst_voltage));
		
		if(IsRelayDone == 0)
		{
			//			DAC_SetValue(W_VREF,0);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_6);
			Relay_ON(AMP_RELAY_7);
			Delay_ms(15);
			Relay_OFF(AMP_RELAY_3);
			Delay_ms(15);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(DC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE, 400);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE, ON);
			
			IsRelayDone = 1;
		}
	}
	
	else
	{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE, OFF);
		DAC_SetValue(W_VREF, 0);
		CD4051_D15_State_Set(AC_VOL_FB);
		Delay_ms(100);
		
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_6);
		Relay_OFF(AMP_RELAY_7);
		Relay_OFF(AMP_RELAY_3);
	}
	
}

void DC_Set_Output_DA(uint16_t da_value)
{
	static uint8_t IsRelayDone = 0;
	
	if(da_value)
	{
		DAC_SetValue(W_VREF, da_value);
		
		if(IsRelayDone == 0)
		{
			//			DAC_SetValue(W_VREF,0);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_6);
			Relay_ON(AMP_RELAY_7);
			Delay_ms(15);
			Relay_OFF(AMP_RELAY_3);
			Delay_ms(15);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(DC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE, 400);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE, ON);
			
			IsRelayDone = 1;
		}
	}
	
	else
	{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE, OFF);
		DAC_SetValue(W_VREF, 0);
		CD4051_D15_State_Set(AC_VOL_FB);
		Delay_ms(100);
		
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_6);
		Relay_OFF(AMP_RELAY_7);
		Relay_OFF(AMP_RELAY_3);
	}
}



uint16_t DC_GetVoltage(void)
{
	uint8_t i = 0;
	uint16_t AD_Value;
	float k;
	
	for(; i < 10; i++)Read_AD_Value(W_V_AD_IN);
	
	AD_Value = Read_AD_Value(W_V_AD_IN);
	
	if(AD_Value <= Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].AD_value)
		return ((float)AD_Value / (float)Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].AD_value) * Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[0].Voltage;
		
	for(i = 0; i < Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num; i++)
	{
		if(AD_Value < Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].AD_value)break;
	}
	
	if(i == Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	
	k = (float)(Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i + 1].Voltage - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].Voltage) / \
	    (float)(Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i + 1].AD_value - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].AD_value);
	return Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].Voltage + k * (AD_Value - Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[i].AD_value);
}



uint32_t AC_GetCurrent(void)
{
	uint8_t i = 0;
	uint8_t sample_state = Sampling_Relay_State_Get();
	
	for(; i < 10; i++)Read_AD_Value(W_I_AD_IN);
	
	return (uint32_t)((float)(Read_AD_Value(W_I_AD_IN)) / (float)(Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[sample_state].AD_value) \
	                  * (float)(Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[sample_state].Current));
	// 	switch(Sampling_Relay_State_Get()){
	// 		case DC_2mA:
	//
	// 		default:
	//
	// 		return 0;
	// 	}
	
}

uint32_t DC_GetCurrent(void)
{

	uint8_t i = 0;
	uint8_t sample_state = Sampling_Relay_State_Get();
	
	for(; i < 10; i++)Read_AD_Value(W_I_AD_IN);
	
	return (uint32_t)((float)(Read_AD_Value(W_I_AD_IN)) / (float)(Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[sample_state].AD_value) \
	                  * (float)(Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[sample_state].Current));
	                  
	                  
	// 	uint8_t i = 0;
	// 	switch(Sampling_Relay_State_Get()){
	// 		case DC_2mA:
	// 			for(;i<10;i++)Read_AD_Value(W_I_AD_IN);
	// 			return (uint32_t)((float)(Read_AD_Value(W_I_AD_IN)) / 2116 * 0.905f * 10000);
	// 		default:
	//
	// 		return 0;
	// 	}
}


