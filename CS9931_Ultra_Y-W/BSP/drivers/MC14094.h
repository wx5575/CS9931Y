/*
 * Copyright(c) 2013,
 * All rights reserved
 * 文件名称：MC14094.h
 * 摘  要  ：头文件
 * 当前版本：V2.0，孙世成编写
 * 修改记录：
 * V1.0, 2013.09.05, 此版本配合赵工设计CS88xx(简易型)及CS99xx系列主板设计，目前仅适用CS88xx(简易型)及CS99xx驱动系列
 * V2.0, 2014.07.26, 此版本在CS88xx(简易型)及CS99xx驱动基础上配合赵工设计CS99xxZ(7寸屏)系列主板修改，目前仅适用CS99xxZ(7寸屏)系列
 *
 */
 
#ifndef __MC14094_H
#define __MC14094_H


#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif
			
/******************* <Include> ********************/
#include <stm32f4xx.h>
			
/******************* <define> *********************/

enum 
{
	MC14094,
	MC14094K,
};

enum
{
	W_IR_FILE6,
	W_IR_FILE1,
	RET_GND_SELECT,
	AC_DC,
	W_IR_FILE2,
	W_IR_FILE3,
	W_IR_FILE4,
	W_IR_FILE5,
};

enum
{
	AMP_RELAY4,
	AMP_RELAY3,
	AMP_RELAY2,
	AMP_RELAY1,
	AMP_RELAY5,
	AMP_RELAY6,
	AMP_RELAY7,
	GFI_GND_SELECT,
	
	EXT_DRIVER_O4,
	EXT_DRIVER_O3,
	EXT_DRIVER_O2,
	EXT_DRIVER_O1,
	EXT_DRIVER_O5,
	EXT_DRIVER_O6,
	EXT_DRIVER_O7,
	EXT_DRIVER_O8,
};

/******************* 函数声明 *********************/
void MC14094_Init(void);
void MC14094_CMD(u8 id,u8 bits,u8 status);
void MC14094_Updata(u8 id);

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif

