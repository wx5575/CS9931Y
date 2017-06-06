/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：Text.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include <rtthread.h>
#include "stm32f4xx.h"
#include "ui_config.h"
#include "bsp_font.h"

#define	text_color		0x53fa
#define _font_color		0xf800


static void textbox_draw_caret(struct font_info_t *font, struct rect_type *rect, char *str, rt_uint16_t position)
{
	rt_enter_critical();
	clr_win(font->panel,text_color,rect->x,rect->y,rect->h,rect->w);
// 	font_info_set(box,_font_color,0x0,1,16);
	font_draw(rect->x,rect->y+2,font,str);
	clr_win(font->panel,0,rect->x+position*8,rect->y+2,16,2);
	rt_exit_critical();
	window_updata(font->panel,rect);
}

static void textbox_draw_carets(struct font_info_t *font, struct rect_type *rect, char *str, rt_uint16_t position)
{
	rt_enter_critical();
	clr_win(font->panel,text_color,rect->x,rect->y,rect->h,rect->w);
// 	font_info_set(box,_font_color,0x0,1,16);
	clr_win(font->panel,CL_YELLOW,rect->x+position*8,rect->y+2,16,8);
	font_draw(rect->x,rect->y+2,font,str);
	rt_exit_critical();
	window_updata(font->panel,rect);
}

static void textbox_draw(struct font_info_t *font, struct rect_type *rect, char *str)
{
	rt_enter_critical();
	clr_win(font->panel,text_color,rect->x,rect->y,rect->h,rect->w);
// 	font_info_set(box,_font_color,0x0,1,16);
	font_draw(rect->x,rect->y+2,font,str);
	rt_exit_critical();
	window_updata(font->panel,rect);
}

static void insert_char(char *str,char ch,rt_uint8_t position)
{
	char buf[20];
	strcpy(buf,str+position);
	str[position] = ch;
	strcpy(str+position+1,buf);
}
static void delete_char(char *str,rt_uint8_t position)
{
	strcpy(str+position-1,str+position);
}

const char *num_keys[10]={"0+/-","1ABCabc","2DEFdef","3GHIghi","4JKLjkl","5MNOmno","6PQRpqr","7STUstu","8VWXvwx","9YZyz "};

rt_uint8_t text_input(struct panel_type *parent,struct rect_type *rect,char *str)
{
	rt_uint32_t msg,code,mcode=0;
	rt_uint8_t	timeout=0;
	rt_uint8_t	caret_pos=rt_strlen(str);
	rt_uint8_t	input_n = 0;
	struct font_info_t _font={0,_font_color,0x0,1,0,16};
	_font.panel = parent;
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			if(msg & KEY_DOWN)
			{
				code = msg & 0x1f;
				switch(code)
				{
					case KEY_F6:
					case KEY_EXIT:
						return 1;
					case KEY_D:
					case KEY_ENTER:
						return 0;
					case KEY_L:
						if(caret_pos > 0)
							delete_char(str,caret_pos--);
						timeout = 0;
						textbox_draw_carets(&_font,rect,str,caret_pos-1);
						break;
					case KEY_R:
						if(caret_pos < rt_strlen(str))
							delete_char(str,caret_pos+1);
						timeout = 0;
						textbox_draw_carets(&_font,rect,str,caret_pos-1);
						break;
					default:
						if((code >= KEY_NUM0) && (code <= KEY_NUM9))
						{
							if(mcode != code)
							{
								input_n	= 1;

								insert_char(str,num_keys[code-KEY_NUM0][0],caret_pos);
								caret_pos++;
								mcode = code;
							}
							else
							{
								if(input_n > rt_strlen(num_keys[code-KEY_NUM0]))input_n=1;
								str[caret_pos-1] = num_keys[code-KEY_NUM0][input_n-1];
								input_n++;
								
							}
							timeout = 0;
							textbox_draw_carets(&_font,rect,str,caret_pos-1);
						}
						break;
				}
			}
			else
				switch(msg)
				{
					case CODE_RIGHT:
						if(caret_pos < rt_strlen(str))caret_pos++;
						timeout = 0;
						textbox_draw_carets(&_font,rect,str,caret_pos-1);
						break;
					case CODE_LEFT:
						if(caret_pos > 0)caret_pos--;
						timeout = 0;
						textbox_draw_carets(&_font,rect,str,caret_pos-1);
						break;
				}
		}
		else
		{
			timeout ++;
			if(input_n ==0)
			{
				if(timeout > 1)
				{
					textbox_draw_caret(&_font,rect,str,caret_pos);
					timeout = 0;
				}
				else
				{
					textbox_draw(&_font,rect,str);
				}
			}
			else
			{
				if(timeout>2)
				{
					timeout = 0;
					input_n = 0;
					mcode	= 0;
					textbox_draw_caret(&_font,rect,str,caret_pos);
				}
			}
		}
	}
}

