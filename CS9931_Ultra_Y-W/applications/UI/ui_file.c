#include "CS99xx.h"
#include "memory.h"
#include "bsp_ico.h"
#include "memorymanagement.h"
#include "bsp_listbox.h"

static void file_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y);
static rt_uint8_t file_new_win_create(struct file_info_t *f);
static rt_uint8_t file_win_edit(struct file_info_t *f);
static rt_uint8_t file_delete_win_create(struct file_info_t *f);
static void file_init(struct file_info_t *f);
static rt_uint8_t file_memory_win_create(rt_uint8_t index);
static rt_uint8_t file_win_newname(struct file_info_t *f);

struct
{
    u16 x;
	u16 y;
	const char *data_en;
	const char *data_ch;
}file_title_name[5]={
	{10,10,"NO.","编号"},
	{120,10,"Name","文件名"},
	{250,10,"Mode","模式"},
	{350,10,"Step","总步数"},
	{500,10,"Time","建立时间"},
};

static struct rect_type list_box_rect={10,40,26,660};
struct rtgui_listctrl list_box={
	&panel_home,
	13,
	FILE_NUM-1,
	0,0,
	&list_box_rect,
	file_item_draw
};
void ui_file_thread(void)
{
	u8	i;
	rt_uint32_t msg;
	struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};
	
	if(panel_flag == 1)
	{
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);
		
//		{
//			extern void Reset_LED_PLC(void);
//			Reset_LED_PLC();
//		}
		
// 		file_info[0].en = 1;
// 		rt_strncpy(file_info[0].name,"A",rt_strlen("A")+1);
// 		file_info[0].mode = N_WORK;
		
	// 	rt_enter_critical();
		
		/* 标题 */
