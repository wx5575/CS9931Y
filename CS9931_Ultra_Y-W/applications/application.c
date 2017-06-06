/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <stdio.h>

#include "stm32f4xx.h"
#include <board.h>
#include <rtthread.h>
#include "CS9931_Config.h"
#include "stm32f4xx_pwr.h"
#ifdef RT_USING_DFS
/* 包含DFS 的头文件*/
#include <dfs_fs.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32_eth.h"
#endif

#include "FILE_SYS.H"
// #include "mb.h"
#include "HAL.h"
extern uint8_t CH376_Interrupte(void);

static void  modbus_server(void* parameter);




void rt_init_thread_entry(void* parameter)
{
// 	rt_thread_delay( RT_TICK_PER_SECOND/2 ); //等待LCD 硬件复位
// #ifdef RT_USING_COMPONENTS_INIT
//     /* initialization RT-Thread Components */
//     rt_components_init();
// 	rtgui_system_server_init();
// #endif
	
    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        /* register ethernetif device */
        eth_system_device_init();

        rt_hw_stm32_eth_init();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif
	
//KEY
	{
		extern void rt_hw_key_init(void);
		rt_hw_key_init();
	}

	
//GUI
	{
		extern void application_init(void);
		application_init();
	}
	
//RTC
	{
		extern void rt_hw_rtc_init(void);
		rt_hw_rtc_init();
	}
//BKPSRAM	
	{
		/* Enable the Backup SRAM low power Regulator to retain it's content in VBAT mode */
		PWR_BackupRegulatorCmd(ENABLE);
		/* Wait until the Backup SRAM low power Regulator is ready */
		while(PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET)
		{
		}
	}
	
	cs99xx_init();
}


ALIGN(RT_ALIGN_SIZE)
u8 usb_host_flag=0;
u8 update_barcode_st=0;
static char thread_led_stack[256];
struct rt_thread thread_led;
extern u8 USB_DEVICE_CHECK(void);
extern u8 usb_mem_flag;
extern uint8_t Get_Scan_Data(void);
// u8 scan_buf[64];
static void rt_thread_entry_led(void* parameter)
{
	u8 s;
	rt_thread_delay( RT_TICK_PER_SECOND/2 );
	
 	while(1)
	{
// 		bsp_display(LED_PASS,1);
// 		bsp_display(LED_KEY1,1);
// 		bsp_display(LED_KEY2,0);
// 		rt_thread_delay( RT_TICK_PER_SECOND/5 ); /* sleep 0.5 second and switch to other thread */
//		bsp_display(LED_PASS | LED_FAIL | LED_TEST,2);
// 		bsp_display(LED_KEY1,0);
// 		bsp_display(LED_KEY2,1);
		
		
// 		rt_enter_critical();
		if( usb_mem_flag ==0)
		{
			USB_Device_Chg(USB_2);
			if(usb_host_flag == 0)
			{
				s = Wait376Interrupts();
				if(s==21)
				{
					s = CH376_Interrupte();
					if(s==0)
					{
						buzzer(20);
						usb_host_flag = 1;
                        update_barcode_st = 1;
					}
				}
				rt_thread_delay( RT_TICK_PER_SECOND /10); /* sleep 0.5 second and switch to other thread */
			}
			else
			{
				if(panel_flag == 0)
				{
					Get_Scan_Data();
					rt_thread_delay( RT_TICK_PER_SECOND /100);
				}
				s = USB_DEVICE_CHECK();
				if(s==0x16)
				{
					usb_host_flag = 0;
					rt_thread_delay( RT_TICK_PER_SECOND /10);
					buzzer(10);
                    update_barcode_st = 1;
				}
			}
		}
		else
		{
			rt_thread_delay( RT_TICK_PER_SECOND);
		}
// 		rt_exit_critical();
	}
}



u8 selfcheck=1;
extern const char *self_test_item_name(unsigned char index);

extern const char *self_test_item_result(unsigned char index);

#include "sui_window.h"
#include "logo.h"
#include "name.h"
#include "http.h"
#include "model.h"
#include "bsp_ico.h"
/* 邮箱控制块*/
struct rt_mailbox startup_mb;
struct rt_mailbox modbus_mb;
/* 用于放邮件的内存池*/
static char startup_mb_pool[4];
static char modbus_mb_pool[4];
extern u16 bmp_buf[800];

static void self_test_warnning(const char *p)
{
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0x53fa,1,1,16};
	u16    warning_size = 0;
	const  char *p_temp = p;
	for(;*p_temp != 0;){
		if(*p_temp < 0x80){
			warning_size += font.high / 2;
			p_temp++;
		}else{
			warning_size += font.high;
			p_temp++;
			p_temp++;
		}
	}
	
	rect.x = 190;
	rect.y = 140;
	rect.h = 200;
	rect.w = 300;
	
	win = sui_window_create(T_STR("自检信息","Self-Checking Info "),&rect);
	font.panel = win;
	font_draw((rect.w - warning_size)/2,80,&font,p);
	sui_window_update(win);
}

static void rt_startup_thread_entry(void* parameter)
{
	u8 msg;
	u16 i,j,x=270,*p;
	const char *pp;
	struct panel_type panel	={(u16 *)ExternSramAddr+480*800*2,480,800,0,0};
	struct font_info_t font = {0,0xffff,0x1f,1,1,16};
	char buf[100];
	
	font.panel = &panel;
//void clr_win(struct panel_type *p,u16 c,u16 x,u16 y,u16 h,u16 w)
	clr_win(&panel,0x0000,0,0,480,800);
//void ico_color_set(rt_uint8_t alpha,rt_uint16_t color,rt_uint16_t bcolor)
	ico_color_set(0,0xf800,0xffff);
//void ico_darw(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data)
#ifdef RT_USING_DFS
	{
		extern int dfs_init(void);
		extern void rt_hw_sdcard_init(void);
		
		rt_hw_sdcard_init();
		/* 初始化设备文件系统*/
		dfs_init();
#ifdef RT_USING_DFS_ELMFAT
		/* 如果使用的是ELM 的FAT 文件系统，需要对它进行初始化*/
		elm_init();
		/* 调用dfs_mount 函数对设备进行装载*/
		if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
			rt_kprintf("File System initialized!\n");
		else
			rt_kprintf("File System init failed!\n");
#endif
	}
#endif
	memory_systems_open();
	
	//读取配置文件
	{
			int fd = 0;
			
//		  rt_enter_critical();

			fd = open("//Config.bin", O_RDONLY, 0);
		
//			if(fd != 0){
		
				read(fd,(uint8_t *)&CS9931_Config,sizeof(CS9931_CONFIG_STRUCT));
				
				close(fd);
				
//			}
//			rt_exit_critical();
	}
	
#ifdef CS9931YS_PT_2K
	ico_copy_bm(&panel,20,20,(u8 *)elitech_logo_bm);
	rt_sprintf(buf,T_STR("型号:%s","Model:%s"), CS9931_Config.Decive_Name[language]);
	font_draw(250,40,&font,buf);
    
	font_draw(250,70,&font,T_STR("版本:"TEST_SOFT_VERISON,"Ver:"TEST_SOFT_VERISON));
	font_draw(500,450,&font,T_STR("PT.SINKO PRIMA ALLOY","PT.SINKO PRIMA ALLOY"));
#else
	ico_darw(&panel,10,10,(u8 *)gImage_logo);
	rt_sprintf(buf,T_STR("型号:%s","Model:%s"), CS9931_Config.Decive_Name[language]);
	font_draw(100,40,&font,buf);
	font_draw(100,70,&font,T_STR("版本:V1.00.08","Ver: V1.00.08"));
	font_draw(400,70,&font,T_STR("网址:http://www.csallwin.com","Web: http://www.csallwin.com"));
	font_draw(500,450,&font,T_STR("南京长盛仪器有限公司","NANJING CHANGSHENG INSTRUMENT CO.,LTD"));
#endif
	
	panel_update(panel);
	
	for(pp=self_test_item_name(0),i=0;pp!=0;pp=self_test_item_name(++i))
	{
		font_draw(10,100+i*30,&font,pp);
		if(language==0)
		{
		font_draw(10+rt_strlen(pp)*8+10,100+i*30,&font,"··································");
		}
		else
		{
		font_draw(10+rt_strlen(pp)*8+10,100+i*30,&font,"·····························");
		}
		pp = self_test_item_result(i);
		if(pp == 0)
		{
			font_draw(700,100+i*30,&font,"[OK.]");
			panel_update(panel);
		}
		else
		{
//			font_draw(20,450,&font,pp);
			font_draw(700,100+i*30,&font,"[NG.]");
			panel_update(panel);
			self_test_warnning(pp);
			while(1)
			{
				if((pp = self_test_item_result(i)) == 0){
					font_draw(700,100+i*30,&font,"[OK.]");
					font_draw(20,450,&font,"                                       ");
					panel_update(panel);
					break;
				}else{
//					clr_win(&panel,0x0000,20,450,20,300);
//					font_draw(20,450,&font,pp);
//					panel_update(panel);
				}
			}
		}
	}
	
	//rt_thread_delay( RT_TICK_PER_SECOND*3 );
	clr_win(&panel,0x0000,0,0,480,800);
	panel_update(panel);
	selfcheck = 0;
#ifdef CS9931YS_PT_2K
    draw_elitech_custom_logo(290,130);
#else
 	RA8875_DrawICO(360,130,0xf800,0xffff,gImage_logo);
#endif
//	RA8875_DrawICO(285,230,0xffff,0,gImage_name);
//	RA8875_DrawICO(25,25,0xffff,0,gImage_model);
// 	RA8875_DrawICO(550,450,0xffff,0,gImage_http);
	
	for(i=0;i<300;i++)
		bmp_buf[i] = bmp_buf[i+300] = 0xffff;
	for(i=270;i<278;i++)
		bmp_buf[i] = bmp_buf[i+10] = bmp_buf[i+20] = 0x001f;
	
	buzzer(30);
	
	RA8875_SetDispWin(250, 280, 16, 300);
	RA8875_REG = 0x02;
	/* 初始化一个mailbox */
	rt_mb_init(&startup_mb,"startup_mb", startup_mb_pool, sizeof(startup_mb_pool)/4, RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	rt_mb_init(&modbus_mb ,"modbus_mb",  modbus_mb_pool , sizeof(modbus_mb_pool)/4 , RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待*/
	{
		rt_thread_t modbus_thread;
		modbus_thread = rt_thread_create("modbus_serve",
										 modbus_server, RT_NULL,
										 2048, 3, 20);
		if (modbus_thread != RT_NULL)
        rt_thread_startup(modbus_thread);
	}
 	while(1)
	{
		if (rt_mb_recv(&startup_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/10) == RT_EOK)
		{
			startup_flag = 1;
			RA8875_DrawBMP(0,0,480,800,(u16 *)ExternSramAddr);
			return;
		}
		else
		{
			for(i=0;i<16;i++)
			{
				p = &bmp_buf[x];
				for(j=0;j<300;j++)
					RA8875_RAM = *p++;
			}
			if(x>=30)x-=30;
			else x=270;
		}
	}
}


static void  modbus_server(void* parameter)
{
	u8 msg;
// 	u8 i;
	while(1)
	{
		if (rt_mb_recv(&modbus_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
// 			for(i=0;i<50;i++)
// 				eMBPoll();
		}
	}
}


int rt_application_init()
{
    rt_thread_t init_thread;
	
		/* File System */

	
#if (RT_THREAD_PRIORITY_MAX == 32)
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);

	
#if 1
    //------- init led1 thread
    rt_thread_init(&thread_led,
                   "led1",
                   rt_thread_entry_led,
                   RT_NULL,
                   &thread_led_stack[0],
                   sizeof(thread_led_stack),15,5);
    rt_thread_startup(&thread_led);
#endif
	
#if 1
	init_thread = rt_thread_create("startup",
                                   rt_startup_thread_entry, RT_NULL,
                                   2048, 5, 20);
    if (init_thread != RT_NULL)
        rt_thread_startup(init_thread);
#endif
    return 0;
}

/*@}*/
