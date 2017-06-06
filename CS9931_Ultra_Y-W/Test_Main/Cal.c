#include   "Cal.h"
#include   "CS99xx_cal.h"
#include   "AD_DA.h"
#include   "Relay_Change.h"
#include   "LC.h"


static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

//总的校准系数
ALL_FACTER  Global_Cal_Facter;  





/*******************************
函数名：  ACW_Cal_Facter_Refresh
参  数：  item:电压还是电流
					cal: 第几个较准点
					val: 真实值
返回值：  无
********************************/
uint8_t  ACW_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val)
{
	uint16_t i = 0;
	Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num  = 3;
	if(cal > Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num)
		Global_Cal_Facter.ACW_Facter.Vol_Cal_Point_Num  = cal;
	
	switch(item)
	{
		case CAL_VOL:// 电压
			Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[cal-1].DA_value = DAC_GetValue(W_VREF);
			
			Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[cal-1].Voltage  = val;
			for(i=0;i<500;i++){Delay_ms(1);Read_AD_Value_Cal(W_V_AD_IN);}
			Global_Cal_Facter.ACW_Facter.Vol_Cal_Point[cal-1].AD_value = Read_AD_Value_Cal(W_V_AD_IN);
		break;
		case CAL_CUR:// 电流
			switch(Sampling_Relay_State_Get())
			{
				case DC_100mA:
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_100mA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_100mA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_20mA:
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_20mA].Current    = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_20mA].AD_value   = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_2mA:
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_2mA].Current     = val * 10;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_2mA].AD_value    = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_200uA:
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_200uA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.ACW_Facter.Cur_Cal_Point[DC_200uA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
			}
		break;
		case CAL_ARC:
				Global_Cal_Facter.ARC_Facter.ACW_ARC_Base = val;
		break;
		default:
			
		break;
	}
	return 0;
}



/*******************************
函数名：  DCW_Cal_Facter_Refresh
参  数：  item:电压还是电流
					cal: 第几个较准点
					val: 真实值
返回值：  无
********************************/
uint8_t  DCW_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val)
{
	uint8_t i = 0;
	Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num  = 3;
	if(cal > Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num)Global_Cal_Facter.DCW_Facter.Vol_Cal_Point_Num  = cal;
	switch(item)
	{
		case CAL_VOL:// 电压
			Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[cal-1].DA_value = DAC_GetValue(W_VREF);
			
			Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[cal-1].Voltage  = val;
			for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_V_AD_IN);}
			Global_Cal_Facter.DCW_Facter.Vol_Cal_Point[cal-1].AD_value = Read_AD_Value(W_V_AD_IN);
		break;
		case CAL_CUR:// 电流
			switch(Sampling_Relay_State_Get())
			{
				case DC_100mA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_100mA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_100mA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_20mA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_20mA].Current    = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_20mA].AD_value   = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_2mA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_2mA].Current     = val * 10;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_2mA].AD_value    = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_200uA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_200uA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_200uA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_20uA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_20uA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_20uA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
				case DC_2uA:
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_2uA].Current   = val;
					for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(W_I_AD_IN);}
					Global_Cal_Facter.DCW_Facter.Cur_Cal_Point[DC_2uA].AD_value  = Read_AD_Value(W_I_AD_IN);
				break;
			}
		break;
		case CAL_ARC:
				Global_Cal_Facter.ARC_Facter.DCW_ARC_Base = val;
		break;
		default:
			
		break;
	}
	return 0;
}


