/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：bsp_graph.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __BSP_GRAPH_H
#define __BSP_GRAPH_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "stm32f4xx.h"
#include <rtthread.h>
#include "CS99xx.h"		
		





/******************* 函数声明 *********************/
void draw_fillrect(struct panel_type *p,struct rect_type *rect,u16 c);
void draw_rect(struct panel_type *p,struct rect_type *rect,u16 c,u8 bold);
void draw_bmp(struct panel_type *p,struct rect_type *rect,u16 *d);
void draw_alphabmp(struct panel_type *p,struct rect_type *rect,u16 *d,u16 ac);

void window_copy(struct panel_type *p1,struct panel_type *p2,struct rect_type *rect);


/********************** 外部用到的变量 **************************/


/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
