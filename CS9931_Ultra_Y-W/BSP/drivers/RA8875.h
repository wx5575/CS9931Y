/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：RA8875.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __RA8875_H
#define __RA8875_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
		
#include "stm32f4xx.h"
#include "ui_config.h"
#include <rtthread.h>

#define RA8875_BASE		((uint32_t)(0x60000000))

#define RA8875_REG		*(__IO uint16_t *)(RA8875_BASE +  (1 << (0 + 1)))	/* FSMC 16位总线模式下，FSMC_A18口线对应物理地址A19 */
#define RA8875_RAM		*(__IO uint16_t *)(RA8875_BASE)


/* 可供外部模块调用的函数 */
void RA8875_InitHard(void);
void RA8875_DrawBMP(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth, uint16_t *_ptr);
void window_updata(struct panel_type *parent,struct rect_type *rect);

void RA8875_DrawICO(u16 x,u16 y,u16 c,u16 bc,const unsigned char* data);
		
void RA8875_SetDispWin(uint16_t _usX, uint16_t _usY, uint16_t _usHeight, uint16_t _usWidth);		
/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