rt_uint32_t	num_input(struct font_info_t *font,struct rect_type *rect,struct num_format *num)
{
	rt_uint32_t msg,code=0;
	rt_uint8_t	timeout=0,caret_pos=0,pos_max=0;
	char buf[20],format[20];
	u16 x0,y0;
	
	if(num->_dec>0)
	{
		strcpy(format,"%02d.%03d");
		format[2] = '0' + num->_int;
		format[7] = '0' + num->_dec;
		rt_sprintf(buf,format,num->num/exp10(num->_dec),num->num%exp10(num->_dec));
		pos_max = num->_int+num->_dec;
	}
	else
	{
		strcpy(format,"%01d");
		format[2] = '0' + num->_int;
		rt_sprintf(buf,format,num->num);
		pos_max = num->_int-1;
	}
	strcat(buf,num->unit);
	x0 = rect->x + (rect->w-rt_strlen(buf)*8)/2;
	y0 = rect->y + 17;
	
	ui_text_draw(font,rect,buf);
	while(1)
	{
		if (rt_mb_recv(&key_mb, (rt_uint32_t*)&msg, RT_TICK_PER_SECOND/2) == RT_EOK)
		{
			if((msg & KEY_DOWN)!=0 || msg==CODE_LEFT || msg==CODE_RIGHT)
			{
				code = msg & 0x1f;
				if((code >= KEY_NUM0) && (code <= KEY_NUM9))
				{
					code -= KEY_NUM0;
					buf[caret_pos] = '0' + code;
					if(caret_pos<pos_max)caret_pos++;
					else caret_pos=0;
					if(buf[caret_pos] == '.')caret_pos++;
				}
				else if(code == KEY_L)
				{
					if(caret_pos>0)caret_pos--;
					else caret_pos=pos_max;
					if(buf[caret_pos] == '.')caret_pos--;
				}
				else if(code == KEY_R)
				{
					if(caret_pos<pos_max)caret_pos++;
					else caret_pos=0;
					if(buf[caret_pos] == '.')caret_pos++;
				}
				else if(code == KEY_U || code==CODE_RIGHT)
				{
					if(buf[caret_pos] < '9')buf[caret_pos]++;
				}
				else if(code == KEY_D || code==CODE_LEFT)
				{
					if(buf[caret_pos] > '0')buf[caret_pos]--;
				}
				ui_text_draw(font,rect,buf);
				clr_win(font->panel,font->fontcolor,x0+caret_pos*8,y0,2,8);
				window_updata(font->panel,rect);
			}
			else if(msg == (KEY_ENTER | KEY_UP))
			{
				msg = strtonum(num->_int,num->_dec,buf);
				if(msg > num->max)msg = num->max;
				if(msg < num->min)msg = num->min;
				return msg;
			}
			else if((msg == (KEY_EXIT | KEY_UP)) || (msg == (KEY_F6 | KEY_UP)))
			{
				return 0xffffffff;
			}
		}
		else
		{
			timeout ++;
			if(timeout > 1)
			{
				ui_text_draw(font,rect,buf);
				timeout = 0;
			}
			else
			{
				ui_text_draw(font,rect,buf);
				clr_win(font->panel,font->fontcolor,x0+caret_pos*8,y0,2,8);
				window_updata(font->panel,rect);
			}
		}
	}
}

u8	log10(u32 num)
{
	u8 n=1;
	while(num>=10)
	{
		num/=10;
		n++;
	}
	return n;
}

u32 exp10(u8 n)
{
	u32 num=1;
	while(n--)
		num *= 10;
	return num;
}

u32 strtonum(u8 _int,u8 _dec,char *str)
{
	u8 i;
	u32 num=0,t=1;
	char buf[20];
	strcpy(buf,str);
	if(_dec>0)
		strcpy(buf+_int,str+_int+1);
	for(i=_int+_dec;i>0;i--)
	{
		num+= t*(buf[i-1]-'0');
		t*=10;
	}
	return num;
}
