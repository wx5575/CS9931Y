#include "CS99xx.h"
#include "memory.h"
#include "memorymanagement.h"
#include "bsp_listbox.h"
#include "CS9931_Config.h"

#define ASSIST_VOL_MAX	2500

#ifdef CS9932YS_1_40A
#define GR_CUR_MAX		400 //GR输出电流的最大值
#else
#define GR_CUR_MAX		320 //GR输出电流的最大值
#endif

#define GR_MAX_CUR_RES	150	//最大电流测电阻150mOhm
#define WARNIGN_VOL		600 //报警电源最大值为6v

struct step_cw_gr_t
{
	rt_uint16_t groutcur; // output current.
	rt_uint16_t grreshigh; // resistance high limit.
	rt_uint16_t grreslow; // resistance low limit.
	rt_uint16_t gralarmvol; // alarm voltage.
	rt_uint16_t groutfreq; // out frequency.
	rt_uint32_t offsetvalue; //offset value.
};

/*************************************************************************************************************************************/
/*************************************************************************************************************************************/
/************************* <<<<  步骤菜单  >>>>*********************/

static void step_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y);
static rt_uint8_t window_step_set(UN_STR *un);
static rt_uint8_t window_network_set(rt_uint8_t id);
static rt_uint8_t window_medical_set(struct step_lc_t *lc);
static rt_uint8_t window_CW_GR_set(struct step_cw_gr_t *cw_gr);

static uint8_t  Search_mode_next(uint8_t current_mode);
static uint8_t  Search_mode_previous(uint8_t current_mode);

static void dla_draw_items(struct font_info_t *font,struct step_lc_t *lc,rt_uint8_t pos_old,rt_uint8_t pos);

extern rt_uint32_t	num_input(struct font_info_t *font,struct rect_type *rect,struct num_format *num);

extern const char *curgear_name_DCW[];
extern const char *curgear_name_LC[];


struct 
{
    u16 x;
	u16 y;
	const char *data_en;
	const char *data_ch;
}step_title_name[5]={
	{10,10,"NO.","编号"},{100,10,"Mode","测试模式"},{230,10,"OutOption","输出项目"},{360,10,"TestTime","测试时间"},{490,10,"StepCon.","步间连续"},
};


static struct rect_type list_box_rect={10,40,26,660};
struct rtgui_listctrl step_list_box={
	&panel_home,
	13,
	TOTALSTEP_NUM,
	0,0,
	&list_box_rect,
	step_item_draw
};
void ui_step_thread(void)
{
	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};
	
	if(panel_flag == 2)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
		
	// 	rt_enter_critical();
		
		/* 标题 */
		for(i=0;i<5;i++)
		{
			/* 语言判断 */
			if(language==1)
				font_draw(step_title_name[i].x,step_title_name[i].y,&font,step_title_name[i].data_en);
			else
				font_draw(step_title_name[i].x,step_title_name[i].y,&font,step_title_name[i].data_ch);
		}	
		step_list_box.current_item = 0;
		step_list_box.start_item = 0;
		g_cur_step = cs99xx_list.head;//指向步骤文件头
		listbox_draw(&step_list_box);
	// rt_exit_critical();
		
		rt_mb_send(&screen_mb, UPDATE_HOME);
		
		if(cs99xx_list.size == 1)
			ui_key_updata(0x38);
		else
			ui_key_updata(0x10);
	}
	while(panel_flag == 2)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 详细 */
				case KEY_F1 | KEY_UP:
				case KEY_ENTER | KEY_UP:
					if(step_list_box.current_item < cs99xx_list.size)
					{
						g_cur_step = position_step(step_list_box.current_item+1);
						window_step_set(&rt_list_entry(g_cur_step, STEP_NODE, list)->un);
						step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
						g_cur_step = position_step(step_list_box.start_item+1);
						listbox_draw(&step_list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
					}
					break;
				/* 新建 */
				case KEY_F2 | KEY_UP:
					if(step_list_box.current_item < TOTALSTEP_NUM)
					{
						if(step_list_box.current_item < cs99xx_list.size)
						{
							insert_after(step_list_box.current_item+1);
							step_list_box.current_item++;
							step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
							
						}
						else
						{
							insert_after(cs99xx_list.size);
							step_list_box.current_item=cs99xx_list.size;
							step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
							
						}
						
						g_cur_step = position_step(step_list_box.start_item+1);
						listbox_draw(&step_list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
						
						if(step_list_box.current_item >= cs99xx_list.size)
							ui_key_updata(0xb8);
						else if(cs99xx_list.size == 1)
							ui_key_updata(0x38);
						else if(step_list_box.current_item == 0)
							ui_key_updata(0x10);
						else if(step_list_box.current_item == cs99xx_list.size-1)
							ui_key_updata(0x08);
						else
						ui_key_updata(0);
						
						save_steps_to_flash(flash_info.current_file);
						file_info[flash_info.current_file].totalstep = cs99xx_list.size;
						file_info[flash_info.current_file].offset_en = 0;
						save_file_to_flash(flash_info.current_file);

					}					
					break;
				/* 删除 */
				case KEY_F3 | KEY_UP:
					if(step_list_box.current_item < cs99xx_list.size)
					{
						remove_step(step_list_box.current_item +1);
						
						step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
						g_cur_step = position_step(step_list_box.start_item+1);
						listbox_draw(&step_list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
						
						save_steps_to_flash(flash_info.current_file);
						file_info[flash_info.current_file].totalstep = cs99xx_list.size;
						save_file_to_flash(flash_info.current_file);
					}
					if(step_list_box.current_item >= cs99xx_list.size)
						ui_key_updata(0xb8);
					else if(cs99xx_list.size == 1)
						ui_key_updata(0x38);
					else if(step_list_box.current_item == 0)
						ui_key_updata(0x10);
					else if(step_list_box.current_item == cs99xx_list.size-1)
						ui_key_updata(0x08);
					else
						ui_key_updata(0);
					break;
				/* 前移 */
				case KEY_F4 | KEY_UP:
					if(step_list_box.current_item < cs99xx_list.size && step_list_box.current_item != 0)
					{
						swap_step(step_list_box.current_item+1,step_list_box.current_item);
						step_list_box.current_item--;
						step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
						g_cur_step = position_step(step_list_box.start_item+1);
						listbox_draw(&step_list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
						
						if(step_list_box.current_item >= cs99xx_list.size)
							ui_key_updata(0xb8);
						else if(cs99xx_list.size == 1)
							ui_key_updata(0x38);
						else if(step_list_box.current_item == 0)
							ui_key_updata(0x10);
						else if(step_list_box.current_item == cs99xx_list.size-1)
							ui_key_updata(0x08);
						else
							ui_key_updata(0);
						
						save_steps_to_flash(flash_info.current_file);
						file_info[flash_info.current_file].totalstep = cs99xx_list.size;
						save_file_to_flash(flash_info.current_file);
					}
					break;
				/* 后移 */
				case KEY_F5 | KEY_UP:
					if(step_list_box.current_item < cs99xx_list.size-1)
					{
						swap_step(step_list_box.current_item+1,step_list_box.current_item+2);
						step_list_box.current_item++;
						step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
						g_cur_step = position_step(step_list_box.start_item+1);
						listbox_draw(&step_list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
						
						if(step_list_box.current_item >= cs99xx_list.size)
							ui_key_updata(0xb8);
						else if(cs99xx_list.size == 1)
							ui_key_updata(0x38);
						else if(step_list_box.current_item == 0)
							ui_key_updata(0x10);
						else if(step_list_box.current_item == cs99xx_list.size-1)
							ui_key_updata(0x08);
						else
							ui_key_updata(0);
						
						save_steps_to_flash(flash_info.current_file);
						file_info[flash_info.current_file].totalstep = cs99xx_list.size;
						save_file_to_flash(flash_info.current_file);
					}
					break;
				/* 返回 */
				case KEY_F6 | KEY_UP:
					panel_flag = 0;
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(step_list_box.current_item>0)step_list_box.current_item --;
					step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
					g_cur_step = position_step(step_list_box.start_item+1);
					listbox_draw(&step_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					if(step_list_box.current_item >= cs99xx_list.size)
						ui_key_updata(0xb8);
					else if(cs99xx_list.size == 1)
						ui_key_updata(0x38);
					else if(step_list_box.current_item == 0)
						ui_key_updata(0x10);
					else if(step_list_box.current_item == cs99xx_list.size-1)
						ui_key_updata(0x08);
					else
						ui_key_updata(0);
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(step_list_box.current_item<step_list_box.total_items-1)step_list_box.current_item ++;
					step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
					g_cur_step = position_step(step_list_box.start_item+1);
					listbox_draw(&step_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					if(step_list_box.current_item >= cs99xx_list.size)
						ui_key_updata(0xb8);
					else if(cs99xx_list.size == 1)
						ui_key_updata(0x38);
					else if(step_list_box.current_item == 0)
						ui_key_updata(0x10);
					else if(step_list_box.current_item == cs99xx_list.size-1)
						ui_key_updata(0x08);
					else
						ui_key_updata(0);
					break;
				case KEY_L | KEY_DOWN:
					if(step_list_box.current_item >= step_list_box.items_count)
						step_list_box.current_item -= step_list_box.items_count;
					step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
					g_cur_step = position_step(step_list_box.start_item+1);
					listbox_draw(&step_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					if(step_list_box.current_item >= cs99xx_list.size)
						ui_key_updata(0xb8);
					else if(cs99xx_list.size == 1)
						ui_key_updata(0x38);
					else if(step_list_box.current_item == 0)
						ui_key_updata(0x10);
					else if(step_list_box.current_item == cs99xx_list.size-1)
						ui_key_updata(0x08);
					else
						ui_key_updata(0);
					break;
				case KEY_R | KEY_DOWN:
					step_list_box.current_item += step_list_box.items_count;
					if(step_list_box.current_item>step_list_box.total_items-1)step_list_box.current_item=step_list_box.total_items-1;
					step_list_box.start_item = (step_list_box.current_item / step_list_box.items_count)*step_list_box.items_count;
					g_cur_step = position_step(step_list_box.start_item+1);
					listbox_draw(&step_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					if(step_list_box.current_item >= cs99xx_list.size)
						ui_key_updata(0xb8);
					else if(cs99xx_list.size == 1)
						ui_key_updata(0x38);
					else if(step_list_box.current_item == 0)
						ui_key_updata(0x10);
					else if(step_list_box.current_item == cs99xx_list.size-1)
						ui_key_updata(0x08);
					else
						ui_key_updata(0);
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

static void step_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y)
{
	char buf[20];
	u32		temp;
	/* 显示编号 */
	rt_sprintf(buf,"%03d",index+1);
	font_draw(x+8,y,font,buf);
	
	/* 判断下一个步骤是否存在 */
	if(g_cur_step != NULL && (index<cs99xx_list.size))
	{
	/* 是，则显示步骤信息 */
		switch(rt_list_entry(g_cur_step, STEP_NODE, list)->un.com.mode)
		{
			case ACW:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("ACW")*8)/2,y,font,"ACW");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw.outvol;
				rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case DCW:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("DCW")*8)/2,y,font,"DCW");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw.outvol;
				rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case IR:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("IR")*8)/2,y,font,"IR");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.ir.outvol;
				rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.ir.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.ir.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case GR:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("GR")*8)/2,y,font,"GR");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.gr.outcur;
				rt_sprintf(buf,"%d.%dA", temp/10,temp%10);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.gr.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.gr.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case LC:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("LC")*8)/2,y,font,"LC");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.lc.outvol;
				rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.lc.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.lc.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case PW:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("PW")*8)/2,y,font,"PW");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.pw.outvol;
				rt_sprintf(buf,"%d.%01dV", temp/10,temp%10);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.pw.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.pw.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case ACW_GR:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("ACW_GR")*8)/2,y,font,"ACW_GR");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw_gr.outvol;
				rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw_gr.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.acw_gr.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
			case DCW_GR:
				/* 模式名字 */
				font_draw(x+90+(80-rt_strlen("DCW_GR")*8)/2,y,font,"DCW_GR");
				/* 输出电压 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw_gr.outvol;
				rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
				font_draw(x+200+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 测试时间 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw_gr.testtime;
				rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
				font_draw(x+330+(120-rt_strlen(buf)*8)/2,y,font,buf);
				/* 步间连续 */
				temp = rt_list_entry(g_cur_step, STEP_NODE, list)->un.dcw_gr.stepcontinuation;
				font_draw(x+480+(80-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);
				break;
		}
		g_cur_step = g_cur_step->next;
	}
	/* 否，则退出*/
}

const char *step_acw_name[2][18]={
	{"测试模式:","输出电压:","启动电压:","电流档位:","电流上限:","电流下限:","真实电流:","电弧侦测:","输出频率:",
	 "等待时间:","上升时间:","测试时间:","下降时间:","间隔时间:","步间PASS:","步间连续:","失败继续:","多路选择:"},
	 {"Test Mode :","OutputVol.:","Start Vol.:","Range     :","Hi Limit  :","Low Limit :","Real Cur. :","ARC       :",
	  "Frequence :","Wait Time :","Rise Time :","Test Time :","Fall Time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :","Ports :"}
};
const char *step_dcw_name[2][17]={
	{"测试模式:","输出电压:","启动电压:","电流档位:","电流上限:","电流下限:","充电检测:","电弧侦测:",
	 "等待时间:","上升时间:","测试时间:","下降时间:","间隔时间:","步间PASS:","步间连续:","失败继续:","多路选择:"},
	 {"Test Mode :","Output Vol:","Start Vol :","Range     :","Hi Limit  :","Low Limit :","ChargeCur.:","ARC       :",
	  "Wait Time :","Rise Time :","Test Time :","Fall Time :","Inter.Time:","Step PASS :","Step Con.  :","Fail Con. :","Ports :"}
};
const char *step_ir_name[2][12]={
	{"测试模式:","输出电压:","电阻档位:","电阻上限:","电阻下限:","失败继续:",
	 "等待时间","上升时间:","测试时间:","间隔时间:","步间PASS:","步间连续:"},
	{"Test Mode :","Output Vol:","Range     :","Hi Limit  :","Low Limit :","Fail Con. :",
	 "Wait Time :","Rise Time :","Test Time :","Inter.Time:","Step PASS :","Step Con. :"}
};
const char *step_gr_name[2][12]={
	{"测试模式:","输出电流:","电阻上限:","电阻下限:","报警电压:","输出频率:",
	 "上升时间:","测试时间:","间隔时间:","步间PASS:","步间连续:","失败继续:"},
	{"Test Mode :","OutputCur.:","Hi Limit  :","Low Limit :","Alarm Vol.:","Frequence :",
	 "Rise time :","Test time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :"}
};
const char *step_lc_name[2][16]={
	{"测试模式:","输出电压:","电流档位:","电流上限:","电流下限:","N or L相:","输出频率:","检波方式:",
	 "上升时间:","测试时间:","间隔时间:","步间PASS:","步间连续:","失败继续:","MD端电压:","人体网络:"},
	{"Test Mode :","OutputVol.:","Range     :","Hi Limit  :","Low Limit :","N or L    :","Frequence :","DetectMode:",
	 "Rise time :","Test time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :","MD Voltage:","Human Net :"}
};
const char *step_pw_name[2][16]={
	{"测试模式:","输出电压:","电流上限:","电流下限:","功率上限:","功率下限:","输出频率:",
	 "因数上限:","因数下限:","测试时间:","间隔时间:","步间PASS:","步间连续:","失败继续:"},
	{"Test Mode :","OutputVol.:","Hi Limit  :","Low Limit :","Power High:","Power Low :","Frequence :",
     "FactorHigh:","Factor Low:","Test Time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :"}
};

