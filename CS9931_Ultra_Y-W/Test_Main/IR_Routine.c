/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "DCW_Routine.h"
#include "Test_Sched.h"
#include "memorymanagement.h"
#include "Cal.h"
#include "PLC.h"
#include "IR.h"

#define  UPPER_LIMIT          (0)
#define  DOWN_LIMIT           (1)
#define  TEST_PASS            (2)
#define  SHORT_WARNING        (3)
#define  GFI_WARNING          (4)
#define  ARC_WARNING          (5)
#define  VOL_ERROR            (6)


#define  GEAR_10M             (1)
#define  GEAR_100M            (2)
#define  GEAR_1G              (3)
#define  GEAR_10G             (4)

extern  char       mem_str_vol[],mem_str_cur[],mem_str_res[],mem_str_time[];


void IR_Test_Inerface(uint8_t type,void *value);
static uint8_t Auto_Change_Gear(void);

typedef struct{
	uint8_t   Test_Keep_Flag : 1;                     //持续测试标志
	uint8_t   Gear           : 4;                     //档位
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
	uint32_t  IR_Res_Value;
}TEST_PARAM;






static struct step_ir_t IR_Mode_Param,*p_IR_Mode_Param;
static TEST_PARAM IR_Test_Param;




static void IRModeTestEnvironmentEnter(struct step_ir_t *ir_test_para)
{
	IR_Mode_Param = *ir_test_para;
		
	if(IR_Mode_Param.waittime < 1)IR_Test_Param.Wait_Time = 1;             //等待时间设置
 	else IR_Test_Param.Wait_Time = IR_Mode_Param.waittime;                 //确保至少有0.1s
	
	if(IR_Mode_Param.ramptime==0){
		IR_Test_Param.Voltage_Rise_Number = 1;
	}else{
		IR_Test_Param.Voltage_Rise_Number = IR_Mode_Param.ramptime;
	}
	
	
	IR_Test_Param.Test_Keep_Flag        = 0;
	IR_Test_Param.Test_Time             = IR_Mode_Param.testtime;
	IR_Test_Param.Pause_Time            = IR_Mode_Param.pausetime;
	
	if(IR_Mode_Param.testtime == 0)IR_Test_Param.Test_Keep_Flag = 1;
	
	IR_Test_Param.Voltage_Start_Value   = 0;
	
	IR_Test_Param.Voltage_Process_Value = 0;
	
	IR_Test_Param.Voltage_Final_Value   = IR_Mode_Param.outvol;
	
	IR_Test_Param.Voltage_Wait_Count    = 0;
	
	IR_Test_Param.Voltage_Rise_Count    = 0;
	
	IR_Test_Param.Testing_Count         = 0;
	
	IR_Test_Param.current_value         = 0;
	
	IR_Test_Param.Pause_Count           = 0;
	
	IR_Test_Param.Gear                  = IR_Mode_Param.autogear;
	
	IR_Test_Param.Voltage_Rise_Interval = (IR_Mode_Param.outvol - 0) / IR_Test_Param.Voltage_Rise_Number;


	IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TEST_WAIT);
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
	
	
	switch(IR_Mode_Param.autogear)
	{
		case 0:  //AUTO_GEAR
			IR_Test_Param.Gear = GEAR_10M;
			Sampling_Relay_State_CHange(DC_2mA);
		break;
		
		case 1:  //10MΩ
			Sampling_Relay_State_CHange(DC_2mA);
		break;
		
		case 2:  //100MΩ
			Sampling_Relay_State_CHange(DC_200uA);
		break;
		
		case 3:  //1GΩ
			Sampling_Relay_State_CHange(DC_20uA);
		break;
		
		case 4:  //10GΩ
			Sampling_Relay_State_CHange(DC_2uA);
		break;
		
		case 5:  //100GΩ
			IR_Test_Param.Gear = GEAR_10G;
			Sampling_Relay_State_CHange(DC_2uA);
		break;
		
		default:
			Sampling_Relay_State_CHange(DC_2mA);
		
		break;
		
	}
		
	
	
	
	
	Relay_ON(ACW_DCW_IR);
	ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin

}


void IRModeTestEnvironmentExit(void)
{
	DC_SetVoltage(0);
	DC_Output_Disable();
	Relay_OFF(ACW_DCW_IR);
}


void IR_Range_Check(uint8_t type)
{
	if(IR_Test_Param.IR_Res_Value == 0xFFFFFFFF)return;
	switch(type){
		case UPPER_LIMIT:
			if(IR_Mode_Param.reshigh == 0)return;
			if(IR_Test_Param.IR_Res_Value > IR_Mode_Param.reshigh){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				IR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				IRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(IR,HIGH,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
			}
		break;
		
		case DOWN_LIMIT:
			if(IR_Mode_Param.reslow == 0)return;
			if(IR_Test_Param.IR_Res_Value < IR_Mode_Param.reslow){
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				IR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				IRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(IR,LOW,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
			}
		break;
			
		case VOL_ERROR:    //判断电压是否异常
			if(IR_Test_Param.Voltage_value  < (IR_Test_Param.Voltage_Final_Value / 2)){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				IR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)VOL_ERROR);
				IR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(IR_Test_Param.Voltage_value));		  //将电压值刷新到屏幕
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				IRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(IR,VOL_ABNORMAL,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
			}
			
		break;
		
		default:
			
		break;
	}
}


