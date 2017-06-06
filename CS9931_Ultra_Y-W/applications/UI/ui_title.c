#include "CS99xx.h"

static void title_entry(void *parameter)
{
	

		RA8875_DrawBMP(0,0,30,800,(u16 *)ExternSramTitleAddr);

	
	
	
	while(1)
	{
		rt_thread_delay( RT_TICK_PER_SECOND/5 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(0,0,30,800,p);
		rt_thread_delay( RT_TICK_PER_SECOND/5 ); /* sleep 0.2 second and switch to other thread */
// 		RA8875_DrawBMP(0,0,30,800,p+800*480);
	}
}

void ui_title_init(void)
{
    static rt_bool_t inited = RT_FALSE;

    if (inited == RT_FALSE) /* 避免重复初始化而做的保护 */
    {
        rt_thread_t tid;

		
        tid = rt_thread_create("title",
                               title_entry, RT_NULL,
                               2048, 26, 10);

        if (tid != RT_NULL)
            rt_thread_startup(tid);
				
        inited = RT_TRUE;
    }
}
