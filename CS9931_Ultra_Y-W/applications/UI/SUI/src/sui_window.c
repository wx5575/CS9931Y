/*
 * File      : sui_window.c
 * This file is part of SUI
 * COPYRIGHT (C) 2014 - 2018, SSC
 *
 *  This program is window.c
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-11-25     SSC          the first version
 */

#include "ui_config.h"
#include "sui_window.h"
// #include "sui_color.h"

#include "bsp_font.h"
extern void _sui_lcd_graph(sui_uint16_t x,sui_uint16_t y,sui_uint16_t h,sui_uint16_t w,sui_uint16_t *data);
/*
struct sui_panel panel;
struct sui_panel *sui_window_create(const char *name, struct sui_rect *rect)
{
	sui_uint16_t i,j,h,w,*p;

	panel.data = (sui_uint16_t *)ExternSramWinAddr;
	panel.rect = rect;
	
	h = (rect->y2-rect->y1);
	w = (rect->x2-rect->x1);
	p = panel.data;
	for(i=0;i<(30);i++)
	{
		for(j=0;j<w;j++)
		{
			*p++ = 0xf800;
		}
	}
	for(i=0;i<(h-30);i++)
	{
		for(j=0;j<(w);j++)
		{
			*p++ = 0xffff;
		}
	}
	p = panel.data;
	for(i=0;i<(w);i++)
	{
		*p++ = 0;
	}
	p = panel.data + (w)*(h-1);
	for(i=0;i<(w);i++)
	{
		*p++ = 0;
	}
	p = panel.data;
	for(i=0;i<(h);i++)
	{
		*p = 0;
		*(p+w-1) = 0;
		p += w;
	}
	
	return &panel;
}


void sui_window_update(struct sui_panel *panel)
{
	_sui_lcd_graph(panel->rect->x1,panel->rect->y1,panel->rect->y2-panel->rect->y1,panel->rect->x2-panel->rect->x1,panel->data);
}
*/

struct panel_type __panel;
struct panel_type *sui_window_create(const char *name, struct rect_type *rect)
{
	sui_uint16_t i,j,*p;
	struct font_info_t font = {0,0xffff,0x1f,1,1,16};

	__panel.data = (sui_uint16_t *)ExternSramWinAddr;
	__panel.x	 = rect->x;
	__panel.y	 = rect->y;
	__panel.h	 = rect->h;
	__panel.w	 = rect->w;
	
	font.panel = &__panel;

	p = __panel.data;
	for(i=0;i<(30);i++)
	{
		for(j=0;j<__panel.w;j++)
		{
			*p++ = 0XFC00;
		}
	}
	for(i=0;i<(__panel.h-30);i++)
	{
		for(j=0;j<(__panel.w);j++)
		{
			*p++ = 0XE73C;
		}
	}
	p = __panel.data;
	for(i=0;i<(__panel.w);i++)
	{
		*p++ = 0;
	}
	p = __panel.data + (__panel.w)*(__panel.h-1);
	for(i=0;i<(__panel.w);i++)
	{
		*p++ = 0;
	}
	p = __panel.data;
	for(i=0;i<(__panel.h);i++)
	{
		*p = 0;
		*(p+__panel.w-1) = 0;
		p += __panel.w;
	}
	
	font_draw(10,7,&font,name);
	return &__panel;
}


void sui_window_update(struct panel_type *panel)
{
	_sui_lcd_graph(panel->x,panel->y,panel->h,panel->w,panel->data);
}

void sui_window_xor(struct panel_type *panel,struct rect_type *rect)
{
	sui_uint16_t *p,i,j;
	p = panel->data + panel->w*rect->y + rect->x;
	for(i=0;i<rect->h;i++)
	{
		for(j=0;j<rect->w;j++)
		{
			*p = (*p^0xFFFF);
			p++;
		}
		p += (panel->w-rect->w);
	}
	
// 	window_updata(panel,rect);
}
