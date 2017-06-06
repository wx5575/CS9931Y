#ifndef __SPI_CPLD_H
#define __SPI_CPLD_H

#if defined(__cplusplus)
    extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*包含的头文件*/
#include <stdint.h>
#include "stm32f4xx.h"

		
/******************************* 功能选择开关 ***********************************/
#define  CPLD_WRITE          (0x01)
#define  CPLD_READ           (0x02)		

#define  CMD_GPIO_WRITE      (0x00)	
#define  CMD_IO16_WRITE      (0x01)			
#define  CMD_ADCW_RATE_WRITE (0x02)
#define  CMD_GR_RATE_WRITE   (0x03)		
#define  CMD_SINE_CONTROL    (0x04)			

#define  CMD_ERROR_READ      (0x00)	
#define  CMD_OSC_READ        (0x01)			
			
#define  Self_CD4051_A		   (1<<0)	
#define  Self_CD4051_B       (1<<1)	
#define  Self_CD4051_C       (1<<2)	
#define  Self_CD4051_EN      (1<<3)	
#define  METER_SOURCE_C      (1<<4)	
#define  W_OUT_C             (1<<5)	
#define  OUT_C               (1<<6)	
#define  GR_CD4053_B         (1<<7)	
#define  GR_CD4053_A         (1<<8)	
#define  GR_CD4053_C         (1<<9)	
#define  W_CD4051_A          (1<<10)	
#define  W_CD4051_B          (1<<11)	
#define  W_CD4053_B          (1<<12)	
#define  W_CD4053_A          (1<<13)	
#define  W_CD4053_C          (1<<14)	
#define  SINE_CD4053_A       (1<<15)	
#define  SINE_CD4053_B       (1<<16)	

#define  ADCW_SINE           (1<<0)
#define  GR_SINE             (1<<0)
#define  ADCW_SINE_CMD       (0x0066)
#define  GR_SINE_CMD         (0xEE00)

#define  ON                  (1)
#define  OFF                 (0)

/********************** 外部用到的变量 **************************/

		

/**************************************************/
#if defined(__cplusplus)
    }
#endif 
		
		
		
/******************* 函数声明 *********************/
void spi_cpld_init(void);
uint16_t CPLD_Contorl(uint8_t W_or_R,uint8_t cmd,uint16_t data);
void CPLD_GPIO_Control(uint32_t pin,uint8_t dst_stat);
void CPLD_Sine_SetRate(uint8_t ch,uint16_t rate);
void CPLD_Sine_Control(uint8_t ch,uint8_t ON_or_OFF);		
		
/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
