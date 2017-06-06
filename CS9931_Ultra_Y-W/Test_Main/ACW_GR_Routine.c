/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "ACW_Routine.h"
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

#define  OPEN_WIRE            (7)
#define  UPPER_LIMIT_V        (8)

// extern  uint16_t   usSRegHoldBuf[];
extern  char       mem_str_vol[],mem_str_cur[],mem_str_realcur[],mem_str_res[],mem_str_time[];

static void ACW_Test_Inerface_err(uint8_t type,void *value);
extern uint32_t Read_Real_Current(uint32_t current);
extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);

static void ACW_Test_Inerface(uint8_t type,void *value);
static void GR_Test_Inerface(uint8_t type,void *value);

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
	uint32_t  real_current_value; 
	uint32_t  Voltage_value; 	
	
	uint16_t  Current_Rise_Number;                    //电流上升的次数
	uint16_t  Current_Down_Number;                    //电流下降的次数
	uint32_t  Current_Rise_Interval;                  //电流上升的间隔
	uint32_t  Current_Down_Interval;                  //电流下降的间隔
	uint16_t  Current_Start_Value;                    //起始输出电流
	uint16_t  Current_Process_Value;                  //过程输出电流
	uint32_t  Current_Final_Value;                    //最终输出电流
	uint16_t  Current_Out_Freq;                       //输出电压频率
	uint16_t  res_value;
	uint8_t   warning_index;
	uint32_t  gr_current_value;  
	uint32_t  gr_Voltage_value; 
}TEST_PARAM;



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


static struct step_acw_gr_t Acw_gr_Mode_Param,*p_Acw_gr_Mode_Param;
static TEST_PARAM ACW_GR_Test_Param;



