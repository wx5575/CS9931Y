#include "CS99xx.h"

#define		KEY_FONT_X	10
#define		KEY_FONT_Y	10
#define		KEY_FONT_COLOR	0x0000



static void key_entry(void *parameter)
{
	
// 	rt_mb_send(&screen_mb, UPDATE_KEY);
	while(1)
	{
		rt_thread_delay( RT_TICK_PER_SECOND/1 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(800-120,54,402,120,p);
		rt_thread_delay( RT_TICK_PER_SECOND/1 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(800-120,54,402,120,p+800*480);
	}
}

void ui_key_init(void)
{
    static rt_bool_t inited = RT_FALSE;

    if (inited == RT_FALSE) /* 避免重复初始化而做的保护 */
    {
        rt_thread_t tid;

		
        tid = rt_thread_create("key",
                               key_entry, RT_NULL,
                               2048, 21, 10);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
				
        inited = RT_TRUE;
    }
}
