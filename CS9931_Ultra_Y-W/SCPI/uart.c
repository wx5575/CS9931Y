/****************************************Copyright (c)****************************************************
**                                 http://www.PowerAVR.com
**								   http://www.PowerMCU.com
**--------------File Info---------------------------------------------------------------------------------
** File name:           uart.c
** Last modified Date:  2013-09-01
** Last Version:        V1.00
** Descriptions:        
**
**--------------------------------------------------------------------------------------------------------
** Created by:          PowerAVR
** Created date:        2013-09-01
** Version:             V1.00
** Descriptions:        编写示例代码
**
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/
#include "lpc17xx.h"                              /* LPC17xx definitions    */
#include "uart.h"
#include "typedef.h"
#include "param.h"
#include "macro.h"
#include "scpi_Parser.h"



#define FOSC                        11059200                            /*  振荡器频率                  */

#define FCCLK                      (FOSC  * 8)                          /*  主时钟频率<=100Mhz          */
                                                                        /*  FOSC的整数倍                */
#define FCCO                       (FCCLK * 3)                          /*  PLL频率(275Mhz~550Mhz)      */
                                                                        /*  与FCCLK相同，或是其的偶数倍 */
#define FPCLK                      (FCCLK/4)                          /*  外设时钟频率,FCCLK的1/2、1/4*/
                                                                        /*  或与FCCLK相同               */

#define UART0_BPS     9600                                             /* 串口0通信波特率             */
#define UART2_BPS     9600                                            /* 串口2通信波特率             */

/*********************************************************************************************************
** Function name:       UART0_Init
** Descriptions:        按默认值初始化串口0的引脚和通讯参数。设置为8位数据位，1位停止位，无奇偶校验
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/

volatile uint16_t Uart2RecvBuf;

volatile uint32_t UART0Status, UART1Status, UART2Status;
volatile uint8_t UART0TxEmpty = 1, UART1TxEmpty = 1, UART2TxEmpty = 1;
volatile uint8_t UART0Buffer[BUFSIZE], UART1Buffer[BUFSIZE], UART2Buffer[BUFSIZE];
volatile uint8_t UART2Send_Buffer[BUFSIZE];
volatile uint32_t UART0Count = 0, UART1Count = 0, UART2Count = 0;
volatile uint32_t	UART2Rec_Count = 0;
void UART0_IRQHandler(void)   
{  
    unsigned int iir;  
    /*IIR中断标志寄存器，读取中断标志寄存器，  */  
  
    iir = LPC_UART0->IIR;  
  
    iir >>= 1;       /* 跳过中断状态*/  
  
    iir &= 0x07;    /* check bit 1~3, interrupt identification */    
    if (iir == 0x02 )     /* 0x02=0000 0010，中断标志：010：接收数据可用（RDA）， */  
    {  
       UART0_SendByte(LPC_UART0->RBR);  
    }   
} 

// int Send_check(void);
// void UART2_IRQHandler (void) 
// {
//   uint8_t IIRValue, LSRValue;
//   uint8_t Dummy = Dummy;
// 	
//   IIRValue = LPC_UART2->IIR;
//     
//   IIRValue >>= 1;			/* skip pending bit in IIR */
//   IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
//   if ( IIRValue == IIR_RLS )		/* Receive Line Status */
//   {
// 	LSRValue = LPC_UART2->LSR;
// 	/* Receive Line Status */
// 	if ( LSRValue & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
// 	{
// 	  /* There are errors or break interrupt */
// 	  /* Read LSR will clear the interrupt */
// 	  UART2Status = LSRValue;
// 	  Dummy = LPC_UART2->RBR;		/* Dummy read on RX to clear 
// 								interrupt, then bail out */
// 	  return;
// 	}
// 	if ( LSRValue & LSR_RDR )	/* Receive Data Ready */			
// 	{
// 	  /* If no error on RLS, normal ready, save into the data buffer. */
// 	  /* Note: read RBR will clear the interrupt */
// 	  UART2Buffer[UART2Count] = LPC_UART2->RBR;
// 	  UART2Count++;
// 	  if ( UART2Count == BUFSIZE )
// 	  {
// 		UART2Count = 0;		/* buffer overflow */
// 	  }	
// 	}
//   }
//   else if ( IIRValue == IIR_RDA )	/* Receive Data Available */
//   {
// 	/* Receive Data Available */
// 	UART2Buffer[UART2Count] = LPC_UART2->RBR;
// 	UART2Count++;
// 		if(UART2Count>=2)
// 		{
// 			if ( UART2Count == UART2Buffer[2] )
// 			{
// 				UART2Count = 0;		/* buffer overflow */
// 				UART2_Rec_flag=1;
// 				Check_result=Send_check();
// 			}
// 		}
//   }
//   else if ( IIRValue == IIR_CTI )	/* Character timeout indicator */
//   {
// 	/* Character Time-out indicator */
// 	UART2Status |= 0x100;		/* Bit 9 as the CTI error */
//   }
//   else if ( IIRValue == IIR_THRE )	/* THRE, transmit holding register empty */
//   {
// 	/* THRE interrupt */
// 	LSRValue = LPC_UART2->LSR;		/* Check status in the LSR to see if
// 								valid data in U0THR or not */
// 	if ( LSRValue & LSR_THRE )
// 	{
// 	  UART2TxEmpty = 1;
// 	}
// 	else
// 	{
// 	  UART2TxEmpty = 0;
// 	}
//   }