static void ACWModeTestEnvironmentEnter(struct step_acw_gr_t *acw_gr_test_para)
{
	Acw_gr_Mode_Param   = *acw_gr_test_para;
	p_Acw_gr_Mode_Param = acw_gr_test_para;
	
	if(Acw_gr_Mode_Param.waittime < 1)ACW_GR_Test_Param.Wait_Time = 1;             //等待时间设置
 	else ACW_GR_Test_Param.Wait_Time = Acw_gr_Mode_Param.waittime;                 //确保至少有0.1s
	
	if(Acw_gr_Mode_Param.ramptime==0){
		ACW_GR_Test_Param.Voltage_Rise_Number = 1;
	}else{
		ACW_GR_Test_Param.Voltage_Rise_Number = Acw_gr_Mode_Param.ramptime;
	}
	
	if(Acw_gr_Mode_Param.downtime==0){
		ACW_GR_Test_Param.Voltage_Down_Number = 1;
	}else{
		ACW_GR_Test_Param.Voltage_Down_Number = Acw_gr_Mode_Param.downtime;
	}
	
	ACW_GR_Test_Param.Test_Keep_Flag        = 0;
	ACW_GR_Test_Param.Test_Time             = Acw_gr_Mode_Param.testtime;
	ACW_GR_Test_Param.Pause_Time            = Acw_gr_Mode_Param.pausetime;
	
	if(Acw_gr_Mode_Param.testtime == 0)ACW_GR_Test_Param.Test_Keep_Flag = 1;
	
	ACW_GR_Test_Param.Voltage_Start_Value   = Acw_gr_Mode_Param.startvol;
	
	ACW_GR_Test_Param.Voltage_Process_Value = Acw_gr_Mode_Param.startvol;
	
	ACW_GR_Test_Param.Voltage_Final_Value   = Acw_gr_Mode_Param.outvol;
	
	ACW_GR_Test_Param.Voltage_Out_Freq      = Acw_gr_Mode_Param.outfreq / 10;
	
	ACW_GR_Test_Param.ARC_Grade             = Acw_gr_Mode_Param.arc;
	
	ACW_GR_Test_Param.Voltage_Wait_Count    = 0;
	
	ACW_GR_Test_Param.Voltage_Rise_Count    = 0;
	
	ACW_GR_Test_Param.Testing_Count         = 0;
	
	ACW_GR_Test_Param.Voltage_Down_Count    = 0;
	
	ACW_GR_Test_Param.current_value         = 0;
	
	ACW_GR_Test_Param.real_current_value    = 0;
	
	ACW_GR_Test_Param.Pause_Count           = 0;
	
	ACW_GR_Test_Param.Voltage_Rise_Interval = (Acw_gr_Mode_Param.outvol - Acw_gr_Mode_Param.startvol) / ACW_GR_Test_Param.Voltage_Rise_Number;

	ACW_GR_Test_Param.Voltage_Down_Interval = Acw_gr_Mode_Param.outvol / ACW_GR_Test_Param.Voltage_Down_Number;
	


	switch(Acw_gr_Mode_Param.curgear){
		case I3uA:
			Sampling_Relay_State_CHange(DC_2uA);
			/*更改短路基准*/
			DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
		break;
		case I30uA:
			Sampling_Relay_State_CHange(DC_20uA);
			/*更改短路基准*/
			DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
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
			if(Acw_gr_Mode_Param.curhigh <= 500){
				DAC_SetValue(Short_VREF,(1.30 / 6.6) * 4096);//1.05
			}else if(Acw_gr_Mode_Param.curhigh <= 800){
				DAC_SetValue(Short_VREF,(2.00 / 6.6) * 4096);//1.68
			}else if(Acw_gr_Mode_Param.curhigh <= 1000){
				DAC_SetValue(Short_VREF,(2.50 / 6.6) * 4096);//2.10
			}else if(Acw_gr_Mode_Param.curhigh <= 1500){
				DAC_SetValue(Short_VREF,(3.50 / 6.6) * 4096);//3.15
			}else if(Acw_gr_Mode_Param.curhigh <= 2000){
				DAC_SetValue(Short_VREF,(4.70 / 6.6) * 4096);//4.20
			}else{
				DAC_SetValue(Short_VREF,(6.6 / 6.6) * 4096);
			}
		break;
		case I100mA:
			Sampling_Relay_State_CHange(DC_100mA);
			/*更改短路基准*/
			if(Acw_gr_Mode_Param.curhigh <= 50){
				DAC_SetValue(Short_VREF,(1.05 / 6.6) * 4096);
			}else if(Acw_gr_Mode_Param.curhigh <= 80){
				DAC_SetValue(Short_VREF,(1.68 / 6.6) * 4096);
			}else if(Acw_gr_Mode_Param.curhigh <= 100){
				DAC_SetValue(Short_VREF,(2.10 / 6.6) * 4096);
			}else if(Acw_gr_Mode_Param.curhigh <= 150){
				DAC_SetValue(Short_VREF,(3.15 / 6.6) * 4096);
			}else if(Acw_gr_Mode_Param.curhigh <= 200){
				DAC_SetValue(Short_VREF,(4.20 / 6.6) * 4096);
			}else{
				DAC_SetValue(Short_VREF,(6.6 / 6.6) * 4096);
			}
		break;
		default:
			
		break;
	}
	ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TEST_WAIT);
	if(Test_Sched_Param.Stop_Flag == 0)AC_Output_Enable();  //使能AC输出
	
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
	
	
	
	/*更改ARC基准*/
	
	if(ACW_GR_Test_Param.ARC_Grade)
	{
		Test_Sched_Param.arc_Isable = 1;
		DAC_SetValue(ARC_VREF,560 * ARC_Facter_Table[ACW_GR_Test_Param.ARC_Grade] * ((float)Global_Cal_Facter.ARC_Facter.ACW_ARC_Base) / 2000);
	}	
	else
	{
		Test_Sched_Param.arc_Isable = 0;
	}
	
	/*********************************************/
	//2016.11.26 wangxin 更新gfi断电器状态
	update_gif_protect_function();
	/*********************************************/
	
	Relay_ON(ACW_DCW_IR);
	ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin
	
	Relay_ON(EXT_DRIVER_O7);
	if(Test_Sched_Param.Stop_Flag == 0)GR_Output_Enable();  //使能GR输出
	
	ACW_GR_Test_Param.Current_Rise_Number  = ACW_GR_Test_Param.Voltage_Rise_Number;
	
	ACW_GR_Test_Param.Current_Down_Number  = 1;
	
	ACW_GR_Test_Param.Current_Process_Value = 0;
	
	ACW_GR_Test_Param.Current_Final_Value   = Acw_gr_Mode_Param.groutcur * 100;
	
	ACW_GR_Test_Param.Current_Out_Freq      = Acw_gr_Mode_Param.groutfreq / 10;
	
	ACW_GR_Test_Param.res_value             = 0;
	
	ACW_GR_Test_Param.Pause_Count           = 0;
	
	ACW_GR_Test_Param.warning_index         = 0;
	
	ACW_GR_Test_Param.Current_Rise_Interval = (ACW_GR_Test_Param.Current_Final_Value - ACW_GR_Test_Param.Current_Start_Value) / ACW_GR_Test_Param.Current_Rise_Number;

	ACW_GR_Test_Param.Current_Down_Interval = ACW_GR_Test_Param.Current_Final_Value / ACW_GR_Test_Param.Current_Down_Number;

}


