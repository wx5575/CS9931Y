/*包含的头文件*/
#include "driver.h"
#include <stdint.h>
#include "stm32f4xx.h"
#include "LC_Routine.h"
#include "Test_Sched.h"
#include "memorymanagement.h"
#include "PLC.h"




/********************************************************************/

static struct step_lc_t *local_lc_test_para;

SLITHER_AVE_INFO lc_fs[2];
SLITHER_AVE_INFO acw_fs[2];


void clear_slither_data(void)
{
    memset(&lc_fs, 0, sizeof(lc_fs));
    memset(&acw_fs, 0, sizeof(acw_fs));
}

uint16_t slither_f(SLITHER_AVE_INFO *info, uint16_t value)
{
    uint16_t res = 0;
	uint16_t deep = SLITHER_DEEP;
    
    if(info->index != 0)
    {
        if(info->buf[info->index] > value)
        {
            if(info->buf[info->index] - value > 50)
            {
                clear_slither_data();
            }
        }
        else
        {
            if(value - info->buf[info->index] > 50)
            {
                clear_slither_data();
            }
        }
    }
    
    info->index = (info->index + 1) % deep;
    
    info->sum -= info->buf[info->index];
    
    info->buf[info->index] = value;
    
    info->sum += info->buf[info->index];
    
    if(info->c < deep)
    {
        info->c++;
    }
    else
    {
        info->c = deep;
    }
    
    res = info->sum / info->c;
    
    return res;
}
/********************************************************************/
















#define  UPPER_LIMIT          (0)
#define  DOWN_LIMIT           (1)
#define  TEST_PASS            (2)
#define  SELF_V_HIGH          (3)
#define  VOL_ERROR            (6)
#define  SELF_PW_HIGH		  (7)
#define  MD1_C                (1)
#define  MD2_C                (2)
#define  MD3_C                (3)
#define  MD4_C                (4)

#define    LC_ASSIT_REFRESH_EVENT       (6)     //辅助电压事件
#define    NET_REFRESH_EVENT            (7)     //告警刷新事件

extern  char       mem_str_vol[],mem_str_cur[],mem_str_res[],mem_str_time[];


void LC_Test_Inerface(uint8_t type,void *value);

typedef struct{
	uint8_t   Test_Keep_Flag;                         //持续测试标志
	uint16_t  Wait_Time;                              //输出电压前的等待时间
	uint16_t  Test_Time;                              //测试时间
	uint16_t  Down_Time;                              //下降时间
	uint16_t  Pause_Time;                             //步间时间
	uint16_t  Voltage_Rise_Number;                    //电流上升的次数
	uint16_t  Voltage_Down_Number;                    //电流下降的次数
	uint32_t  Voltage_Rise_Interval;                  //电流上升的间隔
	uint32_t  Voltage_Down_Interval;                  //电流下降的间隔
	uint16_t  Voltage_Start_Value;                    //起始输出电流
	uint32_t  Voltage_Process_Value;                  //过程输出电流
	uint32_t  Voltage_Final_Value;                    //最终输出电流
	uint16_t  Voltage_Out_Freq;                       //输出电压频率
	uint16_t  Voltage_Wait_Count;                     //测试等待计数 
	uint16_t  Voltage_Rise_Count;                     //电压上升计数
	uint16_t  Testing_Count;                          //测试时间计数
	uint16_t  Current_Down_Count;                     //电压下降计数
	uint16_t  Pause_Count;                            //步间间隔计数
	uint16_t  current_value;
	uint32_t  main_voltage;
	uint32_t  assist_voltage;
	uint16_t  self_voltage;
	uint32_t  main_current; //主测试电流
	uint32_t  main_power;//主功率
	float real_cur;//读真实电流值
}TEST_PARAM;


static struct step_lc_t LC_Mode_Param,*p_LC_Mode_Param;
static TEST_PARAM LC_Test_Param;

#define   LC_K1         I_200uA_2mA_20mA
#define   LC_K2         GB9706_1
#define   LC_K3         LC_NY
#define   LC_K4         I_200uA_2mA_20mA
#define   LC_K5         ULN544NP
#define   LC_K6         ULN544P
#define   LC_K7         I_200uA_2mA_20mA
#define   LC_K11        GB4943_GBT12113
#define   LC_K12        FIG4_FIG5
#define   LC_K13        FIG3_FIG4
#define   LC_K14        I_200uA_2mA_20mA
#define   LC_K15        GBT12133
#define   LC_K16        I_200uA_2mA_20mA
#define   LC_K17        UL1563
#define   LC_K20        I_200uA_2mA_20mA
#define   LC_K25        I_FILES
#define   LC_K26        I_AC_DC_AC
#define   LC_K27        SELV_AC_DC_AC
#define   LC_K33        MD_HI_GND
#define   LC_K34        G_Change
#define   LC_K35        S3_CONTROL

