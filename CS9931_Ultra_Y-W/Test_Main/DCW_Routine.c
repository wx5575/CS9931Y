/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "DCW_Routine.h"
#include "Test_Sched.h"
#include "memorymanagement.h"
#include "Cal.h"
#include "PLC.h"

#define  UPPER_LIMIT          (0)
#define  DOWN_LIMIT           (1)
#define  TEST_PASS            (2)
#define  SHORT_WARNING        (3)
#define  GFI_WARNING          (4)
#define  ARC_WARNING          (5)
#define  VOL_ERROR            (6)

extern  char       mem_str_vol[],mem_str_cur[],mem_str_res[],mem_str_time[];

//电弧侦测表
static  const  float  ARC_Facter_Table[] = 
{
	0.000,
	7.143,
	6.428,
	6.3,
	6.2,
	6.0,
	5.7,
	3.1,
	2.05,
	1.2
};

void DCW_Test_Inerface(uint8_t type,void *value);

typedef struct{
	uint8_t   Test_Keep_Flag : 1;                     //持续测试标志
	uint8_t   ARC_Grade      : 4;                     //电弧侦测等级
	uint16_t  Wait_Time;                              //输出电压前的等待时间
	uint16_t  Test_Time;                              //测试时间
	uint16_t  Down_Time;                              //下降时间
	uint16_t  Pause_Time;                             //步间时间
	uint16_t  Voltage_Rise_Number;                    //电压上升的次数
	uint16_t  Voltage_Down_Number;                    //电压下降的次数
	uint32_t  Voltage_Rise_Interval;                  //电压上升的间隔
	uint32_t  Voltage_Down_Interval;                  //电压下降的间隔
	uint16_t  Voltage_Start_Value;                    //起始输出电压
	uint16_t  Voltage_Process_Value;                  //过程输出电压
	uint16_t  Voltage_Final_Value;                    //最终输出电压
	uint16_t  Voltage_Out_Freq;                       //输出电压频率
	uint16_t  Voltage_Wait_Count;                     //测试等待计数                 
	uint16_t  Voltage_Rise_Count;                     //电压上升计数
	uint16_t  Testing_Count;                          //测试时间计数
	uint16_t  Voltage_Down_Count;                     //电压下降计数
	uint16_t  Pause_Count;                            //步间间隔计数
	uint32_t  current_value;
	uint32_t  Voltage_value; 
}TEST_PARAM;






static struct step_dcw_t Dcw_Mode_Param,*p_Dcw_Mode_Param;
static TEST_PARAM DCW_Test_Param;




