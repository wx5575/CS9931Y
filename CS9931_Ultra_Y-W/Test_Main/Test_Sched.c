#include  "Test_Sched.h"
#include  "ACW_Routine.h"
#include  "DCW_Routine.h"
#include  "GR_Routine.h"
#include  "LC_Routine.h"
#include  "CS99xx.h"
#include  "memorymanagement.h"
#include  "PLC.h"
#include  "sui_window.h"
#include "STM32_GPIO_Config.h"
#include "ext.h" 


/*全局变量设置*/

TEST_SCHED_PARAM    Test_Sched_Param;           //调度测试参数
static  rt_timer_t          Test_Sched_Timer;           //调度函数的定时器指针
rt_timer_t          Test_Sampling_Timer;
//static  rt_thread_t         Test_Sched_thread;          //测试线程

struct  rt_mailbox test_sever_mb;
/* 用于放邮件的内存池*/
static  char test_sever_mb_pool[4];

static  char test_loop_flag;

struct  rt_mailbox test_sever_clock_mb;

static  char test_sever_clock_mb_pool[4];


/*函数声明*/

void Test_Sched_Main(void *);                     //主测试函数
extern void ui_teststr_darw(struct font_info_t *font, struct rect_type *rect, char *str);
extern   uint8_t  Cal_Facter_Recover(void);
// extern   uint8_t mapanflag;


static void Test_Sched_Sever(void *parameter);
static void Test_Sched_Clock_Sever(void *parameter);
static void Test_Sampling_Clock_Sever(void *parameter);
struct rt_thread thread_Test_Sched_Sever;
static char thread_Test_Sched_Sever_stack[4096];

