/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：_ICO.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef _ICO_H
#define _ICO_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
		
#include "stm32f4xx.h"
#include <rtthread.h>
#include "ui_config.h"
		
extern const rt_uint8_t ico1_data[];

void ico_color_set(rt_uint8_t alpha,rt_uint16_t color,rt_uint16_t bcolor);
void ico_update(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data);
void ico_copy_bm(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data);
void ico_darw(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,rt_uint8_t *data);
rt_uint8_t sd_ico_darw(struct panel_type *panel,rt_uint16_t x,rt_uint16_t y,const char *path);
/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif	