static void DCWModeTestEnvironmentEnter(struct step_dcw_t *dcw_test_para)
{
	Dcw_Mode_Param = *dcw_test_para;
	p_Dcw_Mode_Param = dcw_test_para;	
	
	
	/*********************************************/
	//2016.11.26 wangxin
	update_gif_protect_function();
	/*********************************************/
	
	if(Dcw_Mode_Param.waittime < 1)DCW_Test_Param.Wait_Time = 1;             //等待时间设置
 	else DCW_Test_Param.Wait_Time = Dcw_Mode_Param.waittime;                 //确保至少有0.1s
	
	if(Dcw_Mode_Param.ramptime==0){
		DCW_Test_Param.Voltage_Rise_Number = 1;
	}else{
		DCW_Test_Param.Voltage_Rise_Number = Dcw_Mode_Param.ramptime;
	}
	
	if(Dcw_Mode_Param.downtime==0){
		DCW_Test_Param.Voltage_Down_Number = 1;
	}else{
		DCW_Test_Param.Voltage_Down_Number = Dcw_Mode_Param.downtime;
	}
	
	DCW_Test_Param.Test_Keep_Flag        = 0;
	DCW_Test_Param.Test_Time             = Dcw_Mode_Param.testtime;
	DCW_Test_Param.Pause_Time            = Dcw_Mode_Param.pausetime;
	
	if(Dcw_Mode_Param.testtime == 0)DCW_Test_Param.Test_Keep_Flag = 1;
	
	DCW_Test_Param.Voltage_Start_Value   = Dcw_Mode_Param.startvol;
	
	DCW_Test_Param.Voltage_Process_Value = Dcw_Mode_Param.startvol;
	
	DCW_Test_Param.Voltage_Final_Value   = Dcw_Mode_Param.outvol;
	
//	DCW_Test_Param.Voltage_Out_Freq      = Dcw_Mode_Param.outfreq / 10;
	
	DCW_Test_Param.ARC_Grade             = Dcw_Mode_Param.arc;
	
	DCW_Test_Param.Voltage_Wait_Count    = 0;
	
	DCW_Test_Param.Voltage_Rise_Count    = 0;
	
	DCW_Test_Param.Testing_Count         = 0;
	
	DCW_Test_Param.Voltage_Down_Count    = 0;
	
	DCW_Test_Param.current_value         = 0;
	
	DCW_Test_Param.Pause_Count   = 0;
	
	DCW_Test_Param.Voltage_Rise_Interval = (Dcw_Mode_Param.outvol - Dcw_Mode_Param.startvol) / DCW_Test_Param.Voltage_Rise_Number;

	DCW_Test_Param.Voltage_Down_Interval = Dcw_Mode_Param.outvol / DCW_Test_Param.Voltage_Down_Number;

	switch(Dcw_Mode_Param.curgear){
		case I3uA:
			Sampling_Relay_State_CHange(DC_2uA);
		break;
		case I30uA:
			Sampling_Relay_State_CHange(DC_20uA);
		break;
		case I300uA:
			Sampling_Relay_State_CHange(DC_200uA);
			/*更改短路基准*/
			DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
		break;
		case I3mA:
			Sampling_Relay_State_CHange(DC_2mA);
			/*更改短路基准*/
			DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
		break;
		case I30mA:
			Sampling_Relay_State_CHange(DC_20mA);
			/*更改短路基准*/
			if(Dcw_Mode_Param.curhigh <= 500){
				DAC_SetValue(Short_VREF,(1.30 / 6.6) * 4096);//1.05
			}else if(Dcw_Mode_Param.curhigh <= 800){
				DAC_SetValue(Short_VREF,(1.80 / 6.6) * 4096);//1.68
			}else if(Dcw_Mode_Param.curhigh <= 1000){
				DAC_SetValue(Short_VREF,(2.50 / 6.6) * 4096);//2.10
			}else if(Dcw_Mode_Param.curhigh <= 1500){
				DAC_SetValue(Short_VREF,(3.15 / 6.6) * 4096);
			}else if(Dcw_Mode_Param.curhigh <= 2000){
				DAC_SetValue(Short_VREF,(4.20 / 6.6) * 4096);
			}else{
				DAC_SetValue(Short_VREF,(6.6 / 6.6) * 4096);
			}
		break;
		case I100mA:
			Sampling_Relay_State_CHange(DC_100mA);
			/*更改短路基准*/
			if(Dcw_Mode_Param.curhigh <= 50){
				DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
			}else if(Dcw_Mode_Param.curhigh <= 80){
				DAC_SetValue(Short_VREF,(1.68 / 6.6) * 4096);
			}else if(Dcw_Mode_Param.curhigh <= 100){
				DAC_SetValue(Short_VREF,(2.10 / 6.6) * 4096);
			}else if(Dcw_Mode_Param.curhigh <= 150){
				DAC_SetValue(Short_VREF,(3.15 / 6.6) * 4096);
			}else if(Dcw_Mode_Param.curhigh <= 200){
				DAC_SetValue(Short_VREF,(4.20 / 6.6) * 4096);
			}else{
				DAC_SetValue(Short_VREF,(6.6 / 6.6) * 4096);
			}
		break;
		default:
			
		break;
	}
	DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TEST_WAIT);
	if(Test_Sched_Param.Stop_Flag == 0)DC_Output_Enable();  //使能DC输出
	
	LC_Relay_Control(LC_NY,0,1);
	
	{
		EXTI_InitTypeDef EXTI_InitStructure;

		/* Configure  EXTI  */
		EXTI_InitStructure.EXTI_Line = EXTI_Line15 | EXTI_Line7 | EXTI_Line10;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

		EXTI_Init(&EXTI_InitStructure);
	}
	
	
	/*更改短路基准*/
