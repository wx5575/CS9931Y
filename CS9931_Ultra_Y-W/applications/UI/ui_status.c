#include "CS99xx.h"

static void status_entry(void *parameter)
{
	
	while(1)
	{
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(0,456,24,800,p);
		rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(0,456,24,800,p+800*480);
	}
}

void ui_status_init(void)
{
    static rt_bool_t inited = RT_FALSE;

    if (inited == RT_FALSE) /* 避免重复初始化而做的保护 */
    {
        rt_thread_t tid;

		
        tid = rt_thread_create("status",
                               status_entry, RT_NULL,
                               2048, 19, 10);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
				
        inited = RT_TRUE;
    }
}
