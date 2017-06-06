/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：bsp_font.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __BSP_FONT_H
#define __BSP_FONT_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "stm32f4xx.h"
#include <rtthread.h>
#include "CS99xx.h"		
		
/*
 * bitmap font engine
 */
struct rtgui_font_bitmap
{
    const rt_uint8_t  *bmp;         /* bitmap font data */
    const rt_uint8_t  *char_width;  /* each character width, NULL for fixed font */
    const rt_uint32_t *offset;      /* offset for each character */

    rt_uint16_t width;              /* font width  */
    rt_uint16_t height;             /* font height */

    rt_uint8_t first_char;
    rt_uint8_t last_char;
};


/*
 * bitmap font engine
 */
struct font_info_t
{
    struct panel_type *panel;
	
	rt_uint16_t	fontcolor;
	rt_uint16_t	backcolor;
	
	rt_uint8_t	alpha;// 透明
	rt_uint8_t	center;//是否居中
	
	rt_uint8_t	high;
	
};

/******************* 函数声明 *********************/
// void font_info_set(struct panel_type *p,rt_uint16_t fc,rt_uint16_t bc,rt_uint8_t a,rt_uint8_t h);

void fontasctosram(u16 x, u16 y, struct font_info_t *font_info,struct rtgui_font_bitmap *font,const char ch);
void fonthztosram(u16 x, u16 y, struct font_info_t *font_info,struct rtgui_font_bitmap *bmp_font,const char *text);
void font_draw(u16 x, u16 y, struct font_info_t *font_info,const char *text);
void ui_text_draw(struct font_info_t *font_info,struct rect_type *rect,char *str);
void ui_text_draw_alpha(struct font_info_t *font_info,struct rect_type *rect,char *str);

/********************** 外部用到的变量 **************************/

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
