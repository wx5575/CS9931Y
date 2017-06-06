#include "CS99xx.h"
#include "bsp_listbox.h"
#include "memory.h"
#include "memorymanagement.h"
#include "sui_window.h"

static void result_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y);
u8 usb_mem_flag = 0;

static u8 result_detail(void *arg);
static u8 result_delete(void *arg);
static u8 result_Stat(void *arg);
static u8 result_export(void *arg);
static u8 result_jump(void *arg);

struct 
{
    u16 x;
	u16 y;
	const char *data_en;
	const char *data_ch;
}result_title_name[5]={
	{10,10,"NO.","编号"},{100,10,"Mode","测试模式"},{230,10,"Error","错误项目"},{360,10,"Result","测试结果"},{490,10,"Time","记录时间"},
};

static struct rect_type list_box_rect={10,40,26,660};
struct rtgui_listctrl result_list_box={
	&panel_home,
	13,
	8000,
	0,0,
	&list_box_rect,
	result_item_draw
};



void ui_result_thread(void)
{
	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};

// __system_loop:
	if(panel_flag == 4)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
// 		rt_memcpy((void *)ExternSramHomeAddr,(void *)ExternSramNullAddr,ExternSramHomeSize);
		/* 标题 */
		for(i=0;i<5;i++)
		{
			/* 语言判断 */
			if(language==1)
				font_draw(result_title_name[i].x,result_title_name[i].y,&font,result_title_name[i].data_en);
			else
				font_draw(result_title_name[i].x,result_title_name[i].y,&font,result_title_name[i].data_ch);
		}	
		result_list_box.current_item = 0;
		result_list_box.start_item = 0;
		listbox_draw(&result_list_box);
		rt_mb_send(&screen_mb, UPDATE_HOME);
		ui_key_updata(0);
	}
	while(panel_flag == 4)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 详细 */
				case KEY_F1 | KEY_UP:
				case KEY_ENTER | KEY_UP:
					result_detail((void *)&(result_list_box.current_item));
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 删除 */
				case KEY_F2 | KEY_UP:
					result_delete(0);
					result_list_box.current_item = 0;
					result_list_box.start_item = 0;
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 统计 */
				case KEY_F3 | KEY_UP:
					result_Stat(0);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 导出 */
				case KEY_F4 | KEY_UP:
					usb_mem_flag = 1;
					result_export(0);
					usb_mem_flag = 0;
					rt_mb_send(&screen_mb, UPDATE_HOME);
// 				{
// 					struct result_info_t temp;
// 					strcpy(temp.name,"file");
// 					temp.mode = 0;
// 					temp.error = 1;
// 					temp.time.year = 2014;
// 					temp.time.month = 2;
// 					temp.time.date = 2;
// 					temp.time.hours = 11;
// 					temp.time.minutes = 20;
// 					temp.time.seconds = 45;
// 					memory_result_write(&temp);
// 				}
					break;
				/* 跳转 */
				case KEY_F5 | KEY_UP:
					result_jump(0);
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 返回 */
				case KEY_F6 | KEY_UP:
					panel_flag = 0;
					break;

				
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(result_list_box.current_item>0)result_list_box.current_item --;
					result_list_box.start_item = (result_list_box.current_item / result_list_box.items_count)*result_list_box.items_count;
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(result_list_box.current_item<result_list_box.total_items-1)result_list_box.current_item ++;
					result_list_box.start_item = (result_list_box.current_item / result_list_box.items_count)*result_list_box.items_count;
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					break;
				case KEY_L | KEY_DOWN:
					if(result_list_box.current_item >= result_list_box.items_count)
						result_list_box.current_item -= result_list_box.items_count;
					result_list_box.start_item = (result_list_box.current_item / result_list_box.items_count)*result_list_box.items_count;
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
					
					break;
				case KEY_R | KEY_DOWN:
					result_list_box.current_item += result_list_box.items_count;
					if(result_list_box.current_item>result_list_box.total_items-1)result_list_box.current_item=result_list_box.total_items-1;
					result_list_box.start_item = (result_list_box.current_item / result_list_box.items_count)*result_list_box.items_count;
					listbox_draw(&result_list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
				
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

static void result_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y)
{
	char buf[20];
	static struct result_info_t *temp;
	/* 显示编号 */
	rt_sprintf(buf,"%04d",index+1);
	font_draw(x+8,y,font,buf);
	
	temp = memory_result_read(index);
	if(temp != NULL)
	{
		/* 模式名字 */
		font_draw(x+90+(80-rt_strlen(mode_name[temp->mode])*8)/2,y,font,mode_name[temp->mode]);
		/* 错误项目 */
		font_draw(x+200+(120-rt_strlen(error_name[language][temp->error])*8)/2,y,font,error_name[language][temp->error]);
		/* 测试结果 */
		font_draw(x+330+(120-rt_strlen("PASS")*8)/2,y,font,(temp->error == PASS? "PASS":"FAIL"));
		/* 记录时间 */
		rt_sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",temp->time.year+2000,temp->time.month,temp->time.date,temp->time.hours,temp->time.minutes,temp->time.seconds);
		font_draw(x+480+(120-rt_strlen(buf)*8)/2,y,font,buf);
	}
}

static u8 result_detail(void *arg)
{
	rt_uint32_t msg;
	char buf[20];
	struct panel_type *win;
	struct rect_type rect={140,105,290,400};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};
	struct result_info_t *temp;

	msg = *(rt_int16_t *)arg;
	temp = memory_result_read(msg);
	if(temp == NULL)
		return 1;
	win = sui_window_create(T_STR("结果详情","Results details"),&rect);
	font.panel = win;
	
#define		_XOFFSET1	10
#define		_XOFFSET2	210
#define		_XSIZE		80
#define		_YOFFSET	30

	rt_sprintf(buf,"%d",msg+1);
	font_draw(_XOFFSET1,_YOFFSET+10,&font,T_STR("结果编号:","ResultNum:"));		font_draw(_XOFFSET1+_XSIZE,_YOFFSET+10,&font,buf);
	font_draw(_XOFFSET1,_YOFFSET+30,&font,T_STR("文 件 名:","File Name:")); 	font_draw(_XOFFSET1+_XSIZE,_YOFFSET+30,&font,temp->name);
	font_draw(_XOFFSET1,_YOFFSET+50,&font,T_STR("测试模式:","Test Mode:"));		font_draw(_XOFFSET1+_XSIZE,_YOFFSET+50,&font,mode_name[temp->mode]);
	
	switch(temp->mode)
	{
		case ACW:
			rt_sprintf(buf,"%d.%03dkV",temp->s_voltage/1000,temp->s_voltage%1000);
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("输出电压:","OutputV :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			switch(temp->s_gear){
				
				case I3uA:
					rt_sprintf(buf,"%d.%03duA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->current/1000,temp->current%1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->currents/1000,temp->currents%1000);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
				break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->current/100,temp->current%100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->currents/100,temp->currents%100);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
				break;
				
				
				case I300uA:
					rt_sprintf(buf,"%d.%duA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%duA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%duA",temp->current/10,temp->current%10);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%duA",temp->currents/10,temp->currents%10);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
					
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->current/1000,temp->current%1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->currents/1000,temp->currents%1000);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->current/100,temp->current%100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->currents/100,temp->currents%100);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%dmA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%dmA",temp->current/10,temp->current%10);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%dmA",temp->currents/10,temp->currents%10);
					font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
					break;
				default:
					break;
			}
			
			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","File Step:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