//总测试调度环境初始化函数
/*******************************
函数名：  Test_Sched_Environment_Init
参  数：  无

返回值：  无
********************************/
void Test_Sched_Environment_Init(void)
{
	//1、恢复各校准系数
	Cal_Facter_Recover();
	
	//2、初始化测试参数
	Test_Sched_Param.Test_Sched_State     = TEST_SCHED_STATE_INIT;          //测试调度器状态为初始化状态
	Test_Sched_Param.Test_Step_State      = TEST_STEP_OUT;                  //不在测试状态
	Test_Sched_Param.Stop_Flag            = 0;                              //清除STOP标志位
	Test_Sched_Param.Short_Flag           = 0;                              //清除Short_int标志位
	Test_Sched_Param.Pass_Flag            = 1;                              //置位测试合格标志位
	Test_Sched_Param.Warning_Flag         = 0;                              //清除Warning_Flag标志位
	Test_Sched_Param.Test_Status          = TEST_STATE_WAIT;                //测试状态为等待测试
	Test_Sched_Param.Offset_Get_Flag      = 0;                              //清除offset获取标志位
	Test_Sched_Param.Offset_Is_Flag       = 0;                              //清除offset使能标志位
	Test_Sched_Timer = rt_timer_create( "Test_Sched_Timer",                 /* 定时器名字*/
	                                    Test_Sched_Clock_Sever, /* 超时时回调的处理函数 */
	                                    RT_NULL, /* 超时函数的入口参数 */
	                                    TEST_SCHED_PERIODIC, /* 定时长度，以OS Tick为单位，即timeout_value个OS Tick */
	                                    RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
   
    #if LC_SAMPLING_DEBUG
    Test_Sampling_Timer = rt_timer_create( "Test_Sampling_Timer",/* 定时器名字*/
                                    Test_Sampling_Clock_Sever, /* 超时时回调的处理函数 */
                                    RT_NULL, /* 超时函数的入口参数 */
                                    TEST_SAMPLE_PERIODIC, /* 定时长度，以OS Tick为单位，即timeout_value个OS Tick */
                                    RT_TIMER_FLAG_PERIODIC); /* 周期性定时器 */
	#endif
    if (Test_Sched_Timer != RT_NULL)
	{
		/*错误处理*/
	}
	
	//创建测试服务线程
	rt_mb_init(&test_sever_clock_mb , "test_sever_clock_mb",  test_sever_clock_mb_pool , sizeof(test_sever_clock_mb_pool) / 4 , RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	
	//	Test_Sched_thread = rt_thread_create("test_sched_serve",
	//																		 Test_Sched_Sever, RT_NULL,
	//																		 4096, 6, 20);
	rt_thread_init(&thread_Test_Sched_Sever,
	               "Test_Sched_Sever",
	               Test_Sched_Sever,
	               RT_NULL,
	               &thread_Test_Sched_Sever_stack[0],
	               4096, 6, 20);
	rt_thread_startup(&thread_Test_Sched_Sever);
	
	//	if (Test_Sched_thread != RT_NULL)
	//			rt_thread_startup(Test_Sched_thread);
	
	/*防止功放第一次启动保护*/
	AC_SetVoltage(0, 60);
	DC_SetVoltage(0);
	AC_SetVoltage(1, 60);
	DC_SetVoltage(1);
	AC_SetVoltage(0, 60);
	DC_SetVoltage(0);
	
	//初始化所有的指示灯和PLC状态
	bsp_display(LED_PASS, 0);
	bsp_display(LED_FAIL, 0);
	bsp_display(LED_TEST, 0);
	
	PLC_Testing_Out(0);
	PLC_Pass_Out(0);
	PLC_Fail_Out(0);
	
	AC_Output_Disable();
	DC_Output_Disable();
	GR_Output_Disable();
	LC_Assit_Output_Disable();
	LC_Main_Output_Disable();
	
}

void Reset_LED_PLC(void)
{
	//初始化所有的指示灯和PLC状态
	bsp_display(LED_PASS, 0);
	bsp_display(LED_FAIL, 0);
	bsp_display(LED_TEST, 0);
	
	PLC_Testing_Out(0);
	PLC_Pass_Out(0);
	PLC_Fail_Out(0);
}

uint8_t Get_Test_Warning_State(void)
{
	return Test_Sched_Param.Warning_Flag;
}

typedef struct
{
	rt_uint16_t   routine_test_year;  //出厂检验年份
	rt_uint16_t   routine_test_month; //出厂检验月份
	rt_uint16_t   routine_test_day;   //出厂检验日期
	rt_uint16_t   start_count;        //开机计数
	rt_uint32_t   test_count;         //测试计数
	rt_uint32_t   single_runtime;     //当次运行时间
	rt_uint32_t   total_runtime;      //累计运行时间
} system_run_info;

#define  SYS_RUN_INF   (*((system_run_info *)0x40024000))
extern u8 usb_host_flag;
extern u8 bar_code_flag;
//测试调度开始
/*******************************
函数名：  Test_Sched_Start
参  数：  获取测试参数的回调指针

返回值：  无
********************************/
void Test_Sched_Start(CALLBACK_GET_TESTPARAM   *p_Get_Test_Param)
{
    if(current_step_num == 1 && bar_code_flag == 0)
    {
        test_flag.bar_code_sw = START_SCAN_BAR_CODE;
        update_bar_code(NULL, "");
        clear_scan_code_buf();
    }
    
	if(usb_host_flag == 1)//如果插入了扫码枪，则必须扫码后才能启动一次测试
	{
		if(bar_code_flag == 1) //已经扫码了
		{
            test_flag.bar_code_sw = STOP_SCAN_BAR_CODE;
            bar_code_flag = 0;
		}
		else
		{
			struct panel_type *win;
			struct rect_type rect = {190, 140, 200, 300};
			struct font_info_t font = {0, 0X4208, 0x53fa, 1, 1, 16};
			
			win = sui_window_create(T_STR("警告！！！","Warning! ! ! "), &rect);
			font.panel = win;
			rect.x = 0;
			rect.y = 80;
			rect.h = 8;
			rect.w = 300;
            
            if(language == 0)
            {
                ui_text_draw_alpha(&font, &rect, "已经插入扫码枪，测试前请扫码！");
			}
            else
            {
                ui_text_draw_alpha(&font, &rect, "Sweep gun inserted.");
                rect.y = 80 + 3 * rect.h;
                ui_text_draw_alpha(&font, &rect, "Sweep code before test.");
            }
			
			sui_window_update(win);
			return;
		}
	}
	
    
    
	// 	if(Test_Sched_Param.Pass_Flag == 0)                              return;
	if(Test_Sched_Param.Warning_Flag)                                return;
	
	if(Test_Sched_Param.Test_Sched_State == TEST_SCHED_STATE_RUNNING  &&
	        Test_Sched_Param.Pause_Flag       == 0)
		return;  //如果当前正在测试，直接退出
		
	Test_Sched_Param.Test_Sched_State     = TEST_SCHED_STATE_RUNNING;         //测试调度器状态为正在测试
	Test_Sched_Param.p_Get_Test_Param_Fun = p_Get_Test_Param;
	Test_Sched_Param.Pass_Flag            = 1;                                //置位测试合格标志位
	Test_Sched_Param.Stop_Flag            = 0;                              //清除STOP标志位
	Test_Sched_Param.Short_Flag           = 0;                              //清除Short_int标志位
	Test_Sched_Param.gfi_Flag             = 0;                              //清除arc_int标志位
	Test_Sched_Param.arc_Flag             = 0;                              //清除arc_int标志位
	Test_Sched_Param.arc_Isable           = 0;                              //清除arc_int使能
	Test_Sched_Param.Continue_Flag        = 0;                              //清除步间继续标志
	
	if(Test_Sched_Param.Pause_Flag)                                         //判断是不是由步间PAUSE后启动
	{
		Test_Sched_Param.Pause_Flag         = 0;                              //清除步间Pause标志
		Test_Sched_Param.Continue_Flag      = 1;                              //置位标志位，以便从下一步开始测试
	}
	
	else
	{
		Test_Sched_Param.Continue_Flag      = 0;
	}
	
    #if LC_SAMPLING_DEBUG
    rt_timer_start(Test_Sampling_Timer);/* 启动采样定时器 */
    #endif
	rt_timer_start(Test_Sched_Timer); /* 启动定时器 */
	test_loop_flag = 1;
	rt_mb_send(&test_sever_mb, 1);
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource15);
	// 	mapanflag = 2;
	bsp_display(LED_PASS, 0);
	ui_key_updata(0xfc);
	SYS_RUN_INF.test_count++;
}

//测试调度关闭
/*******************************
函数名：  Test_Sched_Close
参  数：  无

返回值：  无
********************************/
extern  void  Reset_Home (void);
void Test_Sched_Close(void)
{
    
	/* 紧急停止功放输出操作 */
	switch(Test_Sched_Param.p_Test_Param->com.mode)
	{
	//交流耐压
	case ACW:
	
		AC_Output_Disable();
		
		break;
		
	//直流耐压
	case DCW:
	
		DC_Output_Disable();
		
		break;
		
	//绝缘电阻
	case IR:
	
		break;
		
	//接地电阻
	case GR:
		GR_Output_Disable();
		break;
		
	//泄漏
	case LC:
		LC_Assit_Output_Disable();
		LC_Main_Output_Disable();
		break;
		
	//PW
	case PW:
		LC_Main_Output_Disable();
		break;
		
	//ACW_GR
	case ACW_GR:
		AC_Output_Disable();
		GR_Output_Disable();
		break;
		
	//DCW_GR
	case DCW_GR:
		DC_Output_Disable();
		GR_Output_Disable();
		break;
		
	default:
	
		break;
	}
	
	
	bsp_display(LED_PASS, 0);
	
	if(Test_Sched_Param.Warning_Flag == 0)ui_key_updata(0);                                  //恢复按键功能
	
	if(Test_Sched_Param.Stop_Flag)
	{
		bsp_display(LED_FAIL, 0);
		bsp_display(FMQ, 0);
		Test_Sched_Param.Warning_Flag         = 0;
		Test_Sched_Param.Short_Flag           = 0;
		Test_Sched_Param.arc_Flag             = 0;
		Test_Sched_Param.gfi_Flag             = 0;
		Reset_Home();
		
    test_flag.bar_code_sw = START_SCAN_BAR_CODE;
    update_bar_code(NULL, "");
    
		if(Test_Sched_Param.Test_Sched_State == TEST_SCHED_STATE_RUNNING)
		{
			char buf[20] = "";
			struct font_info_t font = {&panel_home, 0xffff, 0x0, 1, 1, 32};
			
			struct rect_type rect = {440, 72, 38, 200};
			strcpy(buf, T_STR("停止测试","STOP"));
			ui_teststr_darw(&font, &rect, buf);
			Test_Sched_Param.Test_Sched_State     = TEST_SCHED_STATE_STOP;         //测试调度器状态为停止测试
			Test_Sched_Param.Test_Status          = TEST_STATE_WAIT;
		}
	}
	
	else
	{
	
		Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
		Test_Sched_Param.Pass_Flag            = 0;
		
	}
	
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource15);
	
	//按stop返回主界面
	{
		rt_mb_send(&key_mb, KEY_EXIT | KEY_UP);
		rt_mb_send(&key_mb, KEY_EXIT | KEY_UP);
		rt_mb_send(&key_mb, KEY_EXIT | KEY_DOWN);
		rt_mb_send(&key_mb, KEY_EXIT | KEY_DOWN);
		panel_flag = 0;
	}
	//	ui_key_updata(0);
}