// }
void UART2_IRQHandler(void)
{
	uint32_t iir,i;

	iir = LPC_UART2->IIR;

	iir >>= 1;			    /* skip pending bit in IIR */
	iir &= 0x07;			/* check bit 1~3, interrupt identification */

	//level = rt_hw_interrupt_disable();
    // 接收数据达到触发深度产生中断--即接收数据有效
    if (IIR_RDA == iir)	    /* Receive Data Available */
	{
		for(i = 0; i < 8; i++)
        {
            int_API_PEMRead(LPC_UART2->RBR);
        }
			}
			
				else if (IIR_CTI == iir)
    {
        for(i = 0; i < 8; i++)
        {
            // 判断接收缓冲寄存器中是否有数据
            if (LPC_UART2->LSR & 0x01)
            {
                int_API_PEMRead(LPC_UART2->RBR);
            }
            else
            {
                break;
            }
        }
    }
    else if (IIR_RLS == iir)
    {
        iir                     = LPC_UART2->LSR;
        __NOP();
    }
}
void UART0_Init (void)
{
	uint16_t usFdiv;
    /* UART0 */
    LPC_PINCON->PINSEL0 |= (1 << 4);            
    LPC_PINCON->PINSEL0 |= (1 << 6);            
  
  	LPC_UART0->LCR  = 0x83;                      /* 允许设置波特率               */
    usFdiv = (FPCLK / 16) / UART0_BPS;           /* 设置波特率                   */
    LPC_UART0->DLM  = usFdiv / 256;
    LPC_UART0->DLL  = usFdiv % 256; 
    LPC_UART0->LCR  = 0x03;                      /* 锁定波特率                   */
    LPC_UART0->FCR  = 0x07; 				  
		NVIC_EnableIRQ(UART0_IRQn); //enable irq in nvic
		LPC_UART0->IER = IER_RBR | IER_THRE | IER_RLS; //enable receive interrupt
}

/*********************************************************************************************************
** Function name:       UART0_SendByte
** Descriptions:        从串口0发送数据
** input parameters:    data: 发送的数据
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
int UART0_SendByte (int ucData)
{
	while (!(LPC_UART0->LSR & 0x20));
    return (LPC_UART0->THR = ucData);
}

/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int UART0_GetChar (void) 
{
  	while (!(LPC_UART0->LSR & 0x01));
  	return (LPC_UART0->RBR);
}

/*********************************************************************************************************
Write character to Serial Port
** Function name:       UART0_SendString
** Descriptions:	    向串口发送字符串
** input parameters:    s:   要发送的字符串指针
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void UART0_SendString (unsigned char *s) 
{
  	while (*s != 0) 
	{
   		UART0_SendByte(*s++);
	}
}

/*********************************************************************************************************
** Function name:       UART2_Init
** Descriptions:        按默认值初始化串口2的引脚和通讯参数。设置为8位数据位，1位停止位，无奇偶校验
** input parameters:    无
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
// void LIB_SoftDelayMs(uint16_t delayMs)
// {
// 	uint8_t delayi;
// 	while(--delayMs)
// 	{
// 		for(delayi=0;delayi<124;delayi++);
// 	}
// }


uint32_t rt_uart_write(const void* buffer, uint32_t size)
{
	char *ptr;
	
	ptr = (char*)buffer;

	LPC_GPIO0 -> FIODIR |= 1<<16;
	LPC_GPIO0 -> FIOSET |= 1<<16;
	LIB_SoftDelayMs(100);
	
	/* stream mode */
	while (size)
	{
		if (*ptr == '\n')
		{
			/* THRE status, contain valid data */
			while ( !(LPC_UART2->LSR & LSR_THRE) );
			/* write data */
			//UART->THR = '\r';
		}

		/* THRE status, contain valid data */
		while ( !(LPC_UART2->LSR & LSR_THRE) );	
		/* write data */
		LPC_UART2->THR = *ptr;
		
		ptr ++;
		size --;
	}
	
	LIB_SoftDelayMs(100);
	LIB_SoftDelayMs(100);
 	LIB_SoftDelayMs(100);
	LIB_SoftDelayMs(100);
	LPC_GPIO0 -> FIODIR |= 1<<16;
	LPC_GPIO0 -> FIOCLR |= (1<<16);
	return (uint32_t) ptr - (uint32_t) buffer;
}

