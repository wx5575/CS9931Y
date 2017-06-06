#include <rtthread.h>
#include "stm32f4xx.h"
#include "rtc.h"
#include "CS99xx.h"

static struct rt_device rtc;

static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
	struct rtc_time_type *time;
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;
	
    RT_ASSERT(dev != RT_NULL);
	
    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
		time = (struct rtc_time_type *)args;
		RTC_GetDate(RTC_Format_BIN,&rtc_date);
		RTC_GetTime(RTC_Format_BIN,&rtc_time);
		
		time->year		= rtc_date.RTC_Year;
		time->month		= rtc_date.RTC_Month;
		time->date		= rtc_date.RTC_Date;
		time->day		= rtc_date.RTC_WeekDay;
		time->hours		= rtc_time.RTC_Hours;
		time->minutes	= rtc_time.RTC_Minutes;
		time->seconds	= rtc_time.RTC_Seconds;
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
		
        RTC_WriteBackupRegister(RTC_BKP_DR0, 0xA5A5);
    }
    break;
    }

    return RT_EOK;
}

void RTC_Nvic_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	EXTI_ClearITPendingBit(EXTI_Line22);
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	RTC_ITConfig(RTC_IT_WUT, ENABLE);
	RTC_WakeUpCmd(ENABLE);
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);
	RTC_SetWakeUpCounter(0);
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Configuration(void)
{
	RTC_InitTypeDef  RTC_InitStructure;
	RTC_TimeTypeDef  RTC_TimeStructure;
	RTC_DateTypeDef  RTC_DateStructure;
	
    /* 使能PWR时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
// 	RCC_APB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
	
// 	RTC_DeInit();
	/* 允许访问RTC */
	PWR_BackupAccessCmd(ENABLE);
	
	/* 使能LSE振荡器  */
	RCC_LSEConfig(RCC_LSE_ON);

	/* 等待就绪 */  
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
	{
	}

	/* 选择RTC时钟源 */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
	
	/* 使能RTC时钟 */
	RCC_RTCCLKCmd(ENABLE);

	/* 等待RTC APB寄存器同步 */
	RTC_WaitForSynchro();
	
	/* 配置RTC数据寄存器和分频器  */
	RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
	RTC_InitStructure.RTC_SynchPrediv = 0xFF;
	RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
	RTC_Init(&RTC_InitStructure);
	
	
	/* 设置年月日和星期 */
	RTC_DateStructure.RTC_Year = 0x14;
	RTC_DateStructure.RTC_Month = RTC_Month_July;
	RTC_DateStructure.RTC_Date = 0x07;
	RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Monday;
	RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);

	/* 设置时分秒，以及显示格式 */
	RTC_TimeStructure.RTC_H12     = RTC_H12_AM;
	RTC_TimeStructure.RTC_Hours   = 0x12;
	RTC_TimeStructure.RTC_Minutes = 0x00;
	RTC_TimeStructure.RTC_Seconds = 0x00; 
	RTC_SetTime(RTC_Format_BCD, &RTC_TimeStructure);  
	
	/* 配置备份寄存器，表示已经设置过RTC */
	RTC_WriteBackupRegister(RTC_BKP_DR0, 0xA5A5);
	
    return 0;
}


void rt_hw_rtc_init(void)
{
    rtc.type	= RT_Device_Class_RTC;

    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0xA5A5)
    {
//         rt_kprintf("rtc is not configured\n");
//         rt_kprintf("please configure with set_date and set_time\n");
        if ( RTC_Configuration() != 0)
        {
//             rt_kprintf("rtc configure fail...\r\n");
            return ;
        }
    }
    else
    {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_BackupAccessCmd(ENABLE);
		RTC_WaitForSynchro();
		RTC_ClearFlag(RTC_FLAG_ALRAF);
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		RTC_ClearITPendingBit(RTC_IT_ALRB);
		EXTI_ClearITPendingBit(EXTI_Line17);
		EXTI_ClearITPendingBit(EXTI_Line22);
		
		RTC->WPR = 0XCA;
		RTC->WPR = 0X53;
		RTC->CR = 0;
		RTC->WPR = 0XFF;
    }
	RTC_Nvic_Configuration();

    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

    return;
}

