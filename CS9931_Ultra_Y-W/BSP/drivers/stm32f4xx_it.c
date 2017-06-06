/**
  ******************************************************************************
  * @file    IO_Toggle/stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <rtthread.h>
#include "board.h"
#include "CS99xx.h"
#include "Test_Sched.h"

// extern struct rt_mailbox status_mb;
/** @addtogroup STM32F4_Discovery_Peripheral_Examples
  * @{
  */

/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
//void HardFault_Handler(void)
//{
//    // definition in libcpu/arm/cortex-m4/context_*.S
//}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)
//{
//    // definition in libcpu/arm/cortex-m4/context_*.S
//}

static void Irq_Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}

static void Irq_Delay(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<1000;dly_i++);
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//    // definition in boarc.c
//}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void USART1_IRQHandler_unuse(void)
{
#ifdef RT_USING_UART1
    extern struct rt_device uart1_device;
	extern void rt_hw_serial_isr(struct rt_device *device);

    /* enter interrupt */
    rt_interrupt_enter();

    rt_hw_serial_isr(&uart1_device);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

// void USART2_IRQHandler(void)
// {
// #ifdef RT_USING_UART2
//     extern struct rt_device uart2_device;
// 	extern void rt_hw_serial_isr(struct rt_device *device);

//     /* enter interrupt */
//     rt_interrupt_enter();

//     rt_hw_serial_isr(&uart2_device);

//     /* leave interrupt */
//     rt_interrupt_leave();
// #endif
// }

void USART3_IRQHandler(void)
{
#ifdef RT_USING_UART3
    extern struct rt_device uart3_device;
	extern void rt_hw_serial_isr(struct rt_device *device);

    /* enter interrupt */
    rt_interrupt_enter();

    rt_hw_serial_isr(&uart3_device);

    /* leave interrupt */
    rt_interrupt_leave();
#endif
}

void EXTI3_IRQHandler(void)
{
	if(panel_flag != 0){
		if(system_parameter_t.key_lock==0)rt_mb_send(&key_mb, CODE_RIGHT);
		buzzer(3);
	}
	EXTI_ClearITPendingBit(EXTI_Line3);
}

extern void short_int(void);
extern void arc_int(void);
extern rt_uint8_t panel_flag;
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line15) != RESET)
	{
		if(panel_flag != 0){

			if(system_parameter_t.key_lock==0)rt_mb_send(&key_mb, CODE_LEFT);
			buzzer(3);
		}else{
			Irq_Delay(2);
			if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_15) == 0)
				short_int();
		}
		EXTI_ClearITPendingBit(EXTI_Line15);
	}
	else
	{
		
	}
	
	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
	{	
		EXTI->IMR &= ~(1<<10);	             /* 关闭中断       */
		EXTI_ClearITPendingBit(EXTI_Line10); /* 清除中断标志位 */
			
		Irq_Delay(1000);
		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_10)){EXTI->IMR |= (1<<10);return;}
		short_int();
		EXTI->IMR |= (1<<10);
	}
	
}


extern void Test_Sched_Close(void);
extern void GFI_int(void);
void EXTI9_5_IRQHandler(void)
{
	/* 测试停止中断 */
	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		
		EXTI->IMR &= ~(1<<8);	              /* 关闭中断       */
		EXTI_ClearITPendingBit(EXTI_Line8); /* 清除中断标志位 */
		 
		Irq_Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_8)){EXTI->IMR |= (1<<8);return;}
// 		Test_Sched_Close();
// 		EXTI->IMR |= (1<<8);
		
		/* 复位中断 */
		if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_8) == 0)
		{
			Test_Sched_Close();
			EXTI->IMR |= (1<<8);
			return;
		}
		
		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_8) == 0){
			GFI_int();
			EXTI->IMR |= (1<<8);
			return;
		}
		
		EXTI->IMR |= (1<<8);
		
	}
	else
	{
		
	}
	
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{	
		EXTI->IMR &= ~(1<<7);	             /* 关闭中断       */
		EXTI_ClearITPendingBit(EXTI_Line7); /* 清除中断标志位 */
		
		Irq_Delay(5);
		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_7)){EXTI->IMR |= (1<<7);return;}
		short_int();
		EXTI->IMR |= (1<<7);
	}
	
	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
	{	
		EXTI->IMR &= ~(1<<6);	             /* 关闭中断       */
		EXTI_ClearITPendingBit(EXTI_Line6); /* 清除中断标志位 */
			
		Irq_Delay(2);
		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_6)){EXTI->IMR |= (1<<6);return;}
		arc_int();
		EXTI->IMR |= (1<<6);
	}
}
/*
*********************************************************************************************************
*	函 数 名: RTC_Alarm_IRQHandler
*	功能说明: 闹钟中断。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void RTC_WKUP_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
	{
		rt_mb_send(&screen_mb, UPDATE_STATUS | STATUS_TIME_EVENT);
		RTC_ClearITPendingBit(RTC_IT_WUT);
		EXTI_ClearITPendingBit(EXTI_Line22);
	} 
}
/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
