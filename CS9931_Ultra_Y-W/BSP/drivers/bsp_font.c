/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：bsp_font.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "bsp_font.h"

extern struct rtgui_font_bitmap asc16;
extern struct rtgui_font_bitmap hz16;

extern struct rtgui_font_bitmap asc20;
extern struct rtgui_font_bitmap hz20;

extern struct rtgui_font_bitmap asc24;
extern struct rtgui_font_bitmap hz24;

extern struct rtgui_font_bitmap asc32;
extern struct rtgui_font_bitmap hz32;
/*
 * bitmap font engine
 */
// struct font_info_t font_info;

// void font_info_set(struct panel_type *p,rt_uint16_t fc,rt_uint16_t bc,rt_uint8_t a,rt_uint8_t h)
// {
// 	font_info.panel = p;
// 	font_info.fontcolor = fc;
// 	font_info.backcolor = bc;
// 	font_info.alpha = a;
// 	font_info.high = h;
// }



/*
 * 函数名：fonttosram
 * 描述  ：kong
 * 输入  ：空
 * 输出  ：空
 */
void fontasctosram(u16 x, u16 y, struct font_info_t *font_info,struct rtgui_font_bitmap *font,const char ch)
{
    const rt_uint8_t *font_ptr;
    register rt_base_t i, j, /*k,*/ word_bytes;
	u16 *sc;


	word_bytes = (((font->width - 1) / 8) + 1);
	
	if(font->height == 32 && (ch>font->last_char || ch<font->first_char))
	{
		extern rt_uint16_t rtgui_asc32_file_font_setoff(rt_uint8_t ch);
		font_ptr = font->bmp + (rtgui_asc32_file_font_setoff(ch)) * word_bytes * font->height;
	}
	else
	font_ptr = font->bmp + (ch - font->first_char) * word_bytes * font->height;
	
    for (i = 0; i < font->height; i++)
    {
        rt_uint8_t chr = 0;
        const rt_uint8_t *ptr = font_ptr + i * word_bytes;
		sc = font_info->panel->data + font_info->panel->w*(i+y) + x;//计算行地址
        for (j = 0; j < font->width; j++)
        {
            if (j % 8 == 0)
                chr = *ptr++;
            if (chr & 0x80)
				*sc = font_info->fontcolor;
            else if (font_info->alpha == 0)
				*sc = font_info->backcolor;
                
            chr <<= 1;
			sc++;
        }
    }
}
static const rt_uint8_t *_rtgui_hz_bitmap_get_font_ptr(struct rtgui_font_bitmap *bmp_font,
        rt_uint8_t *str,
        rt_base_t font_bytes)
{
    rt_ubase_t sect, index;

    /* calculate section and index */
    sect  = *str - 0xA0;
    index = *(str + 1) - 0xA0;

	/* 不连续字体添加 */
// #ifdef RTGUI_USING_FONT20
	if(bmp_font->height == 20)
	{
		extern rt_uint16_t rtgui_hz20_file_font_setoff(rt_uint8_t qu, rt_uint8_t wei);
		return bmp_font->bmp + rtgui_hz20_file_font_setoff(sect, index) * font_bytes;
	}else
// #endif
// #ifdef RTGUI_USING_FONT24
	if(bmp_font->height == 24)
	{
		extern rt_uint16_t rtgui_hz24_file_font_setoff(rt_uint8_t qu, rt_uint8_t wei);
		return bmp_font->bmp + rtgui_hz24_file_font_setoff(sect, index) * font_bytes;
	}
// #endif
// #ifdef RTGUI_USING_FONT32
	if(bmp_font->height == 32)
	{
		extern rt_uint16_t rtgui_hz32_file_font_setoff(rt_uint8_t qu, rt_uint8_t wei);
		return bmp_font->bmp + rtgui_hz32_file_font_setoff(sect, index) * font_bytes;
	}
// #endif
	/**********************/
	
    /* get font pixel data */
    return bmp_font->bmp + (94 * (sect - 1) + (index - 1)) * font_bytes;
}

/*
 * 函数名：fonttosram
 * 描述  ：kong
 * 输入  ：空
 * 输出  ：空
 */
void fonthztosram(u16 x, u16 y, struct font_info_t *font_info,struct rtgui_font_bitmap *bmp_font,const char *text)
{
	u16 *sc;
	rt_uint8_t *str;
	register rt_base_t word_bytes, font_bytes;

    word_bytes = (bmp_font->width + 7) / 8;
    font_bytes = word_bytes * bmp_font->height;

	str = (rt_uint8_t *)text;

    while (*str != 0)
    {
        const rt_uint8_t *font_ptr;
        register rt_base_t i, j, k;

        /* get font pixel data */
        font_ptr = _rtgui_hz_bitmap_get_font_ptr(bmp_font, str, font_bytes);
        /* draw word */
        for (i = 0; i < bmp_font->height; i ++)
        {
			sc = font_info->panel->data + font_info->panel->w*(i+y) + x;//计算行地址
            for (j = 0; j < word_bytes; j++)
			{
                for (k = 0; k < 8; k++)
                {
                    if (((font_ptr[i * word_bytes + j] >> (7 - k)) & 0x01) != 0)
						*sc = font_info->fontcolor;
					else if (font_info->alpha == 0)
						*sc = font_info->backcolor;
					sc++;
                }
			}
        }
		x += bmp_font->width;
        str += 2;
    }
}
/* draw a text */
void font_draw(u16 x, u16 y, struct font_info_t *font_info,const char *text)
{
	rt_uint8_t *str=(rt_uint8_t *)text,hz[3]={0,0,0};
	struct rtgui_font_bitmap *font_asc,*font_hz;
    
	switch(font_info->high)
	{
		case 16:
			font_asc 	= &asc16;
			font_hz		= &hz16;
			break;
		case 20:
			font_asc 	= &asc20;
			font_hz		= &hz20;
			break;
		case 24:
			font_asc 	= &asc24;
			font_hz		= &hz24;
			break;
		case 32:
			font_asc 	= &asc32;
			font_hz		= &hz32;
			break;
		default:
			font_asc 	= &asc16;
			font_hz		= &hz16;
			break;
	}
	while(*str != 0)
	{
		if(*str < 0x80)
		{
			fontasctosram(x,y,font_info,font_asc,*str);
			str++;
			x+=font_asc->width;
		}
		else
		{
			hz[0]=*str++;
			hz[1]=*str++;
			fonthztosram(x,y,font_info,font_hz,(const char *)hz);
			x+=font_hz->width;
		}
	}
}

/*
 * 函数名：ui_text_draw
 * 描述  ：kong
 * 输入  ：空
 * 输出  ：空
 */
void ui_text_draw(struct font_info_t *font_info,struct rect_type *rect,char *str)
{
	clr_win(font_info->panel,font_info->backcolor,rect->x,rect->y,rect->h,rect->w);
// 	rt_enter_critical();
	if(font_info->center!=0)
		font_draw(rect->x+(rect->w-rt_strlen(str)*font_info->high/2)/2,rect->y+(rect->h-font_info->high)/2,font_info,str);
	else 
		font_draw(rect->x,rect->y+2,font_info,str);
// 	rt_exit_critical();
	window_updata(font_info->panel,rect);
}

/*
 * 函数名：ui_text_draw
 * 描述  ：kong
 * 输入  ：空
 * 输出  ：空
 */
void ui_text_draw_alpha(struct font_info_t *font_info,struct rect_type *rect,char *str)
{
//	clr_win(font_info->panel,font_info->backcolor,rect->x,rect->y,rect->h,rect->w);
// 	rt_enter_critical();
	if(font_info->center!=0)
		font_draw(rect->x+(rect->w-rt_strlen(str)*font_info->high/2)/2,rect->y+(rect->h-font_info->high)/2,font_info,str);
	else 
		font_draw(rect->x,rect->y+2,font_info,str);
// 	rt_exit_critical();
	window_updata(font_info->panel,rect);
}

/********************************************************************************************/