// 		font_info_set(&panel_home,CL_YELLOW,0x0,1,20);
		for(i=0;i<5;i++)
		{
			/* 语言判断 */
			if(language==1)
				font_draw(file_title_name[i].x,file_title_name[i].y,&font,file_title_name[i].data_en);
			else
				font_draw(file_title_name[i].x,file_title_name[i].y,&font,file_title_name[i].data_ch);
		}	
		list_box.current_item = 0;
		listbox_draw(&list_box);
	// rt_exit_critical();
		
		rt_mb_send(&screen_mb, UPDATE_HOME);
		if(file_info[1].en != 0)
			ui_key_updata(0x40);
		else
			ui_key_updata(0x38);
	}	
	while(panel_flag == 1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/100) == RT_EOK)
		{
			switch(msg)
			{
				/* 存贮 */
				case KEY_F1 | KEY_UP:
					if(1 == file_memory_win_create(list_box.current_item+1))list_box.current_item += 1;
						
					listbox_draw(&list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					break;
				/* 新建 */
				case KEY_F2 | KEY_UP:
					if(file_info[list_box.current_item+1].en == 0)
					{
						file_init(&file_info[list_box.current_item+1]);
						if(file_new_win_create(&file_info[list_box.current_item+1]) == 0)
						{
							init_list();	
							save_file_to_flash(list_box.current_item+1);
							save_steps_to_flash(list_box.current_item+1);
							read_flash_to_list(flash_info.current_file);
							
							if(list_box.current_item<list_box.total_items-1)list_box.current_item ++;
							list_box.start_item = (list_box.current_item / list_box.items_count)*list_box.items_count;
						}
						listbox_draw(&list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
					}
					break;
				/* 读取 */
				case KEY_F3 | KEY_UP:
					if(file_info[list_box.current_item+1].en != 0)
					{
						flash_info.current_file = list_box.current_item+1;
						init_list();
						read_flash_to_list(flash_info.current_file);
						FLASH_CS_SET(1);	// 选择参数flash
						sf_WriteBuffer((uint8_t *)&flash_info,FLASH_BKP_ADDR,sizeof(flash_info));
					}
					break;
				/* 编辑 */
				case KEY_F4 | KEY_UP:
					if(file_info[list_box.current_item+1].en != 0)
					{
						if(file_win_edit(&file_info[list_box.current_item+1]) == 0)
						{
//							init_list();	
							save_file_to_flash(list_box.current_item+1);
//							save_steps_to_flash(list_box.current_item+1);
//							read_flash_to_list(flash_info.current_file);
						}
						
						listbox_draw(&list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
					}
					break;
				/* 删除 */
				case KEY_F5 | KEY_UP:
					if(file_info[list_box.current_item+1].en != 0)
					{
						u8 status = file_delete_win_create(&file_info[list_box.current_item+1]);
						if(status == 1)
						{
							save_file_to_flash(list_box.current_item+1);
						}
						else if(status == 2)
						{
							save_files_to_flash();
						}
						init_list();
						read_flash_to_list(flash_info.current_file);
						FLASH_CS_SET(1);	// 选择参数flash
						sf_WriteBuffer((uint8_t *)&flash_info,FLASH_BKP_ADDR,sizeof(flash_info));
						listbox_draw(&list_box);
						rt_mb_send(&screen_mb, UPDATE_HOME);
					}
					break;
				/* 更多 */
				case KEY_F6 | KEY_UP:
					panel_flag = 7;
					break;
				case CODE_RIGHT:
				case KEY_U | KEY_DOWN:
					if(list_box.current_item>0)list_box.current_item --;
					list_box.start_item = (list_box.current_item / list_box.items_count)*list_box.items_count;
					listbox_draw(&list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					if(file_info[list_box.current_item+1].en != 0)
					{
						ui_key_updata(0x40);
					}
					else
					{
						ui_key_updata(0x38);
					}
					break;
				case CODE_LEFT:
				case KEY_D | KEY_DOWN:
					if(list_box.current_item<list_box.total_items-1)list_box.current_item ++;
					list_box.start_item = (list_box.current_item / list_box.items_count)*list_box.items_count;
					listbox_draw(&list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					if(file_info[list_box.current_item+1].en != 0)
					{
						ui_key_updata(0x40);
					}
					else
					{
						ui_key_updata(0x38);
					}
					break;
				case KEY_L | KEY_DOWN:
					if(list_box.current_item>=list_box.items_count)list_box.current_item -=list_box.items_count;
					list_box.start_item = (list_box.current_item / list_box.items_count)*list_box.items_count;
					listbox_draw(&list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					if(file_info[list_box.current_item+1].en != 0)
					{
						ui_key_updata(0x40);
					}
					else
					{
						ui_key_updata(0x38);
					}
					break;
				case KEY_R | KEY_DOWN:
					list_box.current_item += list_box.items_count;
					if(list_box.current_item>list_box.total_items-1)list_box.current_item=list_box.total_items-1;
					list_box.start_item = (list_box.current_item / list_box.items_count)*list_box.items_count;
					listbox_draw(&list_box);
					rt_mb_send(&screen_mb, UPDATE_HOME);
					if(file_info[list_box.current_item+1].en != 0)
					{
						ui_key_updata(0x40);
					}
					else
					{
						ui_key_updata(0x38);
					}
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

void refresh_file_list_box(void)
{
		u8	i;
		struct font_info_t font={&panel_home,CL_YELLOW,0x0,1,0,20};
	
		clr_mem((u16 *)ExternSramHomeAddr,0x19f6,ExternSramHomeSize/2);

		for(i=0;i<5;i++)
		{
			/* 语言判断 */
			if(language==1)
				font_draw(file_title_name[i].x,file_title_name[i].y,&font,file_title_name[i].data_en);
			else
				font_draw(file_title_name[i].x,file_title_name[i].y,&font,file_title_name[i].data_ch);
		}	
		list_box.current_item = 0;
		listbox_draw(&list_box);
		
		rt_mb_send(&screen_mb, UPDATE_HOME);
}

static void file_init(struct file_info_t *f)
{
	f->en = 0;
	f->name[0] = 0;
	f->mode = N_WORK;
	f->totalstep = 1;
// 	f->time;
	f->arc = 0;
	f->outdelay = 0;
	f->passtime = 2;
	f->buzzertime = 2;
}


const char *file_new_name[2][6]={
	{"文  件  名    :",
	 "工 作 模 式   :",
	 "电弧侦测模式  :",
	 "输 出 延 时   :",
	 "PASS 保持时间 :",
	 "Buzzer保持时间:"},
	 
	{"File name     :",
	 "Work Mode     :",
	 "Arc Sense Mode:",
	 "Output Delayed: ",
	 "PASS keepTime :",
	 "BuzzerKeepTime:"}
};
static rt_uint8_t file_new_win_create(struct file_info_t *f)
{
	rt_uint32_t msg;
	rt_uint8_t pos = 0,i;
	char buf[10];
	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,200,300,190,140};
	struct rect_type rect = {132,38,20,160};
	struct font_info_t font = {0,0xffff,0x1f,1,1,16};
	u16 y=15;
	font.panel = &panel_win;
	
			
	clr_mem((u16 *)ExternSramWinAddr,CL_orange,30*300);
	clr_mem((u16 *)ExternSramWinAddr+30*300,0x1f,170*300);/*0x19f6*/
	clr_win(&panel_win,0xffff,0,0,1,300);
	clr_win(&panel_win,0xffff,0,199,1,300);
	clr_win(&panel_win,0xffff,0,0,230,1);
	clr_win(&panel_win,0xffff,299,0,230,1);

// 	font_info_set(&panel_win,0xffff,0x0,1,16);
	font.fontcolor	= 0xf800;
	font_draw(10,7,&font,T_STR("<文件新建>","<New File>"));

// 	font_info_set(&panel_win,0,0x0,1,16);
	font.fontcolor	= 0xffff;
	for(i=0;i<6;i++)
	{
		font_draw(10,y+=25,&font,file_new_name[language][i]);
		switch(i)
		{
			case 0:
				clr_win(&panel_win,	0x53fa,132,y-2,	20,160);
// 				f->name[0] = 0;
				break;
			case 1:
// 				f->mode = N_WORK;
				if(f->mode == G_WORK)
					font_draw(132+(160-rt_strlen(T_STR("G 模式","G Mode"))*8)/2,y,&font,T_STR("G 模式","G Mode"));
				else
					font_draw(132+(160-rt_strlen(T_STR("N 模式","N Mode"))*8)/2,y,&font,T_STR("N 模式","N Mode"));
				break;
			case 2:
// 				f->arc = 0;
				if(f->arc != 0)
						font_draw(132+(160-rt_strlen(T_STR("电流模式","Current Mode"))*8)/2,y,&font,T_STR("电流模式","Current Mode"));
				
				else
						font_draw(132+(160-rt_strlen(T_STR("等级模式","Grade Mode"))*8)/2,y,&font,T_STR("等级模式","Grade Mode"));
				
				break;
			case 3:
// 				f->outdelay = 0;
				rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
			case 4:
// 				f->passtime = 2;
				rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
			case 5:
// 				f->buzzertime = 2;
				rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
		}
	}
	panel_update(panel_win);

	/* 文件名输入 */
	if(text_input(&panel_win,&rect,f->name) != 0)return 1;
	if(rt_strlen(f->name)==0)strcpy(f->name,"untitled");
	ui_text_draw(&font,&rect,f->name);
	pos++;
	font.fontcolor = 0xf800;
	font.backcolor = 0x53fa;
	rect.y += 25;
	if(f->mode == N_WORK)
		ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
	
	else
		ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(pos)
			{
				case 1:
					switch(msg)
					{
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->mode != N_WORK)f->mode = N_WORK;
								ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
							break;
						case CODE_RIGHT:
						case KEY_R | KEY_DOWN:
							if(f->mode != G_WORK)f->mode = G_WORK;
						
								ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							if(f->mode == N_WORK)
								ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
							else
								ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
								
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;
							if(f->arc == 0)
								ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
							else
								ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
							break;
					}
					break;
				case 2:
					switch(msg)
					{
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->arc != 0)f->arc = 0;
							ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
						break;
						case CODE_RIGHT:
						case KEY_R | KEY_DOWN:
							if(f->arc != 1)f->arc = 1;
								ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
						break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							if(f->arc == 0)
								ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
							else
								ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
				
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;
							rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 3:
					switch(msg)
					{
						case CODE_RIGHT:
 					  case KEY_R | KEY_DOWN:
							if(f->outdelay<9999){
								f->outdelay++;						
								rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case CODE_LEFT:
            case KEY_L | KEY_DOWN:
							if(f->outdelay>0){
								f->outdelay--;
								rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
							ui_text_draw(&font,&rect,buf);
							
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;							
							rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 4:
					switch(msg)
					{
						case CODE_RIGHT:
 						case KEY_R | KEY_DOWN:
							if(f->passtime<9999){
								f->passtime++;
								rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->passtime>0){
								f->passtime--;
								rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
							ui_text_draw(&font,&rect,buf);
							
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;							
							rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 5:
					switch(msg)
					{
						case CODE_RIGHT:
 						case KEY_R | KEY_DOWN:
							if(f->buzzertime<9999){
								f->buzzertime++;						
								rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
								ui_text_draw(&font,&rect,buf);
							}
						break;
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->buzzertime>0){
								f->buzzertime--;			
								rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
								ui_text_draw(&font,&rect,buf);
							}
							
						break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_ENTER | KEY_DOWN:
							f->totalstep = 1;
							f->en = 1;
							{
								rt_device_t device;
							
								device = rt_device_find("rtc");
								if(device != RT_NULL)
								{
									rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &f->time);						
								}
							}
							
							return 0;
					}
					break;
			}
		}
	}		
}


static rt_uint8_t file_win_edit(struct file_info_t *f)
{
	rt_uint32_t msg;
	rt_uint8_t pos = 0,i;
	char buf[10];
	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,200,300,190,140};
	struct rect_type rect = {132,38,20,160};
	struct font_info_t font = {0,0xffff,0x1f,1,1,16};
	u16 y=15;
	font.panel = &panel_win;
	
			
	clr_mem((u16 *)ExternSramWinAddr,CL_orange,30*300);
	clr_mem((u16 *)ExternSramWinAddr+30*300,0x1f,170*300);/*0x19f6*/
	clr_win(&panel_win,0xffff,0,0,1,300);
	clr_win(&panel_win,0xffff,0,199,1,300);
	clr_win(&panel_win,0xffff,0,0,230,1);
	clr_win(&panel_win,0xffff,299,0,230,1);

// 	font_info_set(&panel_win,0xffff,0x0,1,16);
	font.fontcolor	= 0xf800;

	font_draw(10,7,&font,T_STR("<文件新建>","<New File>"));

	// 	font_info_set(&panel_win,0,0x0,1,16);
	font.fontcolor	= 0xffff;
	for(i=0;i<6;i++)
	{
		font_draw(10,y+=25,&font,file_new_name[language][i]);
		switch(i)
		{
			case 0:
				clr_win(&panel_win,	0x53fa,132,y-2,	20,160);
// 				f->name[0] = 0;
				break;
			case 1:
// 				f->mode = N_WORK;
				if(f->mode == G_WORK)
					font_draw(132+(160-rt_strlen(T_STR("G 模式","G State"))*8)/2,y,&font,T_STR("G 模式","G Mode"));
				else
						font_draw(132+(160-rt_strlen(T_STR("N 模式","N Mode"))*8)/2,y,&font,T_STR("N 模式","N Mode"));
				break;
			case 2:
// 				f->arc = 0;
				if(f->arc != 0)
					font_draw(132+(160-rt_strlen(T_STR("电流模式","Current Mode"))*8)/2,y,&font,T_STR("电流模式","Current Mode"));
				else
					font_draw(132+(160-rt_strlen(T_STR("等级模式","Grade Mode"))*8)/2,y,&font,T_STR("等级模式","Grade Mode"));
				break;
			case 3:
// 				f->outdelay = 0;
				rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
			case 4:
// 				f->passtime = 2;
				rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
			case 5:
// 				f->buzzertime = 2;
				rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
				font_draw(132+(160-rt_strlen(buf)*8)/2,y,&font,buf);
				break;
		}
	}
	panel_update(panel_win);

	/* 文件名输入 */
	if(text_input(&panel_win,&rect,f->name) != 0)return 1;
	if(rt_strlen(f->name)==0)strcpy(f->name,"untitled");
	ui_text_draw(&font,&rect,f->name);
	pos++;
	font.fontcolor = 0xf800;
	font.backcolor = 0x53fa;
	rect.y += 25;
	if(f->mode == N_WORK)
		ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
	else
		ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(pos)
			{
				case 1:
					switch(msg)
					{
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->mode != N_WORK)f->mode = N_WORK;
							ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
							break;
						case CODE_RIGHT:
						case KEY_R | KEY_DOWN:
							if(f->mode != G_WORK)f->mode = G_WORK;
								ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
							break;
						
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							if(f->mode == N_WORK)
								ui_text_draw(&font,&rect,T_STR("<<N 模式>>","<<N Mode>>"));
							else
								ui_text_draw(&font,&rect,T_STR("<<G 模式>>","<<G Mode>>"));
							
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;
							if(f->arc == 0)
								ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
							else
								ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
							break;
					}
					break;
				case 2:
					switch(msg)
					{
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->arc != 0)f->arc = 0;
								ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
							break;
						case CODE_RIGHT:
						case KEY_R | KEY_DOWN:
							if(f->arc != 1)f->arc = 1;
								ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							if(f->arc == 0)
									ui_text_draw(&font,&rect,T_STR("等级模式","Grade Mode"));
							else
									ui_text_draw(&font,&rect,T_STR("电流模式","Current Mode"));
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;
							rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 3:
					switch(msg)
					{
						case CODE_RIGHT:
 					  case KEY_R | KEY_DOWN:
							if(f->outdelay<9999){
								f->outdelay++;						
								rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case CODE_LEFT:
            case KEY_L | KEY_DOWN:
							if(f->outdelay>0){
								f->outdelay--;
								rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							rt_sprintf(buf,"%d.%ds",f->outdelay/10,f->outdelay%10);
							ui_text_draw(&font,&rect,buf);
							
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;							
							rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 4:
					switch(msg)
					{
						case CODE_RIGHT:
 						case KEY_R | KEY_DOWN:
							if(f->passtime<9999){
								f->passtime++;
								rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->passtime>0){
								f->passtime--;
								rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
								ui_text_draw(&font,&rect,buf);
							}
							break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_D | KEY_DOWN:
						case KEY_ENTER | KEY_DOWN:
							font.fontcolor = 0xffff;
							font.backcolor = 0x1f;
							rt_sprintf(buf,"%d.%ds",f->passtime/10,f->passtime%10);
							ui_text_draw(&font,&rect,buf);
							
							pos++;
							font.fontcolor = 0xf800;
							font.backcolor = 0x53fa;
							rect.y += 25;							
							rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
							ui_text_draw(&font,&rect,buf);
							break;
					}
					break;
				case 5:
					switch(msg)
					{
						case CODE_RIGHT:
 						case KEY_R | KEY_DOWN:
							if(f->buzzertime<9999){
								f->buzzertime++;						
								rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
								ui_text_draw(&font,&rect,buf);
							}
						break;
						case CODE_LEFT:
						case KEY_L | KEY_DOWN:
							if(f->buzzertime>0){
								f->buzzertime--;			
								rt_sprintf(buf,"%d.%ds",f->buzzertime/10,f->buzzertime%10);
								ui_text_draw(&font,&rect,buf);
							}
							
						break;
						case KEY_EXIT | KEY_DOWN:
							return 1;
						case KEY_ENTER | KEY_DOWN:
							f->en = 1;
							{
								rt_device_t device;
							
								device = rt_device_find("rtc");
								if(device != RT_NULL)
								{
									rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &f->time);						
								}
							}
							
							return 0;
					}
					break;
			}
		}
	}		
}

static rt_uint8_t file_win_newname(struct file_info_t *f)
{

	rt_uint8_t i;

	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,200,300,190,140};
	struct rect_type rect = {132,38,20,160};
	struct font_info_t font = {0,0xffff,0x1f,1,1,16};
	u16 y=15;
	font.panel = &panel_win;
	
			
	clr_mem((u16 *)ExternSramWinAddr,CL_orange,30*300);
	clr_mem((u16 *)ExternSramWinAddr+30*300,0x1f,170*300);/*0x19f6*/
	clr_win(&panel_win,0xffff,0,0,1,300);
	clr_win(&panel_win,0xffff,0,199,1,300);
	clr_win(&panel_win,0xffff,0,0,230,1);
	clr_win(&panel_win,0xffff,299,0,230,1);

// 	font_info_set(&panel_win,0xffff,0x0,1,16);
	font.fontcolor	= 0xf800;


	font_draw(10,7,&font,T_STR("<重命名>","<Rename>"));

// 	font_info_set(&panel_win,0,0x0,1,16);
	font.fontcolor	= 0xffff;
	for(i=0;i<1;i++)
	{
		font_draw(10,y+=75,&font,file_new_name[language][i]);
		switch(i)
		{
			case 0:
				clr_win(&panel_win,	0x53fa,132,y-2,	20,160);
// 				f->name[0] = 0;
				break;
		}
	}
	panel_update(panel_win);
	rect.y += 50;
	/* 文件名输入 */
	if(text_input(&panel_win,&rect,f->name) != 0)return 1;
	if(rt_strlen(f->name)==0)strcpy(f->name,"untitled");
	ui_text_draw(&font,&rect,f->name);

	return 0;
		
}

static rt_uint8_t file_delete_win_create(struct file_info_t *f)
{
	rt_uint32_t msg;
	rt_uint8_t	pos=0;
	
	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,200,300,190,140};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	font.panel = &panel_win;
	clr_mem((u16 *)ExternSramWinAddr,CL_orange,30*300);
	clr_mem((u16 *)ExternSramWinAddr+30*300,0xffff,170*300);/*0x19f6*/
	clr_win(&panel_win,CL_GREY,0,199,1,300);
	clr_win(&panel_win,CL_GREY,299,30,200,1);
	
	clr_win(&panel_win,CL_orange,40,140,30,80);
	clr_win(&panel_win,CL_orange,180,140,30,80);

// 	font_info_set(&panel_win,0xffff,0x0,1,16);
		font_draw(10,7,&font,T_STR("<文件删除>","<File Delete>"));

// 	font_info_set(&panel_win,0,0x0,1,16);
	font.fontcolor = 0x0000;
	
	font_draw(140,60,&font,T_STR("单项删除","Single Delete"));
	font_draw(140,95,&font,T_STR("全部删除","Delete All"));
	font_draw(60,148,&font,T_STR("确 定","OK"));
	font_draw(201,148,&font,T_STR("取 消","Cancel"));
	
	ico_color_set(1,0xf800,0x0000);
	ico_darw(&panel_win,95,64,(rt_uint8_t *)ico1_data);
	
	panel_update(panel_win);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_U | KEY_DOWN:
					pos = 0;
					clr_win(&panel_win,0xffff,95,98,9,18);
					ico_darw(&panel_win,95,64,(rt_uint8_t *)ico1_data);
					panel_update(panel_win);
					break;
				case KEY_D | KEY_DOWN:
					pos = 1;
					clr_win(&panel_win,0xffff,95,64,9,18);
					ico_darw(&panel_win,95,98,(rt_uint8_t *)ico1_data);
					panel_update(panel_win);
					break;
				case KEY_ENTER | KEY_DOWN:
					flash_info.current_file = 0;
					if(pos==0)
					{
						f->en = 0;
						return 1;
					}
					else
					{
						rt_uint16_t i;
						for(i=0;i<FILE_NUM;i++)
							file_info[i].en = 0;
						return 2;
					}
				case KEY_EXIT | KEY_DOWN:
					return 0;
			}
		}
	}
}

