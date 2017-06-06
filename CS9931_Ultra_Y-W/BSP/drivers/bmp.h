/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：BMP.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __BMP_H
#define __BMP_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "stm32f4xx.h"
#include <rtthread.h>
#include "CS99xx.h"		
		
struct tag_bmp_file_header
{
	rt_uint16_t	type;
	rt_uint32_t	size;
	rt_uint16_t	reserved1;
	rt_uint16_t	reserved2;
	rt_uint32_t	offbits;
};


#define RTGUI_RGB(R,G,B) 	(((R>>3)<<11) | ((G>>2)<<5) | ((B>>3)))


/******************* 函数声明 *********************/
void bmptorgb(u16 h,u16 w,u16 *p1,u8 *p2);
void bmp16torgb(u16 h,u16 w,u16 *p1,u8 *p2);
u8 loadbmptosram(const char *path,u16 *data);
u8 loadbmpbintosram(const char *path,u16 *data);

/********************** 外部用到的变量 **************************/


/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
