#include "Multiplexer.h"

#include "spi_cpld.h"

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */

#endif

/**************************************************************************
 *                           函数内联关键字宏定义 -- 编译器相关
***************************************************************************/ 

#define     INLINE                                   __inline 

static struct{
	
	uint8_t ADG509A_State;         //ADG509A D20 D21状态
	uint8_t CD4053_D18_State;      //CD4053  D18状态
	uint8_t CD4053_D17_State;      //CD4053  D17状态
	uint8_t CD4053_D14_State;      //CD4053  D14状态
	uint8_t CD4051_D15_State;      //CD4051  D15状态
	
}Multiplexer_State;
			
static void Delay(volatile uint32_t t)
{
	while(t--){__NOP();}
}
			
			
/*ADG509A  D20  D21*/

#define  ADG509A_A1_PORT           GPIOG
#define  ADG509A_A1_PIN            GPIO_Pin_14
#define  ADG509A_A1_SET()          ((GPIO_TypeDef *)ADG509A_A1_PORT)->BSRRL = ADG509A_A1_PIN
#define  ADG509A_A1_CLR()          ((GPIO_TypeDef *)ADG509A_A1_PORT)->BSRRH = ADG509A_A1_PIN

#define  ADG509A_A0_PORT           GPIOG
#define  ADG509A_A0_PIN            GPIO_Pin_13
#define  ADG509A_A0_SET()          ((GPIO_TypeDef *)ADG509A_A0_PORT)->BSRRL = ADG509A_A0_PIN
#define  ADG509A_A0_CLR()          ((GPIO_TypeDef *)ADG509A_A0_PORT)->BSRRH = ADG509A_A0_PIN

//ADG509A控制引脚初始化
/*******************************
函数名：  ADG509A_CtrIO_Init
参  数：  无
返回值：  无
********************************/
static void ADG509A_CtrIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		  /* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	  /* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
	
	GPIO_InitStructure.GPIO_Pin = ADG509A_A0_PIN | ADG509A_A1_PIN;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	/*V1.1 更改添加*/
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		  /* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	  /* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

}



//ADG509A状态设置函数
/*******************************
函数名：  ADG509A_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void ADG509A_State_Set(uint8_t state)
{
	if(state>3) return;
	Multiplexer_State.ADG509A_State = state;
	if(state & 0x01){ADG509A_A0_SET();}else{ADG509A_A0_CLR();}
	if(state & 0x02){ADG509A_A1_SET();}else{ADG509A_A1_CLR();}
	Delay(30000);
}

//继电器状态组合获取函数
/*******************************
函数名：  ADG509A_State_Get
参  数：  无
返回值：  ADG509A状态
********************************/
uint8_t ADG509A_State_Get(void)
{
	return Multiplexer_State.ADG509A_State;
}


//ADG509A初始化函数
/*******************************
函数名：  ADG509A_Init
参  数：  无
返回值：  无
********************************/
static void ADG509A_Init(void)
{
	ADG509A_CtrIO_Init();
	ADG509A_State_Set(ADG509A_S1);
}



//CD4053_D18状态设置函数
/*******************************
函数名：  CD4053_D18_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void CD4053_D18_State_Set(uint8_t state)
{
	if(state>3) return;
	Multiplexer_State.CD4053_D18_State = state;
	if(state & 0x01){CPLD_GPIO_Control(SINE_CD4053_A,1);}else{CPLD_GPIO_Control(SINE_CD4053_A,0);}
	if(state & 0x02){CPLD_GPIO_Control(SINE_CD4053_B,1);}else{CPLD_GPIO_Control(SINE_CD4053_B,0);}
//	Delay(30000);
}

//继电器状态组合获取函数
/*******************************
函数名：  CD4053_D18_State_Get
参  数：  无
返回值：  CD4053_D18状态
********************************/
uint8_t CD4053_D18_State_Get(void)
{
	return Multiplexer_State.CD4053_D18_State;
}


//CD4053_D18初始化函数
/*******************************
函数名：  CD4053_D18_Init
参  数：  无
返回值：  无
********************************/
static void CD4053_D18_Init(void)
{
	CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
}