const char *step_acw_gr_name[2][18]={
	{"测试模式:","输出电压:","启动电压:","电流档位:","电流上限:","电流下限:","真实电流:","电弧侦测:","输出频率:",
	 "等待时间:","上升时间:","测试时间:","下降时间:","间隔时间:","步间PASS:","步间连续:","失败继续:","接地设置:"},
	{"Test Mode :","Output Vol:","Start Vol :","Range     :","Hi Limit  :","Low Limit :","Real Cur  :","ARC       :","OutputRate:",
	 "Wait Time :","Rise Time :","Test Time :","Fall Time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :","Earth Set :"}
};

const char *step_dcw_gr_name[2][17]={
	{"测试模式:","输出电压:","启动电压:","电流档位:","电流上限:","电流下限:","充电检测:","电弧侦测:","接地设置:",
	 "等待时间:","上升时间:","测试时间:","下降时间:","间隔时间:","步间PASS:","步间连续:","失败继续:",},
	{"Test Mode :","Output Vol:","Start Vol :","Range     :","Hi Limit  :","Low Limit :","ChargeTest:","ARC       :","Earth Set :",
	 "Wait Time :","Rise Time :","Test Time :","Fall Time :","Inter.Time:","Step PASS :","Step Con. :","Fail Con. :"}
};

const char *IR_Gear_Name[] = {			
	"AUTO","10MΩ","100MΩ","1GΩ","10GΩ","100GΩ"
};

static void step_set_mode_title(struct font_info_t *font,rt_uint8_t mode)
{
	switch(mode)
	{
		case ACW:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<9;i++)
			{
				font_draw(x,y,font,step_acw_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=9;i<17;i++)
			{
				font_draw(x,y,font,step_acw_name[language][i]);
				y+=30;
			}
		}
			break;
		case DCW:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<8;i++)
			{
				font_draw(x,y,font,step_dcw_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=8;i<16;i++)
			{
				font_draw(x,y,font,step_dcw_name[language][i]);
				y+=30;
			}
		}
			break;
		case IR:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<6;i++)
			{
				font_draw(x,y,font,step_ir_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=6;i<12;i++)
			{
				font_draw(x,y,font,step_ir_name[language][i]);
				y+=30;
			}
		}
			break;
		case GR:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<6;i++)
			{
				font_draw(x,y,font,step_gr_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=6;i<12;i++)
			{
				font_draw(x,y,font,step_gr_name[language][i]);
				y+=30;
			}
		}
			break;
		case LC:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<8;i++)
			{
				font_draw(x,y,font,step_lc_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=8;i<16;i++)
			{
				font_draw(x,y,font,step_lc_name[language][i]);
				y+=30;
			}
		}
			break;
		case PW:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<7;i++)
			{
				font_draw(x,y,font,step_pw_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=7;i<14;i++)
			{
				font_draw(x,y,font,step_pw_name[language][i]);
				y+=30;
			}
		}
			break;
		case ACW_GR:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<9;i++)
			{
				font_draw(x,y,font,step_acw_gr_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=9;i<18;i++)
			{
				font_draw(x,y,font,step_acw_gr_name[language][i]);
				y+=30;
			}
		}
			break;
		case DCW_GR:
		{
			u16 x,y,i;
			x=20;
			y=20+30;
			for(i=0;i<9;i++)
			{
				font_draw(x,y,font,step_dcw_gr_name[language][i]);
				y+=30;
			}
			x = 260;
			y = 20+30;
			for(i=9;i<17;i++)
			{
				font_draw(x,y,font,step_dcw_gr_name[language][i]);
				y+=30;
			}
		}
			break;
	}
}

#define items_width		100
#define items_backcolor	CL_orange

static void step_set_items(struct font_info_t *font,UN_STR *un,rt_uint8_t pos_old,rt_uint8_t pos)
{
	rt_uint32_t		temp;
	char buf[20];
	switch(un->com.mode)
	{
		case ACW:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/9),y+30*(pos_old%9)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/9),y+30*(pos%9)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->acw.outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 启动电压 */
			temp = un->acw.startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流档位 */
			font_draw(x+(items_width-rt_strlen(curgear_name[un->acw.curgear])*8)/2,y,font,curgear_name[un->acw.curgear]);y+=30;
			/* 电流上限 */
			temp = un->acw.curhigh;
			switch(un->acw.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;  //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->acw.curlow;
			switch(un->acw.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;    //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 真实电流 */
			temp = un->acw.rmscur;
			switch(un->acw.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;   //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电弧侦测 */
			temp = un->acw.arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 输出频率 */
			temp = un->acw.outfreq;
			rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			x+= 240;y=20+30;
			/* 等待时间 */
			temp = un->acw.waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 上升时间 */
			temp = un->acw.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->acw.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 下降时间 */
			temp = un->acw.downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->acw.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->acw.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->acw.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->acw.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 多路选择 */
//			temp = un->acw.scanport;
//			font_draw(x+(items_width-rt_strlen(boolean_name[language][0])*8)/2,y,font,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));y+=30;
		}
			break;
		case DCW:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old>=8?1:0),y+30*(pos_old>=8?pos_old-8:pos_old)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos>=8?1:0),y+30*(pos>=8?pos-8:pos)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->dcw.outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 启动电压 */
			temp = un->dcw.startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流档位 */
			font_draw(x+(items_width-rt_strlen(curgear_name_DCW[un->dcw.curgear])*8)/2,y,font,curgear_name_DCW[un->dcw.curgear]);y+=30;
			/* 电流上限 */
			temp = un->dcw.curhigh;
			switch(un->dcw.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);	
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					if(temp>1000)temp=1000;
					break;
				default:
					
					break;
			}
			
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->dcw.curlow;
			switch(un->dcw.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 充电检测 */
			temp = un->dcw.chargecur;
			switch(un->dcw.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					if(temp>1000)temp=1000;
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电弧侦测 */
			temp = un->dcw.arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			x+= 240;y=20+30;
			/* 等待时间 */
			temp = un->dcw.waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 上升时间 */
			temp = un->dcw.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->dcw.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 下降时间 */
			temp = un->dcw.downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->dcw.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->dcw.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->dcw.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->dcw.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 多路选择 */
//			temp = un->acw.scanport;
//			font_draw(x+(items_width-rt_strlen(boolean_name[language][0])*8)/2,y,font,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));y+=30;
		}
			break;
		case IR:
		{
			
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/6),y+30*(pos_old%6)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/6),y+30*(pos%6)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->ir.outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电阻档位 */
			temp = un->ir.autogear;
			rt_sprintf(buf,"%s", IR_Gear_Name[temp]);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电阻上限 */
			temp = un->ir.reshigh;
			switch(un->ir.autogear)
			{
				case 0:   //AUTO
				{
					temp = temp;											
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
				}
				break;
				case 1:  //10MΩ
				{
					temp = temp;											
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
				}
				break;
				case 2: //100MΩ
				{
					temp = temp / 10;											
					rt_sprintf(buf,"%d.%01dMΩ", temp/10,temp%10);
				}
				break;
				case 3: //1GΩ
				{
					temp = temp / 100;											
					rt_sprintf(buf,"%d.%03dGΩ", temp/1000,temp%1000);
				}
				break;
				case 4: //10GΩ
				{
					temp = temp / 1000;											
					rt_sprintf(buf,"%d.%02dGΩ", temp/100,temp%100);
				}
				break;	
				case 5: //100GΩ
				{
					temp = temp / 10000;											
					rt_sprintf(buf,"%d.%01dGΩ", temp/10,temp%10);
				}							
				default:
					
				break;
			}	
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电阻下限 */
			temp = un->ir.reslow;		
			switch(un->ir.autogear)
			{
				case 0:   //AUTO
				{
					temp = temp;											
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
				}
				break;
				case 1:  //10MΩ
				{
					temp = temp;											
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
				}
				break;
				case 2: //100MΩ
				{
					temp = temp / 10;											
					rt_sprintf(buf,"%d.%01dMΩ", temp/10,temp%10);
				}
				break;
				case 3: //1GΩ
				{
					temp = temp / 100;											
					rt_sprintf(buf,"%d.%03dGΩ", temp/1000,temp%1000);
				}
				break;
				case 4: //10GΩ
				{
					temp = temp / 1000;											
					rt_sprintf(buf,"%d.%02dGΩ", temp/100,temp%100);
				}
				break;	
				case 5: //100GΩ
				{
					temp = temp / 10000;											
					rt_sprintf(buf,"%d.%01dGΩ", temp/10,temp%10);
				}							
				default:
					
				break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 失败继续 */
			temp = un->ir.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
				
			
			x+= 240;y=20+30;
			/* 上升时间 */
			temp = un->ir.waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 上升时间 */
			temp = un->ir.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->ir.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->ir.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->ir.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->ir.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			
		}
			break;
		case GR:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/6),y+30*(pos_old%6)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/6),y+30*(pos%6)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电流 */
			temp = un->gr.outcur;
			rt_sprintf(buf,"%d.%dA", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电阻上限 */
			temp = un->gr.reshigh;
			rt_sprintf(buf,"%dmΩ", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电阻下限 */
			temp = un->gr.reslow;
			rt_sprintf(buf,"%dmΩ", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 报警电压 */
			temp = un->gr.alarmvol;
			rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 输出频率 */
			temp = un->gr.outfreq;
			rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			
			x+= 240;y=20+30;
			/* 上升时间 */
			temp = un->gr.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->gr.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->gr.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->gr.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->gr.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->gr.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
		}
			break;
		case LC:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/8),y+30*(pos_old%8)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/8),y+30*(pos%8)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->lc.outvol;
			rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流档位 */
			temp = un->lc.curgear;
			font_draw(x+(items_width-rt_strlen(curgear_name_LC[temp])*8)/2,y,font,curgear_name_LC[temp]);y+=30;
			/* 电流上限 */
			temp = un->lc.curhigh;
			switch(un->dcw.curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					if(temp>3000)temp=3000;
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					if(temp>3000)temp=3000;
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					if(temp>3000)temp=3000;
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->lc.curlow;
			switch(un->dcw.curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					if(temp>3000)temp=3000;
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					if(temp>3000)temp=3000;
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					if(temp>3000)temp=3000;
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* N or L相 */
			temp = un->lc.NorLphase;
			font_draw(x+(items_width-rt_strlen(lc_phase_name[language][temp])*8)/2,y,font,lc_phase_name[language][temp]);y+=30;
			/* 输出频率 */
			temp = un->lc.outfreq;
			rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 检波方式 */
			temp = un->lc.curdetection;
			font_draw(x+(items_width-rt_strlen(lc_detection_name[temp])*8)/2,y,font,lc_detection_name[temp]);y+=30;
			
			x+= 240;y=20+30;
			/* 上升时间 */
			temp = un->lc.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->lc.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->lc.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->lc.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->lc.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->lc.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* MD端电压 */
			temp = un->lc.MDvol;
			rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 人体网络 */
			temp = un->lc.MDnetwork;
			font_draw(x+(items_width-rt_strlen(lc_MDnetwork_name[temp])*8)/2,y,font,lc_MDnetwork_name[temp]);y+=30;
		}
			break;
		case PW:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/7),y+30*(pos_old%7)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/7),y+30*(pos%7)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->pw.outvol;
			rt_sprintf(buf,"%d.%01dV", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流上限 */
			temp = un->pw.curhigh;
			rt_sprintf(buf,"%d.%02dA", temp/100,temp%100);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->pw.curlow;
			rt_sprintf(buf,"%d.%02dA", temp/100,temp%100);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 功率上限 */
			temp = un->pw.pwhigh;
			rt_sprintf(buf,"%d.%03dkW", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 功率下限 */
			temp = un->pw.pwlow;
			rt_sprintf(buf,"%d.%03dkW", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 输出频率 */
			temp = un->pw.outfreq;
			rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			
			x+= 240;y=20+30;
			/* 因数上限 */
			temp = un->pw.factorhigh;
			rt_sprintf(buf,"%01d.%03d", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 因数下限 */
			temp = un->pw.factorlow;
			rt_sprintf(buf,"%01d.%03d", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
								
			/* 测试时间 */
			temp = un->pw.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->pw.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->pw.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->pw.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->pw.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;

		}	
		break;
		case ACW_GR:	
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old/9),y+30*(pos_old%9)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos/9),y+30*(pos%9)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->acw_gr.outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 启动电压 */
			temp = un->acw_gr.startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流档位 */
			font_draw(x+(items_width-rt_strlen(curgear_name[un->acw_gr.curgear])*8)/2,y,font,curgear_name[un->acw_gr.curgear]);y+=30;
			/* 电流上限 */
			temp = un->acw_gr.curhigh;
			switch(un->acw_gr.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;  //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->acw_gr.curlow;
			switch(un->acw_gr.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;    //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 真实电流 */
			temp = un->acw_gr.rmscur;
			switch(un->acw_gr.curgear)
			{
				case I300uA:
					if(temp>2000)temp=2000;   //douyijun
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电弧侦测 */
			temp = un->acw_gr.arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 输出频率 */
			temp = un->acw_gr.outfreq;
			rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			x+= 240;y=20+30;
			/* 等待时间 */
			temp = un->acw_gr.waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 上升时间 */
			temp = un->acw_gr.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->acw_gr.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 下降时间 */
			temp = un->acw_gr.downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->acw_gr.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->acw_gr.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->acw_gr.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->acw_gr.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 接地设置 */
//			temp = un->acw.scanport;
			rt_sprintf(buf,T_STR("接地选项","Earthing set"));
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
		}	
			break;
		case DCW_GR:
		{
			u16 x,y;
			x=20+72+20;
			y=20+30;
			clr_win(font->panel,0x1f,x+240*(pos_old>=9?1:0),y+30*(pos_old>=9?pos_old-9:pos_old)-2,20,items_width);
			clr_win(font->panel,CL_orange,x+240*(pos>=9?1:0),y+30*(pos>=9?pos-9:pos)-2,20,items_width);
			font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,font,mode_name[un->com.mode]);y+=30;
			/* 输出电压 */
			temp = un->dcw_gr.outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 启动电压 */
			temp = un->dcw_gr.startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流档位 */
			font_draw(x+(items_width-rt_strlen(curgear_name_DCW[un->dcw_gr.curgear])*8)/2,y,font,curgear_name_DCW[un->dcw_gr.curgear]);y+=30;
			/* 电流上限 */
			temp = un->dcw_gr.curhigh;
			switch(un->dcw_gr.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);	
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					if(temp>1000)temp=1000;
					break;
				default:
					
					break;
			}
			
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电流下限 */
			temp = un->dcw_gr.curlow;
			switch(un->dcw_gr.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 充电检测 */
			temp = un->dcw_gr.chargecur;
			switch(un->dcw_gr.curgear)
			{
				case I3uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					if(temp>2000)temp=2000;
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					if(temp>1000)temp=1000;
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					if(temp>1000)temp=1000;
					break;
				default:
					
					break;
			}
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 电弧侦测 */
			temp = un->dcw_gr.arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			rt_sprintf(buf,T_STR("接地选项","Earthing set"));
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			
			x+= 240;y=20+30;
			/* 等待时间 */
			temp = un->dcw_gr.waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 上升时间 */
			temp = un->dcw_gr.ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 测试时间 */
			temp = un->dcw_gr.testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 下降时间 */
			temp = un->dcw_gr.downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 间隔时间 */
			temp = un->dcw_gr.pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
			/* 步间PASS */
			temp = un->dcw_gr.steppass;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 步间连续 */
			temp = un->dcw_gr.stepcontinuation;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 失败继续 */
			temp = un->dcw_gr.failstop;
			font_draw(x+(items_width-rt_strlen(boolean_name[language][temp])*8)/2,y,font,boolean_name[language][temp]);y+=30;
			/* 多路选择 */
//			temp = un->acw.scanport;
//			font_draw(x+(items_width-rt_strlen(boolean_name[language][0])*8)/2,y,font,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));y+=30;
		}	
		break;
	}
}

static rt_uint32_t step_set_scanport(rt_uint32_t port)
{
	struct panel_type scanport_win	={(u16 *)ExternSramWinAddr+350*500,230,450,90+25,54+35+65};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	rt_uint32_t msg;
	rt_uint8_t pos=0;
	char *status_[3]={"X","H","L"};
	
	font.panel = &scanport_win;
	clr_mem(scanport_win.data,0xf800,30*scanport_win.w);
	clr_mem(scanport_win.data+30*scanport_win.w,0x0010,(scanport_win.h-30)*scanport_win.w);
// 	font_info_set(&network_win,0xffff,0x0,1,16);
	font_draw(10,7,&font,T_STR("<多路扫描设置>","<Many-way Scan set>"));

	font_draw(20,60 ,&font,T_STR("端口号:","Port:"));
	font_draw(20,100,&font,T_STR("状  态:","state:"));
	
	clr_win(&scanport_win,0x0010,98,98,20,222);
	clr_win(&scanport_win,0x53fa,98,98,20,12);
	{
		u16 x,y,i;
		char *num_[8]={"1","2","3","4","5","6","7","8"};
		
		x=100;
		y=60;
		for(i=0;i<8;i++)
		{
			font_draw(x,y ,&font,num_[i]);
			x+=30;
		}
		x=100;
		y=100;
		for(i=0;i<8;i++)
		{
			font_draw(x,y ,&font,status_[((port>>(i*2))&(0x00000003))]);
			x+=30;
		}
	}
	font_draw(20,160,&font,T_STR("提示:按左右键移动光标，上下键设置，参数范围 X、H、L。","Tips:key move cursor, Up and down keys set range X,H,L."));
	
	panel_update(scanport_win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case CODE_LEFT:
				case KEY_L | KEY_UP:
					if(pos>0)pos--;
					else pos=7;
					clr_win(&scanport_win,0x0010,98,98,20,222);
					clr_win(&scanport_win,0x53fa,98+pos*30,98,20,12);
					{
						u16 x,y,i;
						x=100;
						y=100;
						for(i=0;i<8;i++)
						{
							font_draw(x,y ,&font,status_[((port>>(i*2))&(0x00000003))]);
							x+=30;
						}
					}
					panel_update(scanport_win);
					break;
				case CODE_RIGHT:
				case KEY_R | KEY_UP:
					if(pos<7)pos++;
					else pos=0;
					clr_win(&scanport_win,0x0010,98,98,20,222);
					clr_win(&scanport_win,0x53fa,98+pos*30,98,20,12);
					{
						u16 x,y,i;
						x=100;
						y=100;
						for(i=0;i<8;i++)
						{
							font_draw(x,y ,&font,status_[((port>>(i*2))&(0x00000003))]);
							x+=30;
						}
					}
					panel_update(scanport_win);
					break;
				case KEY_U | KEY_UP:
					switch(((port>>(pos*2))&(0x00000003)))
					{
						case 0:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000002<<(pos*2));
							break;
						case 1:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000000<<(pos*2));
							break;
						case 2:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000001<<(pos*2));
							break;
					}
					clr_win(&scanport_win,0x0010,98,98,20,222);
					clr_win(&scanport_win,0x53fa,98+pos*30,98,20,12);
					{
						u16 x,y,i;
						x=100;
						y=100;
						for(i=0;i<8;i++)
						{
							font_draw(x,y ,&font,status_[((port>>(i*2))&(0x00000003))]);
							x+=30;
						}
					}
					panel_update(scanport_win);
					break;
				case KEY_D | KEY_UP:
					switch(((port>>(pos*2))&(0x00000003)))
					{
						case 0:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000001<<(pos*2));
							break;
						case 1:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000002<<(pos*2));
							break;
						case 2:
							port &= ~(0x00000003<<(pos*2));
							port |= (0x00000000<<(pos*2));
							break;
					}
					clr_win(&scanport_win,0x0010,98,98,20,222);
					clr_win(&scanport_win,0x53fa,98+pos*30,98,20,12);
					{
						u16 x,y,i;
						x=100;
						y=100;
						for(i=0;i<8;i++)
						{
							font_draw(x,y ,&font,status_[((port>>(i*2))&(0x00000003))]);
							x+=30;
						}
					}
					panel_update(scanport_win);
					break;
				case KEY_ENTER | KEY_UP:

					return (port);
				case KEY_EXIT | KEY_UP:

					
					return 0xffffffff;
			}
		}
	}
}


