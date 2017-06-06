/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：spi_cpld.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "Self_Test.h"
#include  "CH376_USB_UI.H"
#include  "driver.h"
#include  "rtthread.h"
#include  "CS99xx.h"


static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}



/*
*********************************************************************************************************
*	函 数 名: VREF_SelfTest
*	功能说明: 校验参考电压
*	形    参: ch:0~7 选择的通道
*	返 回 值: 0:错误  1:正确
*********************************************************************************************************
*/
uint8_t VREF_SelfTest(uint8_t ch)
{
	uint16_t offset,ADC_Value,i=0;
	if(ch>7)return 0;

	VREF_SelfTest_CH_Change(ch);
	offset = DAC_GetValue(ch) / 3;
	for(;i<100;i++){Delay_ms(1);Read_AD_Value(VREF_SELF_AD_IN);}
	ADC_Value = Read_AD_Value(VREF_SELF_AD_IN);
	if((ADC_Value >= (DAC_GetValue(ch) - offset)) && (ADC_Value <= (DAC_GetValue(ch) + offset))) return 1;
	return 0;
}


/*
*********************************************************************************************************
*	函 数 名: Test_Loop_Selftest
*	功能说明: 测量回路的诊断
*	形    参: testway:0~2 选择自诊断的方式
*	返 回 值: 0:错误  1:正确
*********************************************************************************************************
*/

#define   V_SELFTEST_REF     (1160)
#define   V_SELFTEST_REF_UL  (V_SELFTEST_REF + (V_SELFTEST_REF / 2))
#define   V_SELFTEST_REF_DL  (V_SELFTEST_REF - (V_SELFTEST_REF / 2))

