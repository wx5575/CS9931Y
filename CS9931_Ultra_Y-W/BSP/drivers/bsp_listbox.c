/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：bsp_listbox.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "bsp_listbox.h"

#define		listbox_backcolor	0x19f6
#define		selected_color	0x53fa
// #define		selected_high	26
// #define		selected_weight	660

/* the number of item in a page */
rt_uint16_t mem_item;
struct font_info_t listbox_font={0,0xffff,0x0,1,0,16};

void listbox_draw(struct rtgui_listctrl *box)
{
	u16 i;
	
	listbox_font.panel = box->parent;
	/* 列表 */
// 	rt_enter_critical();
						
// 	font_info_set(box->parent,0xffff,0x0,1,16);
	for(i=box->start_item; i < box->items_count + box->start_item; i++)
	{
		if(i== box->current_item)
		{
			clr_win(box->parent,
				selected_color,
				box->rect->x,
				box->rect->y + box->rect->h * (i - box->start_item),
				box->rect->h,
				box->rect->w);
			mem_item = box->current_item;
		}
		else
			clr_win(box->parent,
				listbox_backcolor,
				box->rect->x,
				box->rect->y + box->rect->h * (i - box->start_item),
				box->rect->h,
				box->rect->w);
		
		if(i>box->total_items-1)continue;
		box->on_item_draw(&listbox_font,i,box->rect->x, box->rect->y + box->rect->h*(i - box->start_item) + 5);
	}
	
// 	rt_exit_critical();
}

// void listbox_set(struct rtgui_listctrl *box)
// {
// 	if(mem_item % (box->current_item - 1) == 0)
// 	{
// 		listbox_draw(box);
// 		return;
// 	}
// // 	font_info_set(box->parent,0xffff,0x0,1,16);
// 	clr_win(box->parent,
// 				listbox_backcolor,box->x,box->y+selected_high*(mem_item-box->start_item),
// 				selected_high,selected_weight);

// 		box->on_item_draw(&listbox_font,mem_item,box->x,box->y+selected_high*(mem_item-box->start_item)+5);
// 	
// 	clr_win(box->parent,
// 				selected_color,box->x,box->y+selected_high*(box->current_item-box->start_item),
// 				selected_high,selected_weight);

// 		box->on_item_draw(&listbox_font,box->current_item,box->x,box->y+selected_high*(box->current_item-box->start_item)+5);
// 	
// 	mem_item = box->current_item;
// }

/********************************************************************************************/