static rt_uint8_t step_set_ACW(struct panel_type *panel,struct step_acw_t *acw,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/9);
	rect.y = y+30*(pos%9)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = acw->outvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 50;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				acw->outvol = temp;
			else
				temp = acw->outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 启动电压 */
			num.num  = acw->startvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->startvol = temp;
			else
				temp = acw->startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 3:
			/* 电流档位 */
			ui_text_draw(&font,&rect,(char *)curgear_name[acw->curgear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							
							switch(acw->curgear)
							{
								case I300uA:
// 									acw->curgear = I100mA;
									acw->curgear = I30mA;
									break;
								case I3mA:
									acw->curgear = I300uA;
									break;
								case I30mA:
									acw->curgear = I3mA;
									break;
								case I100mA:
									acw->curgear = I30mA;
									break;
								default:
									acw->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name[acw->curgear]);
							break;
						case KEY_D | KEY_UP:
							switch(acw->curgear)
							{
								case I300uA:
 									acw->curgear = I3mA;
									break;
								case I3mA:
									acw->curgear = I30mA;
									break;
								case I30mA:
									acw->curgear = I300uA;
									break;
								case I100mA:
									acw->curgear = I300uA;
									break;
								default:
									acw->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name[acw->curgear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name[acw->curgear]);
						  //douyijun  更新电流上限
							switch(acw->curgear)
							{
								
								case I3uA:
									acw->curhigh = 500;
									break;
								case I30uA:
									acw->curhigh = 500;
									break;
								case I300uA:
									acw->curhigh = 500;
									break;
								case I3mA:
									acw->curhigh = 500;
									break;
								case I30mA:
									acw->curhigh = 500;
									break;
								case I100mA:
									acw->curhigh = 1000;
									break;
								default:
									
									break;
							}
							if(acw->curlow > (acw->curhigh - 1))
							{
								acw->curlow = acw->curhigh - 1;
							}
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name[acw->curgear]);
							return 1;
					}
				}
			}
		case 4:
			/* 电流上限 */
			switch(acw->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "uA";
					if(acw->curhigh>2000)acw->curhigh=2000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "mA";
					if(acw->curhigh>2000)acw->curhigh=2000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "mA";
					if(acw->curhigh>2000)acw->curhigh=2000;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 1;
					num.max  = 1000;
					num.unit = "mA";
					if(acw->curhigh>1000)acw->curhigh=1000;
					break;
				default:
					
					break;
			}
			
			
			num.num  = acw->curhigh;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->curhigh = temp;
			else
				temp = acw->curhigh;
			switch(acw->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			if(acw->curlow > (acw->curhigh - 1))
			{
				acw->curlow = acw->curhigh - 1;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 5:
			/* 电流下限 */
			switch(acw->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw->curhigh - 1;
					num.unit = "uA";
					if(acw->curlow>acw->curhigh)acw->curlow=acw->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = acw->curhigh - 1;
					num.unit = "mA";
					if(acw->curlow>acw->curhigh)acw->curlow=acw->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = acw->curhigh - 1;
					num.unit = "mA";
					if(acw->curlow>acw->curhigh)acw->curlow=acw->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw->curhigh - 1;
					num.unit = "mA";
					if(acw->curlow>acw->curhigh)acw->curlow=acw->curhigh;
					break;
				default:
					
					break;
			}
			
			num.num  = acw->curlow;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->curlow = temp;
			else
				temp = acw->curlow;
			switch(acw->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 6:
			/* 真实电流 */
			switch(acw->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw->curhigh;
					num.unit = "uA";
					if(acw->rmscur>acw->curhigh)acw->rmscur=acw->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = acw->curhigh;
					num.unit = "mA";
					if(acw->rmscur>acw->curhigh)acw->rmscur=acw->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = acw->curhigh;
					num.unit = "mA";
					if(acw->rmscur>acw->curhigh)acw->rmscur=acw->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw->curhigh;
					num.unit = "mA";
					if(acw->rmscur>acw->curhigh)acw->rmscur=acw->curhigh;
					break;
				default:
					
					break;
			}
			/* 电流上限 */
			
			num.num  = acw->rmscur;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->rmscur = temp;
			else
				temp = acw->rmscur;
			switch(acw->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 7:
			/* 电弧侦测 */
			temp = acw->arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			ui_text_draw(&font,&rect,buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(temp<9)temp++;
							else temp=0;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_D | KEY_UP:
							if(temp>0)temp--;
							else temp=9;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_ENTER | KEY_UP:
							acw->arc=temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 1;
					}
				}
			}
		case 8:
				/* 输出频率 */
			rt_sprintf(buf,"%d.%dHz", acw->outfreq/10,acw->outfreq%10);
			ui_text_draw(&font,&rect,(char *)buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(acw->outfreq != 500)
							{
								acw->outfreq = 500;
							}
							else
							{
								acw->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", acw->outfreq/10,acw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_D | KEY_UP:
							if(acw->outfreq != 500)
							{
								acw->outfreq = 500;
							}
							else
							{
								acw->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", acw->outfreq/10,acw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", acw->outfreq/10,acw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);	
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", acw->outfreq/10,acw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							return 1;
					}
				}
			}
		case 9:
			/* 等待时间 */
			num.num  = acw->waittime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->waittime = temp;
			else
				temp = acw->waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 上升时间 */
			num.num  = acw->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw->ramptime = temp;
			else
				temp = acw->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 测试时间 */
			num.num  = acw->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw->testtime = temp;
			else
				temp = acw->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 12:
			/* 下降时间 */
			num.num  = acw->downtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw->downtime = temp;
			else
				temp = acw->downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 13:
			/* 间隔时间 */
			num.num  = acw->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw->pausetime = temp;
			else
				temp = acw->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 14:
			/* 步间PASS */
			temp = acw->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 15:
			/* 步间连续 */
			temp = acw->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 16:
			/* 失败继续 */
			temp = acw->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 17:
			/* 多路设置 */
//			temp = acw->scanport;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
//			temp=step_set_scanport(temp);
//			if(temp==0xffffffff)return 1;
//			else acw->scanport = temp;
//			font.backcolor = items_backcolor;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
			break;
	}
	return 0;
}

static rt_uint8_t step_set_DCW(struct panel_type *panel,struct step_dcw_t *dcw,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos>=8?1:0);
	rect.y = y+30*(pos>=8?pos-8:pos)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = dcw->outvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 50;
			num.max  = 6000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->outvol = temp;
			else temp = dcw->outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 启动电压 */
			num.num  = dcw->startvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->startvol = temp;
			else temp = dcw->startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 3:
			/* 电流档位 */
			ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw->curgear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(dcw->curgear>0)dcw->curgear--;
							else dcw->curgear=I30mA;                    //douyijun
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw->curgear]);
							break;
						case KEY_D | KEY_UP:
							if(dcw->curgear<4)dcw->curgear++;
							else dcw->curgear=I3uA;
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw->curgear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw->curgear]);
							//douyijun  更新电流上限
							switch(dcw->curgear)
							{
								
								case I3uA:
									dcw->curhigh = 500;
									break;
								case I30uA:
									dcw->curhigh = 500;
									break;
								case I300uA:
									dcw->curhigh = 500;
									break;
								case I3mA:
									dcw->curhigh = 500;
									break;
								case I30mA:
									dcw->curhigh = 200;
									break;
								case I100mA:
									dcw->curhigh = 1000;
									break;
								default:
									
									break;
							}
							if(dcw->curlow > (dcw->curhigh - 1))
							{
								dcw->curlow = dcw->curhigh - 1;
							}
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name[dcw->curgear]);
							return 1;
					}
				}
			}
		case 4:
			/* 电流上限 */
			switch(dcw->curgear)
			{
				case I3uA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw->curhigh>2000)dcw->curhigh=2000;
					break;
				case I30uA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw->curhigh>2000)dcw->curhigh=2000;
					break;
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw->curhigh>2000)dcw->curhigh=2000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "mA";
					if(dcw->curhigh>2000)dcw->curhigh=2000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "mA";
					if(dcw->curhigh>2000)dcw->curhigh=2000;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "mA";
					if(dcw->curhigh>1000)dcw->curhigh=1000;
					break;
				default:
					
					break;
			}
			
			num.num  = dcw->curhigh;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->curhigh = temp;
			else temp = dcw->curhigh;
			switch(dcw->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			
			if(dcw->curlow > (dcw->curhigh - 1))
			{
				dcw->curlow = dcw->curhigh - 1;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 5:
			/* 电流下限 */
			switch(dcw->curgear)
			{
				case I3uA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "uA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				case I30uA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "uA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "uA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "mA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "mA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = dcw->curhigh - 1;
					num.unit = "mA";
					if(dcw->curlow>dcw->curhigh)dcw->curlow=dcw->curhigh;
					break;
				default:
					
					break;
			}
			num.num  = dcw->curlow;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->curlow = temp;
			else temp = dcw->curlow;
			switch(dcw->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 6:
			/* 真实电流 */
			switch(dcw->curgear)
				{
					case I3uA:
						num._int = 1;
						num._dec = 3;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw->chargecur>3000)dcw->chargecur=3000;
						break;
					case I30uA:
						num._int = 2;
						num._dec = 2;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw->chargecur>3000)dcw->chargecur=3000;
						break;
					case I300uA:
						num._int = 3;
						num._dec = 1;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw->chargecur>3000)dcw->chargecur=3000;
						break;
					case I3mA:
						num._int = 1;
						num._dec = 3;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "mA";
						if(dcw->chargecur>3000)dcw->chargecur=3000;
						break;
					case I30mA:
						num._int = 2;
						num._dec = 2;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "mA";
						if(dcw->chargecur>3000)dcw->chargecur=3000;
						break;
					case I100mA:
						num._int = 3;
						num._dec = 1;
						num.min  = 0;
						num.max  = 1000;
						num.unit = "mA";
						if(dcw->chargecur>1000)dcw->chargecur=1000;
						break;
					default:
						
						break;
				}
			num.num  = dcw->chargecur;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->chargecur = temp;
			else temp = dcw->chargecur;
			switch(dcw->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 7:
			/* 电弧侦测 */
			temp = dcw->arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			ui_text_draw(&font,&rect,buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(temp<9)temp++;
							else temp=0;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_D | KEY_UP:
							if(temp>0)temp--;
							else temp=9;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_ENTER | KEY_UP:
							dcw->arc=temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 1;
					}
				}
			}
		
		case 8:
			/* 等待时间 */
			num.num  = dcw->waittime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->waittime = temp;
			else temp = dcw->waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 9:
			/* 上升时间 */
			num.num  = dcw->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw->ramptime = temp;
			else temp = dcw->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 测试时间 */
			num.num  = dcw->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw->testtime = temp;
			else temp = dcw->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 下降时间 */
			num.num  = dcw->downtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw->downtime = temp;
			else temp = dcw->downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 12:
			/* 间隔时间 */
			num.num  = dcw->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw->pausetime = temp;
			else temp = dcw->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 13:
			/* 步间PASS */
			temp = dcw->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 14:
			/* 步间连续 */
			temp = dcw->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 15:
			/* 失败继续 */
			temp = dcw->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 16:
			/* 多路设置 */
//			temp = dcw->scanport;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
//			temp=step_set_scanport(temp);
//			if(temp==0xffffffff)return 1;
//			else dcw->scanport = temp;
//			font.backcolor = items_backcolor;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
			break;
	}
	return 0;
}

