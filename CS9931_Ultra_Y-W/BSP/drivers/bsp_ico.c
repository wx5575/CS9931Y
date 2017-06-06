/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：bsp_ico.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "bsp_ico.h"
#include "RA8875.h"
#include "CS99xx.h"	

rt_uint8_t	ICO_ALPHA	=	1;
rt_uint16_t ICO_COLOR	=	0xf800;
rt_uint16_t ICO_BCOLOR	=	0x0000;

const rt_uint8_t ico1_data[]={0x09,0x00,0x12,0x00,0x00,0x20,0x00,0x00,0x38,0x00,0x00,0x3E,0x00,0xFF,0xFF,0x80,0xFF,0xFF,0xC0,0xFF,0xFF,0x80,0x00,0x3E,0x00,0x00,0x38,0x00,0x00,0x20,0x00};

	
void ico_color_set(rt_uint8_t alpha,rt_uint16_t color,rt_uint16_t bcolor)
{
	ICO_ALPHA = alpha;
	ICO_COLOR = color;
	ICO_BCOLOR = bcolor;
}

void ico_update(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data)
{
	rt_uint16_t h=*(rt_uint16_t *)data,w=*((rt_uint16_t *)data+1);
	struct rect_type rect;
	rect.x = x;
	rect.y = y;
	rect.h = h;
	rect.w = w;
	window_updata(panel,&rect);
}

void ico_copy_bm(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data)
{
	rt_uint8_t i,j,k;
	rt_uint16_t h=*(rt_uint16_t *)data,w=*((rt_uint16_t *)data+1);
	rt_uint8_t *p=data+4,word_bytes = (w + 7) / 8;
	rt_uint16_t *sc;
    rt_uint16_t *pc = (rt_uint16_t *)p;
	for (i = 0; i < h; i ++)
    {
		sc = panel->data + panel->w * (i + y) + x;//计算行地址
        
        for(j = 0; j < w; j++)
        {
            *sc = *pc;
            pc++;
            sc++;
        }
    }
// 	for (i = 0; i < h; i ++)
// 	{
// 		sc = panel->data + panel->w*(i+y) + x;//计算行地址
// 		for (j = 0; j < word_bytes; j++)
// 		{
// // 			for (k = 0; k < 8; k++)
// // 			{
// // // 				if (((*p >> (7 - k)) & 0x01) != 0)
// // // 					*sc = ICO_COLOR;
// // // 				else if(ICO_ALPHA == 0)
// // // 					*sc = ICO_BCOLOR;
// // 				sc++;
// // 			}
//             *sc = *p;
//             sc++;
// 			p++;
// 		}
// 	}
}
void ico_darw(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data)
{
	rt_uint8_t i,j,k;
	rt_uint16_t h=*(rt_uint16_t *)data,w=*((rt_uint16_t *)data+1);
	rt_uint8_t *p=data+4,word_bytes = (w + 7) / 8;
	rt_uint16_t *sc;
	for (i = 0; i < h; i ++)
	{
		sc = panel->data + panel->w*(i+y) + x;//计算行地址
		for (j = 0; j < word_bytes; j++)
		{
			for (k = 0; k < 8; k++)
			{
				if (((*p >> (7 - k)) & 0x01) != 0)
					*sc = ICO_COLOR;
				else if(ICO_ALPHA == 0)
					*sc = ICO_BCOLOR;
				sc++;
			}
			p++;
		}
	}
}

rt_uint8_t sd_ico_darw(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,const char *path)
{
	u16 h,w,word_bytes;
	int fd,length;
	u16 heard[3];
	rt_uint8_t buf[20480];
	rt_uint16_t i,j,k;
	rt_uint16_t *sc;
	rt_uint8_t *p;
	
	rt_enter_critical();
	/* 只读打开进行数据校验*/
	fd = open(path, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("check: open file for read failed\n");
		rt_exit_critical();
		return 1;
	}
	/* 读取数据 */
	length = read(fd, (void *)heard, 6);
	if (length != 6)
	{
		rt_kprintf("check: read file failed\n");
		close(fd);
		rt_exit_critical();
		return 2;
	}
	w = heard[1];
	h = heard[2];
	
	word_bytes = (w + 7) / 8;
	
	length = read(fd, (void *)buf, h*word_bytes);
	
	/* 关闭文件*/
	close(fd);
	rt_exit_critical();
	
	p = buf+6;
	for (i = 0; i < h; i ++)
	{
		sc = panel->data + panel->w*(i+y) + x;//计算行地址
		for (j = 0; j < word_bytes; j++)
		{
			for (k = 0; k < 8; k++)
			{
				if (((*p >> (7 - k)) & 0x01) != 0)
					*sc = ICO_COLOR;
				sc++;
			}
			p++;
		}
	}
	
	return 0;
}