/**************************************************************************************

				K34			K35			K3		K2		K5	  	K6	  	K15			K17			K11			K12			K13
MD-A 							     吸合  吸合  吸合   吸合    吸合   ------   断开    断开    断开
MD-B                   吸合  吸合  吸合   吸合    吸合   ------   断开    吸合    断开
MD-C                   吸合  吸合  吸合   吸合    吸合   ------   断开   ------   吸合
MD-D                   吸合  吸合  吸合   吸合    吸合   ------   吸合   ------   吸合
MD-E                   吸合  断开  吸合  	断开   ------  ------  ------  ------  ------ 
MD-F                   吸合  断开  断开   吸合   ------  ------  ------  ------   吸合
MD-G                   吸合  吸合 ------ ------  ------  ------  ------  ------  ------ 
MD-H									 吸合  吸合  吸合   吸合    吸合    吸合   ------  ------  ------ 

******************************************************************************/
static void Test_Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

void LCModeTestEnvironmentExit(void);

static void LCModeTestEnvironmentEnter(struct step_lc_t *lc_test_para)
{
 	LC_Mode_Param = *lc_test_para;
	p_LC_Mode_Param = lc_test_para;
	
	LC_Test_Param.Wait_Time = 3;                                      //确保至少有0.1s
	
	if(LC_Mode_Param.ramptime==0){
		LC_Test_Param.Voltage_Rise_Number = 1;
	}else{
		LC_Test_Param.Voltage_Rise_Number = LC_Mode_Param.ramptime;
	}
  
//	LC_Relay_Control(PEAK_DISCHARGE,1,1);
//	LC_Relay_Control(PEAK_DISCHARGE,0,1);
	
	LC_Test_Param.Voltage_Down_Number = 1;
	LC_Test_Param.Test_Keep_Flag        = 0;
	LC_Test_Param.Test_Time             = LC_Mode_Param.testtime;
	LC_Test_Param.Pause_Time            = LC_Mode_Param.pausetime;
	
	if(LC_Mode_Param.testtime == 0)LC_Test_Param.Test_Keep_Flag = 1;
	
	LC_Test_Param.Voltage_Start_Value   = 0;
	
	LC_Test_Param.Voltage_Process_Value = 0;
	
	LC_Test_Param.Voltage_Final_Value   = LC_Mode_Param.outvol * 100;
	
	LC_Test_Param.Voltage_Out_Freq      = LC_Mode_Param.outfreq / 10;
	
	LC_Test_Param.Voltage_Rise_Count    = 0;
	
	LC_Test_Param.Testing_Count         = 0;
	LC_Test_Param.current_value         = 0;
	
	LC_Test_Param.Pause_Count   = 0;
	
	LC_Test_Param.Voltage_Rise_Interval = (LC_Test_Param.Voltage_Final_Value - LC_Test_Param.Voltage_Start_Value) / LC_Test_Param.Voltage_Rise_Number;

// 	LC_Test_Param.Voltage_Down_Interval = LC_Mode_Param.outvol / LC_Test_Param.Voltage_Down_Number;


// 	LCModeTestEnvironmentExit();

// 	Test_Delay_ms(20);
//电流档位
	switch(LC_Mode_Param.curgear){
		case I3uA:
			
		break;
		case I30uA:
			
		break;
		case I300uA:
			LC_Relay_Control(I_200uA_2mA_20mA,1,0);
			LC_Relay_Control(LC_K25,0,1);
		break;
		case I3mA:
			LC_Relay_Control(I_200uA_2mA_20mA,1,0);
			LC_Relay_Control(LC_K25,1,1);
		break;
		case I30mA:
			LC_Relay_Control(I_200uA_2mA_20mA,0,0);
			LC_Relay_Control(LC_K25,1,1);
		break;
		case I100mA:
			
		break;
		default:
			
		break;
	}
//检波方式	
	switch(LC_Mode_Param.curdetection){
		case 0:    // AC
			LC_Relay_Control(LC_K26,0,1);
			LC_4051_D1_SELECT(1);
		
			LC_Relay_Control(LC_K27,0,1);
			LC_4051_D15_SELECT(1);	
		break;
		case 1:    // AC+DC
			LC_Relay_Control(LC_K26,1,1);
			LC_4051_D1_SELECT(1);
		
			LC_Relay_Control(LC_K27,1,1);
			LC_4051_D15_SELECT(1);
		break;
		case 2:    // PEAK
			LC_4051_D1_SELECT(0);
			LC_4051_D15_SELECT(0);
//			LC_Relay_Control(PEAK_DISCHARGE,1,0);
			LC_Relay_Control(SELV_PEAK_DISCHARGE,1,1);
		break;
		case 3:    // DC
			LC_4051_D1_SELECT(2);
			LC_4051_D15_SELECT(2);
		break;

		default:
			
		break;
	}

//网络选择
	switch(LC_Mode_Param.MDnetwork){
		
		case MD_A:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
			LC_Relay_Control(LC_K12,0,0);
			LC_Relay_Control(LC_K13,0,1);
		break;
		case MD_B:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,0,0);
		break;
		case MD_C:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,0,0);
// 			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,1,1);
		break;
		case MD_D:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
			LC_Relay_Control(LC_K13,1,1);
		break;
		case MD_E:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,0,0);
 			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,0,1);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_F:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,0,0);
			LC_Relay_Control(LC_K5,0,0);
 			LC_Relay_Control(LC_K6,1,1);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_G:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,0,0);
			LC_Relay_Control(LC_K6,0,0);
		  LC_Relay_Control(LC_K17,0,0);
		  LC_Relay_Control(GB7000_1,0,1);
// 			LC_Relay_Control(LC_K5,0,0);
// 			LC_Relay_Control(LC_K6,1,1);
// 			LC_Relay_Control(LC_K15,1,0);
// 			LC_Relay_Control(LC_K17,1,0);
// 			LC_Relay_Control(LC_K11,1,0);
// 			LC_Relay_Control(LC_K12,1,0);
// 			LC_Relay_Control(LC_K13,1,0);
		break;
		case MD_H:
			LC_Relay_Control(LC_K3,1,0);
			LC_Relay_Control(LC_K2,1,0);
			LC_Relay_Control(LC_K5,1,0);
			LC_Relay_Control(LC_K6,1,0);
			LC_Relay_Control(LC_K15,1,0);
			LC_Relay_Control(LC_K17,1,1);
// 			LC_Relay_Control(LC_K11,0,0);
// 			LC_Relay_Control(LC_K12,0,0);
// 			LC_Relay_Control(LC_K13,0,1);
		break;

		default:
			
		break;
	}
	
	LC_Relay_Control(LC_OUT1,0,0);
	LC_Relay_Control(LC_OUT2,0,0);
	LC_Relay_Control(LC_OUT3,0,0);
	LC_Relay_Control(LC_OUT4,0,0);
	switch(LC_Mode_Param.MDpostion){
		
		case MD1_C:
			LC_Relay_Control(LC_OUT2,1,0);
		break;
		case MD2_C:
			LC_Relay_Control(LC_OUT1,1,0);
		break;
		case MD3_C:
			LC_Relay_Control(LC_OUT4,1,0);
		break;
		case MD4_C:
			LC_Relay_Control(LC_OUT3,1,0);
		break;

		default:
			
		break;
	}
	
	if(LC_Mode_Param.SL){LC_Relay_Control(LC_K33,0,0);}else{LC_Relay_Control(LC_K33,1,0);}
	if(LC_Mode_Param.SH){
		LC_Relay_Control(LC_K3,0,0);
		LC_Relay_Control(LC_K34,1,0);
		LC_Relay_Control(LC_K35,1,0);
	}
	else
	{
		LC_Relay_Control(LC_K3,1,0);
		LC_Relay_Control(LC_K34,0,0);
		LC_Relay_Control(LC_K35,0,0);
	}
	if(LC_Mode_Param.S7){LC_Relay_Control(LC_OUT6,1,0);}else{LC_Relay_Control(LC_OUT6,0,0);}
	if(LC_Mode_Param.S8){LC_Relay_Control(LC_OUT5,1,0);}else{LC_Relay_Control(LC_OUT5,0,0);}
	if(LC_Mode_Param.S10){LC_Relay_Control(LC_OUT8,1,0);}else{LC_Relay_Control(LC_OUT8,0,0);}
	if(LC_Mode_Param.S11){LC_Relay_Control(LC_OUT7,1,0);}else{LC_Relay_Control(LC_OUT7,0,0);}
	if(LC_Mode_Param.S12){LC_Relay_Control(LC_OUT9,1,0);}else{LC_Relay_Control(LC_OUT9,0,0);}	
	if(LC_Mode_Param.S13){LC_Relay_Control(LC_OUT10,1,0);}else{LC_Relay_Control(LC_OUT10,0,0);}
	if(LC_Mode_Param.S14){LC_Relay_Control(LC_OUT11,1,1);}else{LC_Relay_Control(LC_OUT11,0,1);}	
	
	if(LC_Mode_Param.S9){LC_Relay_Control(AUX_VOL_Change_NL,RELAY_4094_ON,1);}else{LC_Relay_Control(AUX_VOL_Change_NL,RELAY_4094_OFF,1);}	
    
	Delay_ms(100);

	ctrl_signal_dault_relay(LC_Mode_Param.singlefault);

	if(LC_Mode_Param.NorLphase){
		LC_Relay_Control(M_VOL_Change_NL,1,1);
		Relay_OFF(EXT_DRIVER_O6);
	}else{
		LC_Relay_Control(M_VOL_Change_NL,0,1);
		Relay_ON(EXT_DRIVER_O6);
	}	
	Delay_ms(10);
	CPLD_GPIO_Control(OUT_C,1);
	