// 	if(Dcw_Mode_Param.curhigh < 5000){
// 		DAC_SetValue(Short_VREF,(1.5 / 6.6) * 4096);
// 	}else if(Dcw_Mode_Param.curhigh < 8000){
// 		DAC_SetValue(Short_VREF,(2.4 / 6.6) * 4096);
// 	}else if(Dcw_Mode_Param.curhigh < 10000){
// 		DAC_SetValue(Short_VREF,(3.0 / 6.6) * 4096);
// 	}
	
	
	/*更改ARC基准*/
	
	if(DCW_Test_Param.ARC_Grade)
	{
		Test_Sched_Param.arc_Isable = 1;
		DAC_SetValue(ARC_VREF,560 * ARC_Facter_Table[DCW_Test_Param.ARC_Grade] * ((float)Global_Cal_Facter.ARC_Facter.DCW_ARC_Base) / 2000);
	}	
	else
	{
		Test_Sched_Param.arc_Isable = 0;
	}
	
	Relay_ON(ACW_DCW_IR);
	ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin

}


void DCWModeTestEnvironmentExit(void)
{
	DC_SetVoltage(0);
	DC_Output_Disable();
	Relay_OFF(ACW_DCW_IR);
}


void DCW_Range_Check(uint8_t type)
{
	uint32_t current;
	switch(Dcw_Mode_Param.curgear){
		case I3uA:
				current = DCW_Test_Param.current_value  ;
			break;
			case I30uA:
				current = DCW_Test_Param.current_value;
			break;
			case I300uA:
				current = DCW_Test_Param.current_value;
			break;
		case I3mA:
		
			current = DCW_Test_Param.current_value / 10;
			
		
		break;
		case I30mA:
		
			current = DCW_Test_Param.current_value;

		
		break;
		case I100mA:
		
			current = DCW_Test_Param.current_value;
			
		break;
		default:
			current = DCW_Test_Param.current_value;
		break;
	}
	switch(type){
		case UPPER_LIMIT:
//			if(Dcw_Mode_Param.curhigh == 0)return;
			if(current > Dcw_Mode_Param.curhigh){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				
				DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)DCW_Test_Param.current_value);
				
				DCWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(DCW,HIGH,(void *)&Dcw_Mode_Param,DCW_Test_Param.Voltage_value,current,0,0,DCW_Test_Param.Testing_Count);
			}
		break;
		
		case DOWN_LIMIT:
			if(Dcw_Mode_Param.curlow == 0)return;
			if(current < Dcw_Mode_Param.curlow){
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				DCWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(DCW,LOW,(void *)&Dcw_Mode_Param,DCW_Test_Param.Voltage_value,current,0,0,DCW_Test_Param.Testing_Count);
			}
		break;
			
		case VOL_ERROR:    //判断电压是否异常
			if(DCW_Test_Param.Voltage_value  < (DCW_Test_Param.Voltage_Final_Value / 2)){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)VOL_ERROR);
				DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(DCW_Test_Param.Voltage_value));		  //将电压值刷新到屏幕
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				DCWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(DCW,VOL_ABNORMAL,(void *)&Dcw_Mode_Param,DCW_Test_Param.Voltage_value,current,0,0,DCW_Test_Param.Testing_Count);
			}
			
		break;
		
		default:
			
		break;
	}
}


