
#include "Key_LED.h"


#define   NUM_OF_4094            (2)                 //4094级联的数量
#define		C4094_DLY				       (10)

#define  GPIO_PORT_IN1           GPIOA
#define  GPIO_PIN_IN1   	 	     GPIO_Pin_12

#define  GPIO_PORT_IN2    	     GPIOH
#define  GPIO_PIN_IN2   	 	     GPIO_Pin_13

#define  GPIO_PORT_IN3    	     GPIOH
#define  GPIO_PIN_IN3   	 	     GPIO_Pin_14

#define  GPIO_PORT_IN4    	     GPIOH
#define  GPIO_PIN_IN4   	       GPIO_Pin_15

#define  GPIO_PORT_START    	   GPIOG
#define  GPIO_PIN_START   	     GPIO_Pin_7

#define  GPIO_PORT_STOP     	   GPIOG
#define  GPIO_PIN_STOP    	     GPIO_Pin_8

#define  C4094_ST_PORT           GPIOA
#define  C4094_ST_PIN            GPIO_Pin_9
#define  C4094_ST_SET()          ((GPIO_TypeDef *)C4094_ST_PORT)->BSRRL = C4094_ST_PIN
#define  C4094_ST_CLR()          ((GPIO_TypeDef *)C4094_ST_PORT)->BSRRH = C4094_ST_PIN

#define  C4094_CLK_PORT          GPIOA
#define  C4094_CLK_PIN           GPIO_Pin_8
#define  C4094_CLK_SET()         ((GPIO_TypeDef *)C4094_CLK_PORT)->BSRRL = C4094_CLK_PIN
#define  C4094_CLK_CLR()         ((GPIO_TypeDef *)C4094_CLK_PORT)->BSRRH = C4094_CLK_PIN

#define  C4094_DATA_PORT         GPIOA 
#define  C4094_DATA_PIN          GPIO_Pin_10
#define  C4094_DATA_SET()        ((GPIO_TypeDef *)C4094_DATA_PORT)->BSRRL = C4094_DATA_PIN  
#define  C4094_DATA_CLR()        ((GPIO_TypeDef *)C4094_DATA_PORT)->BSRRH = C4094_DATA_PIN  

#define  PASSLED_POS             (1<<0)   
#define  FAILLED_POS             (1<<1)   
#define  FMQ_POS                 (1<<2) 
 
#define  TESTLED_PORT            GPIOI
#define  TESTLED_PIN             GPIO_Pin_2


static void C4094_Delay(uint32_t t)
{
	while(t--);
}



static  uint8_t   C4094_data_pool[NUM_OF_4094];   //串行输出缓冲池
static  void  C4094_Output(void)
{
	uint8_t i;
	C4094_ST_CLR();
	for(i=0;i<(NUM_OF_4094*8);i++)
	{	
		C4094_CLK_CLR();
		if(C4094_data_pool[NUM_OF_4094-1-(i/8)] & (0x80>>(i%8)))
		{
			C4094_DATA_SET();
		}
		else
		{
			C4094_DATA_CLR();
		}
		C4094_Delay(C4094_DLY);
		C4094_CLK_SET();	
		C4094_Delay(C4094_DLY);
	}
	C4094_ST_SET();
}

//C4094输出函数
/*******************************
函数名：  C4094_Write
参  数：  Index：芯片索引号
          data： 要输出的数据

返回值：  无
********************************/
static void C4094_Write(uint8_t Index,uint8_t data)
{
	if(Index >= NUM_OF_4094) return;
	C4094_data_pool[Index] = data;
	C4094_Output();
}



//PASS灯控制函数
/*******************************
函数名：  Set_LED_PASSLED
参  数：  state：目标状态  ON or OFF
返回值：  无
********************************/
void Set_LED_PASSLED(uint8_t state)
{
	if(state){
		C4094_data_pool[1] |= PASSLED_POS;
	}else{
		C4094_data_pool[1] &= ~PASSLED_POS;
	}
	C4094_Output();
}