uint8_t Test_Loop_Selftest(uint8_t testway)
{
	uint16_t i=0;
	uint16_t W_V_AD_Value;
//	uint16_t W_I_AD_Value;
	uint16_t GR_I_AD_Value;
//	uint16_t GR_V_AD_Value;
	switch (testway){
		case LOOP_SELFTEST_VREF_1V:
// 			ADG509A_State_Set(ADG509A_S3);
// 			rt_thread_delay( RT_TICK_PER_SECOND*1);
// 			for(i=0;i<100;i++){
// 				W_V_AD_Value  = Read_AD_Value(W_V_AD_IN);
// 				W_I_AD_Value  = Read_AD_Value(W_I_AD_IN);
// 				GR_I_AD_Value = Read_AD_Value(GR_I_AD_IN);
// 				GR_V_AD_Value = Read_AD_Value(GR_V_AD_IN);
// 			}
// 			if(W_V_AD_Value>V_SELFTEST_REF_UL  || W_V_AD_Value<V_SELFTEST_REF_DL)return 0;
// 			if(W_I_AD_Value>V_SELFTEST_REF_UL  || W_I_AD_Value<V_SELFTEST_REF_DL)return 0;
// 			if(GR_I_AD_Value>V_SELFTEST_REF_UL || GR_I_AD_Value<V_SELFTEST_REF_DL)return 0;
// 			if(GR_V_AD_Value>V_SELFTEST_REF_UL || GR_V_AD_Value<V_SELFTEST_REF_DL)return 0;
// 		break;
			ADG509A_State_Set(ADG509A_S3);
			Delay_ms(500);
			for(i=0;i<100;i++){
				Delay_ms(1);
				Read_AD_Value(W_V_AD_IN);
				Read_AD_Value(W_I_AD_IN);
				Read_AD_Value(GR_I_AD_IN);
				Read_AD_Value(GR_V_AD_IN);
			}
			
			if(Read_AD_Value(W_V_AD_IN)>V_SELFTEST_REF_UL || Read_AD_Value(W_V_AD_IN)<V_SELFTEST_REF_DL)return 0;
			if(Read_AD_Value(W_I_AD_IN)>V_SELFTEST_REF_UL || Read_AD_Value(W_I_AD_IN)<V_SELFTEST_REF_DL)return 0;
			if(Read_AD_Value(GR_I_AD_IN)>V_SELFTEST_REF_UL || Read_AD_Value(GR_I_AD_IN)<V_SELFTEST_REF_DL)return 0;
			if(Read_AD_Value(GR_V_AD_IN)>V_SELFTEST_REF_UL || Read_AD_Value(GR_V_AD_IN)<V_SELFTEST_REF_DL)return 0;
		break;
		case LOOP_SELFTEST_SINE_OPEN:
			DAC_SetValue(W_VREF,1000);
			DAC_SetValue(GR_VREF,1000);
			CPLD_Sine_SetRate(ADCW_SINE,100);
			CPLD_Sine_Control(ADCW_SINE,ON);
			CPLD_Sine_SetRate(GR_SINE,100);
			CPLD_Sine_Control(GR_SINE,ON);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y1 | CD4053_D14_Z0);
			CD4053_D17_State_Set(CD4053_D17_X1 | CD4053_D17_Y1 | CD4053_D17_Z0);
			CD4053_D18_State_Set(CD4053_D18_X1 | CD4053_D18_Y1);
			ADG509A_State_Set(ADG509A_S2);
			Delay_ms(500);
			for(i=0;i<100;i++){
				Delay_ms(1);
				W_V_AD_Value  = Read_AD_Value(W_V_AD_IN);
				GR_I_AD_Value = Read_AD_Value(GR_I_AD_IN);
				
			}
			CPLD_Sine_Control(ADCW_SINE,OFF);
			CPLD_Sine_Control(GR_SINE,OFF);
			if(W_V_AD_Value>1500  || W_V_AD_Value<500)return 0;
			if(GR_I_AD_Value>1500 || GR_I_AD_Value<500)return 0;
		break;
		case LOOP_SELFTEST_SINE_CLOSE:
			DAC_SetValue(W_VREF,1000);
			DAC_SetValue(GR_VREF,1000);
			CPLD_Sine_SetRate(ADCW_SINE,100);
			CPLD_Sine_Control(ADCW_SINE,ON);
			CPLD_Sine_SetRate(GR_SINE,100);
			CPLD_Sine_Control(GR_SINE,ON);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			CD4051_D15_State_Set(AC_VOL_FB);
			CD4053_D17_State_Set(CD4053_D17_X1 | CD4053_D17_Y0 | CD4053_D17_Z0);
			CD4053_D18_State_Set(CD4053_D18_X1 | CD4053_D18_Y1);
			ADG509A_State_Set(ADG509A_S2);
			Delay_ms(500);
			for(i=0;i<100;i++){
				Delay_ms(1);
				W_V_AD_Value  = Read_AD_Value(W_V_AD_IN);
				GR_I_AD_Value = Read_AD_Value(GR_I_AD_IN);
			}
			CPLD_Sine_Control(ADCW_SINE,OFF);
			CPLD_Sine_Control(GR_SINE,OFF);
			if(W_V_AD_Value>1600  || W_V_AD_Value<500)return 0;
			if(GR_I_AD_Value>1600 || GR_I_AD_Value<500)return 0;
		break;
		default:
			
		return 0;
	}
	return 1;
}

static uint8_t self_test_prepared = 0;

/*
*********************************************************************************************************
*	函 数 名: self_test_item_name
*	功能说明: 返回自检测试的名称
*	形    参: index:0~10 选择自诊断的方式
*	返 回 值: 项目名称的指针
*********************************************************************************************************
*/
const char *self_test_item_name(unsigned char index)
{
	uint8_t i = 0;
	if(self_test_prepared == 0){
		self_test_prepared = 1;
		/*自检的准备工作*/
		Key_LED_Control_Init();
		AD_DA_Config();
		Multiplexer_Control_Init();
		Relay_Control_Init();
		spi_cpld_init();
		LC_Init();
		for(i=0;i<8;i++)
			DAC_SetValue(i,2000);
		
	}
	
	switch(index){
		
		case 0:
			return T_STR("1. 电源自检    ","1. Power Self-Checking   ");
		case 1:
				return T_STR("2. 键盘自检    ","2. Keyboard Self-Checking");
		case 2:
				return T_STR("3. 内部存储    ","3. Internal Storage      ");
		case 3:
				return T_STR("4. 外部存储    ","4. External Storage      ");
		case 4:
				return T_STR("5. 参考电压    ","5. Reference Voltage     ");
		case 5:
				return T_STR("6. 测试回路    ","6. Test Loop             ");
		case 6:
				return T_STR("7. U盘硬件电路 ","7. Udisk Hardware Circuit");
		case 7:
				return T_STR("8. 泄漏模块测试","8. Reveal Module Test    ");
		case 8:
				return T_STR("9. 校准开关    ","9. Correct Switch        ");
		case 9:
		case 10:
		default:
		return 0;
	}
}

char Wrong_info[40];

/*
*********************************************************************************************************
*	函 数 名: self_test_item_result
*	功能说明: 返回自检测试的结果
*	形    参: index:0~10 选择自诊断的方式
*	返 回 值: 项目名称的指针
*********************************************************************************************************
*/
const char *self_test_item_result(unsigned char index)
{
	uint8_t i = 0;
	static  uint8_t  calibration_switch = 0;
	if(self_test_prepared == 0){
		self_test_prepared = 1;
		/*自检的准备工作*/
		Key_LED_Control_Init();
		AD_DA_Config();
		Multiplexer_Control_Init();
		Relay_Control_Init();
		spi_cpld_init();
		LC_Init();
		for(i=0;i<8;i++)
			DAC_SetValue(i,2000);
		
	}
	Delay_ms(100);
	
	switch(index){
		
		case 0:
			return (const char *)0;
		
		case 1:
			if((KeyValue_Read() & ~(0x02000000)) == 0)  //除了Enter键
				return (const char *)0;
			else
				return T_STR("按键未释放","Button NO Release");
		case 2:
			return (const char *)0;
		
		case 3:
			return (const char *)0;
		
		case 4:
		{
			for(i=0;i<7;i++){VREF_SelfTest_CH_Change(i);if(VREF_SelfTest(i) == 0)break;}
			if(i==7)return (const char *)0;
			if(language==0)
			{
				rt_sprintf(Wrong_info, "参考电压%d出错 Set:%d Get:%d" ,i+1,DAC_GetValue(i),Read_AD_Value(VREF_SELF_AD_IN));
			}
			else
			{
				rt_sprintf(Wrong_info, "ReferenceVol %d error Set:%d Get:%d",i+1,DAC_GetValue(i),Read_AD_Value(VREF_SELF_AD_IN));
			}
			return Wrong_info;
		}
		case 5:
			{			
				return (const char *)0;
				for(i=0;i<3;i++)if(Test_Loop_Selftest(i) == 0)break;
				if(i==3)return (const char *)0;
				if(language==0)
				{
					rt_sprintf(Wrong_info,"测试回路%d出错 Get:%d %d %d %d",i+1,    
									Read_AD_Value(W_V_AD_IN),                                        
									Read_AD_Value(W_I_AD_IN),                                     
									Read_AD_Value(GR_V_AD_IN),                                   
									Read_AD_Value(GR_I_AD_IN));
				}
				else
				{
					rt_sprintf(Wrong_info, "TestLoop %d error Get:%d %d %d %d", i+1,    
									Read_AD_Value(W_V_AD_IN),                                        
									Read_AD_Value(W_I_AD_IN),                                     
									Read_AD_Value(GR_V_AD_IN),                                   
									Read_AD_Value(GR_I_AD_IN));
				}
				
				return Wrong_info;
			}
		case 6:
			USB_Device_Chg(USB_1);
			if(USB_DEVICE_INIT()==SUCCESS)         //初始化CH376
			{
			}
			else
			{
				return T_STR("U盘设备未能初始化","Udisk Failed to Initialize");
			}
			USB_Device_Chg(USB_2);
			if(USB_DEVICE_INIT()==SUCCESS)         //初始化CH376
			{
				
				
			}else{
				return T_STR("U盘设备未能初始化","Udisk Failed to Initialize");
			}
			return (const char *)0;
		case 7:
			
			return (const char *)0;
		
		case 8:
			if(calibration_switch == 0)
				if(KeyValue_Read() == 0x02000000)return (const char *)0;
			if((GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) == 1) || (calibration_switch == 1))
			{
				calibration_switch = 1;
//				return "校准开关打开，请关闭测试仪";
				return (const char *)0;
			}
			else
			{
				return (const char *)0;
			}
		
		case 9:
			return (const char *)0;
		
		case 10:
			return (const char *)0;
		
		default:
			
		return (const char *)0;;
			

	}
	
	
// 	if(index == 9)
// 		return "电源抽筋!";
// 	else
// 		return (const char *)0;
}

/********************************************************************************************/
