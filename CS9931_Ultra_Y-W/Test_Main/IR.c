#include "IR.h"
#include "Multiplexer.h"
#include "AD_DA.h"
#include "spi_cpld.h"
#include "Relay.h"
#include "Cal.h"
#include "Output_Control.h"
#include "Relay_Change.h"

//校准点的基值  单位10kΩ
static const uint32_t IR_Res_Base[] = 
{
	0,
	200,
	2000,
	20000,
	200000,
	1000000,
};



//非自动换挡获取电阻值
/*******************************
函数名:  IR_Get_RS
参  数:  档位
返回值:  电阻值(10kΩ)
********************************/
//uint32_t IR_Get_RS(uint8_t gear)    
//{
//	uint8_t i;
//	uint8_t base_index = 0;
//	uint32_t V1,V2,voltage;
//	uint16_t AD1,AD2,AD_value;
//	
//	float ir_res;
//	
//	uint8_t Sampling_Relay_State = Sampling_Relay_State_Get();
//	
//	
//	
//	for(i=0;i<20;i++)
//	{
//		voltage  = DC_GetVoltage();
//		AD_value = Read_AD_Value(W_I_AD_IN);
//	}
//	
//	if(gear){
//		if(AD_value <= 10) return 0xFFFFFFFF;
//	}else{
//		switch(Sampling_Relay_State)
//		{
//			case DC_2mA:

//				gear = 1;
//			break;
//			
//			case DC_200uA:

//				gear = 2;
//			break;
//			
//			case DC_20uA:

//				gear = 3;
//			break;
//			
//			case DC_2uA:
//				gear = 4;
//			break;
//			
//			default:

//			break;
//		}
//		if(AD_value <= 50){
//			switch(Sampling_Relay_State)
//			{
//				case DC_2mA:
//					Sampling_Relay_State_CHange(DC_200uA);
//					gear = 1;
//				break;
//				
//				case DC_200uA:
//					Sampling_Relay_State_CHange(DC_200uA);
//					gear = 2;
//				break;
//				
//				case DC_20uA:
//					Sampling_Relay_State_CHange(DC_200uA);
//					gear = 3;
//				break;
//				
//				case DC_2uA:
//					gear = 4;
//				break;
//				
//				default:
//					Sampling_Relay_State_CHange(DC_200uA);
//				break;
//			}
//		}
//		else if(AD_value >= 3000)
//		{
//			switch(Sampling_Relay_State)
//			{
//				case DC_2mA:
//					gear = 1;
//				break;
//				
//				case DC_200uA:
//					Sampling_Relay_State_CHange(DC_2mA);
//					gear = 2;
//				break;
//				
//				case DC_20uA:
//					Sampling_Relay_State_CHange(DC_200uA);
//					gear = 3;
//				break;
//				
//				case DC_2uA:
//					Sampling_Relay_State_CHange(DC_20uA);
//					gear = 4;
//				break;
//				
//				default:
//					Sampling_Relay_State_CHange(DC_200uA);
//				break;
//			}
//		}
//			
//	}
//	
//	switch(gear)
//	{
//		case 1:
//			base_index = 1;
//		break;
//		case 2:
//			base_index = 2;
//		break;
//		case 3:
//			base_index = 3;
//		break;
//		case 4:
//			base_index = 4;
//		break;
//		case 5:
//			base_index = 4;
//		break;
//		default:
//			base_index = 3;
//		break;
//	}
//	
//	if(AD_value <= Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[0])
//	{
//		V1  = 0;
//		AD1 = 0;
//		V2  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[0];
//		AD2 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[0];
//	}else{
//		for(i=0;i<10;i++){
//			if(AD_value < Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i])break;
//		}
//		if(i == 10){i--;}
//		i--;	
//		V1  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[i];
//		AD1 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i];
//		V2  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[i+1];
//		AD2 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i+1];
//	}
//	ir_res = (float)voltage / ((float)V1 / IR_Res_Base[base_index] + (float)(V2-V1)/IR_Res_Base[base_index]/(AD2-AD1)*(AD_value-AD1));
//	
//	return (uint32_t)ir_res;
//	
//} 


uint32_t IR_Get_RS(uint8_t gear)    
{
	uint8_t i;
	uint8_t base_index = 0;
	uint32_t V1,V2,voltage;
	uint16_t AD1,AD2,AD_value;
	
	float ir_res;
	
	uint8_t Sampling_Relay_State = Sampling_Relay_State_Get();
	
	
	
	for(i=0;i<20;i++)
	{
		voltage  = DC_GetVoltage();
		AD_value = Read_AD_Value(W_I_AD_IN);
	}
	
	if(gear){
		if(AD_value <= 10) return 0xFFFFFFFF;
	}else{
		gear = 1;
	}
	
	switch(gear)
	{
		case 1:
			base_index = 1;
		break;
		case 2:
			base_index = 2;
		break;
		case 3:
			base_index = 3;
		break;
		case 4:
			base_index = 4;
		break;
		case 5:
			base_index = 4;
		break;
		default:
			base_index = 3;
		break;
	}
	
	if(AD_value <= Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[0])
	{
		V1  = 0;
		AD1 = 0;
		V2  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[0];
		AD2 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[0];
	}else{
		for(i=0;i<10;i++){
			if(AD_value < Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i])break;
		}
		if(i == 10){i--;}
		i--;	
		V1  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[i];
		AD1 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i];
		V2  = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].Voltage[i+1];
		AD2 = Global_Cal_Facter.IR_Facter.Cal_Point[base_index].AD_value[i+1];
	}
	ir_res = (float)voltage / ((float)V1 / IR_Res_Base[base_index] + (float)(V2-V1)/IR_Res_Base[base_index]/(AD2-AD1)*(AD_value-AD1));
	
	return (uint32_t)ir_res;
	
} 