void DCW_Mode_Test(struct step_dcw_t *dcw_test_para)
{
	if(Test_Sched_Param.Short_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)SHORT_WARNING);
		switch(Dcw_Mode_Param.curgear){
				case I3mA:
					result_save(DCW,SHORT,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value / 10,0,0,DCW_Test_Param.Testing_Count);	
				break;
				case I30mA:
				case I100mA:
				default:
					result_save(DCW,SHORT,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value,0,0,DCW_Test_Param.Testing_Count);

				break;
		}
		return;
	}
	
	if(Test_Sched_Param.arc_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)ARC_WARNING);
		switch(Dcw_Mode_Param.curgear){
				case I3mA:
					result_save(DCW,ARC,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value / 10,0,0,DCW_Test_Param.Testing_Count);	
				break;
				case I30mA:
				case I100mA:
				default:
					result_save(DCW,ARC,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value,0,0,DCW_Test_Param.Testing_Count);

				break;
		}
		return;
	}
	
	if(Test_Sched_Param.gfi_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		DCW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)GFI_WARNING);
		switch(Dcw_Mode_Param.curgear){
				case I3mA:
					result_save(DCW,GFI,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value / 10,0,0,DCW_Test_Param.Testing_Count);	
				break;
				case I30mA:
				case I100mA:
				default:
					result_save(DCW,GFI,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value,0,0,DCW_Test_Param.Testing_Count);

				break;
		}
		return;
	}
	
	if(Test_Sched_Param.Stop_Flag){                            //Stop按键按下，处理
		DC_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
	}
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT){     //不在测试状态
		DCWModeTestEnvironmentEnter(dcw_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}
	switch(Test_Sched_Param.Test_Step_State){
				
		//等待测试状态
		case TEST_STEP_TEST_WAIT:
			if(--DCW_Test_Param.Wait_Time == 0){
				if(DCW_Test_Param.Voltage_Rise_Number == 1){           //如果上升时间为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
					DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
					DCW_Test_Param.Voltage_Process_Value = DCW_Test_Param.Voltage_Final_Value;
					DC_SetVoltage(DCW_Test_Param.Voltage_Process_Value);
				}else{
					Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_RISE;
					DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_RISE);
					DC_SetVoltage(DCW_Test_Param.Voltage_Start_Value);
				}
				
			}
		
			DCW_Test_Param.Voltage_Wait_Count++;
			DCW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)DCW_Test_Param.Voltage_Wait_Count);
