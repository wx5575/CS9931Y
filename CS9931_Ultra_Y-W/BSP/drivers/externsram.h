/*
 * Copyright(c) 2013,
 * All rights reserved
 * 文件名称：externsram.h
 * 摘  要  ：头文件
 * 当前版本：V1.0，孙世成编写
 * 修改记录：
 * V1.0, 2014.07.26, 此版本配合赵工设计CS99xxZ(7寸屏)系列主板设计，目前仅适用CS99xxZ(7寸屏)系列
 *
 */
#ifndef __EXTERNSRAM_H
#define __EXTERNSRAM_H


#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
			
/******************* <Include> ********************/
#include <stm32f4xx.h>
#include "CS99xx.h"		
		
/******************* <define> *********************/
#define		EXTERNSRAMADDR		(__IO uint8_t *)((uint32_t)(0x64000000))

#define		SRAM_FILE_START			(0X00000000)
#define		SRAM_FILE_END			(0x00001000)
		
	
struct file_info_sram_t
{
	struct file_info_t	t[FILE_NUM];
};
		
#define		file_info_sram			((struct file_info_sram_t *)(EXTERNSRAMADDR + SRAM_FILE_START))


/******************* 函数声明 *********************/
void externsram_init(void);

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif

