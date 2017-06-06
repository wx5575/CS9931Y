/*
 * Copyright(c) 2014,
 * All rights reserved
 * 文件名称：spi_cpld.C
 * 摘  要  ：主程序
 * 当前版本：V1.0，编写
 * 修改记录：
 */

/******************* 加载文件 *********************/
#include "spi_cpld.h"
#include <math.h>

// static void Irq_Delay_ms(unsigned int dly_ms)
// {
//   unsigned int dly_i;
//   while(dly_ms--)
//     for(dly_i=0;dly_i<18714;dly_i++);
// }


#define			SPI_CS_EN()				GPIO_ResetBits(GPIOF,GPIO_Pin_9)
#define			SPI_CS_DIS()			GPIO_SetBits(GPIOF,GPIO_Pin_9)
#define			SPI_MOSI_HIGH()		GPIO_SetBits(GPIOF,GPIO_Pin_8)
#define			SPI_MOSI_LOW()		GPIO_ResetBits(GPIOF,GPIO_Pin_8)
#define			SPI_CLK_HIGH()		GPIO_SetBits(GPIOF,GPIO_Pin_7)
#define			SPI_CLK_LOW()			GPIO_ResetBits(GPIOF,GPIO_Pin_7)
#define			SPI_MISO_DI()			GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_11)



/*
 * 函数名：cpld_int1_irq
 * 描述  ：cpld中断处理函数
 * 输入  ：空
 * 输出  ：空
 */
void cpld_int1_irq(void)
{
	/*在此处添加中断处理代码*/
}

/*
 * 函数名：cpld_int2_irq
 * 描述  ：cpld中断处理函数
 * 输入  ：空
 * 输出  ：空
 */
void cpld_int2_irq(void)
{
	/*在此处添加中断处理代码*/	
}

/*
 * 函数名：cpld_int3_irq
 * 描述  ：cpld中断处理函数
 * 输入  ：空
 * 输出  ：空
 */
void cpld_int3_irq(void)
{
	/*在此处添加中断处理代码*/	
}

/*
 * 函数名：cpld_int4_irq
 * 描述  ：cpld中断处理函数
 * 输入  ：空
 * 输出  ：空
 */
void cpld_int4_irq(void)
{
	/*在此处添加中断处理代码*/	
}


/*
 * 函数名：spi_gpio_init
 * 描述  ：初始化管脚
 * 输入  ：空
 * 输出  ：空
 */
static void Spi_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOI, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			  /* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		  /* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	  /* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO口最大速度 */

	
	/* CPLD_CLK,CPLD_DI(实际为CPLD的数据输入),CPLD_CS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		    /* 设为输入口 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		    /* 上拉电使能 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
}

/*
 * 函数名：CPLD_INT_NVIC_Configuration
 * 描述  ：初始化中断向量表
 * 输入  ：空
 * 输出  ：空
 */
void CPLD_INT_NVIC_Configuration(void)
{
	
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
// 	NVIC_InitTypeDef   NVIC_InitStructure;
	/*打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE, ENABLE);
	
	/* 配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
// 	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
// 	GPIO_Init(GPIOC, &GPIO_InitStructure);
// 	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
// 	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	
	/* 使能SYSCFG时钟 */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* 连接 EXTI Line10 到 PI10 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOI, EXTI_PinSource10);
	/* 连接 EXTI Line9  到 PI9 引脚 */
// 	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOI, EXTI_PinSource9);
	/* 连接 EXTI Line13 到 PC13 引脚 */
// 	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource13);
	/* 连接 EXTI Line6  到 PE6 引脚 */
// 	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource6);
	
	
	/* 配置 EXTI LineXXX */
	EXTI_InitStructure.EXTI_Line    = EXTI_Line10;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* 设置NVIC优先级分组为Group2：0-3抢占式优先级，0-3的响应式优先级 */