//			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)DC_GetVoltage());
		break;
			
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:
			
			DCW_Test_Param.Voltage_value=DC_GetVoltage();
			DCW_Test_Param.current_value=DC_GetCurrent();
			
			if(--DCW_Test_Param.Voltage_Rise_Number == 0){   //电压到达最终目标电压
				Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;					
				DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
				DC_SetVoltage(DCW_Test_Param.Voltage_Final_Value);					
			}else{
				DCW_Test_Param.Voltage_Process_Value += DCW_Test_Param.Voltage_Rise_Interval;
				DC_SetVoltage(DCW_Test_Param.Voltage_Process_Value);	
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				DCW_Test_Param.current_value = DCW_Test_Param.current_value>(*p_Dcw_Mode_Param).offsetvalue? (DCW_Test_Param.current_value - (*p_Dcw_Mode_Param).offsetvalue) : 0;
			}
			
			/*判断上限*/
			
			
			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(DCW_Test_Param.Voltage_value));
			DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(DCW_Test_Param.current_value));
			DCW_Test_Param.Voltage_Rise_Count++;             //上升时间加1
			
			DCW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)DCW_Test_Param.Voltage_Rise_Count);
			DCW_Range_Check(UPPER_LIMIT);
		
		break;
		//测试状态 
		case TEST_STEP_TESTING:
	
			DCW_Test_Param.Voltage_value=DC_GetVoltage();
			DCW_Test_Param.current_value=DC_GetCurrent();
		
			if(DCW_Test_Param.Test_Keep_Flag){              //测试时间为0，一直测试
				/*判断上限*/
				if(DCW_Test_Param.Testing_Count >= 10000)DCW_Test_Param.Testing_Count = 0;
			}else{
				if(--DCW_Test_Param.Test_Time == 0){
					if(DCW_Test_Param.Voltage_Down_Number == 1){           //如果下降时间为0
						if(DCW_Test_Param.Pause_Time > 0 && current_step_num-1 <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
							Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
							DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
						}else{                                               //步间间隔为0，本次测试结束
							Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
						}
						DCW_Range_Check(VOL_ERROR);
						DC_SetVoltage(0);
						DC_Output_Disable();

					}else{
						Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_DOWN;		
						DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_DOWN);		
					}
				}
			}
			
			DCW_Test_Param.Testing_Count++;             //测试时间加1			
			DCW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)DCW_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				DCW_Test_Param.current_value = DCW_Test_Param.current_value>(*p_Dcw_Mode_Param).offsetvalue? (DCW_Test_Param.current_value - (*p_Dcw_Mode_Param).offsetvalue) : 0;
			}
			
			
			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(DCW_Test_Param.Voltage_value));	
			DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(DCW_Test_Param.current_value));
			DCW_Range_Check(UPPER_LIMIT);
			if(DCW_Test_Param.Testing_Count > 4)DCW_Range_Check(DOWN_LIMIT);
			
			if(Test_Sched_Param.Offset_Get_Flag){
				(*p_Dcw_Mode_Param).offsetvalue = DCW_Test_Param.current_value;
				
			}
			
		break;
		//电压下降状态
		case TEST_STEP_VOL_DOWN:
			
			DCW_Test_Param.Voltage_value=DC_GetVoltage();
			DCW_Test_Param.current_value=DC_GetCurrent();
		
			if(--DCW_Test_Param.Voltage_Down_Number == 0){   //电压到达最终目标电压
				if(DCW_Test_Param.Pause_Time > 0 && current_step_num-1 <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
					DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
				}else{                                               //步间间隔为0，本次测试结束
					Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				}		
//				DCW_Range_Check(VOL_ERROR);
				DC_SetVoltage(0);	
				DC_Output_Disable();
			}else{
				DCW_Test_Param.Voltage_Process_Value -= DCW_Test_Param.Voltage_Down_Interval;
				DC_SetVoltage(DCW_Test_Param.Voltage_Process_Value);
							
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				DCW_Test_Param.current_value = DCW_Test_Param.current_value>(*p_Dcw_Mode_Param).offsetvalue? (DCW_Test_Param.current_value - (*p_Dcw_Mode_Param).offsetvalue) : 0;
			}
			
			DCW_Test_Param.Voltage_Down_Count++;             //测试时间加1
			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(DCW_Test_Param.Voltage_value));	
			DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(DCW_Test_Param.current_value));
			DCW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)DCW_Test_Param.Voltage_Down_Count);
			DCW_Range_Check(UPPER_LIMIT);
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_VOL_DOWN){
				DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(0));	
				DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(0));
			}
		break;
		//步间等待
		case TEST_STEP_PAUSE:
			if(Dcw_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			
			DCW_Range_Check(UPPER_LIMIT);
			DCW_Test_Param.Pause_Count++;
			DCW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)DCW_Test_Param.Pause_Count);
// 			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(DCW_Test_Param.Voltage_value=DC_GetVoltage()));
// 			DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(DCW_Test_Param.current_value=DC_GetCurrent()));
			DCW_Range_Check(UPPER_LIMIT);
			if(--DCW_Test_Param.Pause_Time == 0){
				Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				PLC_Pass_Out(0);
			}
		break;
		//测试停止
		case TEST_STEP_STOP:	
			
//			DCW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
// 			DCW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)0);	
// 			DCW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)0);	
			
			DCWModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
			bsp_display(LED_TEST,0);
			if(DCW_Test_Param.Test_Time == 0 && dcw_test_para->testtime!=0)
			{
				dis_test_pass();
			}
			switch(Dcw_Mode_Param.curgear){
				case I3mA:
					result_save(DCW,PASS,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value / 10,0,0,DCW_Test_Param.Testing_Count);	
				break;
				case I30mA:
				case I100mA:
				default:
					result_save(DCW,PASS,(void *)dcw_test_para,DCW_Test_Param.Voltage_value,DCW_Test_Param.current_value,0,0,DCW_Test_Param.Testing_Count);

				break;
			}
			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}



extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);
void DCW_Test_Inerface(uint8_t type,void *value)
{
	char buf[20] = "";
	struct font_info_t font={&panel_home,0xffff,0x0,1,1,32};
	switch(type){
		
		case TEST_STATE_REFRESH_EVENT:          //测试状态事件
		{
			u32 temp = (u32)value;
			struct rect_type rect={440,72,38,200};
			switch(temp)
			{
				case TEST_STEP_TEST_WAIT:
					Test_Sched_Param.Test_Status = TEST_STATE_WAIT;
					strcpy(buf,T_STR("等待时间","WAIT"));
					break;
				case TEST_STEP_VOL_RISE:
					Test_Sched_Param.Test_Status = TEST_STATE_VOL_RISE;
					strcpy(buf,T_STR("上升时间","RiseTime"));
					break;
				case TEST_STEP_TESTING:
					Test_Sched_Param.Test_Status = TEST_STATE_TEST;
					strcpy(buf,T_STR("正在测试","Testing"));
					break;
				case TEST_STEP_VOL_DOWN:
					Test_Sched_Param.Test_Status = TEST_STATE_VOL_DOWN;
					strcpy(buf,T_STR("下降时间","FallTime"));
					break;
				case TEST_STEP_PAUSE:
					Test_Sched_Param.Test_Status = TEST_STATE_PAUSE;
					strcpy(buf,T_STR("间隔时间","Interval"));
					break;
				case TEST_STEP_STOP:
					Test_Sched_Param.Test_Status = TEST_STATE_STOP;
					strcpy(buf,T_STR("停止测试","STOP"));
					break;
			}
			ui_teststr_darw(&font,&rect,buf);
		}
		break;
		
		case TEST_VOLTAGE_REFRESH_EVENT:        //测试电压事件
//		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		{
			u32 voltage = (u32)value;
			struct rect_type rect={242,140,38,398};
			
			rt_sprintf(buf,"%d.%03dkV", voltage/1000,voltage%1000);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_vol,"                   ");
			strncpy(mem_str_vol,buf,strlen(buf));
		}
// 		bsp_display(LED_TEST,2);
		break;
		
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		strcpy(mem_str_cur,"                   ");
		switch(Dcw_Mode_Param.curgear){
			case I3uA:
				{
					u32 current = (u32)value ;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%03duA", current/1000,current%1000);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			case I30uA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%02duA", current/100,current%100);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			case I300uA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%01duA", current/10,current%10);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			case I3mA:
				{
					u32 current = (u32)value / 10;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%03dmA", current/1000,current%1000);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			case I30mA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%02dmA", current/100,current%100);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			case I100mA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,398};
					
					rt_sprintf(buf,"%d.%01dmA", current/10,current%10);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
				}
			break;
			default:
				
			break;
		}
		break;
		
		case TEST_RESISTER_REFRESH_EVENT:       //测试电阻事件

		break;
		 
		case TEST_WARNING_REFRESH_EVENT:        //测试报警事件
		{
			u32 temp = (u32)value;
			struct rect_type rect={440,72,38,200};
			switch(temp)
			{
				case UPPER_LIMIT:
					strcpy(buf,T_STR("上限报警","High Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case DOWN_LIMIT:
					strcpy(buf,T_STR("下限报警","Low Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case SHORT_WARNING:
					strcpy(buf,T_STR("短路报警","Short Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case GFI_WARNING:
					strcpy(buf,T_STR("GFI报警","GFI Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case ARC_WARNING:
					strcpy(buf,T_STR("ARC报警","ARC Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case VOL_ERROR:
					strcpy(buf,T_STR("电压报警","Vol.Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
			}
			ui_teststr_darw(&font,&rect,buf);
			}
		break;
		
		case TEST_TIME_COUNT_REFRESH_EVENT:     //测试时间事件
		{
			u32 time = (u32)value;
			struct rect_type rect={242,344,38,398};
			
			rt_sprintf(buf,"%d.%ds", time/10,time%10);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_time,"                   ");
			strncpy(mem_str_time,buf,strlen(buf)-1);
		}
		if(Test_Sched_Param.Stop_Flag == 0)bsp_display(LED_TEST,2);
		break;
		
		default:
			
		break;
		
	}
	
}