//FAIL灯控制函数
/*******************************
函数名：  Set_LED_FAILLED
参  数：  state：目标状态  ON or OFF
返回值：  无
********************************/
void Set_LED_FAILLED(uint8_t state)
{
	if(state){
		C4094_data_pool[1] |= FAILLED_POS;
	}else{
		C4094_data_pool[1] &= ~FAILLED_POS;
	}
	C4094_Output();
}

//TEST灯控制函数
/*******************************
函数名：  Set_LED_TESTLED
参  数：  state：目标状态  ON or OFF
返回值：  无
********************************/
void Set_LED_TESTLED(uint8_t state)
{
	if(state){
		((GPIO_TypeDef *)TESTLED_PORT)->BSRRL = TESTLED_PIN ;
	}else{
		((GPIO_TypeDef *)TESTLED_PORT)->BSRRH = TESTLED_PIN ;
	}
}

//蜂鸣器控制函数
/*******************************
函数名：  Set_BUFFER
参  数：  state：目标状态  ON or OFF
返回值：  无
********************************/
void Set_BUFFER(uint8_t state)
{
	if(state){
		C4094_data_pool[1] |= FMQ_POS;
	}else{
		C4094_data_pool[1] &= ~FMQ_POS;
	}
	C4094_Output();
}

//键值读取函数
/*******************************
函数名：  KeyValue_Read
参  数：  无
返回值：  32位的键值
********************************/
uint32_t KeyValue_Read(void)
{
	uint8_t readtemp,i;
	uint32_t KeyValue = 0;
	for(i=0;i<8;i++){
		C4094_Write(0,~(1<<i));
//		C4094_Delay(10);
		readtemp = 0;
		readtemp|=(GPIO_ReadInputDataBit(GPIO_PORT_IN1,GPIO_PIN_IN1)?0:0x01);
		readtemp|=(GPIO_ReadInputDataBit(GPIO_PORT_IN2,GPIO_PIN_IN2)?0:0x02);
		readtemp|=(GPIO_ReadInputDataBit(GPIO_PORT_IN3,GPIO_PIN_IN3)?0:0x04);
		readtemp|=(GPIO_ReadInputDataBit(GPIO_PORT_IN4,GPIO_PIN_IN4)?0:0x08);	
		KeyValue <<= 4;
		KeyValue |= readtemp;
	}
	KeyValue |= (GPIO_ReadInputDataBit(GPIO_PORT_START,GPIO_PIN_START)?0:0x10000000);
// 	KeyValue |= (GPIO_ReadInputDataBit(GPIO_PORT_STOP,GPIO_PIN_STOP)  ?0:0x00000001);
	return KeyValue;
}


/*
*********************************************************************************************************
*	函 数 名: C4094_CTRLine_Init
*	功能说明: C4094初始化硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void C4094_CTRLine_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOG, ENABLE);
	/* 第2步：配置所有的按键GPIO为浮动输入模式(实际上CPU复位后就是输入状态) */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */

	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN1;
	GPIO_Init(GPIO_PORT_IN1, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN2;
	GPIO_Init(GPIO_PORT_IN2, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN3;
	GPIO_Init(GPIO_PORT_IN3, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_IN4;
	GPIO_Init(GPIO_PORT_IN4, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_START;
	GPIO_Init(GPIO_PORT_START, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_STOP;
	GPIO_Init(GPIO_PORT_STOP, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Pin = C4094_ST_PIN;
	GPIO_Init(C4094_ST_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = C4094_CLK_PIN;
	GPIO_Init(C4094_CLK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = C4094_DATA_PIN;
	GPIO_Init(C4094_DATA_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = TESTLED_PIN;
	GPIO_Init(TESTLED_PORT, &GPIO_InitStructure);
	
	/* 使能SYSCFG时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	/* 连接 EXTI Line8 到 PG8 引脚 */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource8);
	
	/* 配置 EXTI LineXXX */
	EXTI_InitStructure.EXTI_Line    = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

}


void Key_LED_Control_Init(void)
{
	C4094_CTRLine_Init();
	C4094_data_pool[0] = 0;
	C4094_data_pool[1] = 0;
	C4094_Output();
}

extern void Key_Stop_Dispose(void);
void Key_Stop_Irq(void)
{
//	Key_Stop_Dispose();
}