// 			rt_sprintf(buf,"%d.%03dmA",temp->s_current/1000,temp->s_current%1000);
// 			font_draw(_XOFFSET2,_YOFFSET+50,&font,"真实电流:");font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
			rt_sprintf(buf,T_STR("等级%d","Rank%d"),temp->s_arc);
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("电弧侦测:","ARC      :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","OutputFre:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","Test Time:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%d.%03dkV",temp->voltage/1000,temp->voltage%1000);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","Test Time:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","ErrorInfo:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);
			
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
			
			break;
		case DCW:
			rt_sprintf(buf,"%d.%03dkV",temp->s_voltage/1000,temp->s_voltage%1000);
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("输出电压:","OutputVol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			switch(temp->s_gear){
				case I3uA:
					rt_sprintf(buf,"%d.%03duA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->current/1000,temp->current%1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"-.---uA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
				break;
				case I30uA:
					rt_sprintf(buf,"%d.%02duA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->current/100,temp->current%100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"--.--uA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
				break;
			
				case I300uA:
					rt_sprintf(buf,"%d.%duA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   : "));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%duA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%duA",temp->current/10,temp->current%10);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"---.-uA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
					
					break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->current/1000,temp->current%1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"-.---mA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
					break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->current/100,temp->current%100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"--.--mA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur  :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
					break;
				case I100mA:
					rt_sprintf(buf,"%d.%dmA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up   :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%dmA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
					rt_sprintf(buf,"%d.%dmA",temp->current/10,temp->current%10);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					strcpy(buf,"---.-mA");
					font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
					break;
				default:
					
					break;
			}
			
		
			

			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","File Step:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
			
			rt_sprintf(buf,"等级%d",temp->s_arc);
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("电弧侦测:","ARC      :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","OutputFre:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,"---");
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","Test Time:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%d.%03dkV",temp->voltage/1000,temp->voltage%1000);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","Test Time:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","ErrorInfo:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);		
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
			font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,"---");
			break;
		case IR:
		{
			uint32_t res;
			const char *gear_name[] = {"AUTO","10MΩ","100MΩ","1GΩ","10GΩ"};
			res = temp->resister;
			if(res<10000)//0~100M
			{
				rt_sprintf(buf,"%d.%02dMΩ", res/100,res%100);
			}else if(res<100000)//100M~1G
			{
				res = res / 10;											
				rt_sprintf(buf,"%d.%01dMΩ", res/10,res%10);
			}else if(res<1000000)//1G~10G
			{
				res = res / 100;											
				rt_sprintf(buf,"%d.%03dGΩ", res/1000,res%1000);
			}
			else if(res<10000000)//10G~100G
			{
				res = res / 1000;											
				rt_sprintf(buf,"%d.%02dGΩ", res/100,res%100);
			}
			else 
			{
				res = res / 10000;											
				rt_sprintf(buf,"%d.%01dGΩ", res/10,res%10);
			}
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("测试电阻:","Test Res:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			
			res = temp->s_hightlimit;
			if(res<10000)//0~100M
			{
				rt_sprintf(buf,"%d.%02dMΩ", res/100,res%100);
			}else if(res<100000)//100M~1G
			{
				res = res / 10;											
				rt_sprintf(buf,"%d.%01dMΩ", res/10,res%10);
			}else if(res<1000000)//1G~10G
			{
				res = res / 100;											
				rt_sprintf(buf,"%d.%03dGΩ", res/1000,res%1000);
			}
			else if(res<10000000)//10G~100G
			{
				res = res / 1000;											
				rt_sprintf(buf,"%d.%02dGΩ", res/100,res%100);
			}
			else 
			{
				res = res / 10000;											
				rt_sprintf(buf,"%d.%01dGΩ", res/10,res%10);
			}
			font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电阻上限:","Res  Up: "));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
			
			res = temp->s_lowlimit;
			if(res<10000)//0~100M
			{
				rt_sprintf(buf,"%d.%02dMΩ", res/100,res%100);
			}else if(res<100000)//100M~1G
			{
				res = res / 10;											
				rt_sprintf(buf,"%d.%01dMΩ", res/10,res%10);
			}else if(res<1000000)//1G~10G
			{
				res = res / 100;											
				rt_sprintf(buf,"%d.%03dGΩ", res/1000,res%1000);
			}
			else if(res<10000000)//10G~100G
			{
				res = res / 1000;											
				rt_sprintf(buf,"%d.%02dGΩ", res/100,res%100);
			}
			else 
			{
				res = res / 10000;											
				rt_sprintf(buf,"%d.%01dGΩ", res/10,res%10);
			}
			font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电阻下限:","Res Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);

			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","FileStep:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
//			rt_sprintf(buf,"-.--A");
			font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("输出电流:","OutputCu:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,"-.--A");
// 			rt_sprintf(buf,"等级%d",temp->s_arc);
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("测试档位:","TestGear:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,gear_name[temp->s_gear]);
//			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","Out Rate:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,"---");
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%d.%03dV",temp->voltage/1000,temp->voltage%1000);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","ErroInfo:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);
			
//			rt_sprintf(buf,"%d.%03dA",temp->current/1000,temp->Cur%1000);
			font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,"-.--A");
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
// 			rt_sprintf(buf,"%d.%03dmA",temp->currents/1000,temp->currents%1000);
			font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur: "));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,"---");
			break;
		}
		case GR:
            if(temp->resister == 0xffff)
            {
                rt_sprintf(buf,">510mΩ");
            }
            else
            {
                rt_sprintf(buf,"%d.%dmΩ",temp->resister/10,temp->resister%10);
            }
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("测试电阻:","Test Res:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%dmΩ",temp->s_hightlimit);
			font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电阻上限:","Res Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%dmΩ",temp->s_lowlimit);
			font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电阻下限:","Res Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);

			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","FileStep:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
			rt_sprintf(buf,"%d.%01dA",temp->s_current/10,temp->s_current%10);
			font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("输出电流:","Out Cur :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
// 			rt_sprintf(buf,"等级%d",temp->s_arc);
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("电弧侦测:","ARC     :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,"---");
			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","Out Rate:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%dmV",temp->voltage);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","Err Info:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);
			
			rt_sprintf(buf,"%d.%03dA",temp->current/1000,temp->current%1000);
			font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
// 			rt_sprintf(buf,"%d.%03dmA",temp->currents/1000,temp->currents%1000);
			font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,"---");
			break;
			
		case PW:
 			rt_sprintf(buf,"%d.%01dV",temp->s_voltage/10,temp->s_voltage%10);
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("设置电压:","Set Vol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%02dA",temp->s_hightlimit);
			font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%d.%02dA",temp->s_lowlimit);
			font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);

			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","FileStep:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
			rt_sprintf(buf,"%d.%03dkW",temp->s_powerhigh/1000,temp->s_powerhigh%1000);
			font_draw(_XOFFSET2,_YOFFSET+50,&font,T_STR("功率上限:","Power Up:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
			rt_sprintf(buf,"%d.%03dkW",temp->s_powerlow/1000,temp->s_powerlow%1000);
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("功率下限:","Pow Down:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","Out Rate:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%d.%01dV",temp->voltage/10,temp->voltage%10);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","Err Info:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);
			
			rt_sprintf(buf,"%d.%02dA",temp->current/1000,temp->current%1000);
			font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
 			rt_sprintf(buf,"%d.%03dkW",temp->power/1000,temp->power%1000);
			font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("测试功率:","TestPwer:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,buf);
			break;
		case LC:
			rt_sprintf(buf,"%d.%dV",temp->s_voltage/10,temp->s_voltage%10);
			font_draw(_XOFFSET1,_YOFFSET+70,&font,T_STR("输出电压:","Out Vol :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%dV",temp->s_current/10,temp->s_current%10);
			font_draw(_XOFFSET2,_YOFFSET+50,&font,"辅助电压:");font_draw(_XOFFSET2+_XSIZE,_YOFFSET+50,&font,buf);
		
			

			rt_sprintf(buf,"%d/%d",temp->current_step,temp->total_step);
			font_draw(_XOFFSET2,_YOFFSET+30,&font,T_STR("文件步骤:","FileStep:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+30,&font,buf);
			
// 			rt_sprintf(buf,"等级%d",temp->s_arc);
			rt_sprintf(buf,"---");
			font_draw(_XOFFSET2,_YOFFSET+70,&font,T_STR("电弧侦测:","ARC     :"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+70,&font,buf);
			rt_sprintf(buf,"%d.%dHz",temp->s_outfreq/10,temp->s_outfreq%10);
			font_draw(_XOFFSET2,_YOFFSET+90,&font,T_STR("输出频率:","Out Rate:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+90,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->s_testtime/10,temp->s_testtime%10);
			font_draw(_XOFFSET2,_YOFFSET+110,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+110,&font,buf);
			
			/**************************************************************************************************************/
			rt_sprintf(buf,"%d.%dV",temp->voltage/10,temp->voltage%10);
			font_draw(_XOFFSET1,_YOFFSET+140,&font,T_STR("测试电压:","Test Vol:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+140,&font,buf);
			rt_sprintf(buf,"%d.%ds",temp->testtime/10,temp->testtime%10);
			font_draw(_XOFFSET1,_YOFFSET+160,&font,T_STR("测试时间:","TestTime:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+160,&font,buf);
			font_draw(_XOFFSET1,_YOFFSET+180,&font,T_STR("错误信息:","Err Info:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+180,&font,error_name[language][temp->error]);
// 			temp->current = temp->current * 1.414;
			switch(temp->s_gear){
				case I3uA:
					rt_sprintf(buf,"%d.%03duA",temp->current / 1000,temp->current % 1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur  Up :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03duA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				case I30uA:
					rt_sprintf(buf,"%d.%02duA",temp->current / 100,temp->current % 100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Curr Up :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02duA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				case I300uA:
					rt_sprintf(buf,"%d.%01duA",temp->current / 10,temp->current % 10);	
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%01duA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%01duA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				break;
				case I3mA:
					rt_sprintf(buf,"%d.%03dmA",temp->current / 1000,temp->current % 1000);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->s_hightlimit/1000,temp->s_hightlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur  Up :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%03dmA",temp->s_lowlimit/1000,temp->s_lowlimit%1000);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				break;
				case I30mA:
					rt_sprintf(buf,"%d.%02dmA",temp->current / 100,temp->current % 100);
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->s_hightlimit/100,temp->s_hightlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%02dmA",temp->s_lowlimit/100,temp->s_lowlimit%100);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				break;
				case I100mA:
					rt_sprintf(buf,"%d.%01dmA",temp->current / 10,temp->current % 10);	
					font_draw(_XOFFSET2,_YOFFSET+140,&font,T_STR("测试电流:","Test Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+140,&font,buf);
					rt_sprintf(buf,"%d.%01dmA",temp->s_hightlimit/10,temp->s_hightlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+90,&font,T_STR("电流上限:","Cur Up  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+90,&font,buf);
					rt_sprintf(buf,"%d.%01dmA",temp->s_lowlimit/10,temp->s_lowlimit%10);
					font_draw(_XOFFSET1,_YOFFSET+110,&font,T_STR("电流下限:","Cur Down:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+110,&font,buf);
				break;
				default:	
				break;
			}
			
			font_draw(_XOFFSET2,_YOFFSET+160,&font,T_STR("测试结果:","Test Res:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+160,&font,(temp->error == PASS? "PASS":"FAIL"));
// 			rt_sprintf(buf,"%d.%03dmA",temp->currents/1000,temp->currents%1000);
			font_draw(_XOFFSET2,_YOFFSET+180,&font,T_STR("真实电流:","Real Cur:"));font_draw(_XOFFSET2+_XSIZE,_YOFFSET+180,&font,"---");
			break;
	}
	
	/**************************************************************************************************************/
	rt_sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",temp->time.year+2000,temp->time.month,temp->time.date,temp->time.hours,temp->time.minutes,temp->time.seconds);
	font_draw(_XOFFSET1,_YOFFSET+210,&font,T_STR("记录时间:","RecrdTime:"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+210,&font,buf);
	font_draw(_XOFFSET1,_YOFFSET+230,&font,T_STR("条形码  :","Barcode  :"));font_draw(_XOFFSET1+_XSIZE,_YOFFSET+230,&font,temp->_bar_code);
		
#undef		_XOFFSET1	
#undef		_XOFFSET2	
#undef		_XSIZE		
#undef		_YOFFSET		
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
				case KEY_F6   | KEY_UP:
				case KEY_EXIT | KEY_UP:
					return 0;
			}
		}
	}
}
static u8 result_delete(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0xf000,0XE73C,1,0,16};
	struct rect_type rects={25,100,20,250};
	rt_uint32_t x=0;
		
	win = sui_window_create(T_STR("结果删除","Delete Results:"),&rect);
	font.panel = win;
	font_draw(94,44,&font,T_STR("确定格式化吗？","Ensure Esvaziar ?"));
	font_draw(26,68,&font,T_STR("提示:格式化将删除所有结果信息。","Tips: Esvaziar will Delete all"));
	clr_win(win,0XC618,25,100,20,250);

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	font_draw(64,152,&font,T_STR("确定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return")); 
	
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_UP:
			
					while(x<250)
					{
						
						clr_win(win,CL_BLUE,25+x,100,20,1);
						window_updata(win,&rects);
						x++;
						
//						rt_thread_delay(1);
					}
					while(x<250)
					{
						clr_win(win,CL_BLUE,25+x,100,20,1);
						window_updata(win,&rects);
						x++;
					}
				
					memory_result_delete();
					rt_thread_delay(100);
					return 0;
//				case KEY_ENTER | KEY_UP:
//					FLASH_CS_SET(4);	// 选择参数flash
//					sf_EraseChip();
//					while(sf_wait_find() != 0)
//					{
//						if(x<250)
//						{
//							clr_win(win,CL_BLUE,25+x,100,20,1);
//							window_updata(win,&rects);
//							x++;
//						}
//						rt_thread_delay(RT_TICK_PER_SECOND/5);
//					}
//					while(x<250)
//					{
//						clr_win(win,CL_BLUE,25+x,100,20,1);
//						window_updata(win,&rects);
//						x++;
//					}
//					sf_wait_find_dis();
//					memory_result_delete();
//					rt_thread_delay(RT_TICK_PER_SECOND/2);
//					return 0;
				/* 返回 */
				case KEY_EXIT | KEY_UP:
				case KEY_F6 | KEY_UP:
					return 0;
			}
		}
	}
}
static u8 result_Stat(void *arg)
{
	rt_uint32_t msg;
	char buf[10];
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};
    rt_uint32_t paas_rate;///<合格率
    rt_uint32_t used_rate;///<使用率
    rt_uint32_t fail_rate;///<失败率

	win = sui_window_create(T_STR("结果统计","Results statistics"),&rect);
	font.panel = win;

#define		_XOFFSET	10
#define		_YOFFSET	30
	font_draw(_XOFFSET,_YOFFSET+10,&font,T_STR("测试结果统计:","Test statistics: "));
	font_draw(_XOFFSET,_YOFFSET+30,&font,T_STR("·测试次数:","·Test Time:"));
	font_draw(_XOFFSET,_YOFFSET+50,&font,T_STR("·成功次数:","·Succ Time:"));
font_draw(150+_XOFFSET,_YOFFSET+50,&font,T_STR("·成功率:","·Rate  :"));
	font_draw(_XOFFSET,_YOFFSET+70,&font,T_STR("·失败次数:","·Fail Time:"));
font_draw(150+_XOFFSET,_YOFFSET+70,&font,T_STR("·失败率:","·Rate  :"));

	font_draw(_XOFFSET,_YOFFSET+100,&font,T_STR("存储器统计:","Storage statistics: "));
		font_draw(_XOFFSET,_YOFFSET+120,&font,T_STR("·总 容 量:","·Total   :"));
	font_draw(150+_XOFFSET,_YOFFSET+120,&font,T_STR("·已使用:","·Using   :"));
		font_draw(_XOFFSET,_YOFFSET+140,&font,T_STR("·使 用 率:","·Use rate:"));
	font_draw(150+_XOFFSET,_YOFFSET+140,&font,T_STR("·余  量:","·Margin  :"));


#define		_NULLOFFSET1	100
#define		_NULLOFFSET2	80+150
    paas_rate = result_headinfo.pass_count * 1000.0 / result_headinfo.test_count;
    used_rate = result_headinfo.test_count * 1000.0 / result_headinfo.total_size;
    fail_rate = 1000 - paas_rate;
	rt_sprintf(buf,"%d",result_headinfo.test_count);	font_draw(_XOFFSET+_NULLOFFSET1,_YOFFSET+30,&font,buf);
	rt_sprintf(buf,"%d",result_headinfo.pass_count);	font_draw(_XOFFSET+_NULLOFFSET1,_YOFFSET+50,&font,buf);
	rt_sprintf(buf,"%d",result_headinfo.test_count-result_headinfo.pass_count);	font_draw(_XOFFSET+_NULLOFFSET1,_YOFFSET+70,&font,buf);
	if(result_headinfo.test_count != 0)
	{
        rt_sprintf(buf,"%d.%d%",paas_rate / 10, paas_rate % 10);//合格率
		
// 		rt_sprintf(buf,"%d.%d%",(result_headinfo.pass_count*100)/result_headinfo.test_count,((result_headinfo.pass_count*1000)/result_headinfo.test_count)%10);
		font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+50,&font,buf);
        
        rt_sprintf(buf,"%d.%d%",fail_rate / 10, fail_rate % 10);//失败率
// 		rt_sprintf(buf,"%d.%d%",((result_headinfo.test_count-result_headinfo.pass_count)*100)/result_headinfo.test_count,(((result_headinfo.test_count-result_headinfo.pass_count)*1000)/result_headinfo.test_count)%10);
		font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+70,&font,buf);
	}
	else
	{
		font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+50,&font,"0.0%");
		font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+70,&font,"0.0%");
	}
	
	rt_sprintf(buf,"%d",result_headinfo.total_size);	font_draw(_XOFFSET+_NULLOFFSET1,_YOFFSET+120,&font,buf);
	rt_sprintf(buf,"%d",result_headinfo.current_numb);	font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+120,&font,buf);
	rt_sprintf(buf,"%d",result_headinfo.total_size-result_headinfo.current_numb);	font_draw(_XOFFSET+_NULLOFFSET2,_YOFFSET+140,&font,buf);
    
    rt_sprintf(buf,"%d.%d%",used_rate / 10, used_rate % 10);//合格率
// 	rt_sprintf(buf,"%d.%d%",(result_headinfo.current_numb*100)/result_headinfo.total_size,((result_headinfo.current_numb*1000)/result_headinfo.total_size)%10);
	font_draw(_XOFFSET+_NULLOFFSET1,_YOFFSET+140,&font,buf);
#undef		_XOFFSET
#undef		_YOFFSET
#undef		_NULLOFFSET1
#undef		_NULLOFFSET2
	
	sui_window_update(win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 返回 */
				case KEY_EXIT | KEY_UP:
				case KEY_F6   | KEY_UP:
					return 0;
			}
		}
	}
}
#include  "CH376_USB_UI.H"
#include "rtc.h"

uint8_t set_file_create_time(uint8_t* file_name, uint16_t date, uint16_t time)
{
	FAT_DIR_INFO fdi;
	uint8_t res = 0;
	
	res = CH376FileOpenPath((uint8_t*)file_name);/* 打开文件 */
	
	if(0x14 != res)
	{
		return 0xff;
	}
	
	//1.用CMD1H_DIR_INFO_READ读取当前文件结构到内存
	CH376SendCmdDatWaitInt( CMD1H_DIR_INFO_READ , 0XFF );
	//2.通过CMD01_RD_USB_DATA0命令从内存缓冲区读出数据
	CH376ReadBlock((uint8_t*)&fdi);
	//3.通过CMD_DIR_INFO_READ命令读取FAT_DIR_INFO结构到缓冲区
	//4.通过CMD20_WR_OFS_DATA命令向内部缓冲区指定偏移地址写入修改后的数据
	
	fdi.DIR_CrtTime = time;/* 0EH,文件创建的时间 */
	fdi.DIR_CrtDate = date;/* 10H,文件创建的日期 */
	
	fdi.DIR_WrtTime = time;/* 16H,文件修改时间,参考前面的宏 MAKE_FILE_TIME */
	fdi.DIR_WrtDate = date;/* 18H,文件修改日期,参考前面的宏 MAKE_FILE_DATE */
	
	CH376WriteOfsBlock((uint8_t*)&fdi, 0, sizeof fdi );
	//5.通过CMD_DIR_INFO_SAVE命令向USB存储设备
	CH376DirInfoSave( );
	
	CH376SendCmdDatWaitInt( CMD1H_DIR_INFO_READ , 0XFF );
	//2.通过CMD01_RD_USB_DATA0命令从内存缓冲区读出数据
	CH376ReadBlock((uint8_t*)&fdi);
	
	CH376FileClose(TRUE);
	
	return 0x14;
}
uint8_t SetFileCreateTime(uint8_t* file_name)
{
	uint16_t year = get_rtc_year();
	uint16_t month = get_rtc_month();
	uint16_t day = get_rtc_day();
	uint16_t hour = get_rtc_hour();
	uint16_t minute = get_rtc_minute();
	uint16_t second = get_rtc_second();
	
	return set_file_create_time(file_name, MAKE_FILE_DATE(year, month, day), MAKE_FILE_TIME(hour, minute, second));
}
#define   Test_File_Name 		T_STR("/测试文件.XLS", "/TEST_LOG.XLS")
#define   Test_File_TITLE		T_STR("编号\t条码\t模式\t错误信息\t测试结果\t电压\t电流\t电阻\t测试时间\t记录时间\r", \
									"NO.\tBarCode\tTestMode\tErr.Inf.\tResult\tVoltage\tCurrent\tResistance\tTestTime\tRecordingTime\r")

static u8 result_export(void *arg)
{
	rt_uint32_t msg;
	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0xf000,0XE73C,1,0,16};
	struct rect_type rects={25,100,20,250};
	rt_uint32_t i=0;
		
	win = sui_window_create(T_STR("结果导出","Results Export"),&rect);
	font.panel = win;


	font_draw(94,44,&font,T_STR("确定导出吗？","Determine Export ? "));
	font_draw(26,68,&font,T_STR("提示:将结果导出到USB存储设备中。","Tips:export Results to USB"));
// 	clr_win(win,0XC618,25,100,20,250);

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	font_draw(64,152,&font,T_STR("确定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return"));
	
	sui_window_update(win);
	USB_Device_Chg(USB_1);
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_UP:
					if(result_headinfo.test_count==0)return 1;
					msg = CH376DiskConnect();
					font.fontcolor = 0Xf000;
					
					
					if(msg != USB_INT_SUCCESS)
					{
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("请插入USB存储设备...","Please insert the USB device"));
						sui_window_update(win);
						while (CH376DiskConnect() != USB_INT_SUCCESS)
						{ 
							if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/10) == RT_EOK)
							{
								switch(msg)
								{
									/* 返回 */
									case KEY_EXIT | KEY_UP:
									case KEY_F6 | KEY_UP:
										return 0;
								}
							}/* 检查U盘是否连接,等待U盘插入,对于SD卡,可以由单片机直接查询SD卡座的插拔状态引脚 */
						}
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("USB存储设备已插入！","USB device already insert"));
						sui_window_update(win);
						rt_thread_delay(200);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					}
					
					clr_win(win,0XE73C,2,32,165,295);
					font_draw(70,68,&font,T_STR("等待就绪......","Wait for ready..."));
					sui_window_update(win);
					rt_thread_delay(100);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
					
					/* 对于检测到USB设备的,最多等待100*50mS,主要针对有些MP3太慢,对于检测到USB设备并且连接DISK_MOUNTED的,最多等待5*50mS,主要针对DiskReady不过的 */
					for (i = 0; i < 100; i ++ )
					{  /* 最长等待时间,100*50mS */
						msg = CH376DiskMount();  /* 初始化磁盘并测试磁盘是否就绪 */
						if (msg == USB_INT_SUCCESS)
						{
							break;  /* 准备好 */
						}
						else if (msg == ERR_DISK_DISCON)
						{
							break;  /* 检测到断开,重新检测并计时 */
						}
						if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5)
						{
							break;  /* 有的U盘总是返回未准备好,不过可以忽略,只要其建立连接MOUNTED且尝试5*50mS */
						}
					}
					if (msg == ERR_DISK_DISCON)
					{
						clr_win(win,0XE73C,2,32,165,295);
						font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
						sui_window_update(win);
						rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
						return 1;
					}
					
					{
						u32		__num= result_headinfo.test_count;
						float   sn=0,n=0;
						u32		__file_len= 0;
						char	__buf[150];
						u16		timeout = 500;
						static struct result_info_t *temp;
						
						/* 创建文件 */
						while(CH376FileCreate(Test_File_Name) != USB_INT_SUCCESS && timeout--)
						{
							rt_thread_delay(10);
						}
						
						if(timeout == 0)
						{
							clr_win(win,0XE73C,2,32,165,295);
							font_draw(70,68,&font,T_STR("不识别的USB设备！","Cannot be identified USB"));
							sui_window_update(win);
							rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
							return 1;
						}
						else
						{
							msg = CH376FileClose(TRUE);
							SetFileCreateTime(Test_File_Name);
							CH376FileOpenPath(Test_File_Name);
							__file_len = strlen(Test_File_TITLE);
							CH376ByteWrite((PUINT8)Test_File_TITLE, __file_len,NULL);
							msg = CH376FileClose(TRUE);
							
							if(msg != USB_INT_SUCCESS)
							{
								clr_win(win,0XE73C,2,32,165,295);
								font_draw(70,68,&font,T_STR("文件出错！","File error"));
								sui_window_update(win);
								rt_thread_delay(500);  /* 延时,可选操作,有的USB存储器需要几十毫秒的延时 */
								return 1;
							}
						}
						
						clr_win(win,0XE73C,2,32,165,295);
						clr_win(win,0XC618,25,100,20,250);
						font_draw(70,68,&font,T_STR("开始导出...","Start export..."));
						
						sui_window_update(win);
						rt_thread_delay(50);
						__num = __num + 1;
						sn = 250.0/__num;
						n=0;
						
						for(i=0;i<__num;i++)
						{
							n+=sn;
							temp = memory_result_read(i);
							
							if(temp != NULL)
							{
								rt_sprintf(__buf,"%04d\t",i+1);
								strcat(__buf,"'");//在条码前面加上’防止被excel修改数据
								strcat(__buf,temp->_bar_code);
                                strcat(__buf,"\t");
								strcat(__buf,mode_name[temp->mode]);strcat(__buf,"\t");
								strcat(__buf,error_name[language][temp->error]);strcat(__buf,"\t");
								strcat(__buf,(temp->error == PASS? "PASS":"FAIL"));strcat(__buf,"\t");
								switch(temp->mode)
								{
									case ACW:
									rt_sprintf(__buf+strlen(__buf),"%d.%03dkV\t",temp->s_voltage/1000,temp->s_voltage%1000);

									switch(temp->s_gear){
										
										case I3uA:
											rt_sprintf(__buf+strlen(__buf),"%d.%03duA\t",temp->current/1000,temp->current%1000);
											break;
										case I30uA:
											rt_sprintf(__buf+strlen(__buf),"%d.%02duA\t",temp->current/100,temp->current%100);
											break;
									
										case I300uA:
											
											rt_sprintf(__buf+strlen(__buf),"%d.%duA\t",temp->current/10,temp->current%10);
											break;
										
										case I3mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%03dmA\t",temp->current/1000,temp->current%1000);
											break;
										case I30mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%02dmA\t",temp->current/100,temp->current%100);
											break;
										case I100mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%dmA\t",temp->current/10,temp->current%10);
											break;
										default:
										
										break;
									}
								
									strcat(__buf,"---\t");
									rt_sprintf(__buf+strlen(__buf),"%d.%ds\t",temp->testtime/10,temp->testtime%10);
								break;
								case DCW:
									rt_sprintf(__buf+strlen(__buf),"%d.%03dkV\t",temp->s_voltage/1000,temp->s_voltage%1000);
									switch(temp->s_gear){
										case I3uA:
											rt_sprintf(__buf+strlen(__buf),"%d.%03duA\t",temp->current/1000,temp->current%1000);
											break;
										case I30uA:
											rt_sprintf(__buf+strlen(__buf),"%d.%02duA\t",temp->current/100,temp->current%100);
											break;
									
										case I300uA:
											
											rt_sprintf(__buf+strlen(__buf),"%d.%duA\t",temp->current/10,temp->current%10);
											break;
										case I3mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%03dmA\t",temp->current/1000,temp->current%1000);
										
										break;
										case I30mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%02dmA\t",temp->current/100,temp->current%100);
										
										break;
										case I100mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%dmA\t",temp->current/10,temp->current%10);
										
										break;
										default:
										
										break;
									}
									strcat(__buf,"---\t");
									rt_sprintf(__buf+strlen(__buf),"%d.%ds\t",temp->testtime/10,temp->testtime%10);
									break;
								case IR:
									break;
								case GR:
									rt_sprintf(__buf+strlen(__buf),"%dmV\t",temp->voltage);
									rt_sprintf(__buf+strlen(__buf),"%d.%01dA\t",temp->s_current/10,temp->s_current%10);
                                    if(temp->resister == 0xffff)
                                    {
                                        rt_sprintf(__buf+strlen(__buf),">510mΩ\t",temp->resister/10,temp->resister%10);
                                    }
                                    else
                                    {
                                        rt_sprintf(__buf+strlen(__buf),"%d.%dmΩ\t",temp->resister/10,temp->resister%10);
                                    }
//                                        rt_sprintf(__buf+strlen(__buf),"%d.%dmΩ\t",temp->resister/10,temp->resister%10);
									rt_sprintf(__buf+strlen(__buf),"%d.%ds\t",temp->testtime/10,temp->testtime%10);
									break;
								case LC:
									rt_sprintf(__buf+strlen(__buf),"%d.%dV\t",temp->s_voltage/10,temp->s_voltage%10);
									
									switch(temp->s_gear){
										case I3uA:
										case I30uA:
										case I300uA:
											rt_sprintf(__buf+strlen(__buf),"%d.%01duA\t",temp->current  / 10,temp->current  % 10);
										break;
										case I3mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%03dmA\t",temp->current / 1000,temp->current % 1000);
										break;
										case I30mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%02dmA\t",temp->current / 100,temp->current % 100);
										break;
										case I100mA:
											rt_sprintf(__buf+strlen(__buf),"%d.%01dmA\t",temp->current / 10,temp->current % 10);	
										break;
										default:	
										break;
									}
									strcat(__buf,"---\t");
									rt_sprintf(__buf+strlen(__buf),"%d.%ds\t",temp->testtime/10,temp->testtime%10);
									break;
								}
								rt_sprintf(__buf+strlen(__buf),"'%04d-%02d-%02d %02d:%02d:%02d\r",temp->time.year+2000,temp->time.month,temp->time.date,temp->time.hours,temp->time.minutes,temp->time.seconds);
								
								msg = CH376FileOpen(Test_File_Name);
								msg = CH376ByteLocate(__file_len);
								msg = CH376ByteWrite((u8 *)__buf,strlen(__buf),NULL);
								msg = CH376FileClose(TRUE);
								__file_len += strlen(__buf);
							}else{
								rt_sprintf(__buf,"\t");
								msg = CH376FileOpen(Test_File_Name);
								msg = CH376ByteLocate(__file_len);
								msg = CH376ByteWrite((u8 *)__buf,strlen(__buf),NULL);
								msg = CH376FileClose(TRUE);
							}
							clr_win(win,CL_BLUE,25,100,20,n);
							window_updata(win,&rects);
						}
					}
					
					clr_win(win,0XE73C,70,68,20,100);
					font_draw(70,68,&font,T_STR("导出完成！","Export finish !"));
					sui_window_update(win);
					rt_thread_delay(200);
					
					return 0;
				/* 返回 */
					case KEY_EXIT | KEY_UP:
				case KEY_F6 | KEY_UP:
					return 0;
			}
		}
	}
	
}

