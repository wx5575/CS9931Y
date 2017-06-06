#include "Relay.h"
#include "spi_cpld.h"

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */

#endif

/**************************************************************************
 *                           函数内联关键字宏定义 -- 编译器相关
***************************************************************************/ 

#define     INLINE                                   __inline 

#define  D25_ST_PORT           GPIOG
#define  D25_ST_PIN            GPIO_Pin_10
#define  D25_ST_SET()          ((GPIO_TypeDef *)D25_ST_PORT)->BSRRL = D25_ST_PIN
#define  D25_ST_CLR()          ((GPIO_TypeDef *)D25_ST_PORT)->BSRRH = D25_ST_PIN

#define  D25_CLK_PORT          GPIOG
#define  D25_CLK_PIN           GPIO_Pin_12
#define  D25_CLK_SET()         ((GPIO_TypeDef *)D25_CLK_PORT)->BSRRL = D25_CLK_PIN
#define  D25_CLK_CLR()         ((GPIO_TypeDef *)D25_CLK_PORT)->BSRRH = D25_CLK_PIN

#define  D25_DATA_PORT         GPIOG
#define  D25_DATA_PIN          GPIO_Pin_11
#define  D25_DATA_SET()        ((GPIO_TypeDef *)D25_DATA_PORT)->BSRRL = D25_DATA_PIN
#define  D25_DATA_CLR()        ((GPIO_TypeDef *)D25_DATA_PORT)->BSRRH = D25_DATA_PIN


#define  D23_ST_PORT           GPIOI
#define  D23_ST_PIN            GPIO_Pin_5
#define  D23_ST_SET()          ((GPIO_TypeDef *)D23_ST_PORT)->BSRRL = D23_ST_PIN
#define  D23_ST_CLR()          ((GPIO_TypeDef *)D23_ST_PORT)->BSRRH = D23_ST_PIN

#define  D23_CLK_PORT          GPIOH
#define  D23_CLK_PIN           GPIO_Pin_4
#define  D23_CLK_SET()         ((GPIO_TypeDef *)D23_CLK_PORT)->BSRRL = D23_CLK_PIN
#define  D23_CLK_CLR()         ((GPIO_TypeDef *)D23_CLK_PORT)->BSRRH = D23_CLK_PIN

#define  D23_DATA_PORT         GPIOF
#define  D23_DATA_PIN          GPIO_Pin_10
#define  D23_DATA_SET()        ((GPIO_TypeDef *)D23_DATA_PORT)->BSRRL = D23_DATA_PIN
#define  D23_DATA_CLR()        ((GPIO_TypeDef *)D23_DATA_PORT)->BSRRH = D23_DATA_PIN

#define  ACW_DCW_IR_PORT       GPIOB
#define  ACW_DCW_IR_PIN        GPIO_Pin_9
#define  ACW_DCW_IR_SET()      ((GPIO_TypeDef *)ACW_DCW_IR_PORT)->BSRRL = ACW_DCW_IR_PIN
#define  ACW_DCW_IR_CLR()      ((GPIO_TypeDef *)ACW_DCW_IR_PORT)->BSRRH = ACW_DCW_IR_PIN


static void Delay(volatile uint32_t t)
{
	t &= 0x3FF;
	while(t--);
}


