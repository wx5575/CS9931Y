/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：spi_cpld.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __SPI_CPLD_H
#define __SPI_CPLD_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "stm32f4xx.h"

		
/******************************* 功能选择开关 ***********************************/


/******************* 函数声明 *********************/
void spi_cpld_init(void);
void cpld_write(u32 SendData);
u16 ReadDataFromCPLD(u32 SendData);


/********************** 外部用到的变量 **************************/

		

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
