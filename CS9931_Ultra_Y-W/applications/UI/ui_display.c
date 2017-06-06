#include "CS99xx.h"
#include "bsp_listbox.h"

#include "sui_window.h"


void ui_display_thread(void)
{
// 	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};

// __system_loop:
	if(panel_flag == 8)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
// 		rt_memcpy((void *)ExternSramHomeAddr,(void *)ExternSramNullAddr,ExternSramHomeSize);
		/* ±ÍÃ‚ */
			/* ”Ô—‘≈–∂œ */
// 			if(language==1)
// 				font_draw(system_title_name.x,system_title_name.y,&font,system_title_name.data_en);
// 			else
// 				font_draw(system_title_name.x,system_title_name.y,&font,system_title_name.data_ch);
		rt_mb_send(&screen_mb, UPDATE_HOME);
		ui_key_updata(0);
	}
	while(panel_flag == 8)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* ∑µªÿ */
				case KEY_F6 | KEY_UP:
					panel_flag = 0;
					break;

				case KEY_ENTER | KEY_UP:
					break;
				default:
					break;
			}
		}
		else
		{
			
		}
	}
}