//CD4053_D14状态设置函数
/*******************************
函数名：  CD4053_D14_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void CD4053_D14_State_Set(uint8_t state)
{
	if(state>7) return;
	Multiplexer_State.CD4053_D14_State = state;
	if(state & 0x01){CPLD_GPIO_Control(W_CD4053_C,1);}else{CPLD_GPIO_Control(W_CD4053_C,0);}
	/*V1.1 更改添加*/
	if(state & 0x01){GPIO_SetBits(GPIOE,GPIO_Pin_5);} else{GPIO_ResetBits(GPIOE,GPIO_Pin_5);}
	if(state & 0x02){CPLD_GPIO_Control(W_CD4053_B,1);}else{CPLD_GPIO_Control(W_CD4053_B,0);}
	if(state & 0x04){CPLD_GPIO_Control(W_CD4053_A,1);}else{CPLD_GPIO_Control(W_CD4053_A,0);}
//	Delay(30000);
}

//继电器状态组合获取函数
/*******************************
函数名：  CD4053_D14_State_Get
参  数：  无
返回值：  CD4053_D14状态
********************************/
uint8_t CD4053_D14_State_Get(void)
{
	return Multiplexer_State.CD4053_D14_State;
}


//CD4053_D14初始化函数
/*******************************
函数名：  CD4053_D14_Init
参  数：  无
返回值：  无
********************************/
static void CD4053_D14_Init(void)
{
	CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
}



//CD4053_D17状态设置函数
/*******************************
函数名：  CD4053_D17_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void CD4053_D17_State_Set(uint8_t state)
{
	if(state>7) return;
	Multiplexer_State.CD4053_D17_State = state;
	if(state & 0x01){CPLD_GPIO_Control(GR_CD4053_C,1);}else{CPLD_GPIO_Control(GR_CD4053_C,0);}
	if(state & 0x02){CPLD_GPIO_Control(GR_CD4053_B,1);}else{CPLD_GPIO_Control(GR_CD4053_B,0);}   //电路图中A C反了
	if(state & 0x04){CPLD_GPIO_Control(GR_CD4053_A,1);}else{CPLD_GPIO_Control(GR_CD4053_A,0);}
	Delay(30000);
}

//继电器状态组合获取函数
/*******************************
函数名：  CD4053_D17_State_Get
参  数：  无
返回值：  CD4053_D17状态
********************************/
uint8_t CD4053_D17_State_Get(void)
{
	return Multiplexer_State.CD4053_D17_State;
}


//CD4053_D14初始化函数
/*******************************
函数名：  CD4053_D14_Init
参  数：  无
返回值：  无
********************************/
static void CD4053_D17_Init(void)
{
	CD4053_D17_State_Set(CD4053_D17_X0 | CD4053_D17_Y1 | CD4053_D17_Z0);
}


//CD4051_D15状态设置函数
/*******************************
函数名：  CD4051_D15_State_Set
参  数：  state：需要操作的索引
返回值：  无
********************************/
void CD4051_D15_State_Set(uint8_t state)
{
	if(state>3) return;
	Multiplexer_State.CD4051_D15_State = state;
	
	if(state & 0x01)
	{
		CPLD_GPIO_Control(W_CD4051_A, 1);
	}
	else
	{
		CPLD_GPIO_Control(W_CD4051_A, 0);
	}
	
	if(state & 0x02)
	{
		CPLD_GPIO_Control(W_CD4051_B, 1);
	}
	else
	{
		CPLD_GPIO_Control(W_CD4051_B, 0);
	}
//	Delay(30000);
}

//继电器状态组合获取函数
/*******************************
函数名：  CD4051_D15_State_Get
参  数：  无
返回值：  CD4051_D15状态
********************************/
uint8_t CD4051_D15_State_Get(void)
{
	return Multiplexer_State.CD4051_D15_State;
}


//CD4051_D15初始化函数
/*******************************
函数名：  CD4051_D15_Init
参  数：  无
返回值：  无
********************************/
static void CD4051_D15_Init(void)
{
	CD4051_D15_State_Set(DC_VOL_FB);
}


//多路控制开关初始化函数
/*******************************
函数名：  Multiplexer_Control_Init
参  数：  无
返回值：  无
********************************/
void Multiplexer_Control_Init(void)
{
	ADG509A_Init();
	CD4053_D18_Init();
	CD4053_D14_Init();
	CD4051_D15_Init();
	CD4053_D17_Init();
}










