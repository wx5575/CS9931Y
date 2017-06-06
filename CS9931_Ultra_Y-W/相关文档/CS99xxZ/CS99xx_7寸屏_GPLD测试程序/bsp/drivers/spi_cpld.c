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


#define			SPI_CS_EN()				GPIO_ResetBits(GPIOF,GPIO_Pin_9)
#define			SPI_CS_DIS()			GPIO_SetBits(GPIOF,GPIO_Pin_9)
#define			SPI_MOSI_HIGH()		GPIO_SetBits(GPIOF,GPIO_Pin_8)
#define			SPI_MOSI_LOW()		GPIO_ResetBits(GPIOF,GPIO_Pin_8)
#define			SPI_CLK_HIGH()		GPIO_SetBits(GPIOF,GPIO_Pin_7)
#define			SPI_CLK_LOW()			GPIO_ResetBits(GPIOF,GPIO_Pin_7)
#define			SPI_MISO_DI()			GPIO_ReadInputDataBit(GPIOI,GPIO_Pin_11)


/*
 * 函数名：spi_gpio_init
 * 描述  ：初始化管脚
 * 输入  ：空
 * 输出  ：空
 */
static void spi_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOE| RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */

	
	/* CPLD_CLK,CPLD_DI(实际为CPLD的数据输入),CPLD_CS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	
	
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		/* 上拉电使能 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOI, &GPIO_InitStructure);

//PG口
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 上下拉电阻不使能 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);




	/* CPLD_INT1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOI, EXTI_PinSource10);
	{
		EXTI_InitTypeDef EXTI_InitStructure;

    /* Configure  EXTI  */
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//Falling下降沿 Rising上升
    /* enable */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    EXTI_ClearITPendingBit(EXTI_Line10);
	}
}

/*
 * 函数名：NVIC_Configuration
 * 描述  ：初始化中断向量表
 * 输入  ：空
 * 输出  ：空
 */
static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    /* Enable the EXTI0 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*
 * 函数名：spi_cpld_init
 * 描述  ：初始化cpld
 * 输入  ：空
 * 输出  ：空
 */
void spi_cpld_init(void)
{
	spi_gpio_init();
	NVIC_Configuration();
	
}

/*
 * 函数名：cpld_write
 * 描述  ：往CPLD写命令，并返回一字节
 * 输入  ：命令
 * 输出  ：返回值
 */
void cpld_write(u32 SendData)
{
	u8  ShiftCounter;
	u32 DataBuffer;
	
	SPI_CS_EN();																				//CS=0;片选CPLD

	for(ShiftCounter=0;ShiftCounter<26;ShiftCounter++)
	{
		DataBuffer 		 = SendData;
		DataBuffer     = DataBuffer & 0x00000001;				//取数据最低位
	  SendData       = SendData >>1;									//数据右移一位 
	  SPI_CLK_HIGH(); 																//时钟置高

	  if(DataBuffer == 0x00000000)
	  {
		  SPI_MOSI_LOW();
	  }
	  else
	  {
		  SPI_MOSI_HIGH();
	  }

	  SPI_CLK_LOW();																	//数据写入CPLD
		
   }
	 SPI_CS_DIS();																		//CS=1;取消片选

}

//从CPLD中读数据
u16 ReadDataFromCPLD(u32 SendData)
{
	u8  ShiftCounter;
	u16 DataBuffer=0;
	u32 SendDataBuffer;
	
	SendDataBuffer   = SendData;	
	cpld_write(SendDataBuffer);																//给CPLD发送读命令
	
	SPI_CS_EN();																							//片选
	
	
	for(ShiftCounter=0;ShiftCounter<16;ShiftCounter++)
	{

		DataBuffer = DataBuffer << 1;													//左移一位

    SPI_CLK_HIGH();																				//时钟置高
	 
	  SPI_CLK_LOW();
    
		
	  if(SPI_MISO_DI() == 1)
  	{
	    DataBuffer = DataBuffer | 0x0001;  									 	//低位置1
 	  }
    else
    {
	    DataBuffer = DataBuffer & 0xfffe; 										//低位置0
	  }		
 
  }	
	SPI_CS_DIS();
	
	return(DataBuffer);
}






/*
 * 函数名：cpld_int1_irq
 * 描述  ：cpld中断处理函数
 * 输入  ：空
 * 输出  ：空
 */
void cpld_int1_irq(void)
{
	
}
/********************************************************************************************/