static rt_uint8_t step_set_GR(struct panel_type *panel,struct step_gr_t *gr,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/6);
	rect.y = y+30*(pos%6)-2;
	
	switch(pos)
	{
		/* 输出电流 */
		case 1:
		{
			num.num  = gr->outcur;
			num._int = 2;
			num._dec = 1;
			num.min  = 30;
			num.max  = GR_CUR_MAX;//2016.9.2 wangxin
			num.unit = "A";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			gr->outcur = temp;
			else temp = gr->outcur;
			rt_sprintf(buf,"%d.%dA", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			//douyijun  更新电阻上下限
			
			{
				rt_uint32_t max_res = (float)GR_CUR_MAX / gr->outcur * GR_MAX_CUR_RES;
				if(gr->reshigh > max_res){
					gr->reshigh = max_res;
					rt_sprintf(buf,"%dmΩ", gr->reshigh);
					x=20+72+20;
					y=20+30;
					rect.x = x+240*(2/9);
					rect.y = y+30*(2%9)-2;
					font.backcolor = 0x1f;
					ui_text_draw(&font,&rect,buf);
				}
				if(gr->reslow > (max_res-1)){
					gr->reslow = max_res - 1;
					rt_sprintf(buf,"%dmΩ", gr->reslow);
					x=20+72+20;
					y=20+30;
					rect.x = x+240*(3/9);
					rect.y = y+30*(3%9)-2;
					font.backcolor = 0x1f;
					ui_text_draw(&font,&rect,buf);
				}
			}
			
			
			return 0;
		}
		/* 电阻上限 */
		case 2:
		{
			num.num  = gr->reshigh;
			num._int = 3;
			num._dec = 0;
			num.min  = 1;
			num.max  = (float)GR_CUR_MAX / gr->outcur * GR_MAX_CUR_RES;
			if(num.max > 510)num.max = 510;
			num.unit = "mΩ";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			gr->reshigh = temp;
			else temp = gr->reshigh;
			rt_sprintf(buf,"%dmΩ", temp);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			
			//douyijun  更新电阻下限
			
			{
				if(gr->reslow > (gr->reshigh - 1))
				{
					gr->reslow = gr->reshigh - 1;
				}
			}
			return 0;
		}
		/* 电阻下限 */
		case 3:
		{
			num.num  = gr->reslow;
			num._int = 3;
			num._dec = 0;
			num.min  = 0;
			num.max  = gr->reshigh - 1;
			num.unit = "mΩ";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			gr->reslow = temp;
			else temp = gr->reslow;
			rt_sprintf(buf,"%dmΩ", temp);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		}
		/* 报警电压 */
		case 4:
		{
			num.num  = gr->alarmvol;
			num._int = 1;
			num._dec = 2;
			num.min  = 0;
			num.max  = WARNIGN_VOL;//800;
			num.unit = "V";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			gr->alarmvol = temp;
			else temp = gr->alarmvol;
			rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		}
		/* 输出频率 */
		case 5:
		{
			rt_sprintf(buf,"%d.%dHz", gr->outfreq/10,gr->outfreq%10);
			ui_text_draw(&font,&rect,(char *)buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(gr->outfreq != 500)
							{
								gr->outfreq = 500;
							}
							else
							{
								gr->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", gr->outfreq/10,gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_D | KEY_UP:
							if(gr->outfreq != 500)
							{
								gr->outfreq = 500;
							}
							else
							{
								gr->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", gr->outfreq/10,gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", gr->outfreq/10,gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);	
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", gr->outfreq/10,gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							return 1;
					}
				}
			}
		
		}
		/* 上升时间 */
		case 6:
		{
			num.num  = gr->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			gr->ramptime = temp;
			else temp = gr->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		}
		/* 测试时间 */
		case 7:
		{
			num.num  = gr->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			gr->testtime = temp;
			else temp = gr->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		}
		/* 间隔时间 */
		case 8:
		{
			num.num  = gr->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			gr->pausetime = temp;
			else temp = gr->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		}
		/* 步间PASS */
		case 9:
		{
			temp = gr->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							gr->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		}
		/* 步间连续 */
		case 10:
		{
			temp = gr->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							gr->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		}
		/* 失败继续 */
		case 11:
		{
			temp = gr->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							gr->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		}
	}
	return 0;
}

static rt_uint8_t step_set_LC(struct panel_type *panel,struct step_lc_t *lc,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/8);
	rect.y = y+30*(pos%8)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = lc->outvol;
			num._int = 3;
			num._dec = 1;
		
#ifdef LC_MAX_VOL_300V
			num.min  = 300;
			num.max  = 3000;
#endif
#ifdef LC_MAX_VOL_250V
			num.min  = 300;
			num.max  = 2500;
#endif
			num.unit = "V";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			lc->outvol = temp;
			else temp = lc->outvol;
			rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 电流档位 */
			ui_text_draw(&font,&rect,(char *)curgear_name_LC[lc->curgear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							switch(lc->curgear)
							{
								case I300uA:
									lc->curgear = I30mA;
									break;
								case I3mA:
									lc->curgear = I300uA;
									break;
								case I30mA:
									lc->curgear = I3mA;
									break;
								default:
									lc->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name_LC[lc->curgear]);
							break;
						case KEY_D | KEY_UP:
							switch(lc->curgear)
							{
								case I300uA:
									lc->curgear = I3mA;
									break;
								case I3mA:
									lc->curgear = I30mA;
									break;
								case I30mA:
									lc->curgear = I300uA;
									break;
								default:
									lc->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name_LC[lc->curgear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name_LC[lc->curgear]);
							//douyijun  更新电流上限
							switch(lc->curgear)
							{
								
								
								
								case I300uA:
									lc->curhigh = 500;
									break;
								case I3mA:
									lc->curhigh = 500;
									break;
								case I30mA:
									lc->curhigh = 500;
									break;
								case I100mA:
									lc->curhigh = 500;
									break;
								default:
									
									break;
							}
							if(lc->curlow > (lc->curhigh - 1))
							{
								lc->curlow = lc->curhigh - 1;
							}
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name_LC[lc->curgear]);
							return 1;
					}
				}
			}
		case 3:
			/* 电流上限 */
			switch(lc->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(lc->curhigh>2000)lc->curhigh=2000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "mA";
					if(lc->curhigh>2000)lc->curhigh=2000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "mA";
					if(lc->curhigh>1000)lc->curhigh=1000;
					break;
				default:
					
					break;
			}
			num.num  = lc->curhigh;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			lc->curhigh = temp;
			else temp = lc->curhigh;
			switch(lc->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				default:
					
					break;
			}
			if(lc->curlow > (lc->curhigh - 1))
			{
				lc->curlow = lc->curhigh - 1;
			}
			
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 4:
			/* 电流下限 */
			switch(lc->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = lc->curhigh - 1;
					num.unit = "uA";
					if(lc->curlow>3000)lc->curlow=3000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = lc->curhigh - 1;
					num.unit = "mA";
					if(lc->curlow>3000)lc->curlow=3000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = lc->curhigh - 1;
					num.unit = "mA";
					if(lc->curlow>3000)lc->curlow=3000;
					break;
				default:
					
					break;
			}
			num.num  = lc->curlow;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			lc->curlow = temp;
			else temp = lc->curlow ;
			switch(lc->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 5:
			/* N or L相 */
			ui_text_draw(&font,&rect,(char *)lc_phase_name[language][lc->NorLphase]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(lc->NorLphase==1)lc->NorLphase=0;
							else lc->NorLphase = 1;
							ui_text_draw(&font,&rect,(char *)lc_phase_name[language][lc->NorLphase]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)lc_phase_name[language][lc->NorLphase]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)lc_phase_name[language][lc->NorLphase]);
							return 1;
					}
				}
			}
		case 6:
			/* 输出频率 */
			rt_sprintf(buf,"%d.%dHz", lc->outfreq/10,lc->outfreq%10);
			ui_text_draw(&font,&rect,(char *)buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(lc->outfreq != 500)
							{
								lc->outfreq = 500;
							}
							else
							{
								lc->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", lc->outfreq/10,lc->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_D | KEY_UP:
							if(lc->outfreq != 500)
							{
								lc->outfreq = 500;
							}
							else
							{
								lc->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", lc->outfreq/10,lc->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", lc->outfreq/10,lc->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);	
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", lc->outfreq/10,lc->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							return 1;
					}
				}
			}
			
		case 7:
			/* 检波方式 */
			ui_text_draw(&font,&rect,(char *)lc_detection_name[lc->curdetection]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(lc->curdetection>0)lc->curdetection--;
							else lc->curdetection=3;
							ui_text_draw(&font,&rect,(char *)lc_detection_name[lc->curdetection]);
							break;
						case KEY_D | KEY_UP:
							if(lc->curdetection<3)lc->curdetection++;
							else lc->curdetection=0;
							ui_text_draw(&font,&rect,(char *)lc_detection_name[lc->curdetection]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)lc_detection_name[lc->curdetection]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)lc_detection_name[lc->curdetection]);
							return 1;
					}
				}
			}
			
		case 8:
			/* 上升时间 */
			num.num  = lc->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 0;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			lc->ramptime = temp;
			else temp = lc->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 9:
			/* 测试时间 */
			num.num  = lc->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			lc->testtime = temp;
			else temp = lc->testtime ;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 间隔时间 */
			num.num  = lc->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			lc->pausetime = temp;
			else temp = lc->pausetime ;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 步间PASS */
			temp = lc->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							lc->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 12:
			/* 步间连续 */
			temp = lc->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							lc->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 13:
			/* 失败继续 */
			temp = lc->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							lc->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 14:
			/* MD端电压 */
			num.num  = lc->MDvol;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 1000;
			num.unit = "V";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			lc->MDvol = temp;
			else temp = lc->MDvol ;
			rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 15:
		{
			u8 status;
			status = window_network_set(lc->MDnetwork);
			if((status & 0x80) != 0) 
			{
				lc->MDnetwork = status & 0x0f;
				if(lc->MDnetwork == MD_E || lc->MDnetwork == MD_F || lc->MDnetwork == MD_G )
				{
					window_medical_set(lc);//单一故障设置
				}
			}
// 			font_info_set(panel,0xffff,0x0,1,16);
			{
				struct panel_type t=*panel;
				panel_update(t);
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,(char *)lc_MDnetwork_name[lc->MDnetwork]);
			return 0;
		}
			
	}
	return 0;
}

static rt_uint8_t step_set_IR(struct panel_type *panel,struct step_ir_t *ir,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/6);
	rect.y = y+30*(pos%6)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = ir->outvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 50;
			num.max  = 1000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				ir->outvol = temp;
			else
				temp = ir->outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 电阻档位 */
			ui_text_draw(&font,&rect,(char *)IR_Gear_Name[ir->autogear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							ir->autogear = (++ir->autogear) % 6;
							
							ui_text_draw(&font,&rect,(char *)IR_Gear_Name[ir->autogear]);
							break;
						case KEY_D | KEY_UP:
							if(ir->autogear > 0){
								ir->autogear--;
							}else{
								ir->autogear = 5;
							}
							ui_text_draw(&font,&rect,(char *)IR_Gear_Name[ir->autogear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)IR_Gear_Name[ir->autogear]);
						  //douyijun  更新电阻上限
							ir->reshigh = 0;
							switch(ir->autogear)
							{
								case 0:
									ir->reslow = 100;
									break;
								case 1:
									ir->reslow = 100;
									break;
								case 2:
									ir->reslow = 1000;
									break;
								case 3:
									ir->reslow = 10000;
									break;
								case 4:
									ir->reslow = 100000;
									break;
								case 5:
									ir->reslow = 1000000;
									break;								
								default:
									
									break;
							}			
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)IR_Gear_Name[ir->autogear]);
							return 1;
					}
				}
			}
		case 3:
			/* 电阻上限 */
			switch(ir->autogear)
			{
				case 0:   //AUTO
				{
					num.num  = ir->reshigh;
					num._int = 6;
					num._dec = 2;
					num.min  = 0;
					num.max  = 10000000;
					num.unit = "MΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp;
					else temp = ir->reshigh;
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
			
				case 1:  //10MΩ
				{
					num.num  = ir->reshigh;
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "MΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp;
					else temp = ir->reshigh;
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
				case 2: //100MΩ
				{
					num.num  = ir->reshigh / 10;
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "MΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp*10;
					else temp = ir->reshigh / 10;
					rt_sprintf(buf,"%d.%01dMΩ", temp/10,temp%10);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
				
				case 3: //1GΩ
					{
					num.num  = ir->reshigh / 100;
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp*100;
					else temp = ir->reshigh / 100;
					rt_sprintf(buf,"%d.%03dGΩ", temp/1000,temp%1000);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
				
				case 4: //10GΩ
					{
					num.num  = ir->reshigh / 1000;
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp * 1000;
					else temp = ir->reshigh / 1000;
					rt_sprintf(buf,"%d.%02dGΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
					
				case 5: //100GΩ
				{
					num.num  = ir->reshigh / 10000;
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reshigh = temp*10000;
					else temp = ir->reshigh / 10000;
					rt_sprintf(buf,"%d.%01dGΩ", temp/10,temp%10);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
											
				default:
					
					break;
			}	
		case 4:
			/* 电阻下限 */
			switch(ir->autogear)
			{
				case 0:   //AUTO
				{
					num.num  = ir->reslow;
					num._int = 6;
					num._dec = 2;
					num.min  = 0;
					num.max  = 10000000;
					num.unit = "MΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp;
					else temp = ir->reslow;
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
			
				case 1:  //10MΩ
				{
					num.num  = ir->reslow;
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "MΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp;
					else temp = ir->reslow;
					rt_sprintf(buf,"%d.%02dMΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
				case 2: //100MΩ
				{
					num.num  = ir->reslow / 10;
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "MΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp*10;
					else temp = ir->reslow / 10;
					rt_sprintf(buf,"%d.%01dMΩ", temp/10,temp%10);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
					
				case 3: //1GΩ
					{
					num.num  = ir->reslow / 100;
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp*100;
					else temp = ir->reslow / 100;
					rt_sprintf(buf,"%d.%03dGΩ", temp/1000,temp%1000);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
				
				case 4: //10GΩ
					{
					num.num  = ir->reslow / 1000;
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";	
					
					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp * 1000;
					else temp = ir->reslow / 1000;
					rt_sprintf(buf,"%d.%02dGΩ", temp/100,temp%100);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
					
				case 5: //100GΩ
				{
					num.num  = ir->reslow / 10000;
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "GΩ";

					temp = num_input(&font,&rect,&num);
					if(temp != 0xffffffff)
					ir->reslow = temp*10000;
					else temp = ir->reslow / 10000;
					rt_sprintf(buf,"%d.%01dGΩ", temp/10,temp%10);
					font.backcolor = items_backcolor;
					ui_text_draw(&font,&rect,buf);
					return 0;
				}
											
				default:
					
					break;
			}		

		case 5:
			/* 失败继续 */
			temp = ir->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							ir->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 6:
			/* 等待时间 */
			num.num  = ir->waittime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			ir->waittime = temp;
			else temp = ir->waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		
		case 7:
			/* 上升时间 */
			num.num  = ir->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			ir->ramptime = temp;
			else temp = ir->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 8:
			/* 测试时间 */
			num.num  = ir->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			ir->testtime = temp;
			else temp = ir->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 9:
			/* 间隔时间 */
			num.num  = ir->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			ir->pausetime = temp;
			else temp = ir->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 步间PASS */
			temp = ir->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							ir->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 11:
			/* 步间连续 */
			temp = ir->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							ir->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		
	}
	return 0;
}

static rt_uint8_t step_set_PW(struct panel_type *panel,struct step_pw_t *pw,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/7);
	rect.y = y+30*(pos%7)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = pw->outvol;
			num._int = 3;
			num._dec = 1;
			num.min  = 800;
			num.max  = 2500;
			num.unit = "V";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->outvol = temp;
			else
				temp = pw->outvol;
			rt_sprintf(buf,"%03d.%01dV", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 电流上限 */
			num.num  = pw->curhigh;
			num._int = 2;
			num._dec = 2;
			num.min  = 1;
			num.max  = 1500;
			num.unit = "A";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->curhigh = temp;
			else
				temp = pw->curhigh;
			rt_sprintf(buf,"%02d.%02dA", temp/100,temp%100);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 3:
			/* 电流下限 */
			num.num  = pw->curlow;
			num._int = 2;
			num._dec = 2;
			num.min  = 0;
			num.max  = pw->curhigh;
			num.unit = "A";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->curlow = temp;
			else
				temp = pw->curlow;
			rt_sprintf(buf,"%02d.%02dA", temp/100,temp%100);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;	
		case 4:
			/* 功率上限 */
			num.num  = pw->pwhigh;
			num._int = 1;
			num._dec = 3;
			num.min  = 1;
			num.max  = 3000;
			num.unit = "kW";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->pwhigh = temp;
			else
				temp = pw->pwhigh;
			rt_sprintf(buf,"%01d.%03dkW", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;	

		case 5:
			/* 功率下限 */
			num.num  = pw->pwlow;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = pw->pwhigh;
			num.unit = "kW";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->pwlow = temp;
			else
				temp = pw->pwlow;
			rt_sprintf(buf,"%01d.%03dkW", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;	
		case 6:
			/* 输出频率 */
			rt_sprintf(buf,"%d.%dHz", pw->outfreq/10,pw->outfreq%10);
			ui_text_draw(&font,&rect,(char *)buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(pw->outfreq != 500)
							{
								pw->outfreq = 500;
							}
							else
							{
								pw->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", pw->outfreq/10,pw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_D | KEY_UP:
							if(pw->outfreq != 500)
							{
								pw->outfreq = 500;
							}
							else
							{
								pw->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", pw->outfreq/10,pw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", pw->outfreq/10,pw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);	
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", pw->outfreq/10,pw->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							return 1;
					}
				}
			}
		case 7:
			/* 功率上限 */
			num.num  = pw->factorhigh;
			num._int = 1;
			num._dec = 3;
			num.min  = 1;
			num.max  = 1000;
			num.unit = "";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->factorhigh = temp;
			else
				temp = pw->factorhigh;
			rt_sprintf(buf,"%01d.%03d", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;	
		
		case 8:
			/* 功率下限 */
			num.num  = pw->factorlow;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = pw->factorhigh;
			num.unit = "";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				pw->factorlow = temp;
			else
				temp = pw->factorlow;
			rt_sprintf(buf,"%01d.%03d", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
			
		
		case 9:
			/* 测试时间 */
			num.num  = pw->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			pw->testtime = temp;
			else temp = pw->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 间隔时间 */
			num.num  = pw->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			pw->pausetime = temp;
			else temp = pw->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 步间PASS */
			temp = pw->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							pw->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 12:
			/* 步间连续 */
			temp = pw->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							pw->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
			
		case 13:
			/* 失败继续 */
			temp = pw->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							pw->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		
	}
	return 0;
}

static rt_uint8_t step_set_ACW_GR(struct panel_type *panel,struct step_acw_gr_t *acw_gr,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos/9);
	rect.y = y+30*(pos%9)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = acw_gr->outvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 50;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
				acw_gr->outvol = temp;
			else
				temp = acw_gr->outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 启动电压 */
			num.num  = acw_gr->startvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->startvol = temp;
			else
				temp = acw_gr->startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 3:
			/* 电流档位 */
			ui_text_draw(&font,&rect,(char *)curgear_name[acw_gr->curgear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							
							switch(acw_gr->curgear)
							{
								case I300uA:
// 									acw_gr->curgear = I100mA;
									acw_gr->curgear = I30mA;
									break;
								case I3mA:
									acw_gr->curgear = I300uA;
									break;
								case I30mA:
									acw_gr->curgear = I3mA;
									break;
								case I100mA:
									acw_gr->curgear = I30mA;
									break;
								default:
									acw_gr->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name[acw_gr->curgear]);
							break;
						case KEY_D | KEY_UP:
							switch(acw_gr->curgear)
							{
								case I300uA:
 									acw_gr->curgear = I3mA;
									break;
								case I3mA:
									acw_gr->curgear = I30mA;
									break;
								case I30mA:
									acw_gr->curgear = I300uA;
									break;
								case I100mA:
									acw_gr->curgear = I300uA;
									break;
								default:
									acw_gr->curgear = I30mA;
									break;
							}
							ui_text_draw(&font,&rect,(char *)curgear_name[acw_gr->curgear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name[acw_gr->curgear]);
						  //douyijun  更新电流上限
							switch(acw_gr->curgear)
							{
								
								case I3uA:
									acw_gr->curhigh = 500;
									break;
								case I30uA:
									acw_gr->curhigh = 500;
									break;
								case I300uA:
									acw_gr->curhigh = 500;
									break;
								case I3mA:
									acw_gr->curhigh = 500;
									break;
								case I30mA:
									acw_gr->curhigh = 500;
									break;
								case I100mA:
									acw_gr->curhigh = 1000;
									break;
								default:
									
									break;
							}
							if(acw_gr->curlow > (acw_gr->curhigh - 1))
							{
								acw_gr->curlow = acw_gr->curhigh - 1;
							}
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name[acw_gr->curgear]);
							return 1;
					}
				}
			}
		case 4:
			/* 电流上限 */
			switch(acw_gr->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "uA";
					if(acw_gr->curhigh>2000)acw_gr->curhigh=2000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "mA";
					if(acw_gr->curhigh>2000)acw_gr->curhigh=2000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 1;
					num.max  = 2000;
					num.unit = "mA";
					if(acw_gr->curhigh>2000)acw_gr->curhigh=2000;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 1;
					num.max  = 1000;
					num.unit = "mA";
					if(acw_gr->curhigh>1000)acw_gr->curhigh=1000;
					break;
				default:
					
					break;
			}
			
			
			num.num  = acw_gr->curhigh;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->curhigh = temp;
			else
				temp = acw_gr->curhigh;
			switch(acw_gr->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			if(acw_gr->curlow > (acw_gr->curhigh - 1))
			{
				acw_gr->curlow = acw_gr->curhigh - 1;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 5:
			/* 电流下限 */
			switch(acw_gr->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw_gr->curhigh - 1;
					num.unit = "uA";
					if(acw_gr->curlow>acw_gr->curhigh)acw_gr->curlow=acw_gr->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = acw_gr->curhigh - 1;
					num.unit = "mA";
					if(acw_gr->curlow>acw_gr->curhigh)acw_gr->curlow=acw_gr->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = acw_gr->curhigh - 1;
					num.unit = "mA";
					if(acw_gr->curlow>acw_gr->curhigh)acw_gr->curlow=acw_gr->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw_gr->curhigh - 1;
					num.unit = "mA";
					if(acw_gr->curlow>acw_gr->curhigh)acw_gr->curlow=acw_gr->curhigh;
					break;
				default:
					
					break;
			}
			
			num.num  = acw_gr->curlow;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->curlow = temp;
			else
				temp = acw_gr->curlow;
			switch(acw_gr->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 6:
			/* 真实电流 */
			switch(acw_gr->curgear)
			{
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw_gr->curhigh;
					num.unit = "uA";
					if(acw_gr->rmscur>acw_gr->curhigh)acw_gr->rmscur=acw_gr->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = acw_gr->curhigh;
					num.unit = "mA";
					if(acw_gr->rmscur>acw_gr->curhigh)acw_gr->rmscur=acw_gr->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = acw_gr->curhigh;
					num.unit = "mA";
					if(acw_gr->rmscur>acw_gr->curhigh)acw_gr->rmscur=acw_gr->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = acw_gr->curhigh;
					num.unit = "mA";
					if(acw_gr->rmscur>acw_gr->curhigh)acw_gr->rmscur=acw_gr->curhigh;
					break;
				default:
					
					break;
			}
			/* 电流上限 */
			
			num.num  = acw_gr->rmscur;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->rmscur = temp;
			else
				temp = acw_gr->rmscur;
			switch(acw_gr->curgear)
			{
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 7:
			/* 电弧侦测 */
			temp = acw_gr->arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			ui_text_draw(&font,&rect,buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(temp<9)temp++;
							else temp=0;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_D | KEY_UP:
							if(temp>0)temp--;
							else temp=9;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_ENTER | KEY_UP:
							acw_gr->arc=temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 1;
					}
				}
			}
		case 8:
				/* 输出频率 */
			rt_sprintf(buf,"%d.%dHz", acw_gr->outfreq/10,acw_gr->outfreq%10);
			ui_text_draw(&font,&rect,(char *)buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(acw_gr->outfreq != 500)
							{
								acw_gr->outfreq = 500;
							}
							else
							{
								acw_gr->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", acw_gr->outfreq/10,acw_gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_D | KEY_UP:
							if(acw_gr->outfreq != 500)
							{
								acw_gr->outfreq = 500;
							}
							else
							{
								acw_gr->outfreq = 600;
							}
							rt_sprintf(buf,"%d.%dHz", acw_gr->outfreq/10,acw_gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", acw_gr->outfreq/10,acw_gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);	
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							rt_sprintf(buf,"%d.%dHz", acw_gr->outfreq/10,acw_gr->outfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							return 1;
					}
				}
			}
		case 9:
			/* 等待时间 */
			num.num  = acw_gr->waittime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->waittime = temp;
			else
				temp = acw_gr->waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 上升时间 */
			num.num  = acw_gr->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw_gr->ramptime = temp;
			else
				temp = acw_gr->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 测试时间 */
			num.num  = acw_gr->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw_gr->testtime = temp;
			else
				temp = acw_gr->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 12:
			/* 下降时间 */
			num.num  = acw_gr->downtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			acw_gr->downtime = temp;
			else
				temp = acw_gr->downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 13:
			/* 间隔时间 */
			num.num  = acw_gr->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			acw_gr->pausetime = temp;
			else
				temp = acw_gr->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 14:
			/* 步间PASS */
			temp = acw_gr->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw_gr->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 15:
			/* 步间连续 */
			temp = acw_gr->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw_gr->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 16:
			/* 失败继续 */
			temp = acw_gr->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							acw_gr->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 17:
			/* 接地设置 */
			window_CW_GR_set((struct step_cw_gr_t *)(&acw_gr->groutcur));
//			temp = acw->scanport;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
//			temp=step_set_scanport(temp);
//			if(temp==0xffffffff)return 1;
//			else acw->scanport = temp;
//			font.backcolor = items_backcolor;
//			ui_text_draw(&font,&rect,(temp==0 ? ((char *)boolean_name[language][0]):((char *)boolean_name[language][1])));
			break;
	}
	return 0;
}

static rt_uint8_t step_set_DCW_GR(struct panel_type *panel,struct step_dcw_gr_t *dcw_gr,rt_uint8_t pos)
{
	rt_uint32_t msg;
	rt_uint32_t		temp;
	char buf[20];
	u16 x,y;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	struct num_format num;
	font.panel = panel;
	
	x=20+72+20;
	y=20+30;
	rect.x = x+240*(pos>=9?1:0);
	rect.y = y+30*(pos>=9?pos-9:pos)-2;
	switch(pos)
	{
		case 1:
			/* 输出电压 */
			num.num  = dcw_gr->outvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 50;
			num.max  = 6000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->outvol = temp;
			else temp = dcw_gr->outvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 2:
			/* 启动电压 */
			num.num  = dcw_gr->startvol;
			num._int = 1;
			num._dec = 3;
			num.min  = 0;
			num.max  = 5000;
			num.unit = "kV";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->startvol = temp;
			else temp = dcw_gr->startvol;
			rt_sprintf(buf,"%d.%03dkV", temp/1000,temp%1000);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 3:
			/* 电流档位 */
			ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw_gr->curgear]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(dcw_gr->curgear>0)dcw_gr->curgear--;
							else dcw_gr->curgear=I30mA;                    //douyijun
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw_gr->curgear]);
							break;
						case KEY_D | KEY_UP:
							if(dcw_gr->curgear<4)dcw_gr->curgear++;
							else dcw_gr->curgear=I3uA;
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw_gr->curgear]);
							break;
						case KEY_ENTER | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw_gr->curgear]);
							//douyijun  更新电流上限
							switch(dcw_gr->curgear)
							{
								
								case I3uA:
									dcw_gr->curhigh = 500;
									break;
								case I30uA:
									dcw_gr->curhigh = 500;
									break;
								case I300uA:
									dcw_gr->curhigh = 500;
									break;
								case I3mA:
									dcw_gr->curhigh = 500;
									break;
								case I30mA:
									dcw_gr->curhigh = 200;
									break;
								case I100mA:
									dcw_gr->curhigh = 1000;
									break;
								default:
									
									break;
							}
							if(dcw_gr->curlow > (dcw_gr->curhigh - 1))
							{
								dcw_gr->curlow = dcw_gr->curhigh - 1;
							}
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)curgear_name_DCW[dcw_gr->curgear]);
							return 1;
					}
				}
			}
		case 4:
			/* 电流上限 */
			switch(dcw_gr->curgear)
			{
				case I3uA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw_gr->curhigh>2000)dcw_gr->curhigh=2000;
					break;
				case I30uA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw_gr->curhigh>2000)dcw_gr->curhigh=2000;
					break;
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "uA";
					if(dcw_gr->curhigh>2000)dcw_gr->curhigh=2000;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = 2000;
					num.unit = "mA";
					if(dcw_gr->curhigh>2000)dcw_gr->curhigh=2000;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "mA";
					if(dcw_gr->curhigh>2000)dcw_gr->curhigh=2000;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = 1000;
					num.unit = "mA";
					if(dcw_gr->curhigh>1000)dcw_gr->curhigh=1000;
					break;
				default:
					
					break;
			}
			
			num.num  = dcw_gr->curhigh;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->curhigh = temp;
			else temp = dcw_gr->curhigh;
			switch(dcw_gr->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			
			if(dcw_gr->curlow > (dcw_gr->curhigh - 1))
			{
				dcw_gr->curlow = dcw_gr->curhigh - 1;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 5:
			/* 电流下限 */
			switch(dcw_gr->curgear)
			{
				case I3uA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "uA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				case I30uA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "uA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				case I300uA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "uA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				case I3mA:
					num._int = 1;
					num._dec = 3;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "mA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				case I30mA:
					num._int = 2;
					num._dec = 2;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "mA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				case I100mA:
					num._int = 3;
					num._dec = 1;
					num.min  = 0;
					num.max  = dcw_gr->curhigh - 1;
					num.unit = "mA";
					if(dcw_gr->curlow>dcw_gr->curhigh)dcw_gr->curlow=dcw_gr->curhigh;
					break;
				default:
					
					break;
			}
			num.num  = dcw_gr->curlow;
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->curlow = temp;
			else temp = dcw_gr->curlow;
			switch(dcw_gr->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 6:
			/* 真实电流 */
			switch(dcw_gr->curgear)
				{
					case I3uA:
						num._int = 1;
						num._dec = 3;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw_gr->chargecur>3000)dcw_gr->chargecur=3000;
						break;
					case I30uA:
						num._int = 2;
						num._dec = 2;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw_gr->chargecur>3000)dcw_gr->chargecur=3000;
						break;
					case I300uA:
						num._int = 3;
						num._dec = 1;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "uA";
						if(dcw_gr->chargecur>3000)dcw_gr->chargecur=3000;
						break;
					case I3mA:
						num._int = 1;
						num._dec = 3;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "mA";
						if(dcw_gr->chargecur>3000)dcw_gr->chargecur=3000;
						break;
					case I30mA:
						num._int = 2;
						num._dec = 2;
						num.min  = 0;
						num.max  = 3000;
						num.unit = "mA";
						if(dcw_gr->chargecur>3000)dcw_gr->chargecur=3000;
						break;
					case I100mA:
						num._int = 3;
						num._dec = 1;
						num.min  = 0;
						num.max  = 1000;
						num.unit = "mA";
						if(dcw_gr->chargecur>1000)dcw_gr->chargecur=1000;
						break;
					default:
						
						break;
				}
			num.num  = dcw_gr->chargecur;
			
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->chargecur = temp;
			else temp = dcw_gr->chargecur;
			switch(dcw_gr->curgear)
			{
				case I3uA:
					rt_sprintf(buf,"%d.%03duA", temp/1000,temp%1000);
					break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA", temp/100,temp%100);
					break;
				case I300uA:
					rt_sprintf(buf,"%d.%duA", temp/10,temp%10);
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA", temp/1000,temp%1000);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA", temp/100,temp%100);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA", temp/10,temp%10);
					break;
				default:
					
					break;
			}
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 7:
			/* 电弧侦测 */
			temp = dcw_gr->arc;
			if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
			else rt_sprintf(buf,"%d", temp);
			ui_text_draw(&font,&rect,buf);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
							if(temp<9)temp++;
							else temp=0;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_D | KEY_UP:
							if(temp>0)temp--;
							else temp=9;
							if(temp==0)strcpy(buf,T_STR("关闭","OFF"));
							else rt_sprintf(buf,"%d", temp);
							ui_text_draw(&font,&rect,buf);
							break;
						case KEY_ENTER | KEY_UP:
							dcw_gr->arc=temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							return 1;
					}
				}
			}
		case 8:
			/* 接地设置 */
			window_CW_GR_set((struct step_cw_gr_t *)(&dcw_gr->groutcur));

			break;
		case 9:
			/* 等待时间 */
			num.num  = dcw_gr->waittime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->waittime = temp;
			else temp = dcw_gr->waittime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 10:
			/* 上升时间 */
			num.num  = dcw_gr->ramptime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw_gr->ramptime = temp;
			else temp = dcw_gr->ramptime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 11:
			/* 测试时间 */
			num.num  = dcw_gr->testtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw_gr->testtime = temp;
			else temp = dcw_gr->testtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 12:
			/* 下降时间 */
			num.num  = dcw_gr->downtime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
