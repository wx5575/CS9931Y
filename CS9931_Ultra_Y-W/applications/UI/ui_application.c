#include "CS99xx.h"
#include "CS9931_Config.h"

rt_thread_t com_tid;


extern u8 selfcheck;
extern u8 usb_host_flag;
extern u8 update_barcode_st;
struct panel_type panel_title	    ={(u16 *)ExternSramTitleAddr,30,800,0,0};
struct panel_type panel_title_back  ={(u16 *)ExternSramTitleBkAddr,30,800,0,0};
struct panel_type panel_menu	    ={(u16 *)ExternSramMenuAddr,24,800,0,30};
struct panel_type panel_key		    ={(u16 *)ExternSramKeyAddr,402,120,680,54};
struct panel_type panel_home	    ={(u16 *)ExternSramHomeAddr,402,680,0,54};
struct panel_type panel_homeback    ={(u16 *)ExternSramNullAddr,402,680,0,54};
struct panel_type panel_status	    ={(u16 *)ExternSramStatusAddr,24,800,0,456};

/* 邮箱控制块*/
struct rt_mailbox screen_mb;
/* 用于放邮件的内存池*/
static char screen_mb_pool[128];

void clr_mem(u16 *p,u16 d,u32 size)
{
	u16 *pt = p;
	while(size--)
		*pt++ = d;
}

void clr_win(struct panel_type *p, u16 c, u16 x, u16 y, u16 h, u16 w)
{
	u16 *sc,i;
	
	for(i = 0;i < h; i++)
	{
		sc = p->data + p->w * (i + y) + x;//计算行地址
		clr_mem((u16 *)sc, c, w);
	}
}

typedef struct{
	rt_uint16_t   routine_test_year;  //出厂检验年份
	rt_uint16_t   routine_test_month; //出厂检验月份
	rt_uint16_t   routine_test_day;   //出厂检验日期
	rt_uint16_t   start_count;        //开机计数
	rt_uint32_t   test_count;         //测试计数
	rt_uint32_t   single_runtime;     //当次运行时间
	rt_uint32_t   total_runtime;      //累计运行时间
}system_run_info; 

#define  SYS_RUN_INF   (*((system_run_info *)0x40024000))

void clear_bar_code_dis_pos(void)
{
    struct rect_type rect = {500,6,22,300};
    struct font_info_t font = {&panel_title,0xffff,0x0,1,0,16};
    
    ui_title_darw(&font,&rect, "");
}
uint8_t barcode_buf[100];
void update_bar_code(struct font_info_t *font_info, uint8_t *barcode)
{
    struct rect_type rect = {500,6,22,300};
    static struct font_info_t font;
    uint8_t buf[2][10] = {"条形码:","BARCODE:"};
    
    if(font_info != NULL)
    {
        memcpy(&font, font_info, sizeof(font));
    }
    if(!usb_host_flag)
    {
        return;
    }
    barcode_buf[0] = 0;
    strcat(barcode_buf, buf[language]);
    strcat(barcode_buf, barcode);
    
    ui_title_darw(&font,&rect, barcode_buf);
}
static void application_entry(void *parameter)
{
	u8	err;
	rt_uint32_t msg;
	while(selfcheck);

	SYS_RUN_INF.start_count++;
	SYS_RUN_INF.single_runtime = 0;
#ifdef CS9931YS_PT_2K
    memset((u8 *)ExternSramAddr, 0xff, 480*800*2);
#else
	err = loadbmpbintosram("/resource/caiye.bin",(u16 *)ExternSramAddr);
#endif
	
	{
		extern struct rt_mailbox startup_mb;
		rt_mb_send(&startup_mb, err);
	
	}
    
	while(startup_flag == 0);
	
// 	err = 
	if(loadbmptosram("/resource/title/1.bmp",(u16 *)ExternSramTitleAddr) != 0)
		clr_mem((u16 *)ExternSramTitleAddr,0xf800,ExternSramTitleSize/2);
	if(loadbmptosram("/resource/title/1.bmp",(u16 *)ExternSramTitleBkAddr) != 0)
		clr_mem((u16 *)ExternSramTitleBkAddr,0xf800,ExternSramTitleBkSize/2);
    
// 	err = 
	if(loadbmptosram("/resource/menu/1.bmp",(u16 *)ExternSramMenuAddr) != 0)
		clr_mem((u16 *)ExternSramMenuAddr,0xffff,ExternSramMenuSize/2);
// // 	err = loadbmpbintosram("/resource/home/1.bin",(u16 *)ExternSramHomeAddr);
// 	err = 
	if(loadbmptosram("/resource/home/1.bmp",(u16 *)ExternSramHomeAddr) != 0)
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
//     panel_update(panel_home);
// 	err = 
	if(loadbmptosram("/resource/key/up.bmp",(u16 *)ExternSramKeyUpAddr) != 0)
		clr_mem((u16 *)ExternSramKeyUpAddr,0x4bf7,ExternSramKeySize/2);
// 	err = 
	if(loadbmptosram("/resource/key/down.bmp",(u16 *)ExternSramKeyDownAddr) != 0)
		clr_mem((u16 *)ExternSramKeyDownAddr,0xabe8,ExternSramKeySize/2);
// 	err = 
	if(loadbmptosram("/resource/status/1.bmp",(u16 *)ExternSramStatusAddr) != 0)
		clr_mem((u16 *)ExternSramStatusAddr,0x07e0,ExternSramStatusSize/2);
	{
	#ifdef CS9931YS_PT_2K
		struct rect_type rect = {5,6,22,600};
	#else
		struct rect_type rect = {30,6,22,600};
	#endif
		struct font_info_t font={&panel_title,0xffff,0x0,1,0,16};								
		ui_text_draw_alpha(&font,&rect,CS9931_Config.Decive_Name[language]);
        
		update_bar_code(&font, "");
	}
	
	rt_memcpy((void *)ExternSramNullAddr,(void *)ExternSramHomeAddr,ExternSramHomeSize);
// 	ui_key_updata(0);
	
	/* 初始化一个mailbox */
	rt_mb_init(&screen_mb,"screen_mb", screen_mb_pool, sizeof(screen_mb_pool)/4, RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	rt_mb_send(&screen_mb, UPDATE_HOME);
    rt_mb_send(&screen_mb, UPDATE_TITLE);
// 	while(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_8) == 0);
	
	
	while(1)
	{
        if(update_barcode_st)
        {
            update_barcode_st = 0;
//             rt_thread_delay( 1000 );
            rt_enter_critical();
            if(usb_host_flag)
            {
                update_bar_code(NULL, "");
            }
            else
            {
                clear_bar_code_dis_pos();
            }
            rt_exit_critical();
        }
        
		if (rt_mb_recv(&screen_mb, (rt_uint32_t*)&msg, RT_WAITING_FOREVER) == RT_EOK)
		{
			switch(msg & 0xf0000000)
			{
				case UPDATE_TITLE:
					panel_update(panel_title);
					break;
				case UPDATE_MENU:
				{
					struct rect_type rect = {0,0,24,800};
					struct font_info_t font={&panel_menu,0X8010,0XF79E,1,0,16};
						panel_update(panel_menu);
						ui_text_draw(&font,&rect,T_STR("功能测试(F1)  文件参数(F2)  步骤参数(F3)  系统参数(F4)  测试结果(F5)  帮助(F6)",
									"FunctionTest(F1)  FilePara.(F2)  StepPara.(F3)  System(F4)  TestResult(F5) Help(F6)"));
						}
					break;
				case UPDATE_KEY:
					panel_update(panel_key);
					break;
				case UPDATE_HOME:
					panel_update(panel_home);
					break;
				case UPDATE_STATUS:
					switch(msg &0x0f000000)
					{
						case STATUS_KEYLOCK_EVENT:
						{
							unsigned char en = msg & 0x000000ff;
							struct rect_type rect = {32,1,22,110};
							struct font_info_t font={&panel_status,0X8010,0XF79E,1,1,16};
							if(en != 0)
							{
								font.fontcolor = 0XFFE0;
								font.backcolor = 0xf800;
									ui_text_draw(&font,&rect,T_STR("键盘锁:已锁 ","Key:Lock   "));
							}
							else
							{
									ui_text_draw(&font,&rect,T_STR("键盘锁:未锁 ","Key:UnLock "));
							}
						}
							break;
						case STATUS_OFFSET_EVENT:
						{
							unsigned char en = msg & 0x000000ff;
							struct rect_type rect = {144,1,22,95};
							struct font_info_t font={&panel_status,0X8010,0XF79E,1,1,16};
							if(en != 0)
							{
								font.backcolor = 0XFFE0;
								ui_text_draw(&font,&rect,T_STR("偏移:开启 ","Offset:ON"));
							}
							else
							{
								ui_text_draw(&font,&rect,T_STR("偏移:关闭 ","Offset:OFF"));
							}
						}
							break;
						case STATUS_CONTENT_EVENT:
						{
							unsigned char en = msg & 0x000000ff;
							char buf[10];
							struct rect_type rect = {241,1,22,93};
							struct font_info_t font={&panel_status,0X8010,0XF79E,1,1,16};
							
							if(en == 0)
							{
								font.fontcolor = 0XFFE0;
								font.backcolor = 0xf800;
									ui_text_draw(&font,&rect,T_STR("容量:不足 ","Volume:UnEnough "));
							}
							else if(en >= 100)
							{
								ui_text_draw(&font,&rect,T_STR("容量:充足 ","Volume:Enough   "));
							}
							else
							{
								rt_sprintf(buf,T_STR("容量:% 2d%  ","Volume:% 2d%  "),en);
								ui_text_draw(&font,&rect,buf);
							}
						}
							break;
						case STATUS_STATUS_EVENT:
						{
							unsigned char en = msg & 0x000000ff;
							struct rect_type rect = {336,1,22,93};
							struct font_info_t font={&panel_status,0X8010,0XF79E,1,1,16};
							
							if(en != 0)
							{
								font.fontcolor = 0x07e0;
								font.backcolor = 0xf800;
									ui_text_draw(&font,&rect,T_STR("状态:远控 ","State:Far"));
							}
							else
							{
									ui_text_draw(&font,&rect,T_STR("状态:本控 ","State:Local"));
							}
						}
							break;
						case STATUS_INTERFACE_EVENT:
						{
							unsigned char en = msg & 0x000000ff;
							struct rect_type rect = {431,1,22,105};
							struct font_info_t font={&panel_status,0X8010,0XF79E,1,1,16};
							switch(en)
							{
								case 0:
										ui_text_draw(&font,&rect,T_STR("接口:RS232 ","Port:RS232 "));
									break;
								case 1:
										ui_text_draw(&font,&rect,T_STR("接口:RS485 ","Port:RS485 "));
									break;
								case 2:
										ui_text_draw(&font,&rect,T_STR("接口:USB   ","Port:USB   "));
									break;
								default:
										ui_text_draw(&font,&rect,T_STR("接口:无    ","Port:NO    "));
									break;
							}
						}
							break;
						case STATUS_TIME_EVENT:
						if(startup_flag!=0)
						{
							rt_device_t device;
							struct rtc_time_type	time;
							struct rect_type rect = {640,2,20,160};
							struct font_info_t font={&panel_status,0x0,0XF79E,0,0,16};
							char time_buf[20];
							
							device = rt_device_find("rtc");
							if(device != RT_NULL)
							{
								rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &time);						
								rt_sprintf(time_buf,"%04d-%02d-%02d %02d:%02d:%02d",time.year+2000,time.month,time.date,time.hours,time.minutes,time.seconds);
								ui_text_draw(&font,&rect,time_buf);
							}
							else
								ui_text_draw(&font,&rect,"2014-10-10 12:00:00");
							
							SYS_RUN_INF.single_runtime++;
							SYS_RUN_INF.total_runtime++;
						}
							break;
						default:
							panel_update(panel_status);
					}
					break;
				default:
					rt_thread_delay( RT_TICK_PER_SECOND/5 ); /* sleep 0.2 second and switch to other thread */
					break;
			}
		}
	}
}


void application_init(void)
{
    static rt_bool_t inited = RT_FALSE;

    if (inited == RT_FALSE) /* 避免重复初始化而做的保护 */
    {
		extern void ui_title_init(void);
		extern void ui_menu_init(void);
		extern void ui_home_init(void);
		extern void ui_status_init(void);
		
// 		extern void ui_file_init(void);
		rt_thread_t tid;

        tid = rt_thread_create("wb",
                               application_entry, RT_NULL,
                               2048 * 2, 11, 10);

        if (tid != RT_NULL)
            rt_thread_startup(tid);

// 		ui_title_init();
// 		ui_menu_init();
// 		ui_status_init();
		
		ui_home_init();
		
// 		ui_file_init();
		
        inited = RT_TRUE;
    }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void application()
{
    application_init();
}
/* finsh的命令输出，可以直接执行application()函数以执行上面的函数 */
FINSH_FUNCTION_EXPORT(application, application ui)
#endif
