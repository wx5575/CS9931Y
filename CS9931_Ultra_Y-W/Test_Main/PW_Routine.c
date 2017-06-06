/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "zwd414b.h"
#include "Test_Sched.h"
#include "memorymanagement.h"
#include "Cal.h"
#include "PLC.h"

#define  UPPER_LIMIT          (0)
#define  DOWN_LIMIT           (1)
#define  TEST_PASS            (2)


// extern  uint16_t   usSRegHoldBuf[];
extern  char       mem_str_vol[],mem_str_cur[],mem_str_realcur[],mem_str_res[],mem_str_time[];

extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);

void PW_Test_Inerface(uint8_t type,void *value);

typedef struct{
	uint8_t   Test_Keep_Flag : 1;                     //持续测试标志
	uint16_t  Wait_Time;                              //输出电压前的等待时间
	uint16_t  Test_Time;                              //测试时间
	uint16_t  Pause_Time;                             //步间时间
	uint32_t  Voltage_Final_Value;
	uint16_t  Voltage_Out_Freq;                       //输出电压频率
	uint16_t  Voltage_Wait_Count;                     //测试等待计数
	uint16_t  Testing_Count;                          //测试时间计数
	uint16_t  Pause_Count;                            //步间间隔计数
	uint32_t  current_value;  
	uint32_t  Voltage_value; 	
	uint32_t  Power_value; 	
	uint32_t  Facter_value; 
}TEST_PARAM;






static struct step_pw_t PW_Mode_Param,*p_PW_Mode_Param;
static TEST_PARAM PW_Test_Param;




static void PWModeTestEnvironmentEnter(struct step_pw_t *pw_test_para)
{
	PW_Mode_Param = *pw_test_para;
	p_PW_Mode_Param = pw_test_para;
	p_PW_Mode_Param = p_PW_Mode_Param;
	
	PW_Test_Param.Test_Keep_Flag        = 0;
	PW_Test_Param.Testing_Count         = 0;
	PW_Test_Param.Pause_Count           = 0;
	
	PW_Test_Param.Test_Time             = PW_Mode_Param.testtime;
	PW_Test_Param.Pause_Time            = PW_Mode_Param.pausetime;
	
	PW_Test_Param.Voltage_Final_Value   = PW_Mode_Param.outvol  * 100;
	PW_Test_Param.Voltage_Out_Freq      = PW_Mode_Param.outfreq / 10;
	
	if(PW_Mode_Param.testtime == 0)PW_Test_Param.Test_Keep_Flag = 1;
	
	CPLD_GPIO_Control(OUT_C,1);
	Relay_ON(GFI_GND_SELECT);
	
	if(Test_Sched_Param.Stop_Flag == 0){
		LC_Main_Output_Enable();    //使能输出
	}
	
	
}


static void PWModeTestEnvironmentExit(void)
{
	LC_Main_Voltage_Set(0,PW_Test_Param.Voltage_Out_Freq);
	LC_Main_Output_Disable();

	Relay_OFF(GFI_GND_SELECT);

	 
	CPLD_GPIO_Control(OUT_C,0);
	Relay_OFF(EXT_DRIVER_O6);
}


