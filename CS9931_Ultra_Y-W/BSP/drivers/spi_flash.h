/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：spi_flash.H
 * 摘  要  ：头文件
 * 当前版本：V1.0，编写
 * 修改记录：
 *
 */
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include "stm32f4xx.h"
#include "string.h"
#include <rtthread.h>

/******************************* 结构体变量 ***********************************/
struct sflash_head
{
	uint32_t	bkp;
	uint32_t	list;
	uint32_t	null_list;
};

typedef struct
{
	uint32_t ChipID;		/* 芯片ID */
	char ChipName[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t TotalSize;		/* 总容量 */
	uint16_t PageSize;		/* 页面大小 */
}SFLASH_T;

/******************* 函数声明 *********************/
void flash_lock(void);
void flash_unlock(void);
		
void spi_flash_init(void);

void FLASH_CS_SET(char n);
uint32_t sf_ReadID(void);
void sf_EraseChip(void);
void sf_EraseSector(uint32_t _uiSectorAddr);
void sf_PageWrite(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t sf_WriteBuffer(uint8_t* _pBuf, uint32_t _uiWriteAddr, uint16_t _usWriteSize);
void sf_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

extern SFLASH_T g_tSF;


char sf_wait_find(void);
void sf_wait_find_dis(void);

/********************** 外部用到的变量 **************************/

		

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