// 	LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TEST_WAIT);
 	
// 	Relay_ON(EXT_DRIVER_O8);
	
	Relay_ON(GFI_GND_SELECT);
//	Relay_ON(RET_GND_SELECT);
	Test_Delay_ms(20);
	
	if(Test_Sched_Param.Stop_Flag == 0)
	{
		LC_Main_Output_Enable();    //使能输出
		LC_Assit_Output_Enable();   //使能输出
		LC_Assit_Voltage_Set(LC_Mode_Param.assistvol * 100,LC_Test_Param.Voltage_Out_Freq);
	}
	
	{
		EXTI_InitTypeDef EXTI_InitStructure;
		
		/* Configure  EXTI  */
		EXTI_InitStructure.EXTI_Line = EXTI_Line7 | EXTI_Line10;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;
		
		EXTI_Init(&EXTI_InitStructure);
	}
}


void LCModeTestEnvironmentExit(void)
{
 	LC_Assit_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
	LC_Main_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
	LC_Assit_Output_Disable();
	LC_Main_Output_Disable();
// 	Relay_OFF(EXT_DRIVER_O8);
	Relay_OFF(GFI_GND_SELECT);
//	Relay_OFF(RET_GND_SELECT);
	LC_Relay_Control(LC_NY,0,1);
//	Relay_OFF(EXT_DRIVER_O8);	
//	Test_Delay_ms(10);
//	Relay_OFF(EXT_DRIVER_O5);
	 
	CPLD_GPIO_Control(OUT_C,0);
	
	LC_Relay_Control(LC_OUT1,0,0);
	LC_Relay_Control(LC_OUT2,0,0);
	LC_Relay_Control(LC_OUT3,0,0);
	LC_Relay_Control(LC_OUT4,0,0);
	LC_Relay_Control(LC_OUT5,0,0);
	LC_Relay_Control(LC_OUT6,0,0);
	LC_Relay_Control(LC_OUT7,0,0);
	LC_Relay_Control(LC_OUT8,0,0);
	LC_Relay_Control(LC_OUT9,0,0);
	LC_Relay_Control(PEAK_DISCHARGE,0,0);
	LC_Relay_Control(SELV_PEAK_DISCHARGE,0,1);
	
	LC_Relay_Control(M_VOL_Change_NL,0,1);
	Delay_ms(10);
	LC_Relay_Control(AUX_VOL_Change_NL,RELAY_4094_OFF,1);
	Relay_OFF(EXT_DRIVER_O6);
//	Test_Delay_ms(10);
}


void LC_Range_Check(uint8_t type)
{
	uint32_t current;
	switch(type){
		case UPPER_LIMIT:
		{
			switch(LC_Mode_Param.curgear){
				case I3uA:
					
				break;
				case I30uA:
					
				break;
				case I300uA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current>= LC_Mode_Param.curhigh){			
						/*超过上限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,HIGH,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I3mA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current >= LC_Mode_Param.curhigh){			
						/*超过上限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,HIGH,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I30mA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current >= LC_Mode_Param.curhigh){		
						/*超过上限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)UPPER_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,HIGH,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I100mA:
					
				break;
				default:
					
				break;
			}
			
		break;
		}
		case DOWN_LIMIT:
		{
			if(LC_Mode_Param.curlow == 0)return;
			switch(LC_Mode_Param.curgear){
				case I3uA:
					
				break;
				case I30uA:
					
				break;
				case I300uA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current <= LC_Mode_Param.curlow){	
						/*低于下限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,LOW,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I3mA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current <= LC_Mode_Param.curlow){	
						/*低于下限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,LOW,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I30mA:
					current = LC_Test_Param.current_value;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					if(current <= LC_Mode_Param.curlow){
						/*低于下限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,LOW,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				case I100mA:
					if(current <= LC_Mode_Param.curlow){		
						/*低于下限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)DOWN_LIMIT);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,LOW,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					}
				break;
				default:
					
				break;
			}
		}
		case SELF_V_HIGH:
		{
					if(LC_Mode_Param.MDvol == 0)return;
					if(LC_Test_Param.self_voltage >= LC_Mode_Param.MDvol*10){
						/*超过上限*/
						Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
						LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)SELF_V_HIGH);
						Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
						bsp_display(LED_TEST,0);
						LCModeTestEnvironmentExit();
						Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
						Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
						Test_Sched_Param.Warning_Flag         = 1;
						result_save(LC,HIGH,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					
					}
		break;
		
		
		case VOL_ERROR:    //判断电压是否异常
			if(LC_Test_Param.main_voltage  < (LC_Test_Param.Voltage_Final_Value / 2)){
				/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_VOL_ABNORMAL;
				LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)VOL_ERROR);
				LC_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(LC_Test_Param.main_voltage ));		  //将电压值刷新到屏幕
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				LCModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(LC,VOL_ABNORMAL,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
				}
			
		break;
			}
		case SELF_PW_HIGH://功率过载报警
		{
			uint32_t power = LC_Test_Param.main_power / 100 / 100 / 100;
			uint32_t current = LC_Test_Param.main_current / 100;
			//超过1.1kV 或 >5A
			if(power > 1100 || current > 50)
			{		/*超过上限*/
				Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				LC_Test_Inerface(TEST_WARNING_REFRESH_EVENT,(void *)SELF_PW_HIGH);
				Test_Sched_Param.Pass_Flag = 0;  //清除测试合格标志位
				bsp_display(LED_TEST,0);
				LCModeTestEnvironmentExit();
				Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
				Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
				Test_Sched_Param.Warning_Flag         = 1;
				result_save(LC,HIGH,(void *)local_lc_test_para,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
					
			}
			
			break;
		}
		default:
			
		break;
	}
}

void LC_Mode_Test(struct step_lc_t *lc_test_para)
{
    local_lc_test_para = lc_test_para;
    
	//Stop按键按下，处理
	if(Test_Sched_Param.Stop_Flag)
	{
		LC_Assit_Output_Disable();
		LC_Main_Output_Disable();
		Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
		LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
		Relay_OFF(EXT_DRIVER_O8);
	}
	
	//不在测试状态
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT)
	{
		LCModeTestEnvironmentEnter(lc_test_para);              //初始化
		Test_Sched_Param.Test_Step_State = TEST_STEP_TEST_WAIT;  //进入测试状态
		PLC_Testing_Out(1);
	}
	
	//等待测试状态
	switch(Test_Sched_Param.Test_Step_State)
	{
		case TEST_STEP_TEST_WAIT:
            clear_slither_data();
			if(--LC_Test_Param.Wait_Time == 0)
			{
				//如果上升时间为0
				if(LC_Test_Param.Voltage_Rise_Number == 1)
				{
					Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;
 					LC_Test_Inerface(TEST_STATE_REFRESH_EVENT, (void *)TEST_STEP_TESTING);
					LC_Test_Param.Voltage_Process_Value = LC_Test_Param.Voltage_Final_Value;
 					LC_Main_Voltage_Set(LC_Test_Param.Voltage_Process_Value,LC_Test_Param.Voltage_Out_Freq);
				}
				else
				{
					Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_RISE;
 					LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_VOL_RISE);
 					LC_Main_Voltage_Set(LC_Test_Param.Voltage_Start_Value,LC_Test_Param.Voltage_Out_Freq);
				}
			}
			
			Relay_ON(EXT_DRIVER_O8);
		break;
		
		//电压上升状态
		case TEST_STEP_VOL_RISE:
			get_lc_main_current(&LC_Test_Param.main_current);
			LC_Test_Param.main_voltage = LC_Get_Main_Voltage();
			LC_Test_Param.main_power = LC_Test_Param.main_current * LC_Test_Param.main_voltage;
			LC_Test_Param.current_value = LC_Get_Current(LC_Mode_Param.MDnetwork - MD_E,   \
										LC_Mode_Param.curdetection,       \
										LC_Mode_Param.curgear - I300uA);
			if(LC_Mode_Param.curdetection == 2)LC_Test_Param.current_value = LC_Test_Param.current_value * 1.414;
			
			//电压到达最终目标电压
			if(--LC_Test_Param.Voltage_Rise_Number == 0)
			{
				Test_Sched_Param.Test_Step_State = TEST_STEP_TESTING;					
 				LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_TESTING);
 				LC_Main_Voltage_Set(LC_Test_Param.Voltage_Final_Value,LC_Test_Param.Voltage_Out_Freq);					
			}
			else
			{
				LC_Test_Param.Voltage_Process_Value += LC_Test_Param.Voltage_Rise_Interval;
				LC_Main_Voltage_Set(LC_Test_Param.Voltage_Process_Value,LC_Test_Param.Voltage_Out_Freq);	
			}
			
			if(Test_Sched_Param.Offset_Is_Flag)
			{
				LC_Test_Param.current_value = LC_Test_Param.current_value>(*p_LC_Mode_Param).offsetvalue? (LC_Test_Param.current_value - (*p_LC_Mode_Param).offsetvalue) : 0;
			}
			/*判断上限*/
			
			LC_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(LC_Test_Param.main_voltage));
			LC_Test_Param.Voltage_Rise_Count++;             //上升时间加1
			if(LC_Test_Param.Voltage_Rise_Count > 2)
			{
				LC_Relay_Control(PEAK_DISCHARGE,1,1);
			}
			
			LC_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)LC_Test_Param.Voltage_Rise_Count);
			LC_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(LC_Test_Param.current_value));	
			LC_Range_Check(UPPER_LIMIT);
			LC_Range_Check(SELF_V_HIGH);
			LC_Range_Check(SELF_PW_HIGH);
		break;
		//测试状态
		case TEST_STEP_TESTING:
			//ReadRealCurrent(&LC_Test_Param.real_cur);
			get_lc_main_current(&LC_Test_Param.main_current);
            
			LC_Test_Param.main_voltage = LC_Get_Main_Voltage();
			LC_Test_Param.main_power = LC_Test_Param.main_current * LC_Test_Param.main_voltage;

		  
//             LC_Test_Param.current_value = slither_f(&lc_fs[LC_FS_CUR], LC_Test_Param.current_value);
            
    #if LC_SAMPLING_DEBUG
              LC_Test_Param.current_value =
                        count_lc_current(cur_ave,LC_Mode_Param.MDnetwork - MD_E,   \
                                                LC_Mode_Param.curdetection,       \
                                                LC_Mode_Param.curgear - I300uA);
    #else
            LC_Test_Param.current_value = LC_Get_Current(LC_Mode_Param.MDnetwork - MD_E,   \
                                        LC_Mode_Param.curdetection,       \
                                        LC_Mode_Param.curgear - I300uA);
    #endif
		  if(LC_Mode_Param.curdetection == 2)
		  {
			  LC_Test_Param.current_value = LC_Test_Param.current_value * 1.414 ;
		  }
			
			if(LC_Test_Param.Test_Keep_Flag)
			{              //测试时间为0，一直测试
				/*判断上限*/
				if(LC_Test_Param.Testing_Count >= 10000)LC_Test_Param.Testing_Count = 0;
			}
			else
			{
				/* 测试时间到 */
				if(--LC_Test_Param.Test_Time == 0)
				{
					LC_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)lc_test_para->testtime);//刷新显示时间
					
					if(LC_Test_Param.Voltage_Down_Number == 1)
					{
						//如果下降时间为0
						if(LC_Test_Param.Pause_Time > 0
							&& current_step_num-1 <= file_info[flash_info.current_file].totalstep)
						{
							//步间间隔不为0
							Test_Sched_Param.Test_Step_State = TEST_STEP_PAUSE;
							LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_PAUSE);
//							LC_Main_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
//							LC_Main_Output_Disable();
//							LC_Assit_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
//							LC_Main_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
//							LC_Assit_Output_Disable();
//							LC_Main_Output_Disable();
						}
						else
						{                                               //步间间隔为0，本次测试结束
							Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
						}
						
						LC_Range_Check(VOL_ERROR);
//						LC_Main_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
						LC_Assit_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
						LC_Main_Voltage_Set(0,LC_Test_Param.Voltage_Out_Freq);
						LC_Assit_Output_Disable();
						LC_Main_Output_Disable();
					}
					else
					{
						Test_Sched_Param.Test_Step_State = TEST_STEP_VOL_DOWN;
						LC_Test_Inerface(TEST_STATE_REFRESH_EVENT, (void *)TEST_STEP_VOL_DOWN);
					}
				}
			}
			
			if(Test_Sched_Param.Test_Step_State != TEST_STEP_TESTING)
			{
				break;
			}
			
			LC_Test_Param.Testing_Count++;             //测试时间加1
			
			LC_Test_Inerface(TEST_TIME_COUNT_REFRESH_EVENT,(void *)LC_Test_Param.Testing_Count);
			
			if(Test_Sched_Param.Offset_Is_Flag)
			{
				LC_Test_Param.current_value = LC_Test_Param.current_value>(*p_LC_Mode_Param).offsetvalue? (LC_Test_Param.current_value - (*p_LC_Mode_Param).offsetvalue) : 0;
			}
			
    #if LC_SAMPLING_DEBUG
 			LC_Test_Inerface(TEST_MAIN_POWER_REFRESH_EVENT,(void *)(LC_Test_Param.main_power));
 			LC_Test_Inerface(TEST_MAIN_CURRENT_REFRESH_EVENT,(void *)(LC_Test_Param.main_current ));
    #endif
            LC_Test_Inerface(TEST_VOLTAGE_REFRESH_EVENT,(void *)(LC_Test_Param.main_voltage ));
			LC_Test_Inerface(TEST_CURRENT_REFRESH_EVENT,(void *)(LC_Test_Param.current_value));
			
			LC_Range_Check(UPPER_LIMIT);
			LC_Range_Check(SELF_V_HIGH);
			LC_Range_Check(SELF_PW_HIGH);
			
			if(LC_Test_Param.Testing_Count > 4)LC_Range_Check(DOWN_LIMIT);
			if(LC_Test_Param.Testing_Count > 2)LC_Relay_Control(PEAK_DISCHARGE,1,1);
			
			if(Test_Sched_Param.Offset_Get_Flag)
			{
				(*p_LC_Mode_Param).offsetvalue = LC_Test_Param.current_value;
			}
			
		break;
		//电压下降状态
		case TEST_STEP_VOL_DOWN:

		break;
		//步间等待
		case TEST_STEP_PAUSE:
			if(LC_Mode_Param.steppass)
			{
				PLC_Pass_Out(1);
				PLC_Testing_Out(0);
			}
			LC_Relay_Control(M_VOL_Change_NL,0,1);
            Delay_ms(10);
			Relay_OFF(EXT_DRIVER_O6);
			LC_Range_Check(UPPER_LIMIT);
			LC_Test_Param.Pause_Count++;
			
			if(--LC_Test_Param.Pause_Time == 0)
			{
				Test_Sched_Param.Test_Step_State = TEST_STEP_STOP;
				PLC_Pass_Out(0);
			}
		break;
		//测试停止
		case TEST_STEP_STOP:
			bsp_display(LED_TEST,0);
			
//			LC_Test_Inerface(TEST_STATE_REFRESH_EVENT,(void *)TEST_STEP_STOP);
			LCModeTestEnvironmentExit();
			Test_Sched_Param.Test_Step_State = TEST_STEP_OUT;//不在测试状态
			if(LC_Test_Param.Test_Time == 0 && lc_test_para->testtime!=0)
			{
				dis_test_pass();
			}
			else
			{
			}
			result_save(LC,PASS,(void *)&LC_Mode_Param,LC_Test_Param.main_voltage/100,LC_Test_Param.current_value,0,0,LC_Test_Param.Testing_Count);
			Test_Sched_Main((void *)0);
		break;
		//其它状态
		default:
			//程序不应跳到此处
		break;
	}
	
}


extern void ui_teststr_darw(struct font_info_t *font,struct rect_type *rect,char *str);
void LC_Test_Inerface(uint8_t type,void *value)
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
		
    #if LC_SAMPLING_DEBUG
		case TEST_MAIN_POWER_REFRESH_EVENT: //测试主功率事件
		{
			struct rect_type rect={242,140,38,150};
			u32 power = (u32)value/100/100/10;
			
			if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)
			{
				return;
			}
			//font={&panel_home,0xffff,0x0,1,1,32};
			font.center = 0;
			rt_sprintf(buf,"%d.%dW", power/10, power%10);
			ui_teststr_darw(&font,&rect,buf);
			break;
		}
		case TEST_MAIN_CURRENT_REFRESH_EVENT: //测试主电流事件
		{
			struct rect_type rect={400 + 120,140,38,100};
			u32 current = (u32)value;
			
			if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)
			{
				return;
			}
			//font={&panel_home,0xffff,0x0,1,1,32};
			font.center = 0;
			rt_sprintf(buf,"%d.%dA", current/1000, current%1000);
			ui_teststr_darw(&font,&rect,buf);
			break;
		}
        #endif
		case TEST_VOLTAGE_REFRESH_EVENT:        //测试电压事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		{
			u32 voltage = (u32)value/100;
			struct rect_type rect={242+150,140,38,398-300};
// 			struct rect_type rect={362,140,38,100};
			
			rt_sprintf(buf,"%d.%dV", voltage/10,voltage%10);
			ui_teststr_darw(&font,&rect,buf);
			strcpy(mem_str_vol,"                   ");
			strncpy(mem_str_vol,buf,strlen(buf));
		}
		
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)break;
		
		// ********************		SELV 电压刷新
		{
// 			u32 voltage = (u32)((float)D3_Mcp3202_Read(1) / 281 * 879);
			u32 voltage = LC_Get_Selv_Voltage();
			struct rect_type rect={242,276,38,199};
			LC_Test_Param.self_voltage = voltage;
			if(LC_Mode_Param.curdetection == 1){
				rt_sprintf(buf,"%d.%02dV", voltage/100,voltage%100);
			}else{
				rt_sprintf(buf,"OFF");
			}	
			
			ui_teststr_darw(&font,&rect,buf);
		}
		// ******************* END ******************
		
		
		
		
		// *******************   辅助 电压刷新
		{
			u32 voltage = LC_Test_Param.assist_voltage = (u32)LC_Get_Assist_Voltage()/100;
			struct rect_type rect={440,276,38,199};
			
			rt_sprintf(buf,"%d.%dV", voltage/10,voltage%10);
			ui_teststr_darw(&font,&rect,buf);
		}
		// ******************* END ******************
		
		
		
		
// 		bsp_display(LED_TEST,2);
		break;
		
		case TEST_CURRENT_REFRESH_EVENT:        //测试电流事件
		if(Test_Sched_Param.Test_Step_State == TEST_STEP_STOP)return;
		strcpy(mem_str_cur,"                   ");
		{
			u32 temp = (u32)value;
			u32 current;
			struct rect_type rect={242,208,38,398};
			
			
			switch(LC_Mode_Param.curgear){
				case I3uA:
					
				break;
				case I30uA:
					
				break;
				case I300uA:
					current = temp ;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					rt_sprintf(buf,"%d.%01duA", current/10,current%10);
				break;
				case I3mA:
					current = temp;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					rt_sprintf(buf,"%d.%03dmA", current/1000,current%1000);
				break;
				case I30mA:
					current = temp ;
//					if(LC_Mode_Param.curdetection == 2)current = current * 1.414;
					rt_sprintf(buf,"%d.%02dmA", current/100,current%100);
				break;
				case I100mA:
					
				break;
				default:
					
				break;
			}
			strncpy(mem_str_cur,buf,strlen(buf));
			switch(LC_Mode_Param.curdetection)
			{
				case 0:    // AC
					strcat(buf," (AC)");
				break;
				case 1:    // AC+DC
					strcat(buf," (RMS)");
				break;
				case 2:    // PEAK
					strcat(buf," (PEAK)");
				break;
				case 3:    // DC
					strcat(buf," (DC)");
				break;
                    
				default:
					
				break;
			}
			
			clear_home_panel(&rect);
			display_LC_L_N(0, 0, NULL);///<刷新相位信息
			lc_ui_teststr_darw(&font,&rect,buf);
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
					Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				break;
				case DOWN_LIMIT:
					strcpy(buf,T_STR("下限报警","Low Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
					Test_Sched_Param.Test_Status          = TEST_STATE_LOW;
				break;
				case SELF_V_HIGH:
					strcpy(buf,T_STR("电压报警","Vol.Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
					Test_Sched_Param.Test_Status          = TEST_STATE_HIGH;
				break;
				case VOL_ERROR:
					strcpy(buf,T_STR("电压报警","Vol.Fail"));
					bsp_display(LED_FAIL,1);
					bsp_display(FMQ,1);
				break;
				case SELF_PW_HIGH://功率过载报警
					Test_Sched_Param.Test_Status = TEST_STATE_OVERLOAD;
					strcpy(buf,T_STR("过载报警","PW.Fail"));
					bsp_display(FMQ,1);
					bsp_display(LED_FAIL,1);
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
		
		if(Test_Sched_Param.Stop_Flag == 0)
		{
			bsp_display(LED_TEST,2);
		}
		break;
		
		default:
			
		break;
	}
	
}


