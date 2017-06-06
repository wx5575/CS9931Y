/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "DCW_Routine.h"

#include "PRJ_HEAD.h"
#include "GUI.h"
#include "WM.h"
#include "DIALOG.h"

#define  TEST_TIME_INTERVAL    (100)               //测试时间间隔ms

#define  DCW_STEP_TEST_WAIT   (1)                  //等待测试状态
#define  DCW_STEP_VOL_RISE    (2)                  //电压上升状态
#define  DCW_STEP_TESTING     (3)                  //测试状态          
#define  DCW_STEP_VOL_DOWN    (4)                  //电压下降状态
#define  DCW_STEP_PAUSE       (5)                  //步间等待

#define  DCW_STEP_STOP        (255)                //测试停止


#define  TEST_STATE_EVENT          (1<<0)
#define  TEST_VOLTAGE_EVENT        (1<<1)
#define  TEST_CURRENT_EVENT        (1<<2)
#define  TEST_RESISTER_EVENT       (1<<3)
#define  TEST_TIME_COUNT_EVENT     (1<<4)
#define  TEST_WARNING_EVENT        (1<<5)


extern WM_HMEM State_Text,Time_Edit,Vol_Edit;


void DCWModeTest(void);
void DCW_Test_Inerface(uint8_t type,void *value);

typedef struct{
	uint8_t   Stop_Flag;                              //Stop标志
	uint8_t   Test_Step;                              //测试步骤
	uint8_t   Test_Keep_Flag;                         //持续测试标志
	uint16_t  Wait_Time;                              //输出电压前的等待时间
	uint16_t  Test_Time;                              //测试时间
	uint16_t  Down_Time;                              //下降时间
	uint16_t  Voltage_Rise_Number;                    //电压上升的次数
	uint16_t  Voltage_Down_Number;                    //电压下降的次数
	uint32_t  Voltage_Rise_Interval;                  //电压上升的间隔
	uint32_t  Voltage_Down_Interval;                  //电压下降的间隔
	uint16_t  Voltage_Start_Value;                    //起始输出电压
	uint16_t  Voltage_Process_Value;                  //过程输出电压
	uint16_t  Voltage_Final_Value;                    //最终输出电压
	uint16_t  Voltage_Out_Freq;                       //输出电压频率
	uint16_t  Voltage_Rise_Count;                     //电压上升计数
	uint16_t  Testing_Count;                          //测试时间计数
	uint16_t  Voltage_Down_Count;                     //电压下降计数
	uint16_t  Voltage_Pause_Count;                    //步间间隔计数
}TEST_PARAM;






static struct step_dcw_t Dcw_Mode_Param;



static TEST_PARAM DCW_Test_Param;

// static void Stop_Flag_Set(void)
// {
// 	DCW_Test_Param.Stop_Flag = 1; 
// }

// static void Stop_Flag_Clr(void)
// {
// 	DCW_Test_Param.Stop_Flag = 0; 
// }

void DCWModeTestEnvironmentEnter(struct step_dcw_t *dcw_test_para)
{
	Dcw_Mode_Param = *dcw_test_para;
	
	DCW_Test_Param.Stop_Flag = 0;                                            //清除Stop标志
	
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
	if(Dcw_Mode_Param.testtime == 0)DCW_Test_Param.Test_Keep_Flag = 1;
	
	DCW_Test_Param.Voltage_Start_Value   = Dcw_Mode_Param.startvol;
	
	DCW_Test_Param.Voltage_Process_Value = Dcw_Mode_Param.startvol;
	
	DCW_Test_Param.Voltage_Final_Value   = Dcw_Mode_Param.outvol;
	
	DCW_Test_Param.Voltage_Out_Freq      = Dcw_Mode_Param.outfreq;
	
	DCW_Test_Param.Voltage_Rise_Count    = 0;
	
	DCW_Test_Param.Testing_Count         = 0;
	
	DCW_Test_Param.Voltage_Down_Count    = 0;
	
	DCW_Test_Param.Voltage_Pause_Count   = 0;
	
	DCW_Test_Param.Voltage_Rise_Interval = (Dcw_Mode_Param.outvol - Dcw_Mode_Param.startvol) / DCW_Test_Param.Voltage_Rise_Number;

	DCW_Test_Param.Voltage_Down_Interval = Dcw_Mode_Param.outvol / DCW_Test_Param.Voltage_Down_Number;

	DCW_Test_Param.Test_Step = DCW_STEP_TEST_WAIT;

	DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"Waiting");
	DC_Output_Enable();
	
	EDIT_SetDecMode  (Vol_Edit, 0, 0, 8000, 0, 0);	
	
	TASK_PRIO_REGISTER(DCWModeTest,USER_PRIO_1);
	TIMER_REGISTER(Timer3,USER_PRIO_1,100,TIMER_PERIODIC);
	TIMER_START(Timer3);

}


void DCWModeTestEnvironmentExit(void)
{
	DC_SetVoltage(0);
	DCW_Test_Param.Stop_Flag = 1;
}






void DCWModeTest(void)
{
	
	switch(DCW_Test_Param.Test_Step){
//等待测试状态
		case DCW_STEP_TEST_WAIT:
			
			if(DCW_Test_Param.Stop_Flag){                           //Stop按键按下，处理
				AC_Output_Disable();
				DCW_Test_Param.Test_Step = DCW_STEP_STOP;
				break;
			}
			if(--DCW_Test_Param.Wait_Time == 0){
				if(DCW_Test_Param.Voltage_Rise_Number == 1){           //如果上升时间为0
					DCW_Test_Param.Test_Step = DCW_STEP_TESTING;
					DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"Testing");
					DC_SetVoltage(DCW_Test_Param.Voltage_Final_Value);
				}else{
					DCW_Test_Param.Test_Step = DCW_STEP_VOL_RISE;
				
					DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"Vol_Ramp");
					DC_SetVoltage(DCW_Test_Param.Voltage_Start_Value);
				}
			}
		
		break;
//电压上升状态
		case DCW_STEP_VOL_RISE:
			if(DCW_Test_Param.Stop_Flag){                    //Stop按键按下，处理
				DC_Output_Disable();
				DCW_Test_Param.Test_Step = DCW_STEP_STOP;
				break;
			}
			if(--DCW_Test_Param.Voltage_Rise_Number == 0){   //电压到达最终目标电压
				DCW_Test_Param.Test_Step = DCW_STEP_TESTING;					
				DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"Testing");
				DC_SetVoltage(DCW_Test_Param.Voltage_Final_Value);					
			}else{
				DCW_Test_Param.Voltage_Process_Value += DCW_Test_Param.Voltage_Rise_Interval;
				DC_SetVoltage(DCW_Test_Param.Voltage_Process_Value);	
			}
			/*判断上限*/
			
			
			DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)AC_GetVoltage());
			DCW_Test_Param.Voltage_Rise_Count++;             //上升时间加1
			
			DCW_Test_Inerface(TEST_TIME_COUNT_EVENT,(void *)DCW_Test_Param.Voltage_Rise_Count);
			
		
		break;
//测试状态 
		case DCW_STEP_TESTING:
			if(DCW_Test_Param.Stop_Flag){                   //Stop按键按下，处理
				DC_Output_Disable();
				DCW_Test_Param.Test_Step = DCW_STEP_STOP;
				break;
			}
			
			if(DCW_Test_Param.Test_Keep_Flag){              //测试时间为0，一直测试
				/*判断上限*/
			}else{
				if(DCW_Test_Param.Test_Time-- == 0){
					if(DCW_Test_Param.Voltage_Down_Number == 1){           //如果上升时间为0
						DCW_Test_Param.Test_Step = DCW_STEP_PAUSE;
						
						DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"PAUSE");
						DC_SetVoltage(0);
					}else{
						DCW_Test_Param.Test_Step = DCW_STEP_VOL_DOWN;
						
						DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"Vol_Down");
						
					}
				}
			}
			DCW_Test_Param.Testing_Count++;             //测试时间加1
			
			DCW_Test_Inerface(TEST_TIME_COUNT_EVENT,(void *)DCW_Test_Param.Testing_Count);
			DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)AC_GetVoltage());	
			
			
		break;
//电压下降状态
		case DCW_STEP_VOL_DOWN:
			if(DCW_Test_Param.Stop_Flag){
				AC_Output_Disable();
				DCW_Test_Param.Test_Step = DCW_STEP_STOP;
				break;
			}
			if(--DCW_Test_Param.Voltage_Down_Number == 0){   //电压到达最终目标电压
				DCW_Test_Param.Test_Step = DCW_STEP_PAUSE;
			
				DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"PAUSE");
				
				DC_SetVoltage(0);
				DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)AC_GetVoltage());		
			}else{
				DCW_Test_Param.Voltage_Process_Value -= DCW_Test_Param.Voltage_Down_Interval;
				DC_SetVoltage(DCW_Test_Param.Voltage_Process_Value);
				DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)AC_GetVoltage());					
			}
			DCW_Test_Param.Voltage_Down_Count++;             //测试时间加1
			
			DCW_Test_Inerface(TEST_TIME_COUNT_EVENT,(void *)DCW_Test_Param.Voltage_Down_Count);
		break;
//步间等待
		case DCW_STEP_PAUSE:
			if(DCW_Test_Param.Stop_Flag){
				AC_Output_Disable();
				DCW_Test_Param.Test_Step = DCW_STEP_STOP;
				break;
			}
			DCW_Test_Param.Voltage_Pause_Count++;
			DCW_Test_Inerface(TEST_TIME_COUNT_EVENT,(void *)DCW_Test_Param.Voltage_Pause_Count);
			DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)AC_GetVoltage());
		break;
		//测试停止
		case DCW_STEP_STOP:
			
			DCW_Test_Inerface(TEST_STATE_EVENT,(void *)"STOP!");
			DCWModeTestEnvironmentExit();
			TIMER_CLOSE(Timer3);
			DCW_Test_Inerface(TEST_VOLTAGE_EVENT,(void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
		
		
		
	}
	
}



void DCW_Test_Inerface(uint8_t type,void *value)
{
	
	if(type & TEST_STATE_EVENT){           //测试状态事件
		
			TEXT_SetText(State_Text,(const char*)value);
			WM_Exec();
		
		
	}
	
	if(type & TEST_VOLTAGE_EVENT){         //测试电压事件
		
		
		EDIT_SetDecMode  (Vol_Edit, (int32_t)value, 0, 8000, 0, 0);
		WM_Exec();
		
	}
	
	if(type & TEST_CURRENT_EVENT){          //测试电流事件
		
		
		
		
	}
	
	if(type & TEST_RESISTER_EVENT){         //测试电阻事件
		
		
		
		
	}
	
	if(type & TEST_WARNING_EVENT){          //测试报警事件
		
		
		
		
	}
	
	if(type & TEST_TIME_COUNT_EVENT){       //测试时间事件
		
		EDIT_SetDecMode  (Time_Edit, (int32_t)value, 0, 9999, 1, 0);
		WM_Exec();
		
		
	}
	
	
	

}