// 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
// 	
// 	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);
// 	
// 	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
// 	NVIC_Init(&NVIC_InitStructure);
// 	
}





/*
 * 函数名：CPLD_Write
 * 描述  ：往CPLD写命令，并返回一字节
 * 输入  ：命令
 * 输出  ：返回值
 */
static void CPLD_Write(uint32_t SendData)
{
	uint8_t  ShiftCounter;
	SPI_CS_EN();																				//CS=0;片选CPLD

	for(ShiftCounter=0;ShiftCounter<26;ShiftCounter++)
	{
	  SPI_CLK_HIGH(); 																//时钟置高
	  if(SendData & 1<<ShiftCounter)
	  {
		  SPI_MOSI_HIGH();
	  }
	  else
	  {
		  SPI_MOSI_LOW();
	  }
	  SPI_CLK_LOW();																	//数据写入CPLD	
   }
	 SPI_CS_DIS();																		//CS=1;取消片选

}

//从CPLD中读数据static 
uint16_t ReadDataFromCPLD(uint32_t SendData)
{
	uint8_t  ShiftCounter;
	uint16_t DataBuffer=0;

	CPLD_Write(SendData);																//给CPLD发送读命令
	
	SPI_CS_EN();																							//片选
		
	for(ShiftCounter=0;ShiftCounter<16;ShiftCounter++)
	{

		DataBuffer = DataBuffer << 1;													//左移一位

    SPI_CLK_HIGH();																				//时钟置高
	 
	  SPI_CLK_LOW();
    
		
	  if(SPI_MISO_DI())
  	{
	    DataBuffer = DataBuffer | 0x0001;  									 	//低位置1
 	  }
//     else
//     {
// 	    DataBuffer = DataBuffer & 0xfffe; 										//低位置0
// 	  }		
 
  }	
	SPI_CS_DIS();
	
	return(DataBuffer);
}

/*
*********************************************************************************************************
*	函 数 名: CPLD_Contorl
*	功能说明: CPLD控制
*	形    参: W_or_R:  读 或者 写
            cmd   :  命令字
						data  :  参数
*	返 回 值: 读出的值
*********************************************************************************************************
*/
uint16_t CPLD_Contorl(uint8_t W_or_R,uint8_t cmd,uint16_t data)
{
	uint32_t cmd_temp = 0;
	switch (W_or_R){
		case CPLD_WRITE:
			cmd_temp |= W_or_R << 24;
			switch (cmd){
				case CMD_GPIO_WRITE:
				case CMD_IO16_WRITE:
				case CMD_ADCW_RATE_WRITE:
				case CMD_GR_RATE_WRITE:
				case CMD_SINE_CONTROL:
						cmd_temp |= cmd << 16;
						CPLD_Write(cmd_temp+data);
				break;
				default:
				
				break;
				
			}
			
		return 0;
//		break;
		
		case CPLD_READ:
			cmd_temp |= W_or_R << 24;
			switch (cmd){
				case CMD_ERROR_READ:
				case CMD_OSC_READ:
						cmd_temp |= cmd << 16;
				return ReadDataFromCPLD(cmd_temp);
				default:
				
				break;
				
			}
		return 0;		
//		break;
		
		default:
		return 0;
//		break;
	}
}

/*
*********************************************************************************************************
*	函 数 名: CPLD_GPIO_Control
*	功能说明: CPLD控制的GPIO
*	形    参: pin        :  操作的引脚
            dst_stat   :  1 or 0
*	返 回 值: 无
*********************************************************************************************************
*/
void CPLD_GPIO_Control(uint32_t pin,uint8_t dst_stat)
{
	static uint16_t gpio_pin_stat = 0;
	if(pin & 1<<16){
		if(dst_stat) CPLD_Contorl(CPLD_WRITE,CMD_IO16_WRITE,0xFFFF);
		else         CPLD_Contorl(CPLD_WRITE,CMD_IO16_WRITE,0x0000);
	}
	
	if(dst_stat){
		gpio_pin_stat |= pin;
	}else{
		gpio_pin_stat &= ~pin;
	}
		
	CPLD_Contorl(CPLD_WRITE,CMD_GPIO_WRITE,gpio_pin_stat);
}