/*******************************
函数名：  GR_Cal_Facter_Refresh
参  数：  item:电压还是电流
					cal: 第几个较准点
					val: 真实值
返回值：  无
********************************/
uint8_t  GR_Cal_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val)
{
	uint8_t i = 0;
	Global_Cal_Facter.GR_Facter.GR_Cal_Point_Num  = 3;
	switch(item)
	{
		case CAL_VOL:// 电压
			Global_Cal_Facter.GR_Facter.GR_Cal_Point[cal-1].I_DA_value = DAC_GetValue(GR_VREF);
			
			Global_Cal_Facter.GR_Facter.GR_Cal_Point[cal-1].I_Value    = val * 10;    //100m欧姆校准电阻
			for(i=0;i<100;i++){Delay_ms(2);Read_AD_Value(GR_I_AD_IN);Read_AD_Value(GR_V_AD_IN);}
			Global_Cal_Facter.GR_Facter.GR_Cal_Point[cal-1].I_AD_value = Read_AD_Value(GR_I_AD_IN);
			Global_Cal_Facter.GR_Facter.GR_Cal_Point[cal-1].V_AD_value = Read_AD_Value(GR_V_AD_IN);
		break;
		case CAL_CUR:// 电流
			
		break;
		default:
			
		break;
	}
	return 0;
}



extern uint16_t  D16_Mcp3202_Read(uint8_t channel); 

/*******************************
函数名：  LC_Cal_M_V_Facter_Refresh
参  数：  item:电压还是电流
					cal: 第几个较准点
					val: 真实值
返回值：  无
********************************/
uint8_t  LC_Cal_M_V_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val)
{
	uint8_t i = 0;
	Global_Cal_Facter.LC_M_Facter.LC_Cal_Point_Num  = 3;
	switch(item)
	{
		case CAL_VOL:// 电压
			Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[cal-1].DA_value   = DAC_GetValue(W_VREF);
			
			Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[cal-1].Voltage    = val * 100;   
			for(i=0;i<100;i++){Delay_ms(2);D16_Mcp3202_Read(0);}
			Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[cal-1].AD_value = D16_Mcp3202_Read(0);
		break;
		case CAL_CUR:// 电流
			
		break;
		default:
			
		break;
	}
	return 0;
}


/*******************************
函数名：  LC_Cal_A_V_Facter_Refresh
参  数：  item:电压还是电流
					cal: 第几个较准点
					val: 真实值
返回值：  无
********************************/
uint8_t  LC_Cal_A_V_Facter_Refresh(uint8_t item, uint8_t cal, uint32_t val)
{
	uint8_t i = 0;
	Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num  = 3;
	
	switch(item)
	{
		case CAL_VOLS:// 电压
			Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[cal-1].DA_value   = DAC_GetValue(LC_ASSIT_VREF);
			
			Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[cal-1].Voltage    = val * 100;
			for(i=0;i<100;i++){Delay_ms(2);D16_Mcp3202_Read(1);}
			Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[cal-1].AD_value = D16_Mcp3202_Read(1);
		break;
		case CAL_CUR:// 电流
		{
			uint32_t count = 5000;
			uint32_t c = 0;
			uint32_t res = 0;
			
			while(count--)
			{
				Delay_ms(2);
				res = ReadCurrentRmsValue(&Global_Cal_Facter.LC_MC_Facter.AD_Currrent);
				if(res == 0)
				{
					if(c++ > 2)
					{
						break;
					}
				}
			}
			
			Global_Cal_Facter.LC_MC_Facter.Current = val;
			break;
		}
		default:
			
		break;
	}
	return 0;
}




#include "spi_flash.h"
//将校准系数保存到FLASH中
/*******************************
函数名：  Cal_Facter_Reserve
参  数：  无
返回值：  无
********************************/
void  Cal_Facter_Reserve(void)
{
	FLASH_CS_SET(2);
	sf_WriteBuffer((uint8_t *)&Global_Cal_Facter, 4096 * 0, sizeof(ALL_FACTER));
}


//将校准系数从FLASH读取到内存中
/*******************************
函数名：  Cal_Facter_Recover
参  数：  无
返回值：  1:成功
          0:CRC校验错误
********************************/
uint8_t  Cal_Facter_Recover(void)
{
	FLASH_CS_SET(2);
	sf_ReadBuffer((uint8_t *)&Global_Cal_Facter,4096*0,sizeof(ALL_FACTER));
	/*此处添加CRC校验*/
	
	
	return 1;
}





