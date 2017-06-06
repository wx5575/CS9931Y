#include "GR.h"
#include "Multiplexer.h"
#include "AD_DA.h"
#include "spi_cpld.h"
#include "Relay.h"
#include "Cal.h"


static uint16_t GR_Get_DA_Value(uint16_t dst_current)
{
 	uint8_t i = 0;
 	float k;
	if(dst_current < Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_Value)
		return ((float)dst_current / (float)Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_Value) * Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_DA_value;
	for(i=1;i<Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num;i++){
		if(dst_current < Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value)break;
	}
	if(i == Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num){i--;}
	i--;
	k = (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i+1].I_DA_value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_DA_value) / \
	    (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i+1].I_Value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value);
	return Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_DA_value + k * (dst_current - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value);	
}

void GR_Output_Enable(void)
{
	Relay_ON(AMP_RELAY_3);
	
	Relay_ON(AMP_RELAY_1);
	Relay_ON(AMP_RELAY_5);
	Relay_OFF(AMP_RELAY_7);
// 	CD4053_D17_State_Set(CD4053_D17_X1 | CD4053_D17_Y0 | CD4053_D17_Z0);

}
void GR_Output_Disable(void)
{
	Relay_OFF(AMP_RELAY_3);/* 2016.8.3 wangxin */
	
	Relay_OFF(AMP_RELAY_1);
	Relay_OFF(AMP_RELAY_5);
	Relay_OFF(AMP_RELAY_7);
	CD4053_D17_State_Set(CD4053_D17_X0 | CD4053_D17_Y1 | CD4053_D17_Z0);
	GR_Set_Current(0,0);
}

//接地测试电流输出函数
/*******************************
函数名：  GR_Set_Current
参  数：  m_ampere 电流(mA)
返回值：  无
********************************/

#include "Output_Control.h"

void GR_Set_Current(uint16_t m_ampere,uint16_t out_rate)
{
// 	static uint8_t IsRelayDone = 0;
	
	GR_Set_ouput_da(GR_Get_DA_Value(m_ampere), out_rate);
	
// 	if(IsRelayDone == 0)
// 	{
// 		if(m_ampere != 0)
// 		{
// 			IsRelayDone = 2;
// 			GR_Output_Enable();
// 		}
// 		else
// 		{
// 			IsRelayDone = 1;
// 			GR_Output_Disable();
// 		}
// 	}
// 	else if(IsRelayDone == 1)
// 	{
// 		
// 	}
	
// 	static uint8_t IsRelayDone = 0;
	
// 	if(m_ampere)
// 	{
// 		if(IsRelayDone == 0)
// 		{
// 			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
// 			CPLD_Sine_SetRate(GR_SINE,out_rate);
// 			CPLD_Sine_Control(GR_SINE,ON);
// //			CD4051_D15_State_Set(GR_FB);
// 			IsRelayDone = 1;
// 		}
// 		
// 		DAC_SetValue(GR_VREF,GR_Get_DA_Value(m_ampere));
// 	}else{
// 		IsRelayDone = 0;
// 		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
// 		CPLD_Sine_Control(GR_SINE,OFF);
// 		DAC_SetValue(GR_VREF,0);
// 	}
}
void GR_Set_ouput_da(uint16_t m_da,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	
	if(m_da)
	{
		if(IsRelayDone == 0)
		{
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CPLD_Sine_SetRate(GR_SINE,out_rate);
			CPLD_Sine_Control(GR_SINE,ON);
			CD4051_D15_State_Set(GR_FB);
			IsRelayDone = 1;
		}
		
		DAC_SetValue(GR_VREF, m_da);
	}else{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(GR_SINE,OFF);
		DAC_SetValue(GR_VREF,0);
	}
}


void GR_Set_DA_Value(uint16_t DA_Value,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	if(DA_Value)
	{
		GR_Output_Enable();
		
		if(IsRelayDone == 0)
		{
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(GR_FB);
			CPLD_Sine_SetRate(GR_SINE,out_rate);
			CPLD_Sine_Control(GR_SINE,ON);
			IsRelayDone = 1;
		}
		
		DAC_SetValue(GR_VREF,DA_Value);
	}
	else
	{
		IsRelayDone = 0;
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		GR_Output_Disable();
		CPLD_Sine_Control(GR_SINE,OFF);
		DAC_SetValue(GR_VREF,0);
	}
}

//接地测试电流输出函数
/*******************************
函数名：  GR_Get_Current
参  数：  
返回值：  m_ampere 电流(mA)
********************************/

uint32_t GR_Get_Current(void)
{
	uint8_t i = 0;
	float k;
	uint16_t AD_Value;
	
	for(;i<10;i++)
	{
		Read_AD_Value(GR_I_AD_IN);
	}
	
	AD_Value = Read_AD_Value(GR_I_AD_IN);
	if(AD_Value<Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_AD_value)
		return ((float)AD_Value / (float)Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_AD_value) * Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_Value;
	for(i=1;i<Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num;i++){
		if(AD_Value<Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_AD_value)break;
	}
	if(i == Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num){i--;}
	i--;
	k = (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i+1].I_Value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value) / \
	    (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i+1].I_AD_value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_AD_value);
	return Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value + k * (AD_Value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_AD_value) ;
}


//接地测试电流输出函数
/*******************************
函数名：  GR_Get_Voltage
参  数：  
返回值：  m_voltage 电流(mV)
********************************/

uint32_t GR_Get_Voltage(void)
{
	
	uint8_t i = 0;
	float k;
	uint16_t AD_Value;
	
	for(;i<10;i++)
	{
		Read_AD_Value(GR_V_AD_IN);
	}
	
	AD_Value = Read_AD_Value(GR_V_AD_IN);
	
	if(AD_Value<Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].V_AD_value)
	{
		return ((float)AD_Value / (float)Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].V_AD_value)
						* Global_Cal_Facter.GR_Facter.GR_Cal_Point[0].I_Value / 10;
	}
	
	for(i=1;i<Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num;i++)
	{
		if(AD_Value<Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].V_AD_value)break;
	}
	
	if(i == Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	
	k = (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i + 1].I_Value
				- Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value) / 
	    (float)(Global_Cal_Facter.GR_Facter.GR_Cal_Point[i + 1].V_AD_value 
		- Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].V_AD_value);
	
	return (Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].I_Value
				+ k * (AD_Value - Global_Cal_Facter.GR_Facter.GR_Cal_Point[i].V_AD_value)) / 10;
}



uint32_t GR_Get_RS(void)
{
	float voltage,ampere;
	voltage = (float)GR_Get_Voltage() / 10;
	ampere  = (float)GR_Get_Current() / 10;
	if(ampere == 0) return 0xFFFF;
	return (uint32_t)(voltage / ampere * 10000);
}