static void ACWModeTestEnvironmentExit(void)
{
	AC_SetVoltage(0,ACW_GR_Test_Param.Voltage_Out_Freq);
	AC_Output_Disable();
	Relay_OFF(ACW_DCW_IR);
	
	Relay_OFF(EXT_DRIVER_O7);
 	GR_Set_Current(0,ACW_GR_Test_Param.Current_Out_Freq);
 	GR_Output_Disable();
}


static void ACW_Range_Check(uint8_t type)
{
	
	uint32_t current;
	switch(Acw_gr_Mode_Param.curgear){
		
		case I3uA:
			current = ACW_GR_Test_Param.current_value / 10;
		break;
		case I30uA:
			current = ACW_GR_Test_Param.current_value;
		break;
		case I300uA:
			current = ACW_GR_Test_Param.current_value;
		break;
		case I3mA:
		
			current = ACW_GR_Test_Param.current_value / 10;
			
		
		break;
		case I30mA:
		
			current = ACW_GR_Test_Param.current_value;

		
		break;
		case I100mA:
		
			current = ACW_GR_Test_Param.current_value;
			
		break;
		default:
			current = ACW_GR_Test_Param.current_value;
		break;
	}
	switch(type){
		case UPPER_LIMIT:
//			if(Acw_gr_Mode_Param.curhigh == 0)return;
			if(current  >= Acw_gr_Mode_Param.curhigh){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(ACW,HIGH,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,current,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
				result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}else if(Acw_gr_Mode_Param.rmscur)
			{
				if(ACW_GR_Test_Param.real_current_value  >= Acw_gr_Mode_Param.rmscur){
					/*超过上限*/
					Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
					ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
					Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
					bsp_display(LED_TEST,0);
					ACWModeTestEnvironmentExit();
					Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
					Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
					Test_Sched_Param.Warning_Flag         = 1;
					result_save(ACW,HIGH,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,current,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
					result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
				}
			}
			
		break;
		
		case DOWN_LIMIT:
			if(Acw_gr_Mode_Param.curlow == 0)return;
			if(current < Acw_gr_Mode_Param.curlow){
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(ACW,LOW ,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,current,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
				result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
		break;
			
		case VOL_ERROR:    //判断电压是否异常
			if(ACW_GR_Test_Param.Voltage_value  < (ACW_GR_Test_Param.Voltage_Final_Value / 2)){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)VOL_ERROR);
				ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value));	  //将电压值刷新到屏幕
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(ACW,VOL_ABNORMAL,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,current,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
				result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
			
		break;
		
		default:
			
		break;
	}
}

static void GR_Range_Check(uint8_t type)
{
	switch(type){
		case UPPER_LIMIT:
			if(ACW_GR_Test_Param.res_value / 10 >= Acw_gr_Mode_Param.grreshigh){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				ACW_GR_Test_Param.warning_index       = 1;
				Test_Sched_Param.Warning_Flag         = 1;
//				result_save(GR,HIGH,(void *)&GR_Test_Param,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
				result_save(ACW,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
			
				result_save_cw_gr(GR,HIGH,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
		break;
		
		
		case DOWN_LIMIT:
			if(Acw_gr_Mode_Param.grreslow == 0)return;
			if(ACW_GR_Test_Param.res_value / 10 <= Acw_gr_Mode_Param.grreslow){
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				ACW_GR_Test_Param.warning_index       = 1;
				Test_Sched_Param.Warning_Flag         = 1;
//				result_save(GR,LOW,(void *)&GR_Test_Param,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
				result_save(ACW,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
			
				result_save_cw_gr(GR,LOW,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
		break;
			
		case OPEN_WIRE:
			if((ACW_GR_Test_Param.gr_current_value=GR_Get_Current()) <= 10){
				/*开路报警*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)OPEN_WIRE);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				ACW_GR_Test_Param.warning_index       = 1;
				Test_Sched_Param.Warning_Flag         = 1;
//				result_save(GR,OPEN,(void *)&GR_Test_Param,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
				result_save(ACW,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
			
				result_save_cw_gr(GR,OPEN,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
		break;
			
		case UPPER_LIMIT_V:
			if(Acw_gr_Mode_Param.gralarmvol == 0)return;
			if(ACW_GR_Test_Param.gr_Voltage_value >= Acw_gr_Mode_Param.gralarmvol * 10){
				/*开路报警*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT_V);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				ACWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				ACW_GR_Test_Param.warning_index       = 1;
				Test_Sched_Param.Warning_Flag         = 1;
//				result_save(GR,HIGH,(void *)&GR_Test_Param,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
				result_save(ACW,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
			
				result_save_cw_gr(GR,HIGH,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			}
		default:
			
		break;
	}
}


void ACW_GR_Mode_Test(struct step_acw_gr_t *acw_gr_test_para)
{
	if(Test_Sched_Param.Short_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)SHORT_WARNING);
		result_save(ACW,SHORT,(void *)acw_gr_test_para,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
		result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
		return;
	}
	
	if(Test_Sched_Param.arc_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)ARC_WARNING);
		result_save(ACW,ARC,(void *)acw_gr_test_para,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
		result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
		return;
	}
	
	if(Test_Sched_Param.gfi_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		ACW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)GFI_WARNING);
		result_save(ACW,GFI,(void *)acw_gr_test_para,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
		result_save_cw_gr(GR,PASS,(void *)&Acw_gr_Mode_Param,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
		return;
	}
	
	if(Test_Sched_Param.Stop_Flag){                            //Stop按键按下，处理
		AC_Output_Disable();
		GR_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
	}
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT){     //不在测试状态
		ACWModeTestEnvironmentEnter(acw_gr_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}
	switch(Test_Sched_Param.Test_Step_State){
				
		//等待测试状态
		case TEST_STEP_TEST_WAIT:
			if(--ACW_GR_Test_Param.Wait_Time == 0){
				if(ACW_GR_Test_Param.Voltage_Rise_Number == 1){           //如果上升时间为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
					ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
					ACW_GR_Test_Param.Voltage_Process_Value = ACW_GR_Test_Param.Voltage_Final_Value;
					AC_SetVoltage(ACW_GR_Test_Param.Voltage_Process_Value,ACW_GR_Test_Param.Voltage_Out_Freq);
					
					ACW_GR_Test_Param.Current_Process_Value = ACW_GR_Test_Param.Current_Final_Value;
					GR_Set_Current(ACW_GR_Test_Param.Current_Process_Value,ACW_GR_Test_Param.Current_Out_Freq);	
				}else{
					Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_RISE;
					ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_RISE);
					AC_SetVoltage(ACW_GR_Test_Param.Voltage_Start_Value,ACW_GR_Test_Param.Voltage_Out_Freq);
					
					GR_Set_Current(ACW_GR_Test_Param.Current_Start_Value,ACW_GR_Test_Param.Current_Out_Freq);	
				}
				
			}
			ACW_GR_Test_Param.Voltage_Wait_Count++;
			ACW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)ACW_GR_Test_Param.Voltage_Wait_Count);
//			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value=AC_GetVoltage()));
		break;
			
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:
			ACW_GR_Test_Param.Voltage_value = AC_GetVoltage();
			ACW_GR_Test_Param.current_value = AC_GetCurrent();
		
		  ACW_GR_Test_Param.gr_Voltage_value=GR_Get_Voltage();
			ACW_GR_Test_Param.gr_current_value=GR_Get_Current();
		
			ACW_GR_Test_Param.res_value = GR_Get_RS();
		
			if(--ACW_GR_Test_Param.Voltage_Rise_Number == 0){   //电压到达最终目标电压
				Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;					
				ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
				ACW_GR_Test_Param.Voltage_Process_Value = ACW_GR_Test_Param.Voltage_Final_Value;
				AC_SetVoltage(ACW_GR_Test_Param.Voltage_Process_Value,ACW_GR_Test_Param.Voltage_Out_Freq);	

				ACW_GR_Test_Param.Current_Process_Value = ACW_GR_Test_Param.Current_Final_Value;
				GR_Set_Current(ACW_GR_Test_Param.Current_Process_Value,ACW_GR_Test_Param.Current_Out_Freq);	
			}else{
				ACW_GR_Test_Param.Voltage_Process_Value += ACW_GR_Test_Param.Voltage_Rise_Interval;
				AC_SetVoltage(ACW_GR_Test_Param.Voltage_Process_Value,ACW_GR_Test_Param.Voltage_Out_Freq);

				ACW_GR_Test_Param.Current_Process_Value += ACW_GR_Test_Param.Current_Rise_Interval;
				GR_Set_Current(ACW_GR_Test_Param.Current_Process_Value,ACW_GR_Test_Param.Current_Out_Freq);
			}
			/*判断上限*/
			if(Test_Sched_Param.Offset_Is_Flag){
				ACW_GR_Test_Param.current_value = ACW_GR_Test_Param.current_value>(*p_Acw_gr_Mode_Param).offsetvalue_cw? (ACW_GR_Test_Param.current_value - (*p_Acw_gr_Mode_Param).offsetvalue_cw) : 0;
			}
			ACW_Range_Check(UPPER_LIMIT);
			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value));
			ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.current_value));
			ACW_GR_Test_Param.Voltage_Rise_Count++;             //上升时间加1
			
			ACW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)ACW_GR_Test_Param.Voltage_Rise_Count);
			
			if(Test_Sched_Param.Offset_Is_Flag){
				ACW_GR_Test_Param.res_value = ACW_GR_Test_Param.res_value>(*p_Acw_gr_Mode_Param).offsetvalue? (ACW_GR_Test_Param.res_value - (*p_Acw_gr_Mode_Param).offsetvalue) : 0;
			}
			
			GR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.gr_Voltage_value));
			GR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.gr_current_value));
			
			if(ACW_GR_Test_Param.Voltage_Rise_Count>Acw_gr_Mode_Param.ramptime / 5){
				GR_Range_Check(OPEN_WIRE);
				if(ACW_GR_Test_Param.warning_index)break;
				GR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.res_value));
				GR_Range_Check(UPPER_LIMIT);
			}
			GR_Range_Check(UPPER_LIMIT_V);
			
		
		break;
		//测试状态 
		case TEST_STEP_TESTING:
	
			ACW_GR_Test_Param.Voltage_value = AC_GetVoltage();
			ACW_GR_Test_Param.current_value = AC_GetCurrent();
		
			ACW_GR_Test_Param.gr_Voltage_value=GR_Get_Voltage();
			ACW_GR_Test_Param.gr_current_value=GR_Get_Current();
		
			ACW_GR_Test_Param.res_value = GR_Get_RS();
		
		
			if(ACW_GR_Test_Param.Test_Keep_Flag){              //测试时间为0，一直测试
				/*判断上限*/
				if(ACW_GR_Test_Param.Testing_Count >= 10000)ACW_GR_Test_Param.Testing_Count = 0;
			}else{
				if(--ACW_GR_Test_Param.Test_Time == 0){
					if(ACW_GR_Test_Param.Voltage_Down_Number == 1){           //如果下降时间为0
						if(ACW_GR_Test_Param.Pause_Time > 0 && current_step_num <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
							Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
							ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
						}else{                                               //步间间隔为0，本次测试结束
							Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
						}
						ACW_Range_Check(VOL_ERROR);
						AC_SetVoltage(0,ACW_GR_Test_Param.Voltage_Out_Freq);
						AC_Output_Disable();
					}else{
						Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_DOWN;		
						
						ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_DOWN);		
					}
					GR_Set_Current(0,ACW_GR_Test_Param.Current_Out_Freq);
				}
			}
			
			ACW_GR_Test_Param.Testing_Count++;             //测试时间加1		

			ACW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)ACW_GR_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				ACW_GR_Test_Param.current_value = ACW_GR_Test_Param.current_value>(*p_Acw_gr_Mode_Param).offsetvalue_cw? (ACW_GR_Test_Param.current_value - (*p_Acw_gr_Mode_Param).offsetvalue_cw) : 0;
			}
			
			
			
			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value));	
			if((ACW_GR_Test_Param.Testing_Count % 2) == 0)
				ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.current_value));
			ACW_Range_Check(UPPER_LIMIT);
			if(ACW_GR_Test_Param.Testing_Count > 4)ACW_Range_Check(DOWN_LIMIT);
			
			if(Test_Sched_Param.Offset_Get_Flag){
				(*p_Acw_gr_Mode_Param).offsetvalue_cw = ACW_GR_Test_Param.current_value;
				
			}
			
			
			if(Test_Sched_Param.Offset_Is_Flag){
				ACW_GR_Test_Param.res_value = ACW_GR_Test_Param.res_value>(*p_Acw_gr_Mode_Param).offsetvalue? (ACW_GR_Test_Param.res_value - (*p_Acw_gr_Mode_Param).offsetvalue) : 0;
			}
			
			GR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.gr_Voltage_value));
			GR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.gr_current_value));
			
			if(ACW_GR_Test_Param.Testing_Count>5){
				GR_Range_Check(OPEN_WIRE);
				if(ACW_GR_Test_Param.warning_index)break;
				GR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.res_value));
				GR_Range_Check(UPPER_LIMIT);
				GR_Range_Check(DOWN_LIMIT);
				GR_Range_Check(UPPER_LIMIT_V);
			}
			
			if(Test_Sched_Param.Offset_Get_Flag){
				(*p_Acw_gr_Mode_Param).offsetvalue = ACW_GR_Test_Param.res_value;
				
			}
			
		break;
		//电压下降状态
		case TEST_STEP_VOL_DOWN:
			
			ACW_GR_Test_Param.Voltage_value = AC_GetVoltage();
			ACW_GR_Test_Param.current_value = AC_GetCurrent();
		
			if(--ACW_GR_Test_Param.Voltage_Down_Number == 0){   //电压到达最终目标电压
				if(ACW_GR_Test_Param.Pause_Time > 0 && current_step_num <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
					ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
				}else{                                               //步间间隔为0，本次测试结束
					Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				}			
//				ACW_Range_Check(VOL_ERROR);
				AC_SetVoltage(0,ACW_GR_Test_Param.Voltage_Out_Freq);	
				AC_Output_Disable();
			}else{
				ACW_GR_Test_Param.Voltage_Process_Value -= ACW_GR_Test_Param.Voltage_Down_Interval;
				AC_SetVoltage(ACW_GR_Test_Param.Voltage_Process_Value,ACW_GR_Test_Param.Voltage_Out_Freq);
							
			}
			if(Test_Sched_Param.Offset_Is_Flag){
				ACW_GR_Test_Param.current_value = ACW_GR_Test_Param.current_value>(*p_Acw_gr_Mode_Param).offsetvalue_cw? (ACW_GR_Test_Param.current_value - (*p_Acw_gr_Mode_Param).offsetvalue_cw) : 0;
			}
			ACW_GR_Test_Param.Voltage_Down_Count++;             //测试时间加1
			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value));	
			ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.current_value));
			ACW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)ACW_GR_Test_Param.Voltage_Down_Count);
			ACW_Range_Check(UPPER_LIMIT);
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_VOL_DOWN){
				ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(0));	
				ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(0));
			}
		break;
		//步间等待
		case TEST_STEP_PAUSE:
			
			if(Acw_gr_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			
			
			ACW_GR_Test_Param.Pause_Count++;
			ACW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)ACW_GR_Test_Param.Pause_Count);
