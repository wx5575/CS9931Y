/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：BMP.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "bmp.h"

/*
 * 函数名：BMP
 * 描述  ：整机IO配置
 * 输入  ：空
 * 输出  ：空
 */
void bmptorgb(u16 h,u16 w,u16 *p1,u8 *p2)
{
	u16 i,j;
	p1 += w*(h-1);
	for(i = 0;i<h;i++)
	{
		for(j=0; j<w; j++)
		{
			*p1++ = ((u16)(*p2++)>>3) | (((u16)(*p2++)>>2)<<5) | (((u16)(*p2++)>>3)<<11);
		}
		p1 -= w*2;
	}
}

void bmp16torgb(u16 h,u16 w,u16 *p1,u8 *p2)
{
	u16 i,j;
	p1 += w*(h-1);
	for(i = 0;i<h;i++)
	{
		for(j=0; j<w; j++)
		{
			*p1++ = ((u16)(*p2++)<<8) | (u16)(*p2++);
		}
		p1 -= w*2;
	}
}

u8 loadbmptosram(const char *path,u16 *data)
{
	char bmp_header[54];
	u32 bmp_w,bmp_h,bmp_c;
	
	int fd,length;
	
// 	rt_enter_critical();
	/* 只读打开进行数据校验*/
	fd = open(path, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("check: open file for read failed\n");
		rt_exit_critical();
		return 1;
	}
	/* 读取数据 */
	length = read(fd, bmp_header, sizeof(bmp_header));
	if (length != sizeof(bmp_header))
	{
		rt_kprintf("check: read file failed\n");
		close(fd);
		rt_exit_critical();
		return 2;
	}
	bmp_w = *(u32 *)(bmp_header+0x12);
	bmp_h = *(u32 *)(bmp_header+0x16);
	bmp_c = *(u32 *)(bmp_header+0x1c);
	
	if(bmp_c == 0x18)
	{
		length = read(fd, (void *)ExternSramNullAddr, bmp_w*bmp_h*3);
		bmptorgb(bmp_h,bmp_w,(u16 *)data,(u8 *)ExternSramNullAddr);
	}
	else if(bmp_c == 0x10)
	{
		length = read(fd, (void *)ExternSramNullAddr, bmp_w*bmp_h*2);
		bmp16torgb(bmp_h,bmp_w,(u16 *)data,(u8 *)ExternSramNullAddr);
	}
		
	/* 关闭文件*/
	close(fd);
// 	rt_exit_critical();
	return 0;
}

u8 loadbmpbintosram(const char *path,u16 *data)
{
	struct
	{
		u16		start;
		u16		w;
		u16		h;
		u16		end;
	}bmp_bin_header;
	
	int fd,length;
	
// 	rt_enter_critical();
	/* 只读打开进行数据校验*/
	fd = open(path, O_RDONLY, 0);
	if (fd < 0)
	{
		rt_kprintf("check: open file for read failed\n");
		rt_exit_critical();
		return 1;
	}
	/* 读取数据 */
	length = read(fd, &bmp_bin_header, sizeof(bmp_bin_header));
	if (length != sizeof(bmp_bin_header))
	{
		rt_kprintf("check: read file failed\n");
		close(fd);
		rt_exit_critical();
		return 2;
	}
	
	length = read(fd, (void *)data, bmp_bin_header.w*bmp_bin_header.h*2);
		
	/* 关闭文件*/
	close(fd);
// 	rt_exit_critical();
	return 0;
}



/********************************************************************************************/