static rt_uint8_t file_memory_win_create(rt_uint8_t index)
{
	rt_uint32_t msg;
	
	struct panel_type panel_win	={(u16 *)ExternSramWinAddr,200,300,190,140};
	struct font_info_t font={0,0xffff,0x0,1,0,16};
	font.panel = &panel_win;
	clr_mem((u16 *)ExternSramWinAddr,CL_orange,30*300);
	clr_mem((u16 *)ExternSramWinAddr+30*300,0xffff,170*300);/*0x19f6*/
	clr_win(&panel_win,CL_GREY,0,199,1,300);
	clr_win(&panel_win,CL_GREY,299,30,200,1);
	
	clr_win(&panel_win,CL_orange,40,140,30,80);
	clr_win(&panel_win,CL_orange,180,140,30,80);

// 	font_info_set(&panel_win,0xffff,0x0,1,16);
	
	font_draw(10,7,&font,T_STR("<文件存贮>","<File Storage>"));
	
// 	font_info_set(&panel_win,0,0x0,1,16);
	font.fontcolor = 0xf800;

	{
		char buf[60];
			rt_sprintf(buf,T_STR("确定将当前文件存贮到编号%03d位置！","Determine file storage %03d site"),index);
		font_draw(10,60,&font,buf);
	}
	
	font_draw(60,148,&font,T_STR("确 定","OK"));
	font_draw(201,148,&font,T_STR("取 消","Cancel"));
	
	panel_update(panel_win);
	
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			switch(msg)
			{
				case KEY_ENTER | KEY_DOWN:
					if(index == flash_info.current_file)
					{
						
					}
					else
					{
						rt_memcpy(&file_info[index],&file_info[flash_info.current_file],sizeof(struct file_info_t));
						if(flash_info.current_file == 0)file_info[index].en = 1;
						file_win_newname(&file_info[index]);
						{
								rt_device_t device;
							
								device = rt_device_find("rtc");
								if(device != RT_NULL)
								{
									rt_device_control(device, RT_DEVICE_CTRL_RTC_GET_TIME, &(file_info[index].time));						
								}
						}
						save_file_to_flash(index);
						save_steps_to_flash(index);
					}
					return 1;
				case KEY_EXIT | KEY_DOWN:
					
					return 0;
			}
		}
	}
}

static void file_item_draw(struct font_info_t *font,rt_uint16_t index,rt_uint16_t x,rt_uint16_t y)
{
	char buf[20];
	/* 显示编号 */
	rt_sprintf(buf,"%03d",index+1);
	font_draw(x+8,y,font,buf);
	
	/* 文件是否存在 */
	if(file_info[index+1].en)
	{
		/* 文件名 */
		font_draw(x+68+(144-rt_strlen(file_info[index+1].name)*8)/2,y,font,file_info[index+1].name);
		/* 工作模式 N or G */
		font_draw(x+256,y,font,workmode_name[file_info[index+1].mode]);
		/* 总步数 */
		rt_sprintf(buf,"%d", file_info[index+1].totalstep);
		font_draw(x+340+(60-rt_strlen(buf)*8)/2,y,font,buf);
		/* 建立时间 */
		rt_sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d",
			file_info[index+1].time.year+2000,
			file_info[index+1].time.month,
			file_info[index+1].time.date,
			file_info[index+1].time.hours,
			file_info[index+1].time.minutes,
			file_info[index+1].time.seconds);
		font_draw(x+454,y,font,buf);
	}
}

