/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"
#include "driver.h"


/**
 * @addtogroup STM32
 */

/*@{*/

/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : Configures Vector Table base location.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NVIC_Configuration(void)
{
#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
//	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */
//	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}

/*******************************************************************************
 * Function Name  : SysTick_Configuration
 * Description    : Configures the SysTick for OS tick.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void  SysTick_Configuration(void)
{
// 	RCC_ClocksTypeDef  rcc_clocks;
// 	rt_uint32_t         cnts;

// 	RCC_GetClocksFreq(&rcc_clocks);

// 	cnts = (rt_uint32_t)rcc_clocks.HCLK_Frequency / RT_TICK_PER_SECOND;
// 	cnts = cnts / 8;

// 	SysTick_Config(cnts);
// 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
	SysTick_Config(SystemCoreClock / 1000);	// 10ms中断一次
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}



/**
 * This is the timer interrupt service routine.
 *
 */
extern void SysTick_Dispose(void);
#include "CS99xx.h"
void SysTick_Handler(void)
{
	static uint32_t SysTick_counter = 0;
	/* enter interrupt */
	rt_interrupt_enter();
	if((++SysTick_counter % 10) == 0)
		rt_tick_increase();
	SysTick_Dispose();
	
/***************************************************/
/* wangxin 2016.11.28 */
	gfi_cycle_clear_count();
/***************************************************/
	/* leave interrupt */
	rt_interrupt_leave();
}

static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

/**
 * This function will initial STM32 board.
 */
// #include "components.h"

extern void Usart1_init(void);
void rt_hw_board_init()
{
	//	AD_DA_Config();
	Multiplexer_Control_Init();
	Relay_Control_Init();
	spi_cpld_init();
	LC_Init();
	
	AC_Output_Disable();
	DC_Output_Disable();
	GR_Output_Disable();
	LC_Assit_Output_Disable();
	LC_Main_Output_Disable();
	
	/* NVIC Configuration */
	NVIC_Configuration();

	/* Configure the SysTick */
	SysTick_Configuration();

	rt_hw_usart_init();
#ifdef RT_USING_CONSOLE
	rt_console_set_device(CONSOLE_DEVICE);
#endif
	
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#ifndef	_WIN32
	CS99xx_GPIO_Config();
	CS99xx_Peripheral_Config();
	
	Usart1_init();
	Delay_ms(2000);
	
	RA8875_InitHard();
	
	{
		u32 timeout=10000,sd_status;
		/* 加载中 */
		
		do{
			sd_status = SD_Init();
		}while(timeout--&&sd_status);
        
		if(timeout==0)
		{
			/* 加载错误 */
		}
	}
	
	{
		extern void spi_flash_init(void);
		spi_flash_init();
	}
	
// 	MC14094_Init();
    
// 	{
// // 		extern void externsram_init(void);
// // 		externsram_init();
// 	}
#endif
}

/*@}*/