/*短路中断*/
void short_int(void)
{



	if(Test_Sched_Param.Stop_Flag == 1)return;
	
	if(Test_Sched_Param.Test_Sched_State != TEST_SCHED_STATE_RUNNING)return;
	
	Test_Sched_Close();
	
	bsp_display(LED_TEST, 0);
	bsp_display(LED_FAIL, 1);
	bsp_display(FMQ, 1);
	
	
	// 	Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
	Test_Sched_Param.Short_Flag           = 1;
	Test_Sched_Param.Pass_Flag            = 0;
	
	Test_Sched_Param.Test_Status = TEST_STATE_SHORT;
}


/*ARC中断*/
void arc_int(void)
{



	if(Test_Sched_Param.arc_Isable == 0)return;
	
	if(Test_Sched_Param.Stop_Flag == 1)return;
	
	if(Test_Sched_Param.Test_Sched_State != TEST_SCHED_STATE_RUNNING)return;
	
	Test_Sched_Close();
	
	bsp_display(LED_TEST, 0);
	bsp_display(LED_FAIL, 1);
	bsp_display(FMQ, 1);
	
	
	// 	Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
	Test_Sched_Param.arc_Flag             = 1;
	Test_Sched_Param.Pass_Flag            = 0;
	
	Test_Sched_Param.Test_Status = TEST_STATE_ARC_ALARM;
}

void gfi_cycle_clear_count(void)
{
    static uint32_t c = 0;
    
    /* 80不报警 90报警 设为100 */
    if(++c > GFI_CLEAR_COUNT_CYCLE)
    {
        test_flag.gfi_delay_count = 0;
        c = 0;
    }
}

/*GFI中断*/
void GFI_int(void)
{
	if(0 == test_flag.gfi_en)
	{
		return;
	}
	
    if(++test_flag.gfi_delay_count < GFI_DELAY_COUNT_MAX)
    {
        return;
    }



	if(Test_Sched_Param.Stop_Flag == 1)return;
	
	if(Test_Sched_Param.Test_Sched_State != TEST_SCHED_STATE_RUNNING)return;
	
	Test_Sched_Close();
	
	bsp_display(LED_TEST, 0);
	bsp_display(LED_FAIL, 1);
	bsp_display(FMQ, 1);
	
	
	// 	Test_Sched_Param.Stop_Flag            = 1;                             //置位STOP标志位
	Test_Sched_Param.gfi_Flag             = 1;
	Test_Sched_Param.Pass_Flag            = 0;
	
	Test_Sched_Param.Test_Status = TEST_STATE_GFI_ALARM;
}

extern void buzzer_test(u8 timer);

extern void ACW_GR_Mode_Test(struct step_acw_gr_t *acw_gr_test_para);
extern void DCW_GR_Mode_Test(struct step_dcw_gr_t *dcw_gr_test_para);
extern void PW_Mode_Test(struct step_pw_t *pw_test_para);
extern void IR_Mode_Test(struct step_ir_t *IR_test_para);

//总测试调度函数
/*******************************
函数名：  Test_Sched_Main
参  数：  无

返回值：  无
********************************/
void Test_Sched_Main(void *p_Param)
{
	uint8_t Is_Fail_Continue = 0;
	// 	static  uint8_t   delay_count = 0;
	// 	uint8_t test_step_state_bk;
	p_Param = p_Param;
	
	
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT)  //不在测试状态
	{
	
		if(current_step_num > 1 && Test_Sched_Param.Continue_Flag == 0)
		{
			switch(Test_Sched_Param.p_Test_Param->com.mode)
			{
			//交流耐压
			case ACW:
			
				if(Test_Sched_Param.p_Test_Param->acw.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->acw.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//直流耐压
			case DCW:
			
				if(Test_Sched_Param.p_Test_Param->dcw.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->dcw.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//绝缘电阻
			case IR:
			
				break;
				
			//接地电阻
			case GR:
			
				if(Test_Sched_Param.p_Test_Param->gr.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->gr.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//泄漏
			case LC:
			
				if(Test_Sched_Param.p_Test_Param->lc.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->lc.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//PW
			case PW:
			
				if(Test_Sched_Param.p_Test_Param->pw.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->pw.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//ACW_GR
			case ACW_GR:
			
				if(Test_Sched_Param.p_Test_Param->acw_gr.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->acw_gr.failstop)Is_Fail_Continue = 1;
				
				break;
				
			//DCW_GR
			case DCW_GR:
			
				if(Test_Sched_Param.p_Test_Param->dcw_gr.stepcontinuation)
				{
				
				}
				else
				{
					Test_Sched_Param.Pause_Flag = 1;
				}
				
				if(Test_Sched_Param.p_Test_Param->dcw_gr.failstop)Is_Fail_Continue = 1;
				
				break;
				
			default:
			
				break;
			}
			
			
		}
		
		
		
		if(Test_Sched_Param.Pause_Flag && Test_Sched_Param.Continue_Flag == 0 &&
		        current_step_num <= file_info[flash_info.current_file].totalstep   &&
		        Test_Sched_Param.Stop_Flag == 0
		  )
		{
			rt_timer_stop(Test_Sched_Timer);
			test_loop_flag = 0;
			Test_Sched_Param.Stop_Flag = 1;
			return;
		}
		
		Test_Sched_Param.Continue_Flag = 0;
		
		
		
		if(Test_Sched_Param.Warning_Flag)
		{
			if(Is_Fail_Continue)
			{
				Test_Sched_Param.Stop_Flag    = 0;
				Test_Sched_Param.Warning_Flag = 0;
				bsp_display(LED_FAIL, 0);
				bsp_display(FMQ, 0);
				Test_Sched_Param.Short_Flag   = 0;
				Test_Sched_Param.arc_Flag     = 0;
				Test_Sched_Param.gfi_Flag     = 0;
			}
		}
		
		else
		{
			if(Test_Sched_Param.Stop_Flag)rt_timer_stop(Test_Sched_Timer);
			
			if(Test_Sched_Param.Stop_Flag)test_loop_flag = 0;
		}
		
		//取下一步参数的指针
		if(Test_Sched_Param.p_Get_Test_Param_Fun != NULL)
			Test_Sched_Param.p_Test_Param = Test_Sched_Param.p_Get_Test_Param_Fun();
			
		if(Test_Sched_Param.p_Test_Param == RT_NULL)
		{
		
			/*测试完成*/
			if(Test_Sched_Param.Pass_Flag)                     //测试合格
			{
				buzzer_test(file_info[flash_info.current_file].buzzertime * 10);   //开启蜂鸣器
				bsp_display(LED_PASS, 1);                        //开启PASS灯
				bsp_display(LED_FAIL, 0);                        //关闭Fail灯
				AC_Output_Disable();                             //关闭AC输出
				DC_Output_Disable();                             //关闭DC输出
				GR_Output_Disable();                             //关闭GR输出
				Test_Sched_Param.Stop_Flag    = 1;               //将停止标志置位，停止测试
				Test_Sched_Param.Test_Status = TEST_STATE_PASS;  //测试合格
				
				if(Test_Sched_Param.Offset_Get_Flag)             //如果是开启了偏移，记录偏移值
				{
					Test_Sched_Param.Offset_Get_Flag = 0;          //清除记录offset的标致位
					save_steps_to_flash(flash_info.current_file);  //保存所有步的offset值
				}
				
			}
			
			else 																						  //测试不合格
			{
				if(Test_Sched_Param.Stop_Flag == 0)              //判断是否由外部操作终止
				{
					bsp_display(LED_FAIL, 1);                      //开启Fail灯
					bsp_display(FMQ, 1);                           //蜂鸣器响
					Test_Sched_Param.Stop_Flag    = 1;             //将停止标志置位，停止测试
					Test_Sched_Param.Warning_Flag = 1;             //测试错误标志置位
				}
			}
			
			bsp_display(LED_TEST, 0);//关闭测试指示灯
			
			Test_Sched_Param.Test_Sched_State = TEST_SCHED_STATE_STOP;         //测试调度器状态为停止测试
			
        #if LC_SAMPLING_DEBUG
            rt_timer_stop(Test_Sampling_Timer);//停止采样定时器
        #endif
			rt_timer_stop(Test_Sched_Timer);                   //停止测试用定时器
			test_loop_flag = 0;                                //退出测试循环，让出CPU资源
			
			SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource15);   //让出中断给拨盘
		}
		
		
	}
	
	switch(Test_Sched_Param.p_Test_Param->com.mode)
	{
	//交流耐压
	case ACW:
	
		ACW_Mode_Test(&Test_Sched_Param.p_Test_Param->acw);
		
		break;
		
	//直流耐压
	case DCW:
	
		DCW_Mode_Test(&Test_Sched_Param.p_Test_Param->dcw);
		
		break;
		
	//绝缘电阻
	case IR:
		IR_Mode_Test(&Test_Sched_Param.p_Test_Param->ir);
		break;
		
	//接地电阻
	case GR:
	
		GR_Mode_Test(&Test_Sched_Param.p_Test_Param->gr);
		
		break;
		
	//泄漏
	case LC:
	
		LC_Mode_Test(&Test_Sched_Param.p_Test_Param->lc);
		
		break;
		
	//PW
	case PW:
	
		PW_Mode_Test(&Test_Sched_Param.p_Test_Param->pw);
		
		break;
		
	//ACW_GR
	case ACW_GR:
	
		ACW_GR_Mode_Test(&Test_Sched_Param.p_Test_Param->acw_gr);
		
		break;
		
	//DCW_GR
	case DCW_GR:
	
		DCW_GR_Mode_Test(&Test_Sched_Param.p_Test_Param->dcw_gr);
		
		break;
		
	default:
	
		break;
	}
	
	
	
	if(Test_Sched_Param.Test_Step_State == TEST_STEP_OUT)
	{
		/*测试停止*/
		
	}
	
	
	
	
	
}



static void Test_Sched_Sever(void *parameter)
{
	u8 msg;
	rt_mb_init(&test_sever_mb , "test_sever_mb",  test_sever_mb_pool , sizeof(test_sever_mb_pool) / 4 , RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	
	while(1)
	{
		if (rt_mb_recv(&test_sever_mb, (rt_uint32_t *)&msg, RT_TICK_PER_SECOND / 10) == RT_EOK)
		{
			while(test_loop_flag)
			{
				if (rt_mb_recv(&test_sever_clock_mb, (rt_uint32_t *)&msg, RT_TICK_PER_SECOND / 100) == RT_EOK)
					Test_Sched_Main(NULL);
					
				/*此处添加GFI的检测*/
				{
					static uint8_t gfi_count = 0;
					
					if(GPIO_ReadInputDataBit(GPIOI, GPIO_Pin_8) == 0)
					{
						gfi_count++;
					}
					
					else
					{
						gfi_count = 0;
					}
					
					if(gfi_count > 2)GFI_int();
				}
			}
		}
	}
	
	
}

static void Test_Sched_Clock_Sever(void *parameter)
{
	rt_mb_send(&test_sever_clock_mb, 1);
}

uint16_t cur_sample_buf[2][1000];
// uint16_t cur_max;
// uint16_t cur_min;
// uint16_t vol_max;
// uint16_t vol_min;
uint16_t cur_ave;
uint16_t vol_ave;
volatile int sampling_mutex_flag;//采样使用的互斥量

void count_max_min_value(uint16_t *buf, uint16_t n, uint16_t *max, uint16_t *min)
{
    int i = 0;
    uint16_t t_max;
    uint16_t t_min;
    
    t_max = buf[0];
    t_min = buf[0];
    
    for(i = 0; i < n; i++)
    {
        if(t_max < buf[i])
        {
            t_max = buf[i];
        }
        
        if(t_min > buf[i])
        {
            t_min = buf[i];
        }
    }
    
    *max = t_max;
    *min = t_min;
}



/********************************************************************/
#define DEEP_SLOW		500
#define DEEP_FAST		5
#define DEEP_MID		80
#define SLITHER_DEEP    DEEP_SLOW
#define SLITHER_DEEP_	DEEP_SLOW
typedef struct{
    uint16_t max;
    uint16_t min;
    uint32_t deep;
    uint32_t index;
    uint64_t sum;
    uint32_t c;
    uint16_t buf[SLITHER_DEEP_];/* 滤波缓冲 */
}SAMBLE_INFO;

SAMBLE_INFO lc_sample_buf[2];
#define     LC_FS_CUR  0
#define     LC_FS_VOL  1


void clear_samble_buf(SAMBLE_INFO *inf)
{
    memset(((uint8_t*)inf) + 8 , 0, sizeof(SAMBLE_INFO) - 8);
}

uint16_t ave_sample(SAMBLE_INFO *info, uint16_t value)
{
    uint16_t res = 0;
	uint16_t deep = info->deep;
    
    info->index = (info->index + 1) % deep;
    
    info->sum -= info->buf[info->index];
    
    info->buf[info->index] = value;
    
    info->sum += info->buf[info->index];
    
    if(info->c < deep)
    {
        info->c++;
        count_max_min_value(&info->buf[1], info->c, &info->max, &info->min);
    }
    else
    {
        info->c = deep;
        count_max_min_value(info->buf, info->c, &info->max, &info->min);
    }
    
    if(value <= 10)
    {
        if(info->index > 0)
        {
            if(info->buf[info->index - 1] <= 10)
            {
                info->max = 0;
            }
        }
        else
        {
            if(info->buf[deep - 1] <= 10)
            {
                info->max = 0;
            }
        }
    }
    
    if(info->index > 3)
    {
        #define CHANGE_GREAD    3
        uint16_t t1 = info->buf[info->index - 2];
        uint16_t t2 = info->buf[info->index - 1] + CHANGE_GREAD;
        uint16_t t3 = info->buf[info->index - 1];
        uint16_t t4 = info->buf[info->index] + CHANGE_GREAD;
        uint16_t t5 = info->buf[info->index - 2] + CHANGE_GREAD;
        uint16_t t6 = info->buf[info->index - 1];
        uint16_t t7 = info->buf[info->index - 1] + CHANGE_GREAD;
        uint16_t t8 = info->buf[info->index];
        
        if(t1 > t2 || t3 > t4)
        {
            info->max = info->min = value;
            clear_samble_buf(info);
        }
        else if(t5 < t6 || t7 < t8)
        {
            info->max = info->min = value;
            clear_samble_buf(info);
        }
    }
    
    if(value + 50 < info->max)
    {
        info->max = value;
        clear_samble_buf(info);
    }
    
    if(info->min + 50 < info->max)
    {
        info->min = info->max;
    }
    
    res = info->sum / info->c;
    
    return res;
}

static void Test_Sampling_Clock_Sever(void *parameter)
{
    static int i = 0;
    static int size = 100;
    uint16_t t_cur;
    uint16_t t_vol;
    
    lc_sample_buf[LC_FS_CUR].deep = SLITHER_DEEP;
    lc_sample_buf[LC_FS_VOL].deep = 20;
    i = (i + 1) % size;
    
    t_cur = D3_Mcp3202_Read(0);
    
    if(0xffff == t_cur)
    {
        return;
    }
//     t_vol = D16_Mcp3202_Read(0);
    ave_sample(&lc_sample_buf[LC_FS_CUR], t_cur);
//     vol_ave = ave_sample(&lc_sample_buf[LC_FS_VOL], t_vol);
//     cur_sample_buf[1][i] = D16_Mcp3202_Read(0);
    
//     count_max_min_value(cur_sample_buf[0], size, &cur_max, &cur_min);
//     count_max_min_value(cur_sample_buf[1], size, &vol_max, &vol_min);
    
    cur_ave = (lc_sample_buf[LC_FS_CUR].max + lc_sample_buf[LC_FS_CUR].min) / 2;
//     vol_ave = (lc_sample_buf[LC_FS_VOL].max + lc_sample_buf[LC_FS_VOL].min) / 2;
}