/*
*********************************************************************************************************
*	函 数 名: CPLD_Sine_Control
*	功能说明: CPLD控制的正弦波输出
*	形    参: ch          :  操作的通道
            ON_or_OFF   :  1 or 0
*	返 回 值: 无
*********************************************************************************************************
*/
static uint16_t CPLD_Sine_State = 0;
void CPLD_Sine_Control(uint8_t ch,uint8_t ON_or_OFF)
{
	if(ch & ADCW_SINE)
	{
		if(ON_or_OFF)
		{
			CPLD_Sine_State |= ADCW_SINE_CMD;
		}
		else
		{
			CPLD_Sine_State &= 0xFF00;		
		}
	}
	
	if(ch & GR_SINE)
	{
		if(ON_or_OFF)
		{
			CPLD_Sine_State |= GR_SINE_CMD;
		}
		else
		{
			CPLD_Sine_State &= 0x00FF;
		}
	}
	
	CPLD_Contorl(CPLD_WRITE,CMD_SINE_CONTROL,CPLD_Sine_State);	
}

/*
*********************************************************************************************************
*	函 数 名: CPLD_Sine_SetRate
*	功能说明: CPLD控制的正弦波的频率
*	形    参: ch          :  操作的通道
            rate        :  频率值
*	返 回 值: 无
*********************************************************************************************************
*/
void CPLD_Sine_SetRate(uint8_t ch,uint16_t rate)
{
	uint16_t rate_temp;
	rate_temp = (uint16_t)((float)rate * 33.554432f);
	if(ch & ADCW_SINE){
		CPLD_Contorl(CPLD_WRITE,CMD_ADCW_RATE_WRITE,rate_temp);	
	}
	if(ch & GR_SINE){
		CPLD_Contorl(CPLD_WRITE,CMD_GR_RATE_WRITE,rate_temp);	
	}
}




/*
*********************************************************************************************************
*	函 数 名: Read_Real_Current
*	功能说明: 从CPLD读取真实电流值
*	形    参: current  :  读取到的电流值
*	返 回 值: 真实电流值
*********************************************************************************************************
*/

#define RC_FLITER_POOL_SIZE     (4)          //真实电流滤波数据池大小 
#define RC_FLITER_FACTER        (4)          //真实电流滤波阶数

uint32_t Read_Real_Current(uint32_t current)
{
	static uint32_t V_count = 0;
	static uint32_t I_count = 0;
	uint32_t D_value;
	double angle ;
	
	static uint32_t V_count_pool[RC_FLITER_POOL_SIZE] = {0,};
	static uint32_t I_count_pool[RC_FLITER_POOL_SIZE] = {0,};
	static uint8_t  fliter_index = 0;
	uint32_t V_count_sum,I_count_sum;
	uint8_t i;
	
	V_count_sum = 0;
	I_count_sum = 0;
	V_count_pool[fliter_index] = ReadDataFromCPLD(0x02020000);
	I_count_pool[fliter_index] = ReadDataFromCPLD(0x02010000);
	if(++fliter_index >= RC_FLITER_POOL_SIZE)fliter_index = 0;
	for(i=0;i<RC_FLITER_POOL_SIZE;i++){
		V_count_sum += V_count_pool[i];
		I_count_sum += I_count_pool[i];
	}
	
	V_count = V_count_sum / RC_FLITER_POOL_SIZE / RC_FLITER_FACTER + V_count / RC_FLITER_FACTER * (RC_FLITER_FACTER - 1);
	I_count = I_count_sum / RC_FLITER_POOL_SIZE / RC_FLITER_FACTER + I_count / RC_FLITER_FACTER * (RC_FLITER_FACTER - 1);
	
// 	V_count = ReadDataFromCPLD(0x02020000);
// 	I_count = ReadDataFromCPLD(0x02010000);
	
	D_value = V_count > I_count ?  (V_count - I_count) : (I_count - V_count);
	if(D_value < 20)return 0;
	
	angle = (double)I_count / (double)V_count * 3.1415926 ;  
	
// 	return V_count;
	
// 	return (uint32_t)(current * cos(angle));
	
 	return cos(angle) >= 0? (int32_t)(current * cos(angle)) : -(int32_t)(current * cos(angle));
	//最后的括号内为系数修正
}