static u8 result_jump(void *arg)
{
	rt_uint32_t msg;

	struct panel_type *win;
	struct rect_type rect={190,140,200,300};
	struct font_info_t font={0,0X4208,0XE73C,1,0,16};
	struct num_format num;

	win = sui_window_create(T_STR("跳转","Skip"),&rect);
	font.panel = win;

							font_draw(20,50,&font,T_STR("跳转编号:","Skip num: "));
	font_draw(20,80,&font,T_STR("输入范围: 0001 - 8000","In Range: 0001 - 8000 "));
	font.fontcolor = 0Xf000;
	font_draw(20,110,&font,T_STR("操作提示:请输入要跳转的编号。","OperaTips: Input Skip num."));

	clr_win(win,CL_orange,40,145,30,80);
	clr_win(win,CL_orange,180,145,30,80);
	
	font.fontcolor = 0X4208;
	font_draw(64,152,&font,T_STR("确定","OK"));
	font_draw(204,152,&font,T_STR("返回","Return"));

	sui_window_update(win);
	
	font.fontcolor = 0Xf000;
	font.center = 1;
	rect.x = 100;
	rect.y = 48;
	rect.h = 20;
	rect.w = 32;
	num.num  = result_list_box.current_item+1;
	num._int = 4;
	num._dec = 0;
	num.min  = 1;
	num.max  = 8000;
	num.unit = "";
	msg = num_input(&font,&rect,&num);
	if(msg == 0xffffffff)return 1;
	result_list_box.current_item = msg-1;
	result_list_box.start_item = (result_list_box.current_item / result_list_box.items_count)*result_list_box.items_count;
	return 0;
}