void UART2_Init (void)
{
	uint16_t usFdiv;
	uint32_t pclkdiv;
	uint32_t pclk;
	ParserContainer    *ptrootParserContainer;
  uint8_t               prootParserContainerCapacity;
    /* UART2 */
		LPC_SC->PCONP = LPC_SC->PCONP|(1<<24);	      /*打开UART2电源控制位	           */
		LPC_PINCON->PINSEL4 |= (1<<17);  
		LPC_PINCON->PINSEL4 |= (1<<19); 
		LPC_PINCON->PINSEL1 &= ~((1<<0) |(1<<1));
		LPC_GPIO0 -> FIODIR |= 1<<16;
		LPC_GPIO0 -> FIOCLR |= 1<<16;
		pclkdiv = (LPC_SC->PCLKSEL1 >> 16) & 0x03;
				switch ( pclkdiv )
		{
		  case 0x00:
		  default:
			pclk = SystemCoreClock/4;
			break;
		  case 0x01:
			pclk = SystemCoreClock;
			break; 
		  case 0x02:
			pclk = SystemCoreClock/2;
			break; 
		  case 0x03:
			pclk = SystemCoreClock/8;
			break;
		}
		
		 LPC_UART2->LCR  = 0x83;                       /* 允许设置波特率                */
    usFdiv = (pclk / 16) / UART2_BPS;            /* 设置波特率                    */
    LPC_UART2->DLM  = usFdiv / 256;
    LPC_UART2->DLL  = usFdiv % 256; 
    LPC_UART2->LCR  = 0x03;                       /* 锁定波特率                    */
    LPC_UART2->FCR  = 0x87;
			while (( LPC_UART2->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
	while ( LPC_UART2->LSR & LSR_RDR )
	{
		usFdiv = LPC_UART2->RBR;	/* Dump data from RX FIFO */
	}
		LPC_UART2->IER = IER_RBR | IER_THRE | IER_RLS; //enable receive interrupt
		NVIC_EnableIRQ(UART2_IRQn); //enable irq in nvic
		
		    //获取根解析结构体信息
    APP_RootParserContainerInfoGet((ParserContainer **)&ptrootParserContainer, &prootParserContainerCapacity);
    //解析器初始化
    API_PEMInit(ptrootParserContainer, prootParserContainerCapacity, 
                2, '1', rt_uart_write);
    APP_CommProtocolLogicInit(1);
}

/*********************************************************************************************************
** Function name:       UART2_SendByte
** Descriptions:        从串口2发送数据
** input parameters:    data: 发送的数据
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
int UART2_SendByte (int ucData)
{
	while (!(LPC_UART2->LSR & 0x20));
    return (LPC_UART2->THR = ucData);
}

/*----------------------------------------------------------------------------
  Read character from Serial Port   (blocking read)
 *----------------------------------------------------------------------------*/
int UART2_GetChar (void) 
{
  	while (!(LPC_UART2->LSR & 0x01));
  	return (LPC_UART2->RBR);
}

/*********************************************************************************************************
** Write character to Serial Port
** Function name:       UART2_SendString
** Descriptions:	    向串口发送字符串
** input parameters:    s:   要发送的字符串指针
** output parameters:   无
** Returned value:      无
*********************************************************************************************************/
void UART2_SendString (unsigned char *s) 
{
  	while (*s != 0) 
	{
   		UART2_SendByte(*s++);
	}
}


void UART0_SendChar(uint16_t disp)
{
	uint16_t dispbuf[4];
	uint8_t i;

	dispbuf[3] = disp%10 + '0';
	dispbuf[2] = disp/10%10 + '0';
	dispbuf[1] = disp/10/10%10 + '0';
	dispbuf[0] = disp/10/10/10%10 + '0';
	for(i=0;i<4;i++)
		UART0_SendByte(dispbuf[i]);	
}

void UART2_SendChar(uint16_t disp)
{
	uint16_t dispbuf[4];
	uint8_t i;

	dispbuf[3] = disp%10 + '0';
	dispbuf[2] = disp/10%10 + '0';
	dispbuf[1] = disp/10/10%10 + '0';
	dispbuf[0] = disp/10/10/10%10 + '0';
	for(i=0;i<4;i++)
		UART2_SendByte(dispbuf[i]);	

}
// int Send_check(void)
// {
// 	uint8_t	frame_header_one;
// 	uint8_t	frame_header_two;
// 	uint8_t	frame_length;
// 	uint8_t	function_code;
// 	uint8_t	model_H;
// 	uint8_t	model_L;
// 	uint8_t	check_code=0;
// 	uint8_t volatile i;
// 	uint8_t volatile Sum=0;

// 	frame_header_one =UART2Buffer[0];
// 	frame_header_two =UART2Buffer[1];
// // 	FRAME_LENGTH     =UART2Buffer[2];
// 	function_code    =UART2Buffer[3];
// 	switch(function_code)
// 	{
// 		case	0x01://联机指令
//  							frame_length=	0x07;
// 							model_H 		=UART2Buffer[4];
// 							model_L			=UART2Buffer[5];
// 							break;
// 		case	0x02://断开指令
//  							frame_length=	0x07;
// 							model_H 		=UART2Buffer[4];
// 							model_L			=UART2Buffer[5];
// 							break;
// 		case	0x03://读取参数指令
//  							frame_length=	0x05;
// 							break;
// 		case	0x04://s参数指令
//  							frame_length=	0x05;
// 							break;
// 		default :
// 							break;
// 							
// 	
// 	}
// 	for( i=0;i<=frame_length-2;i++)
// 	{
// 		check_code+=UART2Buffer[i];
// 	}
//   switch(function_code)
// 	{
// 		case	0x01://已联机指令
// 								UART2Send_Buffer[0]=frame_header_one;
// 								UART2Send_Buffer[1]=frame_header_two;
// 								UART2Send_Buffer[2]=0x07;
// 								UART2Send_Buffer[3]=0x11;
// 								UART2Send_Buffer[4]=model_H;
// 								UART2Send_Buffer[5]=model_L;
// 								break;
// 		case	0x02://已断开指令
// 								UART2Send_Buffer[0]=frame_header_one;
// 								UART2Send_Buffer[1]=frame_header_two;
// 								UART2Send_Buffer[2]=0x07;
// 								UART2Send_Buffer[3]=0x12;
// 								UART2Send_Buffer[4]=model_H;
// 								UART2Send_Buffer[5]=model_L;
// 								break;
// 		case	0x03://读取参数
// 								UART2Send_Buffer[0]=frame_header_one;
// 								UART2Send_Buffer[1]=frame_header_two;
// 								UART2Send_Buffer[2]=0x0B;
// 								UART2Send_Buffer[3]=0x13;
// 								UART2Send_Buffer[4]=0;//电阻高位
// 								UART2Send_Buffer[5]=0;//电阻低位
// 								UART2Send_Buffer[6]=0;//时间高位
//  								UART2Send_Buffer[7]=0;//时间低位
// 								UART2Send_Buffer[8]=0;
// 								UART2Send_Buffer[9]=0;
// 								break;
// 		case	0x04://
// 								UART2Send_Buffer[0]=frame_header_one;
// 								UART2Send_Buffer[1]=frame_header_two;
// 								UART2Send_Buffer[2]=0x0B;
// 								UART2Send_Buffer[3]=0x13;
// 								UART2Send_Buffer[4]=0;//电阻高位
// 								UART2Send_Buffer[5]=0;//电阻低位
// 								UART2Send_Buffer[6]=0;//时间高位
//  								UART2Send_Buffer[7]=0;//时间低位
// 								UART2Send_Buffer[8]=0;
// 								UART2Send_Buffer[9]=0;
// 								break;
// 		default	:
// 								break;
// 	}
// 	for( i=0;i<=UART2Send_Buffer[2]-2;i++)
// 	{
// 		Sum+=UART2Send_Buffer[i];
// 	}
// 	UART2Send_Buffer[UART2Send_Buffer[2]-1]=Sum;
// 	if(UART2Buffer[2]!=frame_length)
// 		return ERROR;
// 	if(UART2Buffer[6]!=check_code)
// 		return ERROR;
// 	return	CORRECT;

// 	
// 	
// }