// 			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.Voltage_value=AC_GetVoltage()));
// 			ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(ACW_GR_Test_Param.current_value=AC_GetCurrent()));
			ACW_Range_Check(UPPER_LIMIT);
			if(--ACW_GR_Test_Param.Pause_Time == 0){
				Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				PLC_Pass_Out(0);
			}
		break;
		//测试停止
		case TEST_STEP_STOP:	
			
//			ACW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
// 			ACW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)0);	
// 			ACW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)0);	

			
			ACWModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
			bsp_display(LED_TEST,0);
			
			result_save(ACW,PASS,(void *)acw_gr_test_para,ACW_GR_Test_Param.Voltage_value,ACW_GR_Test_Param.current_value,ACW_GR_Test_Param.real_current_value,0,ACW_GR_Test_Param.Testing_Count);
			result_save_cw_gr(GR,PASS,(void *)acw_gr_test_para,ACW_GR_Test_Param.gr_Voltage_value,ACW_GR_Test_Param.gr_current_value,0,ACW_GR_Test_Param.res_value,ACW_GR_Test_Param.Testing_Count);
			
			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}

static void ACW_Test_Inerface(uint8_t type,void *value)
{
	char buf[20] = "";
	struct font_info_t font={&panel_home,0xffff,0x0,1,1,32};
	
	switch(type)
	{
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
					strcpy(buf,T_STR("测试时间","TestTime"));
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
			break;
		}
		case TEST_VOLTAGE_REFRESH_EVENT:        //测试电压事件
		{
//		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
			u32 voltage = (u32)value;
			struct rect_type rect={242,140,38,199};
			
			rt_sprintf(buf,"%d.%03dkV", voltage/1000,voltage%1000);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_vol,"                   ");
			strncpy(mem_str_vol,buf,strlen(buf));
// 		if(Test_Sched_Param.Stop_Flag == 0)bsp_display(LED_TEST,2);
			break;
		}
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		{
			if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)
			{
				return;
			}
			
			strcpy(mem_str_cur		,"                   ");
			strcpy(mem_str_realcur	,"                   ");
			
			switch(Acw_gr_Mode_Param.curgear)
			{
				case I3uA:
				{
					u32 current = (u32)value / 10;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%03duA", current/1000,current%1000);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%03duA", ACW_GR_Test_Param.real_current_value/1000,
										ACW_GR_Test_Param.real_current_value%1000);
					}
					else
					{
						strcpy(buf,"-.---uA");
					}
					
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				case I30uA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%02duA", current/100,current%100);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%02duA", ACW_GR_Test_Param.real_current_value/100,
										ACW_GR_Test_Param.real_current_value%100);
					}
					else
					{
						strcpy(buf,"--.--uA");
					}
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				case I300uA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%01duA", current/10,current%10);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%01duA", ACW_GR_Test_Param.real_current_value/10,
										ACW_GR_Test_Param.real_current_value%10);
					}
					else
					{
						strcpy(buf,"---.-uA");
					}
					
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				case I3mA:
				{
					u32 current = (u32)value / 10;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%03dmA", current/1000,current%1000);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%03dmA", ACW_GR_Test_Param.real_current_value/1000,
									ACW_GR_Test_Param.real_current_value%1000);
					}
					else
					{
						strcpy(buf,"-.---mA");
					}
					
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				case I30mA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%02dmA", current/100,current%100);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%02dmA", ACW_GR_Test_Param.real_current_value/100,
									ACW_GR_Test_Param.real_current_value%100);
					}
					else
					{
						strcpy(buf,"--.--mA");
					}
					
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				case I100mA:
				{
					u32 current = (u32)value;
					struct rect_type rect={242,208,38,199};
					
					rt_sprintf(buf,"%d.%01dmA", current/10,current%10);
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_cur,buf,strlen(buf));
					rect.y+=68;
					
					if(Acw_gr_Mode_Param.rmscur)
					{
						ACW_GR_Test_Param.real_current_value = Read_Real_Current(current);
						rt_sprintf(buf,"%d.%01dmA", ACW_GR_Test_Param.real_current_value/10,
								ACW_GR_Test_Param.real_current_value%10);
					}
					else
					{
						strcpy(buf,"---.-mA");
					}
					
					ui_teststr_darw(&font,&rect,buf);
					strncpy(mem_str_realcur,buf,strlen(buf));
					break;
				}
				default:
					
					break;
			}
			
			break;
		}
		case TEST_RESISTER_REFRESH_EVENT:       //测试电阻事件
		{
			break;
		}
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
					strcpy(buf,T_STR("短路报警","ShortFail"));
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
			break;
		}
		case TEST_TIME_COUNT_REFRESH_EVENT:     //测试时间事件
		{
			u32 time = (u32)value;
			
			struct rect_type rect={242,344,38,398};
// 			usSRegHoldBuf[0] = time;
			
			rt_sprintf(buf,"%d.%ds", time/10,time%10);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_time,"                   ");
			strncpy(mem_str_time,buf,strlen(buf)-1);
			
			if(Test_Sched_Param.Stop_Flag == 0)
			{
				bsp_display(LED_TEST,2);
			}
			
			break;
		}
		default:
			break;
	}
}


