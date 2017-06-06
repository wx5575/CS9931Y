/* CH376芯片 软件模拟SPI串行连接的硬件抽象层 V1.0 */
/* 提供I/O接口子程序 */

#include "stm32f4xx.h"
#include	"HAL.H"

/* 安富莱开发板硬件连接方式如下(实际应用电路可以参照修改下述定义及子程序) 
	PF10             CH376的SCS引脚
	PA5/SPI1_SCK     CH376的SCK引脚
	PA6/SPI1_MISO    CH376的SDO引脚
	PA7/SPI1_MOSI	   CH376的SDI引脚
	PG6              CH376输出的中断信号
*/
#define CH376_SCK_0()	GPIO_ResetBits(GPIOB,  GPIO_Pin_13)
#define CH376_SCK_1()	GPIO_SetBits(GPIOB,  GPIO_Pin_13)

#define CH376_SDI_0()	GPIO_ResetBits(GPIOB,  GPIO_Pin_15)
#define CH376_SDI_1()	GPIO_SetBits(GPIOB,  GPIO_Pin_15)

#define CH376_SCS_0()	GPIO_ResetBits(GPIOB,  GPIO_Pin_12)
#define CH376_SCS_1()	GPIO_SetBits(GPIOB,  GPIO_Pin_12)

#define CH376_SDO_HIGH()	(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) != Bit_RESET)
#define CH376_INT_HIGH()	(GPIO_ReadInputDataBit(GPIOI, GPIO_Pin_1) != Bit_RESET)


static uint8_t USB_Device_Choosed = USB_1;

static void USB1_Enable(void)
{
		CH376_SCS_0();
}

static void USB2_Enable(void)
{
		GPIO_ResetBits(GPIOH,  GPIO_Pin_12);
}

static void USB1_Disable(void)
{
		CH376_SCS_1();
}

static void USB2_Disable(void)
{
		GPIO_SetBits(GPIOH,  GPIO_Pin_12);
}

static UINT8 GET_USB1_INTERRUPT(void)
{
	return( GPIO_ReadInputDataBit(GPIOI, GPIO_Pin_0) != Bit_RESET ? FALSE : TRUE );
}

static UINT8 GET_USB2_INTERRUPT(void)
{
	return( GPIO_ReadInputDataBit(GPIOI, GPIO_Pin_1) != Bit_RESET ? FALSE : TRUE );
}


static void(*USB_DEVICE_ENABLE[2])(void)  = {USB1_Enable,USB2_Enable}; 
static void(*USB_DEVICE_DISABLE[2])(void)  = {USB1_Disable,USB2_Disable}; 
static UINT8(*GET_USB_INTERRUPT[2])(void)  = {GET_USB1_INTERRUPT,GET_USB2_INTERRUPT}; 

static void bsp_CfgSPIForCH376(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* 打开SPI时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* 配置SPI硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* STM32的SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 数据位长度 ： 8位 */
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_Cmd(SPI2, DISABLE);			/* 先禁止SPI  */

	SPI_Cmd(SPI2, ENABLE);				/* 使能SPI  */
}

/* 使用软件模拟SPI读写时序,进行初始化 */
void CH376_PORT_INIT( void )  
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 设置SPI_SCS,SPI_SCK,SPI_SDI为输出方向,SPI_SDO为输入方向 */
	
	/* 打开CPU的GPIO时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH, ENABLE);

	/* 配置 SPI1_SCK,SPI1_MOSI为推挽输出 */
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;     
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;      //USB2
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;     
	GPIO_Init(GPIOI, &GPIO_InitStructure);
	
		
	GPIO_SetBits(GPIOH,  GPIO_Pin_12);
	CH376_SCS_1();  /* 禁止SPI片选 */
//	CH376_SCK_1();  /* 默认为高电平,SPI模式3,也可以用SPI模式0,但模拟程序可能需稍做修改 */	
	{
		GPIO_InitTypeDef GPIO_InitStructure;

		/* 配置 SCK, MISO 、 MOSI 为复用功能 */
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

	}	

}

void mDelay0_5uS( void )  /* 至少延时0.5uS,根据单片机主频调整 */
{
	UINT16 i;

	for (i = 0; i < 10; i++);
}

/* SPI输出8个位数据 */
// void Spi376OutByte(UINT8 d)  
// {  
// 	/* 如果是硬件SPI接口,应该是先将数据写入SPI数据寄存器,然后查询SPI状态寄存器以等待SPI字节传输完成 */
// 	UINT8	i;
// 	for ( i = 0; i < 8; i ++ ) 
// 	{
// 		CH376_SCK_0();
// 		mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
// 		if ( d & 0x80 )
// 		{
// 			CH376_SDI_1();
// 		}
// 		else
// 		{
// 			CH376_SDI_0();
// 		}
// 		mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
// 		d <<= 1;  /* 数据位是高位在前 */
// 		CH376_SCK_1();  /* CH376在时钟上升沿采样输入 */
// 		mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
// 	}
// }

