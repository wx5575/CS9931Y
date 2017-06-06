/*
 * Copyright(c) 2013,
 * All rights reserved
 * 文件名称：BUZZER.h
 * 摘  要  ：头文件
 * 当前版本：V1.0，孙世成编写
 * 修改记录：
 * V1.0, 2014.07.26, 此版本配合赵工设计CS99xxZ(7寸屏)系列主板设计，目前仅适用CS99xxZ(7寸屏)系列
 *
 */
#ifndef __BUZZER_H
#define __BUZZER_H


#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
			
/******************* <Include> ********************/
#include <stm32f4xx.h>
			
/******************* <define> *********************/


/******************* 函数声明 *********************/
void buzzer_init(void);

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif

