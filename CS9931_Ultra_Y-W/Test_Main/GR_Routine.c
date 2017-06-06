/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "GR_Routine.h"
#include "Test_Sched.h"
#include "memorymanagement.h"
#include "PLC.h"

#define  UPPER_LIMIT          (0)
#define  DOWN_LIMIT           (1)
#define  TEST_PASS            (2)
#define  OPEN_WIRE            (3)
#define  UPPER_LIMIT_V        (4)


static  struct step_gr_t *local_gr_test_para;
extern  char       mem_str_vol[],mem_str_cur[],mem_str_res[],mem_str_time[];

static  void GR_Test_Inerface(uint8_t type,void *value);

typedef struct{
	uint8_t   Test_Keep_Flag;                         //持续测试标志
	uint16_t  Wait_Time;                              //输出电压前的等待时间
	uint16_t  Test_Time;                              //测试时间
	uint16_t  Down_Time;                              //下降时间
	uint16_t  Pause_Time;                             //步间时间
	uint16_t  Current_Rise_Number;                    //电流上升的次数
	uint16_t  Current_Down_Number;                    //电流下降的次数
	uint32_t  Current_Rise_Interval;                  //电流上升的间隔
	uint32_t  Current_Down_Interval;                  //电流下降的间隔
	uint16_t  Current_Start_Value;                    //起始输出电流
	uint16_t  Current_Process_Value;                  //过程输出电流
	uint32_t  Current_Final_Value;                    //最终输出电流
	uint16_t  Current_Out_Freq;                       //输出电压频率
	uint16_t  Current_Wait_Count;                     //测试等待计数 
	uint16_t  Current_Rise_Count;                     //电压上升计数
	uint16_t  Testing_Count;                          //测试时间计数
	uint16_t  Current_Down_Count;                     //电压下降计数
	uint16_t  Pause_Count;                            //步间间隔计数
	uint16_t  res_value;
	uint8_t   warning_index;
	uint32_t  current_value;  
	uint32_t  Voltage_value; 
}TEST_PARAM;






static struct step_gr_t GR_Mode_Param,*p_GR_Mode_Param;
static TEST_PARAM GR_Test_Param;


static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}


static void GRModeTestEnvironmentEnter(struct step_gr_t *gr_test_para)
{
	GR_Mode_Param = *gr_test_para;
	p_GR_Mode_Param = gr_test_para;
		
	/*********************************************/
	//2016.11.26 wangxin
	update_gif_protect_function();
	/*********************************************/
	
	if(GR_Mode_Param.waittime < 1)GR_Test_Param.Wait_Time = 1;             //等待时间设置
 	else GR_Test_Param.Wait_Time = GR_Mode_Param.waittime;                 //确保至少有0.1s
	
	if(GR_Mode_Param.ramptime==0){
		GR_Test_Param.Current_Rise_Number = 1;
	}else{
		GR_Test_Param.Current_Rise_Number = GR_Mode_Param.ramptime;
	}
	
	GR_Test_Param.Current_Down_Number   = 1;
	
	GR_Test_Param.Test_Keep_Flag        = 0;
	GR_Test_Param.Test_Time             = GR_Mode_Param.testtime;
	GR_Test_Param.Pause_Time            = GR_Mode_Param.pausetime;
	
	if(GR_Mode_Param.testtime == 0)GR_Test_Param.Test_Keep_Flag = 1;
	
	GR_Test_Param.Current_Start_Value   = 0;
	
	GR_Test_Param.Current_Process_Value = 0;
	
	GR_Test_Param.Current_Final_Value   = GR_Mode_Param.outcur * 100;
	
	GR_Test_Param.Current_Out_Freq      = GR_Mode_Param.outfreq / 10;
	
	GR_Test_Param.Current_Rise_Count    = 0;
	
	GR_Test_Param.Testing_Count         = 0;
	
	GR_Test_Param.Current_Down_Count    = 0;
	
	GR_Test_Param.res_value             = 0;
	
	GR_Test_Param.Pause_Count           = 0;
	
	GR_Test_Param.warning_index         = 0;
	
	GR_Test_Param.Current_Rise_Interval = (GR_Test_Param.Current_Final_Value - GR_Test_Param.Current_Start_Value) / GR_Test_Param.Current_Rise_Number;

	GR_Test_Param.Current_Down_Interval = GR_Test_Param.Current_Final_Value / GR_Test_Param.Current_Down_Number;


	GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TEST_WAIT);
	
	Relay_ON(EXT_DRIVER_O7);
	Delay_ms(100);
	
	if(Test_Sched_Param.Stop_Flag == 0)GR_Output_Enable();  //使能GR输出
	
	{
		EXTI_InitTypeDef EXTI_InitStructure;

		/* Configure  EXTI  */
		EXTI_InitStructure.EXTI_Line = EXTI_Line7 | EXTI_Line10;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;

		EXTI_Init(&EXTI_InitStructure);
	}
	ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin

}


static void GRModeTestEnvironmentExit(void)
{
	Relay_OFF(EXT_DRIVER_O7);
 	GR_Set_Current(0,GR_Test_Param.Current_Out_Freq);
 	GR_Output_Disable();
}


static void GR_Range_Check(uint8_t type)
{
	switch(type){
		case UPPER_LIMIT:
			if(GR_Test_Param.res_value / 10 >= GR_Mode_Param.reshigh){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				GRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				GR_Test_Param.warning_index           = 1;
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(GR,HIGH,(void *)local_gr_test_para,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
			}
		break;
		
		
		case DOWN_LIMIT:
			if(GR_Mode_Param.reslow == 0)return;
			if(GR_Test_Param.res_value / 10 <= GR_Mode_Param.reslow){
				/*低于下限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				GRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				GR_Test_Param.warning_index           = 1;
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(GR,LOW,(void *)local_gr_test_para,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
			}
		break;
			
		case OPEN_WIRE:
			if((GR_Test_Param.current_value=GR_Get_Current()) <= 10){
				/*开路报警*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)OPEN_WIRE);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				GRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				GR_Test_Param.warning_index           = 1;
				Test_Sched_Param.Warning_Flag         = 1;
                GR_Test_Param.res_value = 0xffff;//开路报警标记值
				result_save(GR,OPEN,(void *)local_gr_test_para,GR_Test_Param.Voltage_value,
                    GR_Test_Param.current_value,0,GR_Test_Param.res_value,
                    GR_Test_Param.Testing_Count);
			}
		break;
			
		case UPPER_LIMIT_V:
			if(GR_Mode_Param.alarmvol == 0)return;
			if(GR_Test_Param.Voltage_value >= GR_Mode_Param.alarmvol * 10){
				/*开路报警*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				GR_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT_V);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				GRModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				GR_Test_Param.warning_index           = 1;
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(GR,HIGH,(void *)local_gr_test_para,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
			}
		default:
			
		break;
	}
}

void GR_Mode_Test(struct step_gr_t *gr_test_para)
{
    local_gr_test_para = gr_test_para;
	if(Test_Sched_Param.Stop_Flag){                            //Stop按键按下，处理
		GR_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
	}
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT){     //不在测试状态
		GRModeTestEnvironmentEnter(gr_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}
	switch(Test_Sched_Param.Test_Step_State){
				
		//等待测试状态
		case TEST_STEP_TEST_WAIT:
			if(--GR_Test_Param.Wait_Time == 0){
				if(GR_Test_Param.Current_Rise_Number == 1){           //如果上升时间为0
					Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
					GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
 					GR_Test_Param.Current_Process_Value = GR_Test_Param.Current_Final_Value;
					GR_Set_Current(GR_Test_Param.Current_Process_Value, GR_Test_Param.Current_Out_Freq);		
				}else{
					Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_RISE;
					GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_RISE);
 					GR_Set_Current(GR_Test_Param.Current_Start_Value,GR_Test_Param.Current_Out_Freq);
				}
				
			}
			GR_Test_Param.Current_Wait_Count++;
			GR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)GR_Test_Param.Current_Wait_Count);

		break;
			
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:
			
			GR_Test_Param.Voltage_value=GR_Get_Voltage();
			GR_Test_Param.current_value=GR_Get_Current();
		
			GR_Test_Param.res_value = GR_Get_RS();
			
			if(--GR_Test_Param.Current_Rise_Number == 0){   //电压到达最终目标电压
				Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;					
				GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
 				GR_Set_Current(GR_Test_Param.Current_Final_Value,GR_Test_Param.Current_Out_Freq);					
			}else{
				GR_Test_Param.Current_Process_Value += GR_Test_Param.Current_Rise_Interval;
 				GR_Set_Current(GR_Test_Param.Current_Process_Value,GR_Test_Param.Current_Out_Freq);	
			}
			
			
			if(Test_Sched_Param.Offset_Is_Flag){
				GR_Test_Param.res_value = GR_Test_Param.res_value>(*p_GR_Mode_Param).offsetvalue? (GR_Test_Param.res_value - (*p_GR_Mode_Param).offsetvalue) : 0;
			}
				
			/*判断上限*/
			
			

			GR_Test_Param.Current_Rise_Count++;             //上升时间加1
			
			GR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)GR_Test_Param.Current_Rise_Count);
			
			GR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(GR_Test_Param.Voltage_value));
			GR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(GR_Test_Param.current_value));
			
			if(GR_Test_Param.Current_Rise_Count>GR_Mode_Param.ramptime / 5){
				GR_Range_Check(OPEN_WIRE);
				if(GR_Test_Param.warning_index)break;
				GR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(GR_Test_Param.res_value));
				GR_Range_Check(UPPER_LIMIT);
			}
			GR_Range_Check(UPPER_LIMIT_V);
		break;
		//测试状态 
		case TEST_STEP_TESTING:
	
			GR_Test_Param.Voltage_value = GR_Get_Voltage();
			GR_Test_Param.current_value = GR_Get_Current();
			
			GR_Test_Param.res_value = GR_Get_RS();
			
			if(GR_Test_Param.Current_Final_Value < 4 * 1000) //单位mA
			{
				if(GR_Test_Param.res_value < 50 * 10
					&& GR_Test_Param.res_value > 1 * 10) //单位0.1mOhm
				{
					GR_Test_Param.res_value += 6 * 10;
				}
			}
			else if(GR_Test_Param.Current_Final_Value < 7 * 1000)
			{
				if(GR_Test_Param.res_value < 50 * 10
					&& GR_Test_Param.res_value > 1 * 10) //单位0.1mOhm
				{
					GR_Test_Param.res_value += 4 * 10;
				}
			}
			else if(GR_Test_Param.Current_Final_Value < 10 * 1000)
			{
				if(GR_Test_Param.res_value < 50 * 10
					&& GR_Test_Param.res_value > 1 * 10) //单位0.1mOhm
				{
					GR_Test_Param.res_value += 2 * 10;
				}
			}
			
			if(GR_Test_Param.Test_Keep_Flag)
			{              //测试时间为0，一直测试
				/*判断上限*/
				if(GR_Test_Param.Testing_Count >= 10000)GR_Test_Param.Testing_Count = 0;
			}
			else
			{
				if(--GR_Test_Param.Test_Time == 0)
				{
					if(GR_Test_Param.Pause_Time > 0 
						&& current_step_num-1 <= file_info[flash_info.current_file].totalstep)
					{                   //步间间隔不为0
						Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
						GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
					}
					//步间间隔为0，本次测试结束
					else
					{
						Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
					}
					
					GR_Set_Current(0, GR_Test_Param.Current_Out_Freq);
				}
			}
			
			GR_Test_Param.Testing_Count++;             //测试时间加1	
			
			GR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)GR_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}

			if(Test_Sched_Param.Offset_Is_Flag){
				GR_Test_Param.res_value = GR_Test_Param.res_value>(*p_GR_Mode_Param).offsetvalue? (GR_Test_Param.res_value - (*p_GR_Mode_Param).offsetvalue) : 0;
			}
				
 			
			
			GR_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(GR_Test_Param.Voltage_value));
			GR_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(GR_Test_Param.current_value));
			
			if(GR_Test_Param.Testing_Count>7){
				GR_Range_Check(OPEN_WIRE);
				if(GR_Test_Param.warning_index)break;
				GR_Test_Inerface(TEST_RESISTER_REFRESH_EVENT,(void *)(GR_Test_Param.res_value));
				GR_Range_Check(UPPER_LIMIT);
				GR_Range_Check(DOWN_LIMIT);
				GR_Range_Check(UPPER_LIMIT_V);
			}
			
			if(Test_Sched_Param.Offset_Get_Flag){
				(*p_GR_Mode_Param).offsetvalue = GR_Test_Param.res_value;
				
			}
			
		break;
		//电压下降状态
 		case TEST_STEP_VOL_DOWN:
 		

 		break;
		//步间等待
		case TEST_STEP_PAUSE:
			if(GR_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			
			GR_Range_Check(UPPER_LIMIT);
			GR_Range_Check(UPPER_LIMIT_V);
			GR_Test_Param.Pause_Count++;
			GR_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)GR_Test_Param.Pause_Count);
			if(--GR_Test_Param.Pause_Time == 0){
				Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				PLC_Pass_Out(0);
			}
		break;
		//测试停止
		case TEST_STEP_STOP:	
			
//			GR_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
			
			GRModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
			bsp_display(LED_TEST,0);
			if(GR_Test_Param.Test_Time == 0 && gr_test_para->testtime!=0)
			{
				dis_test_pass();
			}
			result_save(GR,PASS,(void *)gr_test_para,GR_Test_Param.Voltage_value,GR_Test_Param.current_value,0,GR_Test_Param.res_value,GR_Test_Param.Testing_Count);
			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}



extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);
static void GR_Test_Inerface(uint8_t type,void *value)
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
		{
			u32 temp = (u32)value / 10;
			struct rect_type rect={242,276,38,398};
			rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
			ui_teststr_darw(&font,&rect,buf);
		}
		break;
		
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		{
			u32 current = (u32)value/100;
			struct rect_type rect={242,140,38,398};
						
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
			struct rect_type rect={242,208,38,398};
			
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
                  {
                        u32 res = (u32)value;
                        struct rect_type rect={242,208,38,398};
                        
                        rt_sprintf(buf,">510mΩ");
                        
                        ui_teststr_darw(&font,&rect,buf);
                        
                        strcpy(mem_str_res,"                   ");
                        strncpy(mem_str_res,buf,strlen(buf));
                    }
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