// /* SPI输入8个位数据 */
// UINT8 Spi376InByte( void )
// {  /* 如果是硬件SPI接口,应该是先查询SPI状态寄存器以等待SPI字节传输完成,然后从SPI数据寄存器读出数据 */
// 	UINT8	i, d;
// 	d = 0;
// 	for ( i = 0; i < 8; i ++ ) 
// 	{
// 		CH376_SCK_0();  /* CH376在时钟下降沿输出 */
// 		mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
// 		d <<= 1;  /* 数据位是高位在前 */
// 		if ( CH376_SDO_HIGH() ) d ++;
// 		CH376_SCK_1();
// 		mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
// 	}
// 	return( d );
// }

void xEndCH376Cmd( )
{
//	 CH376_SCS_1();
//	GPIO_SetBits(GPIOH,  GPIO_Pin_12);
	USB_DEVICE_DISABLE[USB_Device_Choosed-1]();
}

void	xWriteCH376Cmd( UINT8 mCmd )  /* 向CH376写命令 */
{
//	CH376_SCS_1();  /* 防止之前未通过xEndCH376Cmd禁止SPI片选 */
//	GPIO_SetBits(GPIOH,  GPIO_Pin_12);
	USB_DEVICE_DISABLE[USB_Device_Choosed-1]();
	mDelay0_5uS( );
	
//	CH376_SCS_0();  /* SPI片选有效 */
//	GPIO_ResetBits(GPIOH,  GPIO_Pin_12);
	USB_DEVICE_ENABLE[USB_Device_Choosed-1]();
	xWriteCH376Data( mCmd );  /* 发出命令码 */
	
	/* 延时1.5uS确保读写周期大于1.5uS,或者用上面一行的状态查询代替 */
	mDelay0_5uS( ); 
	mDelay0_5uS( ); 
	mDelay0_5uS( );  
}

void xWriteCH376Data( UINT8 mData )  /* 向CH376写数据 */
{
//	Spi376OutByte( mData );
//	mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI2, mData);

	/* 等待接收一个字节任务完成 */
 	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回从SPI总线读到的数据 */
  SPI_I2S_ReceiveData(SPI2);
}

UINT8 xReadCH376Data( void )  /* 从CH376读数据 */
{
//	mDelay0_5uS( );  /* 确保读写周期大于0.6uS */
//	return( Spi376InByte( ) );
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI2, 0xFF);

	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI2);
}

/* 查询CH376中断(INT#低电平) */
UINT8	Query376Interrupt( void )
{
//	return( CH376_INT_HIGH() ? FALSE : TRUE );
	return GET_USB_INTERRUPT[USB_Device_Choosed-1]();
}

UINT8 mInitCH376Host( UINT8 index )  /* 初始化CH376 */
{
	UINT8	res;
	
	CH376_PORT_INIT( );  /* 接口硬件初始化 */
	
	bsp_CfgSPIForCH376();

	//mDelaymS(200);
	
	xWriteCH376Cmd( CMD11_CHECK_EXIST );  /* 测试单片机与CH376之间的通讯接口 */
	xWriteCH376Data( 0x65 );
	res = xReadCH376Data( );
	xEndCH376Cmd( );
	if ( res != 0x9A ) return( ERR_USB_UNKNOWN );  /* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */
	xWriteCH376Cmd( CMD11_SET_USB_MODE );  /* 设备USB工作模式 */
	xWriteCH376Data( 0x06 );
	mDelayuS(50 );
	res = xReadCH376Data( );
	xEndCH376Cmd( );

	if ( res == CMD_RET_SUCCESS ) return( USB_INT_SUCCESS );
	else return( ERR_USB_UNKNOWN );  /* 设置模式错误 */
}


void USB_Device_Chg(UINT8 USB1_or_USB2)
{
	if((USB1_or_USB2 != USB_1) && (USB1_or_USB2 != USB_2))return;
	if(USB_Device_Choosed != USB1_or_USB2){
		USB_Device_Choosed = USB1_or_USB2;
		switch(USB1_or_USB2){
			case USB_1:
				GPIO_SetBits(GPIOH,  GPIO_Pin_12);
				CH376_SCS_0();  
			break;
				
			case USB_2:
				GPIO_ResetBits(GPIOH,  GPIO_Pin_12);
				CH376_SCS_1();  
			break;
			
			default:
				
			break;

		}
		mDelayuS(250);
	}

}