static void Relay_CtrIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH | RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		  /* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	  /* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;	/* IO口最大速度 */
	
	GPIO_InitStructure.GPIO_Pin = D25_ST_PIN | D25_CLK_PIN | D25_DATA_PIN;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = D23_ST_PIN;
	GPIO_Init(D23_ST_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = D23_CLK_PIN;
	GPIO_Init(D23_CLK_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = D23_DATA_PIN;
	GPIO_Init(D23_DATA_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = ACW_DCW_IR_PIN;
	GPIO_Init(ACW_DCW_IR_PORT, &GPIO_InitStructure);
}

static union{
	uint32_t Relay_State;
	struct{
		uint8_t   D23_data_pool[2];
		uint8_t   D25_data_pool;
	}Each_Group;
}Relay;

//D23输出函数
/*******************************
函数名：  D23_Output
参  数：  无

返回值：  无
********************************/
static void  D23_Output(void)
{
	uint8_t i;
	D23_ST_CLR();
	Delay(10000);
	for(i=0;i<16;i++)
	{	
		D23_CLK_CLR();		
		if(Relay.Each_Group.D23_data_pool[(i/8)] & ((0x80)>>(i%8)))
		{
			D23_DATA_SET();
		}
		else
		{
			D23_DATA_CLR();
		}
		Delay(1400);
		D23_CLK_SET();	
		Delay(1400);
	}
	D23_ST_SET();
	Delay(0x3FF);
	D23_ST_CLR();
}

//D25输出函数
/*******************************
函数名：  D25_Output
参  数：  无

返回值：  无
********************************/
static  void  D25_Output(void)
{
	uint8_t i;
	
	D25_ST_CLR();
	
	for(i=0;i<(8);i++)
	{	
		D25_CLK_CLR();
		if(Relay.Each_Group.D25_data_pool & (0x80>>(i)))
		{
			D25_DATA_SET();
		}
		else
		{
			D25_DATA_CLR();
		}
		
		Delay(10);
		D25_CLK_SET();
		Delay(10);
	}
	
	D25_ST_SET();
	Delay(1400);
	D25_ST_CLR();
}

//继电器接通函数
/*******************************
函数名：  Relay_ON
参  数：  relay：需要操作的索引
返回值：  无
********************************/
void Relay_ON(uint32_t relay)
{
	Relay.Relay_State |= relay;
	
	if(relay & 0x0000ffff)
	{
		D23_Output();
	}
	
	if(relay & 0x00ff0000)
	{
		D25_Output();
	}
	
	if(relay & 0xff000000)
	{
		if(relay & ACW_DCW_IR)
		{
			ACW_DCW_IR_SET();
		}
	}
}

void ctrl_relay_EXT_DRIVE_O4_O5(uint8_t st)
{
    if(st == RELAY_ON)
    {
        Relay_ON(EXT_DRIVER_O4);
        Relay_ON(EXT_DRIVER_O5);
    }
    else
    {
        Relay_OFF(EXT_DRIVER_O4);
        Relay_OFF(EXT_DRIVER_O5);
    }
}

//继电器断开函数
/*******************************
函数名：  Relay_OFF
参  数：  relay：需要操作的索引
返回值：  无
********************************/
void Relay_OFF(uint32_t relay)
{
	Relay.Relay_State &= ~relay;
	
	
	if(relay & 0x0000ffff)
	{
		D23_Output();
	}
	
	if(relay & 0x00ff0000)
	{
		D25_Output();
	}
	
	if(relay & 0xff000000)
	{
		if(relay & ACW_DCW_IR)
		{
			ACW_DCW_IR_CLR();
		}
	}
}

//继电器状态设置函数
/*******************************
函数名：  Relay_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void Relay_State_Set(uint32_t state)
{
	Relay.Relay_State = state;
	D23_Output();
	D25_Output();
	if(state & ACW_DCW_IR){
		ACW_DCW_IR_SET();
	}else{
		ACW_DCW_IR_CLR();
	}
}

//继电器状态组合获取函数
/*******************************
函数名：  D23_Relay_State_Get
参  数：  无
返回值：  继电器状态
********************************/
uint16_t D23_Relay_State_Get(void)
{
	return (uint16_t)(Relay.Relay_State & 0x0000ffff);
}

//继电器状态组合获取函数
/*******************************
函数名：  D25_Relay_State_Get
参  数：  无
返回值：  继电器状态
********************************/
uint8_t D25_Relay_State_Get(void)
{
	return Relay.Each_Group.D25_data_pool;
}

//单个继电器状态获取函数
/*******************************
函数名：  Relay_State_Single_Get
参  数：  relay_index:继电器索引
返回值：  继电器状态 1:吸和  0:断开
********************************/
uint8_t Relay_State_Single_Get(uint32_t relay_index)
{
	if(Relay.Relay_State & relay_index)return 1;
	return 0;
}

//以下三个为PLC口控制
void PLC_OUT_C(uint8_t state)
{
	if(state) CPLD_GPIO_Control(OUT_C,1);
	else      CPLD_GPIO_Control(OUT_C,0);
}

void PLC_W_OUT_C(uint8_t state)
{
	if(state) CPLD_GPIO_Control(W_OUT_C,1);
	else      CPLD_GPIO_Control(W_OUT_C,0);
}

void PLC_METER_SOURCE_C(uint8_t state)
{
	if(state) CPLD_GPIO_Control(METER_SOURCE_C,1);
	else      CPLD_GPIO_Control(METER_SOURCE_C,0);
}

//继电器控制初始化函数
/*******************************
函数名：  Relay_Control_Init
参  数：  无
返回值：  继电器状态
********************************/

void Relay_Control_Init(void)
{
	Relay_CtrIO_Init();     //初始化控制引脚
	Relay_State_Set(0);     //断开所有的继电器
}