static void PW_Range_Check(uint8_t type)
{
	switch(type){
		case UPPER_LIMIT:
			if(PW_Test_Param.current_value >= PW_Mode_Param.curhigh)
			{
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save_pw(PW,HIGH,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

								
			}
			else if(PW_Test_Param.Power_value >= PW_Mode_Param.pwhigh)
			{
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save_pw(PW,HIGH,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			}
			else if(PW_Test_Param.Facter_value >= PW_Mode_Param.factorhigh && PW_Mode_Param.factorhigh != 1000)
			{
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save_pw(PW,HIGH,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			}
			else
			{
						
			
			}
		break;
		
		
		case DOWN_LIMIT:
						
			if(PW_Test_Param.current_value <= PW_Mode_Param.curlow && PW_Mode_Param.curlow != 0)
			{
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;		
				result_save_pw(PW,LOW,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			}
			else if(PW_Test_Param.Power_value <= PW_Mode_Param.pwlow && PW_Mode_Param.pwlow != 0)
			{
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save_pw(PW,LOW,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			}
			else if(PW_Test_Param.Facter_value <= PW_Mode_Param.factorlow && PW_Mode_Param.factorlow != 0)
			{
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				PW_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				PWModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save_pw(PW,LOW,(void *)p_PW_Mode_Param,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			}
			else
			{
				
				
			}
		break;
			
			
		default:
			
		break;
	}
}


void PW_Mode_Test(struct step_pw_t *pw_test_para)
{
	if(Test_Sched_Param.Stop_Flag){                            //Stop按键按下，处理
	  LC_Main_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		PW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
		Relay_OFF(EXT_DRIVER_O8);	
	}

	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT){     //不在测试状态
		PWModeTestEnvironmentEnter(pw_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}


	switch(Test_Sched_Param.Test_Step_State){
				
		//等待测试状态
		case TEST_STEP_TEST_WAIT:
			Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
			Relay_ON(EXT_DRIVER_O8);
			LC_Main_Voltage_Set(PW_Test_Param.Voltage_Final_Value,PW_Test_Param.Voltage_Out_Freq);
				
		break;
			
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:

		

			
		
		break;
		//测试状态 
		case TEST_STEP_TESTING:
			
			PW_Test_Param.Voltage_value  =  (uint32_t)(Zwd414b_Receive_data.Voltage.data * 10);
			PW_Test_Param.current_value  =  (uint32_t)(Zwd414b_Receive_data.Current.data * 100);
			if(Zwd414b_Receive_data.Power.data<0)
			{
//				PW_Test_Param.Power_value    =  (uint32_t)(-Zwd414b_Receive_data.Power.data);
				PW_Test_Param.Power_value    =  0;
			}
			else
			{
				PW_Test_Param.Power_value    =  (uint32_t)(Zwd414b_Receive_data.Power.data);
			}
			
			PW_Test_Param.Facter_value   =  (uint32_t)(Zwd414b_Receive_data.PF.data * 1000);
		
			if(PW_Test_Param.Test_Keep_Flag){              //测试时间为0，一直测试
				/*判断上限*/
				if(PW_Test_Param.Testing_Count >= 10000)PW_Test_Param.Testing_Count = 0;
			}else{
				if(--PW_Test_Param.Test_Time == 0){
					if(PW_Test_Param.Pause_Time > 0 && current_step_num <= file_info[flash_info.current_file].totalstep){                   //步间间隔不为0
							Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
							PW_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
						}else{                                               //步间间隔为0，本次测试结束
							Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
						}
				}
			}
			
			PW_Test_Param.Testing_Count++;             //测试时间加1		

			PW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)PW_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}
			
			
			
			
			
			PW_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(PW_Test_Param.Voltage_value));	
		
			PW_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(PW_Test_Param.current_value));
			
			PW_Range_Check(UPPER_LIMIT);

			if(PW_Test_Param.Testing_Count > 4)PW_Range_Check(DOWN_LIMIT);
			
			
		break;
		//电压下降状态
		case TEST_STEP_VOL_DOWN:
			
		
		break;
		//步间等待
		case TEST_STEP_PAUSE:
			
			if(PW_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			
			
			PW_Test_Param.Pause_Count++;
			PW_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)PW_Test_Param.Pause_Count);

			
		break;
		//测试停止
		case TEST_STEP_STOP:	
			


			
			PWModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
			bsp_display(LED_TEST,0);
			result_save_pw(PW,PASS,(void *)pw_test_para,PW_Test_Param.Voltage_value,PW_Test_Param.current_value,PW_Test_Param.Power_value,PW_Test_Param.Facter_value,PW_Test_Param.Testing_Count);

			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}


static void PW_Test_Inerface(uint8_t type,void *value)
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
					
					break;
				case TEST_STEP_TESTING:
					Test_Sched_Param.Test_Status = TEST_STATE_TEST;
					strcpy(buf,T_STR("测试时间","TestTime"));
					break;
				case TEST_STEP_VOL_DOWN:
					
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
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		{
			u32 voltage = (u32)value;
			struct rect_type rect={242,140,38,398};
			
			rt_sprintf(buf,"%d.%01dV", voltage/10,voltage%10);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_vol,"                   ");
			strncpy(mem_str_vol,buf,strlen(buf));
		}
 		if(Test_Sched_Param.Stop_Flag == 0)bsp_display(LED_TEST,2);
		break;
		
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		strcpy(mem_str_cur,"                   ");
		
		{
			u32 current = (u32)value;
			struct rect_type rect={242,208,38,398};
			
			rt_sprintf(buf,"%d.%02d A", current/100,current%100);
			ui_teststr_darw(&font,&rect,buf);
	
		}
		
		{
			struct rect_type rect={242,276,38,199};  //功率
			
			rt_sprintf(buf,"%d.%03dkW", PW_Test_Param.Power_value/1000,PW_Test_Param.Power_value%1000);
			ui_teststr_darw(&font,&rect,buf);
	
		}
		
		{
			struct rect_type rect={242+199,276,38,199};  //功率因数
			
			rt_sprintf(buf,"%d.%03d", PW_Test_Param.Facter_value/1000,PW_Test_Param.Facter_value%1000);
			ui_teststr_darw(&font,&rect,buf);
	
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
				
				
			}
			ui_teststr_darw(&font,&rect,buf);
			}
		break;
		
		case TEST_TIME_COUNT_REFRESH_EVENT:     //测试时间事件
		{
			u32 time = (u32)value;
			
			struct rect_type rect={242,344,38,398};
// 			usSRegHoldBuf[0] = time;
			
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


