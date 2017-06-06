#include "PLC.h"
#include "stm32f4xx.h"
#include "driver.h"

void PLC_Interface_Init(void)
{
	/*引脚初始化*/
	
	GPIO_InitTypeDef GPIO_InitStructure;
	/* 使能 GPIO时钟 */
	//已经在别处初始化过  CS99xx.c
	
	//PLC_Start PD 3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	
	
}

void PLC_Testing_Out(uint8_t state)
{
	if(state)
	{
		Relay_ON(EXT_DRIVER_O1);
	}
	else
	{
		Relay_OFF(EXT_DRIVER_O1);
	}
}


void PLC_Pass_Out(uint8_t state)
{
	if(state)
	{
		Relay_ON(EXT_DRIVER_O3);
	}
	else
	{
		Relay_OFF(EXT_DRIVER_O3);
	}
}


void PLC_Fail_Out(uint8_t state)
{
	if(state)
	{
		Relay_ON(EXT_DRIVER_O2);
	}
	else
	{
		Relay_OFF(EXT_DRIVER_O2);
	}
}