//			if(temp==1 || temp==2)temp=3;
			if(temp != 0xffffffff)
			dcw_gr->downtime = temp;
			else temp = dcw_gr->downtime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 13:
			/* 间隔时间 */
			num.num  = dcw_gr->pausetime;
			num._int = 3;
			num._dec = 1;
			num.min  = 0;
			num.max  = 9999;
			num.unit = "s";
			
			temp = num_input(&font,&rect,&num);
			if(temp != 0xffffffff)
			dcw_gr->pausetime = temp;
			else temp = dcw_gr->pausetime;
			rt_sprintf(buf,"%d.%ds", temp/10,temp%10);
			font.backcolor = items_backcolor;
			ui_text_draw(&font,&rect,buf);
			return 0;
		case 14:
			/* 步间PASS */
			temp = dcw_gr->steppass;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
		
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw_gr->steppass = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 15:
			/* 步间连续 */
			temp = dcw_gr->stepcontinuation;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw_gr->stepcontinuation = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		case 16:
			/* 失败继续 */
			temp = dcw_gr->failstop;
			ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
			while(1)
			{
				if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
				{
					switch(msg)
					{
						case KEY_U | KEY_UP:
						case KEY_D | KEY_UP:
							if(temp!=0)temp=0;
							else temp=1;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							break;
						case KEY_ENTER | KEY_UP:
							dcw_gr->failstop = temp;
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 0;
						case KEY_EXIT | KEY_UP:
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,(char *)boolean_name[language][temp]);
							return 1;
					}
				}
			}
		
	}
	return 0;
}

static rt_uint8_t step_set_mode(struct panel_type *panel,rt_uint8_t *mode)
{
	rt_uint32_t msg;
	struct rect_type rect = {112,48,20,items_width};
	struct font_info_t font = {0,0xffff,0x0000,1,1,16};
	
	rt_uint8_t mode_bk = *mode;
	
	font.panel = panel;
// 	clr_win(panel,0,x,y-2,20,items_width);
// 	font_draw(x+(items_width-rt_strlen(mode_name[un->com.mode])*8)/2,y,mode_name[mode]);
	ui_text_draw(&font,&rect,(char *)mode_name[*mode]);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_U | KEY_UP:
				case CODE_LEFT:
					*mode = Search_mode_previous(*mode);
					ui_text_draw(&font,&rect,(char *)mode_name[*mode]);
					break;
				case KEY_D | KEY_UP:
				case CODE_RIGHT:
					*mode = Search_mode_next(*mode);
					ui_text_draw(&font,&rect,(char *)mode_name[*mode]);
					break;
				case KEY_ENTER | KEY_UP:
					font.backcolor = CL_orange;
					ui_text_draw(&font,&rect,(char *)mode_name[*mode]);
					return 0;
				case KEY_EXIT | KEY_UP:
					*mode = mode_bk;
					font.backcolor = CL_orange;
					ui_text_draw(&font,&rect,(char *)mode_name[*mode]);
					return 1;
			}
		}
	}
}

struct _mode_list
{
	uint8_t mode;
	char    *p_enable;
};

const struct _mode_list mode_list[] = {
	{ACW   ,    &CS9931_Config.ACW_Enable},
	{DCW   ,    &CS9931_Config.DCW_Enable},
	{GR    ,    &CS9931_Config.GR_Enable},
	{LC    ,    &CS9931_Config.LC_Enable},
	{IR    ,    &CS9931_Config.IR_Enable},
	{PW    ,    &CS9931_Config.PW_Enable},
	{ACW_GR,    &CS9931_Config.ACW_GR_Enable},
	{DCW_GR,    &CS9931_Config.DCW_GR_Enable},

};

/*******************************
函数名：  Search_mode_next
参  数：  current_mode :当前的模式

返回值：  下一个模式
********************************/
static uint8_t  Search_mode_next(uint8_t current_mode)
{
	uint8_t index = 0;
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(current_mode == mode_list[i].mode){
			break;
		}
	}
	index = i;
	for(i=0;i<8;i++)
	{
		index = (++index) % 8;
		if(1 == *mode_list[index].p_enable){
			break;
		}
	}
	if(i>=8){
		return 255;
	}else{
		return mode_list[index].mode;
	}
	
}

/*******************************
函数名：  Search_mode
参  数：  current_mode :当前的模式

返回值：  上一个模式
********************************/
static uint8_t  Search_mode_previous(uint8_t current_mode)
{
	uint8_t index = 0;
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(current_mode == mode_list[i].mode){
			break;
		}
	}
	index = i;
	for(i=0;i<8;i++)
	{
		if(index == 0){
			index = 7;
		}else{
			index -= 1;
		}
		
		if(1 == *mode_list[index].p_enable){
			break;
		}
	}
	if(i>=8){
		return 255;
	}else{
		return mode_list[index].mode;
	}
}


const char *step_set_help_name[2][13][20]=
{
	{
		//ACW
		{
			"",
			"0.050kV-5.000kV",
			"0.000kV-5.000kV",
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"0-",
			"0-9",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
		},
		//ACW_GR
		{
			"",
			"0.050kV-5.000kV",
			"0.000kV-5.000kV",
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"0-",
			"0-9",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
			"接地相关参数设置"
		},
		//DCW
		{
			"",
			"0.050kV-6.000kV",
			"0.000kV-6.000kV",
			"2uA、20uA、200uA、2mA、10mA",      //douyijun
			"0-",
			"0-",
			"0-",
			"0-9",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
		},
		//DCW_GR
		{
			"",
			"0.050kV-6.000kV",
			"0.000kV-6.000kV",
			"2uA、20uA、200uA、2mA、10mA",      //douyijun
			"0-",
			"0-",
			"0-",
			"0-9",	
			"接地相关参数设置",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
		},
		//IR
		{
			"",
			"0.050kV-1.000kV",
			"AUTO、10MΩ、100MΩ、1GΩ、10GΩ、100GΩ",
			"",
			"",
			"开启、关闭",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			
			
		},
		//IR_GR
		{
			"",
		},
		//GR
		{
			"",
			#ifdef CS9932YS_1_40A
			"3.0A-40.0A",
			#else
			"3.0A-32.0A",
			#endif 
			"",//"0-510mΩ",
			"",//"0-510mΩ",
			"0.00-6.00V",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
		},
		//LC
		{
			"",
		#if LC_TEST_MODE==LC_YY
			#ifdef LC_MAX_VOL_250V
			"30.0V-250.0V",
			#endif
			#ifdef LC_MAX_VOL_300V
			"30.0V-300.0V",
			#endif
		#else
			"30.0V-300.0V",
		#endif
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"L相、N相",
			"50.0Hz、60.0Hz",
			"AC、AC+DC、PEAK、DC",
			"0.0s-000.0s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
			"30.0V-250.0V",
			"MD-E、MD-F、MD-G",
		},
		//LC_PW
		{
			"",
		},
		//PW
		{
			"",
			"80.0V - 250.0V",
			"0.01A - 15.00A",
			"",
			"0.001kW - 3.000kW",
			"",
			"50.0Hz、60.0Hz",
			"0.001 - 1.000",
			"",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"开启、关闭",
			"开启、关闭",
			"开启、关闭",
		},
	},
	{
		//ACW
		{
			"",
			"0.050kV-5.000kV",
			"0.000kV-5.000kV",
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"0-",
			"0-9",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
		},
		//ACW_GR
		{
			"",
			"0.050kV-5.000kV",
			"0.000kV-5.000kV",
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"0-",
			"0-9",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
			"Earthing set",
		},
		//DCW
		{
			"",
			"0.050kV-6.000kV",
			"0.000kV-6.000kV",
			"2uA、20uA、200uA、2mA、10mA",      //douyijun
			"0-",
			"0-",
			"0-",
			"0-9",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
		},
		//DCW_GR
		{
			"",
			"0.050kV-6.000kV",
			"0.000kV-6.000kV",
			"2uA、20uA、200uA、2mA、10mA",      //douyijun
			"0-",
			"0-",
			"0-",
			"0-9",	
			"Earthing set",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
		},
		//IR
		{
			"",
			"0.050kV-1.000kV",
			"AUTO、10MΩ、100MΩ、1GΩ、10GΩ、100GΩ",
			"",
			"",
			"ON、OFF",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			
			
		},
		//IR_GR
		{
			"",
		},
		//GR
		{
			"",
			#ifdef CS9932YS_1_40A
			"3.0A-40.0A",
			#else
			"3.0A-32.0A",
			#endif 
			"",//"0-510mΩ",
			"",//"0-510mΩ",
			"0.00-6.00V",
			"50.0Hz、60.0Hz",
			"0.0s-999.9s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
		},
		//LC
		{
			"",
		#if LC_TEST_MODE==LC_YY
			#ifdef LC_MAX_VOL_250V
			"30.0V-250.0V",
			#endif
			#ifdef LC_MAX_VOL_300V
			"30.0V-300.0V",
			#endif
		#else
			"30.0V-300.0V",
		#endif
			"200uA、2mA、20mA",
			"0-",
			"0-",
			"Lphase、Nphase",
			"50.0Hz、60.0Hz",
			"AC、AC+DC、PEAK、DC",
			"0.0s-000.0s",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
			"30.0V-250.0V",
			"MD-E、MD-F、MD-G",
		},
		//LC_PW
		{
			"",
		},
		//PW
		{
			"",
			"80.0V - 250.0V",
			"0.01A - 15.00A",
			"",
			"0.001kW - 3.000kW",
			"",
			"50.0Hz、60.0Hz",
			"0.001 - 1.000",
			"",
			"0,0.3s-999.9s",
			"0.0s-999.9s",
			"ON、OFF",
			"ON、OFF",
			"ON、OFF",
		},
	}

};