void IR_Mode_Test(struct step_ir_t *IR_test_para)
{
	if(Test_Sched_Param.Short_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		IR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)SHORT_WARNING);

		result_save(IR,SHORT,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
		return;
	}
	
	
	if(Test_Sched_Param.gfi_Flag)
	{
		Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Warning_Flag         = 1;
		IR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)GFI_WARNING);
		result_save(IR,GFI,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
		return;
	}
	
	if(Test_Sched_Param.Stop_Flag){                            //Stop按键按下，处理
		DC_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
	}
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT){     //不在测试状态
		IRModeTestEnvironmentEnter(IR_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}
	switch(Test_Sched_Param.Test_Step_State){
				
		//等待测试状态
		case TEST_STEP_TEST_WAIT:
			if(--IR_Test_Param.Wait_Time == 0){
				if(IR_Test_Param.Voltage_Rise_Number == 1){           //如果上升时间为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
					IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
					IR_Test_Param.Voltage_Process_Value = IR_Test_Param.Voltage_Final_Value;
					DC_SetVoltage(IR_Test_Param.Voltage_Process_Value);
				}else{
					Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_RISE;
					IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_RISE);
					DC_SetVoltage(IR_Test_Param.Voltage_Start_Value);
				}
				
			}
		
			IR_Test_Param.Voltage_Wait_Count++;
			IR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)IR_Test_Param.Voltage_Wait_Count);
		break;
			
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:
			
			IR_Test_Param.Voltage_value=DC_GetVoltage();
			IR_Test_Param.current_value=DC_GetCurrent();
			
			IR_Test_Param.IR_Res_Value  = IR_Get_RS(IR_Test_Param.Gear);
		
			if(IR_Mode_Param.autogear == 0)  //需要自动换挡
			{
				if(Auto_Change_Gear())
				{
					
				}
			}
			
			if(--IR_Test_Param.Voltage_Rise_Number == 0){   //电压到达最终目标电压
				Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;					
				IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
				DC_SetVoltage(IR_Test_Param.Voltage_Final_Value);					
			}else{
				IR_Test_Param.Voltage_Process_Value += IR_Test_Param.Voltage_Rise_Interval;
				DC_SetVoltage(IR_Test_Param.Voltage_Process_Value);	
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				IR_Test_Param.current_value = IR_Test_Param.current_value>(*p_IR_Mode_Param).offsetvalue? (IR_Test_Param.current_value - (*p_IR_Mode_Param).offsetvalue) : 0;
			}
			
			/*判断上限*/
			
			
			IR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(IR_Test_Param.Voltage_value));
			IR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(IR_Test_Param.current_value));
			IR_Test_Param.Voltage_Rise_Count++;             //上升时间加1
			
			IR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(IR_Test_Param.IR_Res_Value));
			IR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)IR_Test_Param.Voltage_Rise_Count);
			IR_Range_Check(UPPER_LIMIT);
		
		break;
		//测试状态 
		case TEST_STEP_TESTING:
	
			IR_Test_Param.Voltage_value = DC_GetVoltage();
			IR_Test_Param.current_value = DC_GetCurrent();
			
			IR_Test_Param.IR_Res_Value  = IR_Get_RS(IR_Test_Param.Gear);
		
			if(IR_Mode_Param.autogear == 0)  //需要自动换挡
			{
				if(Auto_Change_Gear())
				{
					
				}
			}
		
			if(IR_Test_Param.Test_Keep_Flag){              //测试时间为0，一直测试
				/*判断上限*/
				if(IR_Test_Param.Testing_Count >= 10000)IR_Test_Param.Testing_Count = 0;
			}else{
				if(--IR_Test_Param.Test_Time == 0){
					if(1){           //如果下降时间为0
						if(IR_Test_Param.Pause_Time > 0 && current_step_num <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
							Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
							IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
						}else{                                               //步间间隔为0，本次测试结束
							Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
						}
						IR_Range_Check(VOL_ERROR);
						DC_SetVoltage(0);
						DC_Output_Disable();

					}else{
						Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_DOWN;		
						IR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_DOWN);		
					}
				}
			}
			
			IR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(IR_Test_Param.IR_Res_Value));
			
			IR_Test_Param.Testing_Count++;             //测试时间加1			
			IR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)IR_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}
			
			if(Test_Sched_Param.Offset_Is_Flag){
				IR_Test_Param.current_value = IR_Test_Param.IR_Res_Value>(*p_IR_Mode_Param).offsetvalue? (IR_Test_Param.IR_Res_Value - (*p_IR_Mode_Param).offsetvalue) : 0;
			}
			
			
			IR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(IR_Test_Param.Voltage_value));	
			IR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(IR_Test_Param.current_value));
			IR_Range_Check(UPPER_LIMIT);
			if(IR_Test_Param.Testing_Count > 4)IR_Range_Check(DOWN_LIMIT);
			
			if(Test_Sched_Param.Offset_Get_Flag){
				(*p_IR_Mode_Param).offsetvalue = IR_Test_Param.IR_Res_Value;
				
			}
			
		break;
		//电压下降状态
		case TEST_STEP_VOL_DOWN:
			
		
		break;
		//步间等待
		case TEST_STEP_PAUSE:
			if(IR_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			
			IR_Range_Check(UPPER_LIMIT);
			IR_Test_Param.Pause_Count++;
			IR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)IR_Test_Param.Pause_Count);

			IR_Range_Check(UPPER_LIMIT);
			if(--IR_Test_Param.Pause_Time == 0){
				Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				PLC_Pass_Out(0);
			}
		break;
		//测试停止
		case TEST_STEP_STOP:	
			
			
			IRModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
			bsp_display(LED_TEST,0);
			
			result_save(IR,PASS,(void *)&IR_Mode_Param,IR_Test_Param.Voltage_value,0,0,IR_Test_Param.IR_Res_Value,IR_Test_Param.Testing_Count);
			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}



extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);
void IR_Test_Inerface(uint8_t type,void *value)
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

		break;
		
		case TEST_RESISTER_REFRESH_EVENT:       //测试电阻事件
			if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
			{
				u32 res = (u32)value;
				
				struct rect_type rect={242,208,38,398};
				if(res == 0xFFFFFFFF){
					rt_sprintf(buf,"-.---MΩ");
				}else{
					if(res<10000)//0~100M
					{
						rt_sprintf(buf,"%d.%02dMΩ", res/100,res%100);
					}else if(res<100000)//100M~1G
					{
						res = res / 10;											
						rt_sprintf(buf,"%d.%01dMΩ", res/10,res%10);
					}else if(res<1000000)//1G~10G
					{
						res = res / 100;											
						rt_sprintf(buf,"%d.%03dGΩ", res/1000,res%1000);
					}
					else if(res<10000000)//10G~100G
					{
						res = res / 1000;											
						rt_sprintf(buf,"%d.%02dGΩ", res/100,res%100);
					}
					else 
					{
						res = res / 10000;											
						rt_sprintf(buf,"%d.%01dGΩ", res/10,res%10);
					}
				}
				

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

static uint8_t Auto_Change_Gear(void)
{
	uint8_t Is_need_Change = 0;
	
	switch(IR_Test_Param.Gear)
	{
		case GEAR_10M:
			if(IR_Test_Param.IR_Res_Value>=1100)
			{
				IR_Test_Param.Gear = GEAR_100M;
				Is_need_Change = 1;
			}
		break;
		
		case GEAR_100M:
			if(IR_Test_Param.IR_Res_Value>=11000)
			{
				IR_Test_Param.Gear = GEAR_1G;
				Is_need_Change = 1;
			}
			if(IR_Test_Param.IR_Res_Value<=950)
			{
				IR_Test_Param.Gear = GEAR_10M;
				Is_need_Change = 1;
			}
		break;
		
		case GEAR_1G:
			if(IR_Test_Param.IR_Res_Value>=105000)
			{
				IR_Test_Param.Gear = GEAR_10G;
				Is_need_Change = 1;
			}
			if(IR_Test_Param.IR_Res_Value<=9500)
			{
				IR_Test_Param.Gear = GEAR_100M;
				Is_need_Change = 1;
			}
		break;
		
		case GEAR_10G:
			if(IR_Test_Param.IR_Res_Value<=95000)
			{
				IR_Test_Param.Gear = GEAR_1G;
				Is_need_Change = 1;
			}
		break;
		
		default:
			
		break;
	
	}
	
	if(Is_need_Change)
	{
		switch(IR_Test_Param.Gear)
		{
							
			case GEAR_10M:  //10MΩ
				Sampling_Relay_State_CHange(DC_2mA);
			break;
			
			case GEAR_100M:  //100MΩ
				Sampling_Relay_State_CHange(DC_200uA);
			break;
			
			case GEAR_1G:  //1GΩ
				Sampling_Relay_State_CHange(DC_20uA);
			break;
			
			case GEAR_10G:  //10GΩ
				Sampling_Relay_State_CHange(DC_2uA);
			break;
			
			case 5:  //100GΩ
				Sampling_Relay_State_CHange(DC_2uA);
			break;
			
			default:
				Sampling_Relay_State_CHange(DC_2mA);
			
			break;
			
		}
	}
	
	return Is_need_Change;
}
