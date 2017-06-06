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

#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>


#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
#include "stm32_eth.h"
#endif

void rt_init_thread_entry(void* parameter)
{
	
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
//     rt_components_init();
	rtgui_system_server_init();
#endif
	
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
//FS

//GUI
#ifdef RT_USING_RTGUI
    {
        extern void rt_hw_lcd_init();
        extern void rtgui_touch_hw_init(void);

        rt_device_t lcd;

        /* init lcd */
        rt_hw_lcd_init();

        /* init touch panel */
        rtgui_touch_hw_init();

        /* find lcd device */
        lcd = rt_device_find("lcd");

        /* set lcd device as rtgui graphic driver */
        rtgui_graphic_set_device(lcd);

#ifndef RT_USING_COMPONENTS_INIT
        /* init rtgui system server */
        rtgui_system_server_init();
#endif

//         calibration_set_restore(cali_setup);
//         calibration_set_after(cali_store);
//         calibration_init();
		{	
			extern void application_init(void);
			application_init();
		}
    }
#endif /* #ifdef RT_USING_RTGUI */
	
//RTC
	{
		extern void rt_hw_rtc_init(void);
		rt_hw_rtc_init();
	}
}


ALIGN(RT_ALIGN_SIZE)
static char thread_led1_stack[512];
struct rt_thread thread_led1;
static void rt_thread_entry_led1(void* parameter)
{
	extern void bsp_InitLed(void);
	extern void bsp_LedOn(uint8_t _no);
	extern void bsp_LedOff(uint8_t _no);
    bsp_InitLed();
	while(1)
	{
		bsp_LedOn(2);
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */
		bsp_LedOff(2);
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */
	}
}



int rt_application_init()
{
    rt_thread_t init_thread;

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

    //------- init led1 thread
    rt_thread_init(&thread_led1,
                   "led1",
                   rt_thread_entry_led1,
                   RT_NULL,
                   &thread_led1_stack[0],
                   sizeof(thread_led1_stack),11,5);
    rt_thread_startup(&thread_led1);

	
    return 0;
}

/*@}*/