/*
 * 函数名：spi_cpld_init
 * 描述  ：初始化cpld
 * 输入  ：空
 * 输出  ：空
 */
void spi_cpld_init(void)
{
	Spi_gpio_init();
// 	CPLD_INT_NVIC_Configuration();
	CPLD_Write(0X0105AAAA);							//启动计数器进行计数
}

extern void Key_Stop_Irq(void);
// /*
// *********************************************************************************************************
// *	函 数 名: EXTI9_5_IRQHandler
// *	功能说明: 外部中断服务程序
// *	形    参：无
// *	返 回 值: 无
// *********************************************************************************************************
// */
// void EXTI9_5_IRQHandler(void)
// {
// 	if(EXTI_GetITStatus(EXTI_Line6) != RESET)
// 	{	
// 		EXTI->IMR &= ~(1<<6);	              /* 关闭中断       */
// 		EXTI_ClearITPendingBit(EXTI_Line6); /* 清除中断标志位 */
// 		
// //		Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)){EXTI->IMR |= (1<<6);return;}
// 		cpld_int4_irq();
// 		EXTI->IMR |= (1<<6);	
// 	}
// 	
// 	if(EXTI_GetITStatus(EXTI_Line9) != RESET)
// 	{	
// 		EXTI->IMR &= ~(1<<9);	              /* 关闭中断       */
// 		EXTI_ClearITPendingBit(EXTI_Line9); /* 清除中断标志位 */
// 			
// //		Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_9)){EXTI->IMR |= (1<<9);return;}
// 		cpld_int2_irq();
// 		EXTI->IMR |= (1<<9);
// 	}
// 	
// 	if(EXTI_GetITStatus(EXTI_Line8) != RESET)
// 	{	
// 		
// 		EXTI->IMR &= ~(1<<8);	              /* 关闭中断       */
// 		EXTI_ClearITPendingBit(EXTI_Line8); /* 清除中断标志位 */
// 		 
// 		Irq_Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_8)){EXTI->IMR |= (1<<8);return;}
// 		Key_Stop_Irq();
// 		EXTI->IMR |= (1<<8);	
// 	}
// 			   
// }

// /*
// *********************************************************************************************************
// *	函 数 名: EXTI15_10_IRQHandler
// *	功能说明: 外部中断服务程序
// *	形    参：无
// *	返 回 值: 无
// *********************************************************************************************************
// */
// void EXTI15_10_IRQHandler(void)
// {

// 	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
// 	{	
// 		EXTI->IMR &= ~(1<<10);	             /* 关闭中断       */
// 		EXTI_ClearITPendingBit(EXTI_Line10); /* 清除中断标志位 */
// 			
// //		Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_10)){EXTI->IMR |= (1<<10);return;}
// 		cpld_int1_irq();
// 		EXTI->IMR |= (1<<10);
// 	}
// 	
// 	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
// 	{	
// 		EXTI->IMR &= ~(1<<13);	             /* 关闭中断       */
// 		EXTI_ClearITPendingBit(EXTI_Line13); /* 清除中断标志位 */
// 		
// //		Delay_ms(1);
// 		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)){EXTI->IMR |= (1<<13);return;}
// 		cpld_int3_irq();
// 		EXTI->IMR |= (1<<13);	
// 	}
// 	

// 			   
// }
/********************************************************************************************/
