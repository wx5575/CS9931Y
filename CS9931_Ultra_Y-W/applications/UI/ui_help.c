#include "CS99xx.h"
#include "bsp_listbox.h"

#include "sui_window.h"

char *help_title[2] = {"仪器使用的安全规范","Instrument safety rules"};
char *help_list[2][8] = {
	{
		"1.不要在易燃的环境下使用测试仪.", 
		"2.不要在高温或者阳光直接照射的地方使用测试仪.",
		"3.不要在高湿的环境中使用测试仪.",
		"4.不要在多灰尘的环境下使用测试仪.",
		"5.不要在通风很差的环境下使用测试仪.",
		"6.不要把测试仪放在倾斜的表面或者在晃动的地方使用测试仪.",
		"7.不要在敏感的测试设备或接收设备旁使用测试仪.",
		"8.测试仪的输入电源必有单独的开关控制.",
	},
	{
		"1.Do not use tester in the flammable air.", 
		"2.Do not use tester in the high heat area.",
		"3.Do not use tester in the high humidity area.",
		"4.Do not use tester in the dusty environment.",
		"5.Do not use tester in the poor ventilated area.",
		"6.Do not let tester be on slant surfaces or use tester in shaking places.",
		"7.Do not use tester beside the sensitive equipment or receiving equipment.",
		"8.The input power of tester must have a separate switch control.",
	},
};


void ui_help_thread(void)
{
	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,24};
	char page[10],current_page=1,max_page=1;

	if(panel_flag == 5)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
// 		rt_memcpy((void *)ExternSramHomeAddr,(void *)ExternSramNullAddr,ExternSramHomeSize);
		/* 标题 */
			font_draw((680-rt_strlen(help_title[language])*12)/2,10,&font,help_title[language]);
			rt_sprintf(page,"%02d/%02d",current_page,max_page);
			font.high = 16;
			font.fontcolor = 0xffff;
			font_draw(600,18,&font,page);
		
		/* 绘制水平线 */
			clr_win(&panel_home,0xf800,30,35,3,620);
		
		/* 写内容 */
			for(i=0;i<8;i++)
				font_draw(40,70+30*i,&font,help_list[language][i]);
		
		rt_mb_send(&screen_mb, UPDATE_HOME);
		ui_key_updata(0);
	}
	while(panel_flag == 5)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
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