static void GR_Test_Inerface(uint8_t type,void *value)
{
	char buf[20] = "";
	struct font_info_t font={&panel_home,0xffff,0x0,1,1,32};
	switch(type){
		
		case TEST_VOLTAGE_REFRESH_EVENT:        //测试电压事件
		{
			u32 temp = (u32)value / 10;
			struct rect_type rect={242+199,276,38,199};
			rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
			ui_teststr_darw(&font,&rect,buf);
		}
		break;
		
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		{
			u32 current = (u32)value/100;
			struct rect_type rect={242+199,140,38,199};
						
			rt_sprintf(buf,"%d.%dA", current/10,current%10);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_cur,"                   ");
			strncpy(mem_str_cur,buf,strlen(buf));
		}
// 		bsp_display(LED_TEST,2);
		break;
		
		case TEST_RESISTER_REFRESH_EVENT:       //测试电阻事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
	  {
			u32 res = (u32)value;
			struct rect_type rect={242+199,208,38,199};
			
 			rt_sprintf(buf,"%dmΩ", res/10);

			ui_teststr_darw(&font,&rect,buf);
			
			strcpy(mem_str_res,"                   ");
			strncpy(mem_str_res,buf,strlen(buf));
		}
		break;
		 
		case TEST_WARNING_REFRESH_EVENT:        //测试报警事件
		{
			u32 temp = (u32)value;
			struct rect_type rect={440,72,38,200};
			switch(temp)
			{
				case UPPER_LIMIT:
					strcpy(buf,T_STR("上限报警","High Fail"));
					
				break;
				case DOWN_LIMIT:
					strcpy(buf,T_STR("下限报警","Low Fail"));
				break;
				case OPEN_WIRE:
					strcpy(buf,T_STR("开路报警","Open Fail"));
				break;
				case UPPER_LIMIT_V:
					strcpy(buf,T_STR("电压报警","Vol.Fail"));
				break;
			
			}
			bsp_display(LED_FAIL,1);
			bsp_display(FMQ,1);
			ui_teststr_darw(&font,&rect,buf);
		}
		break;
		
	
		
		default:
			
		break;
		
	}
	
}