static rt_uint8_t step_set_help(struct panel_type *win,rt_uint8_t pos,UN_STR *un)
{
	char temp[30];
	rt_uint32_t data;
	struct font_info_t font={0,0xf800,0x0,1,0,16};
	font.panel = win;

#define help_x	10
#define help_y  320
	clr_win(win,0x1f,help_x,help_y,16,480);
	font_draw(help_x,help_y,&font,T_STR("范围：","Range: "));
	if(pos != 0)
		font_draw(help_x+48,help_y,&font,step_set_help_name[language][un->com.mode][pos]);
	else{
		rt_sprintf(temp,"%s%s%s%s%s%s%s%s",CS9931_Config.ACW_Enable? "ACW ":"",
			                                CS9931_Config.DCW_Enable? "DCW ":"",
											CS9931_Config.GR_Enable? "GR ":"",
											CS9931_Config.LC_Enable? "LC ":"",
											CS9931_Config.ACW_GR_Enable? "AG ":"",
											CS9931_Config.DCW_GR_Enable? "DG ":"",
											CS9931_Config.IR_Enable? "IR ":"",
											CS9931_Config.PW_Enable? "PW":"");
		font_draw(help_x+48,help_y,&font,temp);
	}
	switch(un->com.mode)
	{
		case ACW:
		case ACW_GR:
			switch(pos)
			{
				case 0:
					
					break;
				case 1:
					
					break;
				case 2:
					
					break;
				case 3:
					
					break;
				case 4:
					data = 2000;
					switch(un->acw.curgear)
					{
						case I300uA:
							rt_sprintf(temp,"0.1uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0.001mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0.01mA-%d.%02dmA", data/100,data%100);
							break;
						case I100mA:
							rt_sprintf(temp,"0.1mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 5:
					data = un->acw.curhigh-1;
					switch(un->acw.curgear)
					{
						case I300uA:
							rt_sprintf(temp,"0uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0mA-%d.%02dmA", data/100,data%100);
							break;
						case I100mA:
							rt_sprintf(temp,"0mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 6:
					data = un->acw.curhigh;
					switch(un->acw.curgear)
					{
						case I300uA:
							rt_sprintf(temp,"0uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0mA-%d.%02dmA", data/100,data%100);
							break;
						case I100mA:
							rt_sprintf(temp,"0mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);

					break;
				case 7:
					
					break;
				case 8:
					
					break;
				case 9:
					
					break;
				
				case 10:
					
					break;
				case 11:
					
					break;
				case 12:
					
					break;
				case 13:
					
					break;
				case 14:
					
					break;
				
				case 15:
					
					break;
				case 16:
					
					break;
				case 17:
					
					break;
				case 18:
					
					break;
			}
			break;
		case DCW:
		case DCW_GR:
			switch(pos)
			{
				case 0:
					
					break;
				case 1:
					
					break;
				case 2:
					
					break;
				case 3:
					
					break;
				case 4:
					data = 2000;
					switch(un->dcw.curgear)
					{
						case I3uA:
							rt_sprintf(temp,"0.001uA-%d.%03duA", data/1000,data%1000);
							break;
						case I30uA:
							rt_sprintf(temp,"0.01uA-%d.%02duA", data/100,data%100);
							break;
						case I300uA:
							rt_sprintf(temp,"0.1uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0.001mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0.01mA-%d.%02dmA", data/200,data%200);
							break;
						case I100mA:
							rt_sprintf(temp,"0.1mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 5:
					data = un->dcw.curhigh-1;
					switch(un->dcw.curgear)
					{
						case I3uA:
							rt_sprintf(temp,"0uA-%d.%03duA", data/1000,data%1000);
							break;
						case I30uA:
							rt_sprintf(temp,"0uA-%d.%02duA", data/100,data%100);
							break;
						case I300uA:
							rt_sprintf(temp,"0uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0mA-%d.%02dmA", data/100,data%100);
							break;
						case I100mA:
							rt_sprintf(temp,"0mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 6:
					data = un->dcw.curhigh;
					switch(un->dcw.curgear)
					{
						case I3uA:
							rt_sprintf(temp,"0uA-%d.%03duA", data/1000,data%1000);
							break;
						case I30uA:
							rt_sprintf(temp,"0uA-%d.%02duA", data/100,data%100);
							break;
						case I300uA:
							rt_sprintf(temp,"0uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0mA-%d.%02dmA", data/200,data%200);
							break;
						case I100mA:
							rt_sprintf(temp,"0mA-%d.%dmA", data/10,data%10);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);

					break;
			}
			break;
		case GR:
			switch(pos)
			{
				case 0:
					
					break;
				case 1:
					
					break;
				case 2:
					
					sprintf(temp,"1-%dmΩ",((float)GR_CUR_MAX / un->gr.outcur * GR_MAX_CUR_RES) > 510 ? 510:(int)((float)GR_CUR_MAX / un->gr.outcur * GR_MAX_CUR_RES));
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 3:
					sprintf(temp,"0-%dmΩ",un->gr.reshigh - 1);
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 4:
					
					break;
				case 5:
					
					break;
				case 6:
					
					break;
			}
			break;
		case LC:
			switch(pos)
			{
				case 0:
					
					break;
				case 1:
					
					break;
				case 2:
					
					break;
				case 3:
					data = 2000;
					switch(un->lc.curgear)
					{
						case I300uA:
							rt_sprintf(temp,"0.1uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0.001mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
	//						data = 1000;
							rt_sprintf(temp,"0.01mA-%d.%02dmA", data/100,data%100);
							break;
						default:
							
							break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
					break;
				case 4:
					data = un->lc.curhigh-1;
					switch(un->lc.curgear)
					{
						case I300uA:
							rt_sprintf(temp,"0uA-%d.%duA", data/10,data%10);
							break;
						case I3mA:
							rt_sprintf(temp,"0mA-%d.%03dmA", data/1000,data%1000);
							break;
						case I30mA:
							rt_sprintf(temp,"0mA-%d.%02dmA", data/100,data%100);
							break;
						default:
							
							break;
					}
				
					
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48+16,help_y,&font,temp);
					break;
			}
			break;
		case IR:
			switch(pos)
			{
				case 0:
					
					break;
				case 1:
					
					break;
				case 2:
						
					break;
				case 3:
				case 4:
					switch(un->ir.autogear)
					{
						case 0:   //AUTO
						{
							rt_sprintf(temp,"0MΩ-100.0GΩ");
						}
						break;
						case 1:  //10MΩ
						{
							rt_sprintf(temp,"0MΩ-10.00MΩ");
						}
						break;
						case 2: //100MΩ
						{
							rt_sprintf(temp,"0MΩ-100.0MΩ");
						}
						break;
						case 3: //1GΩ
						{
							rt_sprintf(temp,"0GΩ-1.000GΩ");
						}
						break;
						case 4: //10GΩ
						{
							rt_sprintf(temp,"0GΩ-10.00GΩ");
						}
						break;	
						case 5: //100GΩ
						{
							rt_sprintf(temp,"0GΩ-100.0GΩ");
						}							
						default:
							
						break;
					}
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
				
					
					break;
				case 5:
					
					break;
				case 6:
					
					break;
			}
		break;
		case PW:	
			switch(pos)
			{
				case 0:
					
				break;
				case 1:
					
				break;
				case 2:
					
				break;
				case 3:
					sprintf(temp,"0.00A - %d.%02dA",un->pw.curhigh / 100,un->pw.curhigh % 100);
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
				break;
				case 4:
					
				break;
				case 5:
					sprintf(temp,"0.000kW - %d.%03dkW",un->pw.pwhigh / 1000,un->pw.pwhigh % 1000);
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
				break;
				case 6:
					
				break;
				case 7:
					
				break;
				case 8:
					sprintf(temp,"0.000 - %d.%03d",un->pw.factorhigh / 1000,un->pw.factorhigh % 1000);
					clr_win(win,0x1f,help_x+48,help_y,16,480-48);
					font_draw(help_x+48,help_y,&font,temp);
				break;
				
				default:
					
				break;
			}
		break;
	}
	return 0;
}


/* 按确定输入版 */
static rt_uint8_t window_step_set(UN_STR *un)
{
	rt_uint32_t msg;
	rt_uint32_t pos=0,pos_max,pos_old=0;
// 	rt_uint8_t  en=0;
	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,330+20,500,90,54+35};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	font.panel = &panel_win;
	clr_mem(panel_win.data,CL_orange,30*panel_win.w);
	clr_mem(panel_win.data+30*panel_win.w,0x1f,(panel_win.h-30)*panel_win.w);
	clr_win(&panel_win,0xffff,0,0,1,500);
	clr_win(&panel_win,0xffff,0,329+20,1,500);
	clr_win(&panel_win,0xffff,0,0,330+20,1);
	clr_win(&panel_win,0xffff,499,0,330+20,1);
// 	font_info_set(&panel_win,0xffff,0x0,1,16);
	font.fontcolor	= 0xf800;
	font_draw(10,7,&font,T_STR("<步骤设置>","<Step Setting>"));
	font.fontcolor	= 0xffff;
	step_set_mode_title(&font,un->com.mode);
	step_set_items(&font,un,0,pos);
	step_set_help(&panel_win,pos,un);
	panel_update(panel_win);
	

	switch(un->com.mode)
	{
		case ACW:
			pos_max = 17;
			break;
		case DCW:
			pos_max = 16;
			break;
		case IR:
			pos_max = 12;
			break;
		case GR:
			pos_max = 12;
			break;
		case LC:
			pos_max = 16;
			break;
		case PW:
			pos_max = 14;
		break;
		case ACW_GR:
			pos_max = 18;
			break;
		case DCW_GR:
			pos_max = 17;
			break;
		case DLA:
			break;
	}
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case CODE_RIGHT:
				case KEY_U | KEY_UP:
					if(pos>0)pos--;
					else pos=pos_max-1;
					step_set_items(&font,un,pos_old,pos);
					step_set_help(&panel_win,pos,un);
					panel_update(panel_win);
					pos_old = pos;
					break;
				case CODE_LEFT:
				case KEY_D | KEY_UP:
					if(++pos>=pos_max)pos=0;
					step_set_items(&font,un,pos_old,pos);
					step_set_help(&panel_win,pos,un);
					panel_update(panel_win);
					pos_old = pos;
					break;
				case KEY_L | KEY_UP:
				case KEY_R | KEY_UP:
					if(pos_max % 2 == 0){
						if(pos < (pos_max/2))
							pos += (pos_max/2);
						else
							pos -= (pos_max/2);
					}else{
						if(pos < (pos_max/2 + 1))
							pos += (pos_max/2 + 1);
						else
							pos -= (pos_max/2 + 1);
						if(pos >= pos_max)pos=pos_max-1;
					}
					step_set_items(&font,un,pos_old,pos);
					step_set_help(&panel_win,pos,un);
					panel_update(panel_win);
					pos_old = pos;
					break;
				case KEY_ENTER | KEY_UP:
					if(pos==0)
					{
						if(step_set_mode(&panel_win,&un->com.mode)){break;}
						switch(un->com.mode)
						{
							case ACW:
								pos_max = 17;
								init_acw_step(un);
								break;
							case DCW:
								pos_max = 16;
								init_dcw_step(un);
								break;
							case IR:
								pos_max = 12;
							  init_ir_step(un);
								break;
							case GR:
								pos_max = 12;
								init_gr_step(un);
								break;
							case LC:
								pos_max = 16;
								init_lc_step(un);
								break;
							case PW:
								pos_max = 14;
								init_pw_step(un);
								break;
							case ACW_GR:
								pos_max = 18;
								init_acw_gr_step(un);
								break;
							case DCW_GR:
								pos_max = 17;
								init_dcw_gr_step(un);
								break;
							case DLA:
								break;
						}
						clr_win(&panel_win,0x1f,1,30,299,498);
						step_set_mode_title(&font,un->com.mode);
						step_set_items(&font,un,0,pos);
						step_set_help(&panel_win,pos,un);
						
						panel_update(panel_win);
						save_steps_to_flash(flash_info.current_file);
						
					}
					else
					switch(un->com.mode)
					{
						case ACW:
							step_set_ACW(&panel_win,&un->acw,pos);
							if(pos==3 || pos==4)
							{
								clr_win(&panel_win,0x1f,112,148,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
						case DCW:
							step_set_DCW(&panel_win,&un->dcw,pos);
							if(pos==3 || pos==4)
							{
								clr_win(&panel_win,0x1f,112,148,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
						case IR:
							step_set_IR(&panel_win,&un->ir,pos);
							if(pos == 2)
							{
								clr_win(&panel_win,0x1f,112,100,150,items_width);
								step_set_items(&font,un,pos,pos);
							}
						
							panel_update(panel_win);	
							save_steps_to_flash(flash_info.current_file);
							break;
						case GR:
							step_set_GR(&panel_win,&un->gr,pos);
							if(pos==1 || pos==2)
							{
								clr_win(&panel_win,0x1f,112,100,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
						case LC:
							step_set_LC(&panel_win,&un->lc,pos);
							if(pos==2 || pos==3)
							{
								clr_win(&panel_win,0x1f,112,100,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
						case PW:
							step_set_PW(&panel_win,&un->pw,pos);
							if(pos==2 || pos==3)
							{
								
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;	
						case ACW_GR:
							step_set_ACW_GR(&panel_win,&un->acw_gr,pos);
							if(pos==3 || pos==4)
							{
								clr_win(&panel_win,0x1f,112,148,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
						case DCW_GR:
							step_set_DCW_GR(&panel_win,&un->dcw_gr,pos);
							if(pos==3 || pos==4)
							{
								clr_win(&panel_win,0x1f,112,148,150,items_width);
								step_set_items(&font,un,pos,pos);
								
							}
							panel_update(panel_win);
							save_steps_to_flash(flash_info.current_file);
							break;
							
						case DLA:
							break;
					}
					file_info[flash_info.current_file].offset_en = 0;
					save_file_to_flash(flash_info.current_file);
					break;
				case KEY_EXIT | KEY_UP:
				case KEY_F6 | KEY_UP:
					return 1;
			}
		}
	}
}

static rt_uint8_t window_network_set(rt_uint8_t id)
{
// 	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,330,500,90,54+35};
	struct panel_type network_win	={(u16 *)ExternSramWinAddr+350*500,330,500,90,54+35};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	u8 err,temp=id;
	rt_uint32_t msg;
	font.panel = &network_win;
	clr_mem(network_win.data,CL_orange,30*network_win.w);
	clr_mem(network_win.data+30*network_win.w,0x1f,(network_win.h-30)*network_win.w);
// 	font_info_set(&network_win,0xffff,0x0,1,16);
	font_draw(10,7,&font,T_STR("<人体网络选择>","<Human Network Select>"));
	switch(id)
	{
		case MD_A:
			break;
		case MD_B:
			break;
		case MD_C:
			break;
		case MD_D:
			break;
		case MD_E:
			if(language)
				err = loadbmpbintosram("/resource/dla/E1.bin",network_win.data + 30*network_win.w);
			else
				err = loadbmpbintosram("/resource/dla/E.bin",network_win.data + 30*network_win.w);
// 			sd_ico_darw(&network_win,0,30,"/resource/dla/E1.bin");
			break;
		case MD_F:
			if(language)
				err = loadbmpbintosram("/resource/dla/F1.bin",network_win.data + 30*network_win.w);
			else
				err = loadbmpbintosram("/resource/dla/F.bin",network_win.data + 30*network_win.w);
			break;
		case MD_G:
			if(language)
				err = loadbmpbintosram("/resource/dla/G1.bin",network_win.data + 30*network_win.w);
			else
				err = loadbmpbintosram("/resource/dla/G.bin",network_win.data + 30*network_win.w);
			break;
		case MD_H:
			break;
	}
// 	if(err != 0)return 1;
	panel_update(network_win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_U | KEY_UP:
					if(temp == MD_G)
					{
						temp=MD_F;
						if(language)
							err = loadbmpbintosram("/resource/dla/F1.bin",network_win.data + 30*network_win.w);
						else
							err = loadbmpbintosram("/resource/dla/F.bin",network_win.data + 30*network_win.w);
					}
					else 
					{
						temp = MD_E;
						if(language)
							err = loadbmpbintosram("/resource/dla/E1.bin",network_win.data + 30*network_win.w);
						else
							err = loadbmpbintosram("/resource/dla/E.bin",network_win.data + 30*network_win.w);
					}
					if(err != 0)return 0xff;
					panel_update(network_win);
					break;
				case KEY_D | KEY_UP:
					if(temp == MD_E)
					{
						temp=MD_F;
						if(language)
							err = loadbmpbintosram("/resource/dla/F1.bin",network_win.data + 30*network_win.w);
						else
							err = loadbmpbintosram("/resource/dla/F.bin",network_win.data + 30*network_win.w);
						
					}
					else 
					{
						temp = MD_G;
						if(language)
							err = loadbmpbintosram("/resource/dla/G1.bin",network_win.data + 30*network_win.w);
						else
							err = loadbmpbintosram("/resource/dla/G.bin",network_win.data + 30*network_win.w);
					}
					if(err != 0)return 0xff;
					panel_update(network_win);
					break;
				case KEY_ENTER | KEY_UP:
// 					font_info_set(&panel_win,0xffff,0x0,1,16);
// 					panel_update(panel_win);
					return (temp | 0x80);
				case KEY_EXIT | KEY_UP:
// 					font_info_set(&panel_win,0xffff,0x0,1,16);
// 					panel_update(panel_win);
					return id;
			}
		}
	}
}


static rt_uint8_t window_medical_set(struct step_lc_t *lc)
{
// 	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,330,500,90,54+35};
	struct panel_type medical_win	={(u16 *)ExternSramWinAddr+350*500,260,450,90+25,54+35+35};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	const char *medical_name[2][13]={
		{"辅助电压:","单一故障:","MD 位置 :","开关 SL :","开关 SH :","开关 S7 :","开关 S8 :","开关 S9 :",   \
		 "开关 S10:","开关 S11:","开关 S12:","开关 S13:","开关 S14:"},
		{"Assist Voltage:","Single fault:","MD Port    :","Switch SL :","Switch SH :","Switch S7 :","Switch S8 :","Switch S9 :",   \
		 "Switch S10:","Switch S11:","Switch S12:","Switch S13:","Switch S14:"}
	};
// 	const char *mdlow_name[2][2]={
// 		{"接地","浮地"},
// 		{""}
// 	};
	rt_uint32_t pos=0,pos_max=13,pos_old=0;
	u32	msg;
	font.panel = &medical_win;
	clr_mem(medical_win.data,0xf800,30*medical_win.w);
	clr_mem(medical_win.data+30*medical_win.w,0x0010,(medical_win.h-30)*medical_win.w);
// 	font_info_set(&medical_win,0xffff,0x0,1,16);
	font_draw(10,7,&font,T_STR("<医用参数设置>","<Medical parameter settings>"));
	{
		u16 x,y,i;
		x=20;
		y=20+30;
		for(i=0;i<7;i++)
		{
			font_draw(x,y,&font,medical_name[language][i]);
			y+=30;
		}
		x = 240;
		y = 20+30;
		for(i=7;i<13;i++)
		{
			font_draw(x,y,&font,medical_name[language][i]);
			y+=30;
		}
	}
	dla_draw_items(&font,lc,0,0);
	
	panel_update(medical_win);
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case CODE_LEFT:
				case KEY_U | KEY_UP:
					if(pos>0)pos--;
					else pos=pos_max-1;
					dla_draw_items(&font,lc,pos_old,pos);
					panel_update(medical_win);
					pos_old = pos;
					break;
				case CODE_RIGHT:
				case KEY_D | KEY_UP:
					if(++pos>=pos_max)pos=0;
					dla_draw_items(&font,lc,pos_old,pos);
					panel_update(medical_win);
					pos_old = pos;
					break;
				case KEY_L | KEY_UP:
				case KEY_R | KEY_UP:
					if(pos < (pos_max/2 + 1))
						pos += (pos_max/2 + 1);
					else
						pos -= (pos_max/2 + 1);
					if(pos >= pos_max)pos = pos_max - 1;
					dla_draw_items(&font,lc,pos_old,pos);
					panel_update(medical_win);
					pos_old = pos;
					break;
				case KEY_ENTER | KEY_UP:
				{
					char buf[20],loop;
					u16 x,y;
					rt_uint32_t		temp;
					struct rect_type rect = {112,48,20,items_width};
					struct font_info_t font = {0,0xffff,0x0000,1,1,16};
					struct num_format num;
					font.panel = &medical_win;
					x=20+72+20;	y=20+30;
					rect.x = x+220*(pos/7);
					rect.y = y+30*(pos%7)-2;
					switch(pos)
					{
						case 0:
							/* 辅助电压 */
							num.num  = lc->assistvol;
							num._int = 3;
							num._dec = 1;
							num.min  = 300;
							num.max  = ASSIST_VOL_MAX;//2500;//3000;
							num.unit = "V";
							
							temp = num_input(&font,&rect,&num);
							if(temp == 0xffffffff)return 1;
							lc->assistvol = temp;
							rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							break;
						case 1:
							/* 单一故障 */
							temp = lc->singlefault;
							ui_text_draw(&font,&rect,(char *)single_boolean_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)single_boolean_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->singlefault = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)single_boolean_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 2:
							/* MD位置 */
							temp = lc->MDpostion;
							rt_sprintf(buf,"MD%d", temp);
							ui_text_draw(&font,&rect,(char *)buf);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
											if(temp>1)temp--;
											else temp=4;
											rt_sprintf(buf,"MD%d", temp);
											ui_text_draw(&font,&rect,(char *)buf);
											break;
										case KEY_D | KEY_UP:
											if(temp<4)temp++;
											else temp=1;
											rt_sprintf(buf,"MD%d", temp);
											ui_text_draw(&font,&rect,(char *)buf);
											break;
										case KEY_ENTER | KEY_UP:
											lc->MDpostion = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)buf);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 3:
							/* 接地、浮地 */
// 							temp = lc->MDlow;
							temp = lc->SL;
							ui_text_draw(&font,&rect,(temp==0?"A":"B"));
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											if(lc->SH == 0)temp = 0;      //douyijun
											ui_text_draw(&font,&rect,(temp==0?"A":"B"));
											break;
										case KEY_ENTER | KEY_UP:
											lc->SL = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(temp==0?"A":"B"));
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 4:
							/* SH */
							temp = lc->SH;
							ui_text_draw(&font,&rect,(temp==0?"A":"B"));
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(temp==0?"A":"B"));
											break;
										case KEY_ENTER | KEY_UP:
											lc->SH = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(temp==0?"A":"B"));
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							if(lc->SH == 0)
							{
								if(lc->SL == 0){

								}else{
									lc->SL = 0;
									x=20+72+20;	y=20+30;
									rect.x = x+220*(3/6);
									rect.y = y+30*(3%6)-2;
									font.backcolor = 0x10;
									ui_text_draw(&font,&rect,"A");
// 									dla_draw_items(&font,lc,3,4);
								}
							}
							break;
							
						case 5:
							/* S7 */
							temp = lc->S7;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S7 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 6:
							/* S8 */
							temp = lc->S8;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S8 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						
							case 7:
							/* S9 */
							temp = lc->S9;
							ui_text_draw(&font,&rect,(char *)sw_status_name_static[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name_static[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S9 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name_static[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;							
						
						case 8:
							/* S10 */
							temp = lc->S10;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S10 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;	
						case 9:
							/* S11 */
							temp = lc->S11;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S11 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 10:
							/* S12 */
							temp = lc->S12;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S12 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 11:
							/* S13 */
							temp = lc->S13;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S13 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
						case 12:
							/* S14 */
							temp = lc->S14;
							ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
							loop = 1;
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(temp!=0)temp=0;
											else temp=1;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											break;
										case KEY_ENTER | KEY_UP:
											lc->S14 = temp;
											font.backcolor = items_backcolor;
											ui_text_draw(&font,&rect,(char *)sw_status_name[language][temp]);
											loop = 0;
										case KEY_EXIT | KEY_UP:
											loop = 0;
									}
								}
							}
							break;
					}
				}
					
					break;
				case KEY_EXIT | KEY_UP:
					return 2;
			}
		}
	}
}

static void dla_draw_items(struct font_info_t *font,struct step_lc_t *lc,rt_uint8_t pos_old,rt_uint8_t pos)
{
	u16 x,y;
	u32 temp;
	char buf[20];
	
	
	x=20+72+20;	y=20+30;
	clr_win(font->panel,0x10     ,x+220*(pos_old/7),y+30*(pos_old%7)-2,20,items_width);
	clr_win(font->panel,CL_orange,x+220*(pos/7)    ,y+30*(pos%7)-2    ,20,items_width);
	x=20+72+20;	y=20+30;
	
	/* 辅助电压 */
	temp = lc->assistvol;
	rt_sprintf(buf,"%d.%dV", temp/10,temp%10);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	/* 单一故障 */
	temp = lc->singlefault;
	font_draw(x+(items_width-rt_strlen(single_boolean_name[language][temp])*8)/2,y,font,single_boolean_name[language][temp]);y+=30;
	/* MD位置 */
	temp = lc->MDpostion;
	rt_sprintf(buf,"MD%d", temp);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	/* SL */
	temp = lc->SL;
	font_draw(x+(items_width-8)/2,y,font,(temp==0?"A":"B"));y+=30;
	/* SH */
	temp = lc->SH;
	font_draw(x+(items_width-8)/2,y,font,(temp==0?"A":"B"));y+=30;
	/* S7 */
	temp = lc->S7;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
		
	/* S8 */
	temp = lc->S8;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
	
	x+= 220;y=20+30;
	/* S9 */
	temp = lc->S9;
	font_draw(x+(items_width-rt_strlen(sw_status_name_static[language][temp])*8)/2,y,font,sw_status_name_static[language][temp]);y+=30;
	/* S10 */
	temp = lc->S10;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
	/* S11 */
	temp = lc->S11;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
	/* S12 */
	temp = lc->S12;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
	/* S13 */
	temp = lc->S13;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
	/* S14 */
	temp = lc->S14;
	font_draw(x+(items_width-rt_strlen(sw_status_name[language][temp])*8)/2,y,font,sw_status_name[language][temp]);y+=30;
}


const char *CW_GR_name[2][13]={
		{"输出电流:","电阻上限:","电阻下限:","报警电压:","输出频率:"},
		{"Out Cur.:","Res.High:","Res.Low :","AlarmVol:","Frequence:"}
	};
static void cw_gr_draw_items(struct font_info_t *font,struct step_cw_gr_t *gr,rt_uint8_t pos_old,rt_uint8_t pos)
{
	u16 x,y;
	u32 temp;
	char buf[20];
	
	
	x=20+72+20;	y=20+30;
	clr_win(font->panel,0x10     ,x+220*(pos_old/3),y+30*(pos_old%3)-2,20,items_width);
	clr_win(font->panel,CL_orange,x+220*(pos/3)    ,y+30*(pos%3)-2    ,20,items_width);
	x=20+72+20;	y=20+30;
	
	/* 辅助电流 */
	temp = gr->groutcur;
	rt_sprintf(buf,"%d.%dA", temp/10,temp%10);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	/* 电阻上限 */
	temp = gr->grreshigh;
	rt_sprintf(buf,"%dmΩ", temp);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	/* 电阻下限 */
	temp = gr->grreslow;
	rt_sprintf(buf,"%dmΩ", temp);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	
	x+= 220;y=20+30;
	/* 报警电压 */
	temp = gr->gralarmvol;
	rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	/* 输出频率 */
	temp = gr->groutfreq;
	rt_sprintf(buf,"%d.%dHz", temp/10,temp%10);
	font_draw(x+(items_width-rt_strlen(buf)*8)/2,y,font,buf);y+=30;
	
}
static rt_uint8_t window_CW_GR_set(struct step_cw_gr_t *cw_gr)
{
	
// 	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,330,500,90,54+35};
	struct panel_type CW_GR_win	={(u16 *)ExternSramWinAddr+350*500,260,450,90+25,54+35+35};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	

	rt_uint32_t pos=0,pos_max=5,pos_old=0;
	u32	msg;
	font.panel = &CW_GR_win;
	clr_mem(CW_GR_win.data,0xf800,30*CW_GR_win.w);
	clr_mem(CW_GR_win.data+30*CW_GR_win.w,0x0010,(CW_GR_win.h-30)*CW_GR_win.w);
	font_draw(10,7,&font,T_STR("<接地电阻参数设置>","<GR Parameter seting>"));
	{
		u16 x,y,i;
		x=20;
		y=20+30;
		for(i=0;i<3;i++)
		{
			font_draw(x,y,&font,CW_GR_name[0][i]);
			y+=30;
		}
		x = 240;
		y = 20+30;
		for(i=3;i<5;i++)
		{
			font_draw(x,y,&font,CW_GR_name[0][i]);
			y+=30;
		}
	}
	cw_gr_draw_items(&font,cw_gr,0,0);
	
	panel_update(CW_GR_win);
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case CODE_LEFT:
				case KEY_U | KEY_UP:
					if(pos>0)pos--;
					else pos=pos_max-1;
					cw_gr_draw_items(&font,cw_gr,pos_old,pos);
					panel_update(CW_GR_win);
					pos_old = pos;
					break;
				case CODE_RIGHT:
				case KEY_D | KEY_UP:
					if(++pos>=pos_max)pos=0;
					cw_gr_draw_items(&font,cw_gr,pos_old,pos);
					panel_update(CW_GR_win);
					pos_old = pos;
					break;
				case KEY_L | KEY_UP:
				case KEY_R | KEY_UP:
					if(pos < (pos_max/2 + 1))
						pos += (pos_max/2 + 1);
					else
						pos -= (pos_max/2 + 1);
					if(pos >= pos_max)pos = pos_max - 1;
					cw_gr_draw_items(&font,cw_gr,pos_old,pos);
					panel_update(CW_GR_win);
					pos_old = pos;
					break;
				case KEY_ENTER | KEY_UP:
				{
					char buf[20],loop;
					u16 x,y;
					rt_uint32_t		temp;
					struct rect_type rect = {112,48,20,items_width};
					struct font_info_t font = {0,0xffff,0x0000,1,1,16};
					struct num_format num;
					font.panel = &CW_GR_win;
					x=20+72+20;	y=20+30;
					rect.x = x+220*(pos/3);
					rect.y = y+30*(pos%3)-2;
					
					switch(pos)
					{
						case 0:
							/* 输出电流 */
						num.num  = cw_gr->groutcur;
						num._int = 2;
						num._dec = 1;
						num.min  = 30;
						num.max  = 320;
						num.unit = "A";
						
						temp = num_input(&font,&rect,&num);
						if(temp != 0xffffffff)
						cw_gr->groutcur = temp;
						else temp = cw_gr->groutcur;
						rt_sprintf(buf,"%d.%dA", temp/10,temp%10);
						font.backcolor = items_backcolor;
						ui_text_draw(&font,&rect,buf);
						//douyijun  更新电阻上下限
						
						{
							rt_uint32_t max_res = (float)320 / cw_gr->groutcur * 150;
							if(cw_gr->grreshigh > max_res){
								cw_gr->grreshigh = max_res;
								rt_sprintf(buf,"%dmΩ", cw_gr->grreshigh);
								x=20+72+20;
								y=20+30;
								rect.x = x+240*(1/3);
								rect.y = y+30*(1%3)-2;
								font.backcolor = 0x1f;
								ui_text_draw(&font,&rect,buf);
							}
							if(cw_gr->grreslow > (max_res-1)){
								cw_gr->grreslow = max_res - 1;
								rt_sprintf(buf,"%dmΩ", cw_gr->grreslow);
								x=20+72+20;
								y=20+30;
								rect.x = x+240*(2/3);
								rect.y = y+30*(2%3)-2;
								font.backcolor = 0x1f;
								ui_text_draw(&font,&rect,buf);
							}
						}
						
						
					
						break;
						
						case 1:
							/* 电阻上限 */
							num.num  = cw_gr->grreshigh;
							num._int = 3;
							num._dec = 0;
							num.min  = 1;
							num.max  = (float)320 / cw_gr->groutcur * 150;
							if(num.max > 510)num.max = 510;
							num.unit = "mΩ";
							
							temp = num_input(&font,&rect,&num);
							if(temp != 0xffffffff)
							cw_gr->grreshigh = temp;
							else temp = cw_gr->grreshigh;
							rt_sprintf(buf,"%dmΩ", temp);
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
							
							//douyijun  更新电阻下限
							
							{

								if(cw_gr->grreslow > (cw_gr->grreshigh - 1)){
									cw_gr->grreslow = (cw_gr->grreshigh - 1);
									rt_sprintf(buf,"%dmΩ", cw_gr->grreslow);
									x=20+72+20;
									y=20+30;
									rect.x = x+240*(2/3);
									rect.y = y+30*(2%3)-2;
									font.backcolor = 0x1f;
									ui_text_draw(&font,&rect,buf);
								}
							}
						break;
							
						case 2:
							/* 电阻下限 */
							num.num  = cw_gr->grreslow;
							num._int = 3;
							num._dec = 0;
							num.min  = 0;
							num.max  = cw_gr->grreshigh - 1;
							num.unit = "mΩ";
							
							temp = num_input(&font,&rect,&num);
							if(temp != 0xffffffff)
							cw_gr->grreslow = temp;
							else temp = cw_gr->grreslow;
							rt_sprintf(buf,"%dmΩ", temp);
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
						break;
							
						case 3:
							/* 输出电压 */
							num.num  = cw_gr->gralarmvol;
							num._int = 1;
							num._dec = 2;
							num.min  = 0;
							num.max  = 600;
							num.unit = "V";
							
							temp = num_input(&font,&rect,&num);
							if(temp != 0xffffffff)
							cw_gr->gralarmvol = temp;
							else temp = cw_gr->gralarmvol;
							rt_sprintf(buf,"%d.%02dV", temp/100,temp%100);
							font.backcolor = items_backcolor;
							ui_text_draw(&font,&rect,buf);
						break;
							
						case 4:
							loop = 1;
							/* 输出频率 */
							rt_sprintf(buf,"%d.%dHz", cw_gr->groutfreq/10,cw_gr->groutfreq%10);
							ui_text_draw(&font,&rect,(char *)buf);
							while(loop)
							{
								if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
								{
									switch(msg)
									{
										case KEY_U | KEY_UP:
										case KEY_D | KEY_UP:
											if(cw_gr->groutfreq != 500)
											{
												cw_gr->groutfreq = 500;
											}
											else
											{
												cw_gr->groutfreq = 600;
											}
											rt_sprintf(buf,"%d.%dHz", cw_gr->groutfreq/10,cw_gr->groutfreq%10);
											ui_text_draw(&font,&rect,(char *)buf);
											break;
										
		
										case KEY_ENTER | KEY_UP:
											font.backcolor = items_backcolor;
											rt_sprintf(buf,"%d.%dHz", cw_gr->groutfreq/10,cw_gr->groutfreq%10);
											ui_text_draw(&font,&rect,(char *)buf);	
											loop = 0;
										case KEY_EXIT | KEY_UP:
											font.backcolor = items_backcolor;
											rt_sprintf(buf,"%d.%dHz", cw_gr->groutfreq/10,cw_gr->groutfreq%10);
											ui_text_draw(&font,&rect,(char *)buf);
											loop = 0;
									}
								}
							}
						break;
						
						default:
							
						break;
					}
				

				}
					
					break;
				case KEY_EXIT | KEY_UP:
					return 2;
			}
		}
	}
}
