#include "LC.h"
#include "Relay.h"
#include "Output_Control.h"
#include "CS9931_Config.h"
#include "Relay.h"
#include "AD_DA.h"
#include "spi_cpld.h"
#include "Multiplexer.h"
#include "Relay_Change.h"
#include "Key_LED.h"
#include "jj98.h"
#include "Cal.h"
#include "Test_Sched.h"
#include "stm32f4xx.h"

/*宏定义*/
#define   LC_EXT_O1_PORT          GPIOB
#define   LC_EXT_O1_PIN           GPIO_Pin_10
#define   LC_EXT_O2_PORT          GPIOB
#define   LC_EXT_O2_PIN           GPIO_Pin_11
#define   LC_EXT_O3_PORT          GPIOH
#define   LC_EXT_O3_PIN           GPIO_Pin_6
#define   LC_EXT_O4_PORT          GPIOH
#define   LC_EXT_O4_PIN           GPIO_Pin_7
#define   LC_EXT_O5_PORT          GPIOH
#define   LC_EXT_O5_PIN           GPIO_Pin_8
#define   LC_EXT_O6_PORT          GPIOH
#define   LC_EXT_O6_PIN           GPIO_Pin_9
#define   LC_EXT_O7_PORT          GPIOH
#define   LC_EXT_O7_PIN           GPIO_Pin_10
#define   LC_EXT_O8_PORT          GPIOH
#define   LC_EXT_O8_PIN           GPIO_Pin_11

#define   CS5460_CS_PORT          LC_EXT_O2_PORT
#define   CS5460_CS_PIN           LC_EXT_O2_PIN
#define   AD_CS1_PORT             LC_EXT_O1_PORT
#define   AD_CS1_CS_PIN           LC_EXT_O1_PIN

#define   AD_CS_PORT              LC_EXT_O3_PORT
#define   AD_CS_PIN               LC_EXT_O3_PIN
#define   SPI_MOSI_PORT           LC_EXT_O6_PORT
#define   SPI_MOSI_PIN            LC_EXT_O6_PIN
#define   SPI_MISO_PORT           LC_EXT_O5_PORT
#define   SPI_MISO_PIN            LC_EXT_O5_PIN
#define   CD4094_ST_PORT          LC_EXT_O8_PORT
#define   CD4094_ST_PIN           LC_EXT_O8_PIN
#define   SPI_CLK_PORT            LC_EXT_O7_PORT
#define   SPI_CLK_PIN             LC_EXT_O7_PIN

#define   CS5460_CS_SET()         ((GPIO_TypeDef *)CS5460_CS_PORT)->BSRRL = CS5460_CS_PIN
#define   CS5460_CS_CLR()         ((GPIO_TypeDef *)CS5460_CS_PORT)->BSRRH = CS5460_CS_PIN

#define   AD_CS1_SET()            ((GPIO_TypeDef *)AD_CS1_PORT)->BSRRL = AD_CS1_CS_PIN
#define   AD_CS1_CLR()            ((GPIO_TypeDef *)AD_CS1_PORT)->BSRRH = AD_CS1_CS_PIN

#define   AD_CS_SET()             ((GPIO_TypeDef *)AD_CS_PORT)->BSRRL = AD_CS_PIN
#define   AD_CS_CLR()             ((GPIO_TypeDef *)AD_CS_PORT)->BSRRH = AD_CS_PIN

#define   SPI_MOSI_SET()          ((GPIO_TypeDef *)SPI_MOSI_PORT)->BSRRL = SPI_MOSI_PIN
#define   SPI_MOSI_CLR()          ((GPIO_TypeDef *)SPI_MOSI_PORT)->BSRRH = SPI_MOSI_PIN

#define   CD4094_ST_SET()         ((GPIO_TypeDef *)CD4094_ST_PORT)->BSRRL = CD4094_ST_PIN
#define   CD4094_ST_CLR()         ((GPIO_TypeDef *)CD4094_ST_PORT)->BSRRH = CD4094_ST_PIN

#define   SPI_CLK_SET()           ((GPIO_TypeDef *)SPI_CLK_PORT)->BSRRL = SPI_CLK_PIN
#define   SPI_CLK_CLR()           ((GPIO_TypeDef *)SPI_CLK_PORT)->BSRRH = SPI_CLK_PIN




#define   NUM_OF_4094            (5)                 //4094级联的数量
#define		C4094_DLY				       (2000)


static void Delay_ms(unsigned int dly_ms)
{
  unsigned int dly_i;
  while(dly_ms--)
    for(dly_i=0;dly_i<18714;dly_i++);
}


void LC_Control_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOH, ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽模式 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	/* 无需上下拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* IO口最大速度 */
	
	GPIO_InitStructure.GPIO_Pin = CS5460_CS_PIN | AD_CS1_CS_PIN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = AD_CS_PIN | SPI_MOSI_PIN | CD4094_ST_PIN | SPI_CLK_PIN;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		/* 设为输入口 */
	GPIO_InitStructure.GPIO_Pin = SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无需上下拉电阻 */
	GPIO_Init(GPIOH, &GPIO_InitStructure);
		
}


static void C4094_Delay(uint32_t t)
{
	while(t--);
}

static  uint8_t   C4094_data_pool[NUM_OF_4094];   //串行输出缓冲池
static  void  C4094_Output(void)
{
	uint8_t i;
	
	for(i=0;i<(NUM_OF_4094*8);i++)
	{	
		SPI_CLK_CLR();
		if(C4094_data_pool[NUM_OF_4094-1-(i/8)] & (0x80>>(i%8)))
		{
			SPI_MOSI_SET();
		}
		else
		{
			SPI_MOSI_CLR();
		}
		C4094_Delay(C4094_DLY);
		SPI_CLK_SET();
		C4094_Delay(C4094_DLY);
	}
	CD4094_ST_SET();
	C4094_Delay(C4094_DLY);
	CD4094_ST_CLR();
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
}

void LC_Relay_Control(uint8_t index,uint8_t On_or_off,uint8_t Is_Updata)
{
	if(index > NUM_OF_4094*8-1)return;
	if(On_or_off){
		C4094_data_pool[(index / 8)] |= 1 << (index % 8);
	}else{
		C4094_data_pool[(index / 8)] &= ~(1 << (index % 8));
	}
	if(Is_Updata)	C4094_Output();
}

static void Delay(uint16_t time)
{
	while(time--);
}


static uint8_t SPI_HostReadWriteByte(uint8_t dat) 
{    
    uint8_t i, rByte = 0;
    
//     Delay(1200);
    
    for(i = 0; i < 8; i++)
    {   
		if(dat & (0x80 >>i))
        {
			SPI_MOSI_SET();
        }
		else
        {
			SPI_MOSI_CLR();
        }
        
		Delay(1500);
        SPI_CLK_SET();
		Delay(1500);
		SPI_CLK_CLR();
		Delay(1500);
		rByte <<= 1;
		rByte |= (GPIO_ReadInputDataBit(SPI_MISO_PORT,SPI_MISO_PIN)?1:0);    
    }
    
    return rByte;
}


uint16_t  D3_Mcp3202_Read(uint8_t channel) 
{
	uint16_t Ad_Val;
	uint8_t temp ;
	
    if(sampling_mutex_flag==1)
    {
        return 0xffff;
    }
    sampling_mutex_flag= 1;
    
	if(channel)
		temp = 0xff ;
	else 
		temp = 0xbf ;

// 	AD_CS_SET();
// 	Delay(2000);
	AD_CS_CLR();
	Delay(2000);
    
	SPI_HostReadWriteByte(0x01);              //START
    
	Ad_Val = SPI_HostReadWriteByte(temp) & 0x1F;
	Ad_Val <<= 8;
	Ad_Val |= SPI_HostReadWriteByte(0);

	AD_CS_SET();
    
    sampling_mutex_flag= 0;
	return Ad_Val>>1;
}

uint16_t  D16_Mcp3202_Read(uint8_t channel) 
{
	uint16_t Ad_Val;
	uint8_t temp ;
	
    if(sampling_mutex_flag==1)
    {
        return 0;
    }
    sampling_mutex_flag= 1;
    
	if(channel)
		temp = 0xff ;
	else 
		temp = 0xbf ;

// 	AD_CS1_SET();
// 	Delay(20000);
	AD_CS1_CLR();
	Delay(2000);

	SPI_HostReadWriteByte(0x01);              //START
	
	Ad_Val = SPI_HostReadWriteByte(temp) & 0x1F;
	Ad_Val <<= 8;
	Ad_Val |= SPI_HostReadWriteByte(0);

	AD_CS1_SET();
    sampling_mutex_flag= 0;
	return Ad_Val>>1;
}

void LC_4051_D15_SELECT(uint8_t channel)
{
	if(channel > 2)return;
	if(channel & 0x01){LC_Relay_Control(D15_CD4051_A,1,0);}else{LC_Relay_Control(D15_CD4051_A,0,0);}
	if(channel & 0x02){LC_Relay_Control(D15_CD4051_B,1,1);}else{LC_Relay_Control(D15_CD4051_B,0,1);}
}

void LC_4051_D1_SELECT(uint8_t channel)
{
	if(channel > 2)return;
	if(channel & 0x01){LC_Relay_Control(CD4051_A,1,0);}else{LC_Relay_Control(CD4051_A,0,0);}
	if(channel & 0x02){LC_Relay_Control(CD4051_B,1,1);}else{LC_Relay_Control(CD4051_B,0,1);}
}



/*CS5460部分*/

static void _HAL_CS5460Write(uint8_t command, uint8_t dataHigh, uint8_t dataMiddle, uint8_t dataLow);
void HAL_CS5460Init(void);
uint32_t HAL_CS5460Read(uint8_t command);
void HAL_CS5460Start(void);
/******************************************************************************
 *                          本文件内部宏函数定义
******************************************************************************/

#define                        _CS5460_INNER_DELAY             (30)  //2
#define                        _CS5460_INNER_DELAY_1           (30)

#define                        __CS5460_HARDWARE               (0)
#define                        _CS5460_DEBUG                   (0)

#define                        CS5460_SELECT()                 CS5460_CS_CLR() 
#define                        CS5460_DESELECT()               CS5460_CS_SET()        

#define                        CS5460_SCK_HIGH()               SPI_CLK_SET()
#define                        CS5460_SCK_LOW()                SPI_CLK_CLR() 

#define                        CS5460_DIN_HIGH()               SPI_MOSI_SET()
#define                        CS5460_DIN_LOW()                SPI_MOSI_CLR() 

#define                        CS5460_DOUT_READ_INIT()         
#define                        GET_CS5460_DOUT_VALUE()      	(GPIO_ReadInputDataBit(SPI_MISO_PORT,SPI_MISO_PIN)?1:0)

#define                        SYNC1                            (0xff)
#define                        SYNC0                            (0xfe)
#define                        CYCLE                            (0x4a)          //周期寄存器地址
#define                        STATUS_REG                       (0x1e)          //读状态寄存器
#define                        WRTIE_STATUS_REG                 (0x5e)          //写状态寄存器

#define                        CS5460_COFIG                     (0x40)
#define                        CS5460_COFIG_LOW8                (0x62)
#define                        CS5460_COFIG_MID8                (0x00)
#define                        CS5460_COFIG_HIGHT8              (0x00)
#define                        START_CS5460                     (0xe8)          //以多周期方式启动CS5460A

#define						             APIDelayUs                       Delay

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                             
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                              
 *                                                                            
 *                                                                            
******************************************************************************/ 

void InitCS5460A(void)
{
	HAL_CS5460Init();
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                             
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                              
 *                                                                            
 *                                                                            
******************************************************************************/ 


void HAL_CS5460Init(void)
{
    _HAL_CS5460Write(SYNC1, SYNC1, SYNC1, SYNC0);
	_HAL_CS5460Write(CS5460_COFIG, CS5460_COFIG_HIGHT8, CS5460_COFIG_MID8, CS5460_COFIG_LOW8);
    _HAL_CS5460Write(CYCLE, 0x00, 0x10, 0x00); //周期寄存器地址 目前为0x1000个周期    
    HAL_CS5460Start();
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                             
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                              
 *                                                                            
 *                                                                            
******************************************************************************/

static void _HAL_CS5460WriteInner(uint8_t command)
{
	uint8_t i = 7;

	for (; i != ((uint8_t)-1); i--)          //开始一个字节的CData
	{ 
		APIDelayUs(_CS5460_INNER_DELAY);
		CS5460_SCK_LOW(); 
		APIDelayUs(_CS5460_INNER_DELAY);

		if (0x80 == (command & 0x80))
		{ 
			CS5460_DIN_HIGH();
		}
		else
		{
			CS5460_DIN_LOW();
		}
		command <<= 1; 
		APIDelayUs(_CS5460_INNER_DELAY); 
		CS5460_SCK_HIGH(); 
		APIDelayUs(_CS5460_INNER_DELAY); 
	}
}

static void _HAL_CS5460Write(uint8_t command, uint8_t dataHigh, uint8_t dataMiddle, uint8_t dataLow)
{
    //选择CS5460    
    CS5460_SELECT();
    APIDelayUs(_CS5460_INNER_DELAY_1);
    
    _HAL_CS5460WriteInner(command);    
    _HAL_CS5460WriteInner(dataHigh);  
    _HAL_CS5460WriteInner(dataMiddle);  
    _HAL_CS5460WriteInner(dataLow); 
    
    //释放CS5460 
    CS5460_DESELECT();  
    APIDelayUs(_CS5460_INNER_DELAY_1);
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

static uint8_t _HAL_CS5460ReadInner(void)
{
    uint8_t rt = 0;
    uint8_t i = 7;
    
	//输入相应的命令 FE
	for (; i != ((uint8_t)-1); i--)    
	{
		if (i > 0)
		{
			CS5460_DIN_HIGH();
		}
		else
		{
			CS5460_DIN_LOW();
		}
		APIDelayUs(_CS5460_INNER_DELAY);
		CS5460_SCK_LOW();
		APIDelayUs(_CS5460_INNER_DELAY);
		
		rt <<= 1;
		if (1 == GET_CS5460_DOUT_VALUE())   
		{
			rt |= 0x01;
		}
		
		APIDelayUs(_CS5460_INNER_DELAY);
		CS5460_SCK_HIGH();
		APIDelayUs(_CS5460_INNER_DELAY);
	}
	
    return rt;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                             
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                              
 *                                                                            
 *                                                                            
******************************************************************************/

void HAL_CS5460Start(void)
{
    //选择CS5460
    CS5460_SELECT();
    APIDelayUs(_CS5460_INNER_DELAY_1);
	
    _HAL_CS5460WriteInner(START_CS5460);
	
    //释放CS5460
    CS5460_DESELECT();
	APIDelayUs(_CS5460_INNER_DELAY_1);
	
    //每次初始化或启动CS5460时 都要空读一次电压和电流值
    HAL_CS5460Read(VOLTAGE_RMS_CMD);
    HAL_CS5460Read(CURRENT_RMS_CMD);
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *                                                                            
******************************************************************************/

uint32_t HAL_CS5460Read(uint8_t command)
{
    uint32_t rt = 0;
    uint8_t i = 2;
	
    //选择CS5460
    CS5460_SELECT();
    APIDelayUs(_CS5460_INNER_DELAY_1);
    _HAL_CS5460WriteInner(command);
	
    for (; i != ((uint8_t)-1); i--)
    {
        rt <<= 8;
        rt  |= _HAL_CS5460ReadInner();
    }
    
    if ((TRUE_POWER_CMD == command) && ((rt & 0x00800000) == 0x00800000))
    {
        rt = (~rt) + 1; 
    } 
    
    //释放CS5460 
    CS5460_DESELECT(); 
	APIDelayUs(_CS5460_INNER_DELAY_1);   
	
    return rt & 0x00ffffff;    
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *
******************************************************************************/                                                                            

uint8_t HAL_CS5460GetStatus(void)
{
    uint32_t cs5460Status                      = HAL_CS5460Read(STATUS_REG);

    if ((cs5460Status & 0x00800000) == 0x00800000)
    {
		cs5460Status = 0;
		cs5460Status |= 0x00800000;
		_HAL_CS5460Write(WRTIE_STATUS_REG, *((uint8_t *)&cs5460Status + 2), 
						 *((uint8_t *)&cs5460Status + 1), *((uint8_t *)&cs5460Status ));
		cs5460Status = HAL_CS5460Read(STATUS_REG);
		
		return 1;
    }

    return 0;
}

/******************************************************************************
 *  函数名称 :                                                               
 *                                                                           
 *  函数功能 :                                                              
 *                                                                           
 *  入口参数 :                                                                
 *                                                                             
 *  出口参数 :                                                                
 *                                                                              
 *  编 写 者 :                                                                
 *                                                                                 
 *  日    期 :                                                                 
 *                                                                              
 *  审 核 者 :                                                                                                                               
 *                                                                             
 *  日    期 :                                                                
 *                                                                                                                                       
 *  附    注 :                                                                
 *                                                                            
 *
******************************************************************************/    

uint32_t ReadCurrentRmsValue(uint32_t *value)
{
	uint32_t count = 0;
	
	while(1 != HAL_CS5460GetStatus())
	{
		if(++count > 10)
		{
			return 1;
		}
	}
	
	*value = HAL_CS5460Read(CURRENT_RMS_CMD);
	return 0;
}

uint8_t ReadRealCurrent(float *accessAddr)
{
	if (1 == HAL_CS5460GetStatus())
	{
		uint32_t truePowerValue  = HAL_CS5460Read(TRUE_POWER_CMD);
		uint32_t VoltageRmsValue = HAL_CS5460Read(VOLTAGE_RMS_CMD);
		uint32_t CurrentRmsValue = HAL_CS5460Read(CURRENT_RMS_CMD);
		
		if (VoltageRmsValue == 0)
		{
			return 0;
		}
		else
		{
			(*accessAddr) = (float)truePowerValue*5000/VoltageRmsValue;//(float)truePowerValue*10000/VoltageRmsValue;

			return 1;
		}
	}
	
	return 0;
}

static uint16_t LC_Assist_Get_DA_Value(uint32_t dst_voltage)
{
	uint8_t i = 0;
	float k;
	
	if(dst_voltage <= Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].Voltage)
	{
		return ((float)dst_voltage / (float)Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].Voltage)
					* Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].DA_value;
	}
		for(;i<Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num;i++){
		if(dst_voltage < Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].Voltage)break;
	}
	if(i == Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num){i--;}
	i--;
	k = (float)(Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i+1].DA_value - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].DA_value) / \
	    (float)(Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i+1].Voltage - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].Voltage);
	return Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].DA_value + k * (dst_voltage - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].Voltage);

}

void LC_Assit_Output_Enable(void)
{
	CD4053_D17_State_Set(CD4053_D17_X1 | CD4053_D17_Y0 | CD4053_D17_Z1);
	
}

void LC_Assit_Output_Disable(void)
{
	CD4053_D17_State_Set(CD4053_D17_X0 | CD4053_D17_Y1 | CD4053_D17_Z0);
	Relay_OFF(AMP_RELAY_1);
	Relay_OFF(AMP_RELAY_2);
	DAC_SetValue(LC_ASSIT_VREF,0);
}

void LC_Assit_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	if(dst_voltage){
		
		if(IsRelayDone == 0){
// 			LC_Relay_Control(GB9706_1,1,0);
// 			LC_Relay_Control(LC_NY,1,0);
// 			LC_Relay_Control(MD_HI_GND,1,1);
			
//			Relay_ON(EXT_DRIVER_O8);
//			Delay_ms(10);
			
//			LC_4051_D1_SELECT(1);
			CD4051_D15_State_Set(LC_VOL_FB);
			Relay_ON(AMP_RELAY_1);
			Relay_ON(AMP_RELAY_2);
			Relay_ON(AMP_RELAY_5);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_3);
			ctrl_relay_EXT_DRIVE_O4_O5(RELAY_ON);///<2017.5.11 wangxin
			
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			
			Delay_ms(10);
			
			CPLD_Sine_SetRate(GR_SINE,out_rate);
			CPLD_Sine_Control(GR_SINE,ON);
			Delay_ms(1);
//			Relay_OFF(EXT_DRIVER_O5);
			IsRelayDone = 1;
		}
		DAC_SetValue(LC_ASSIT_VREF, LC_Assist_Get_DA_Value(dst_voltage));
//		LC_Assit_Output_Enable();
	}else{
		IsRelayDone = 0;

		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		CPLD_Sine_Control(GR_SINE,OFF);
		DAC_SetValue(LC_ASSIT_VREF, 0);
		Relay_OFF(AMP_RELAY_5);
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_3);
		
		Relay_OFF(AMP_RELAY_6);//2016.8.3 wangxin

	}
}


void LC_Assit_ADValue_Set(uint32_t ADValue,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	if(ADValue){
		LC_Assit_Output_Enable();
		if(IsRelayDone == 0){
//			LC_4051_D1_SELECT(1);
			CD4051_D15_State_Set(LC_VOL_FB);
			LC_4051_D15_SELECT(LC_VOL_FB);
			Relay_ON(AMP_RELAY_5);
			Relay_ON(AMP_RELAY_4);
			Relay_ON(AMP_RELAY_3);
			CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
			Delay_ms(1);
			
			CPLD_Sine_SetRate(GR_SINE,out_rate);
			CPLD_Sine_Control(GR_SINE,ON);
			IsRelayDone = 1;
//			Relay_ON(EXT_DRIVER_O8);
		}
		
		DAC_SetValue(LC_ASSIT_VREF, ADValue);
	}else{
		IsRelayDone = 0;
		LC_Assit_Output_Disable();
		CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
		Relay_OFF(AMP_RELAY_5);
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(AMP_RELAY_3);
		CPLD_Sine_Control(GR_SINE,OFF);
		DAC_SetValue(LC_ASSIT_VREF, 0);
//		Relay_OFF(EXT_DRIVER_O8);

	}
}


uint32_t LC_Get_Assist_Voltage(void)
{
	uint8_t i = 0;
	uint16_t AD_Value = D16_Mcp3202_Read(1);
	
	float k;
	if(AD_Value <= 12)
		return 0;
	if(AD_Value <= Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].AD_value)
		return ((float)AD_Value / (float)Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].AD_value) * Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[0].Voltage;
	for(;i<Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num;i++){
		if(AD_Value < Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].AD_value)break;
	}
	if(i == Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num){i--;}
	i--;
	k = (float)(Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i+1].Voltage - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].Voltage) / \
	    (float)(Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i+1].AD_value - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].AD_value);
	return Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].Voltage + k * (AD_Value - Global_Cal_Facter.LC_A_Facter.LC_Cal_Point[i].AD_value);
}

#include "CS99XX.H"
static uint16_t LC_Main_Get_DA_Value(uint32_t dst_voltage)
{
// 	return (lc_main_vol_ratio.da_k * dst_voltage / 100.0 + lc_main_vol_ratio.da_b);
	
	
	uint8_t i = 0;
	float k;
	
	if(dst_voltage <= Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].Voltage)
	{
		return ((float)dst_voltage / (float)Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].Voltage)
				* Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].DA_value;
	}
	
	for(;i<Global_Cal_Facter.LC_M_Facter.LC_Cal_Point_Num;i++)
	{
		if(dst_voltage < Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].Voltage)
			break;
	}
	
	if(i == Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num)
	{
		i--;
	}
	
	i--;
	k = (float)(Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i+1].DA_value - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].DA_value) / \
	    (float)(Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i+1].Voltage - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].Voltage);
	
	return Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].DA_value
			+ k * (dst_voltage - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].Voltage);
}


void LC_Main_Output_Enable(void)
{
	CD4053_D14_State_Set(CD4053_D14_X1 | CD4053_D14_Y0 | CD4053_D14_Z0);
	Relay_ON(AMP_RELAY_5);
    CPLD_GPIO_Control(W_OUT_C,0);
	Delay_ms(1);
	Relay_ON(AMP_RELAY_3);
}

void LC_Main_Output_Disable(void)
{
	LC_Main_Voltage_Set(0,0);
	CD4053_D14_State_Set(CD4053_D14_X0 | CD4053_D14_Y1 | CD4053_D14_Z0);
	Relay_OFF(AMP_RELAY_5);
	Relay_OFF(AMP_RELAY_3);
	LC_Relay_Control(LC_NY,0,1);
}
#define JJ98_DELAY  300 //ms
void CAL_LC_Main_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate)
{
    if(dst_voltage != 0)
    {
        ctrl_signal_dault_relay(RELAY_OFF);
    }
    else
    {
        ctrl_signal_dault_relay(RELAY_ON);
    }
    LC_Main_Voltage_Set(dst_voltage, out_rate);
}
void LC_Main_Voltage_Set(uint32_t dst_voltage,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
    
	if(dst_voltage){
		if(IsRelayDone == 0){
// 			LC_Relay_Control(GB9706_1,1,0);
// 			LC_Relay_Control(LC_NY,1,0);
// 			LC_Relay_Control(MD_HI_GND,1,1);
// 			LC_4051_D1_SELECT(1);
// 			LC_4051_D15_SELECT(1);
// 			
// 			Delay_ms(1);
//             ctrl_signal_dault_relay(global_singlefault);
			if(LC_TEST_MODE==LC_YY)
			DAC_SetValue(W_VREF,0);
			
			Delay_ms(10);
// 			Relay_ON(EXT_DRIVER_O8);
			Delay_ms(20);
			Relay_ON(AMP_RELAY_4);
			CD4051_D15_State_Set(LC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE,out_rate);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE,ON);
			IsRelayDone = 1;
		}
		
		/*****************************************/
		/* 2016.8.3 wangxin */
		if(dst_voltage < 260 * 1000) //电压小于260V
		{
			Relay_ON(AMP_RELAY_6);
		}
		else
		{
			Relay_OFF(AMP_RELAY_6);
		}
        
		/****************************************/
		if(LC_TEST_MODE == LC_YY)
		{
			DAC_SetValue(W_VREF,LC_Main_Get_DA_Value(dst_voltage));
		}
		else
		{
			Delay_ms(JJ98_DELAY);
			jj98_set_freq(out_rate, 0);
			Delay_ms(50);
			jj98_set_vol(dst_voltage / 10, 2);
			Delay_ms(50);
			jj98_start_test();
			Delay_ms(100);
			jj98_start_test();
			Delay_ms(100);
			jj98_start_test();
			Delay_ms(100);
			jj98_start_test();
			Delay_ms(100);
			jj98_start_test();
		}
        
        if(LC_TEST_MODE == LC_WY)
		{
            ctrl_relay_EXT_DRIVE_O4_O5(RELAY_ON);///<2017.5.11 wangxin
        }
	}
	else
	{
		IsRelayDone = 0;
		
		if(LC_TEST_MODE == LC_YY)
		{
			DAC_SetValue(W_VREF,0);
		}
		else
		{
			jj98_stop_test();
			 Delay_ms(50);
			jj98_stop_test();
			 Delay_ms(50);
			jj98_stop_test();
		}
		
		Delay_ms(10);
//		Sampling_Relay_State_CHange(DC_100mA);
		
//        ctrl_signal_dault_relay(RELAY_OFF);
		CPLD_Sine_Control(ADCW_SINE | GR_SINE,OFF);
		Relay_OFF(AC_DC);
		
		Relay_OFF(AMP_RELAY_4);
		Relay_OFF(EXT_DRIVER_O8);
        
        if(LC_TEST_MODE == LC_WY)
		{
            ctrl_relay_EXT_DRIVE_O4_O5(RELAY_OFF);///<2017.5.11 wangxin
        }
	}
}

void LC_Main_ADValue_Set(uint32_t ADValue,uint16_t out_rate)
{
	static uint8_t IsRelayDone = 0;
	
	if(ADValue)
	{
		LC_Main_Output_Enable();
		
		if(IsRelayDone == 0)
		{
			/******************************/
			if(LC_TEST_MODE==LC_YY)
			{
				DAC_SetValue(W_VREF,0);
			}
			
			Delay_ms(10);
// 			Relay_ON(EXT_DRIVER_O8);
			Delay_ms(20);
			Relay_ON(AMP_RELAY_4);
			CD4051_D15_State_Set(LC_VOL_FB);
			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);	
			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE,out_rate);
			CPLD_Sine_Control(ADCW_SINE | GR_SINE,ON);
			IsRelayDone = 1;
			/******************************/
		
// 			CD4051_D15_State_Set(LC_VOL_FB);
// 			CD4053_D18_State_Set(CD4053_D18_X0 | CD4053_D18_Y0);
// 			CPLD_Sine_SetRate(ADCW_SINE | GR_SINE,out_rate);
// 			CPLD_Sine_Control(ADCW_SINE | GR_SINE,ON);
// 			IsRelayDone = 1;
// 			Relay_ON(EXT_DRIVER_O8);
		}
// 		DAC_SetValue(W_VREF,0);
		if(LC_TEST_MODE==LC_YY)
		{
			DAC_SetValue(W_VREF, ADValue);//2016.8.9 wangxin
		}
		else
		{
			Delay_ms(JJ98_DELAY);
			jj98_set_freq(out_rate, 0);
			Delay_ms(JJ98_DELAY);
			jj98_set_vol(ADValue, 0);
			Delay_ms(JJ98_DELAY);
			jj98_start_test();
			Delay_ms(JJ98_DELAY);
			jj98_start_test();
		}
	}
	else
	{
		IsRelayDone = 0;
		LC_Main_Output_Disable();
		CPLD_Sine_Control(ADCW_SINE | GR_SINE,OFF);
		if(LC_TEST_MODE==LC_YY)
		{
			DAC_SetValue(W_VREF,0);
		}
		else
		{
			Delay_ms(JJ98_DELAY);
			jj98_stop_test();
			Delay_ms(JJ98_DELAY);
		}
		
		Relay_OFF(AC_DC);
		Relay_OFF(EXT_DRIVER_O8);
	}
}

uint32_t get_lc_main_current(uint32_t *cur_value)
{
	uint32_t AD_Currrent = 0;
	uint32_t Current = 0;
	uint32_t res = 0;
	
	res = ReadCurrentRmsValue(&AD_Currrent);
	if(res == 0)
	{
		*cur_value = 1.0 * Global_Cal_Facter.LC_MC_Facter.Current / Global_Cal_Facter.LC_MC_Facter.AD_Currrent * AD_Currrent;
	}
	
	return res;
}
uint32_t LC_Get_Main_Voltage(void)
{
	uint8_t i = 0;
	uint16_t AD_Value = D16_Mcp3202_Read(0);
	
	float k;
	if(AD_Value <= Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].AD_value)
		return ((float)AD_Value / (float)Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].AD_value) * Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[0].Voltage;
	for(;i<Global_Cal_Facter.LC_M_Facter.LC_Cal_Point_Num;i++){
		if(AD_Value < Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].AD_value)break;
	}
	if(i == Global_Cal_Facter.LC_A_Facter.LC_Cal_Point_Num){i--;}
	i--;
	k = (float)(Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i+1].Voltage - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].Voltage) / \
	    (float)(Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i+1].AD_value - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].AD_value);
	return Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].Voltage + k * (AD_Value - Global_Cal_Facter.LC_M_Facter.LC_Cal_Point[i].AD_value);
}

uint32_t LC_Get_Selv_Voltage(void)
{
	uint16_t AD_Value = D3_Mcp3202_Read(1);
	if(Global_Cal_Facter.SELV_Facter.LC_Cal_Point[0].AD_value == 0)return 0xffff;
	return ((float)AD_Value / (float)Global_Cal_Facter.SELV_Facter.LC_Cal_Point[0].AD_value) * Global_Cal_Facter.SELV_Facter.LC_Cal_Point[0].Voltage;
}


#if LC_SAMPLING_DEBUG
uint32_t count_lc_current(uint16_t value, uint8_t net,uint8_t curdetection,uint8_t curgear)
{
 	uint16_t ADC_temp = value;
    
    return (uint32_t)((float)(ADC_temp) / (float)(Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].AD_value) \
                  * (float)(Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].Current));

}
#endif
uint16_t cur_vol;
uint32_t LC_Get_Current(uint8_t net,uint8_t curdetection,uint8_t curgear)
{
//  	uint8_t i = 0, j = 0;
//	return D3_Mcp3202_Read(0);
//	for(;i<10;i++)D3_Mcp3202_Read(0);
// 	float k;
// 	uint32_t current_temp;
// 	static uint32_t current = 0;
// 	uint16_t ad_temp[6];
 	uint16_t ADC_temp = D3_Mcp3202_Read(0);
    cur_vol = ADC_temp;
// 	for(i=0;i<6;i++)ad_temp[i] = D3_Mcp3202_Read(0);
// 	
// 	for(i=1;i<(6-1);i++)
// 		for(j=0;j<(6-i);j++){
// 			if(ad_temp[j]>ad_temp[j+1]){ADC_temp = ad_temp[j];ad_temp[j] = ad_temp[j+1];ad_temp[j+1] = ADC_temp;}
// 		}
// 		
// 	ADC_temp = (uint32_t)(ad_temp[1] + ad_temp[2] + ad_temp[3] + ad_temp[4]) / 4;
// 	k = (float)(ADC_temp) / (float)(Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].AD_value);
// 	
// 	current_temp = k * Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].Current;
// 	
// 	current = 	(current * 1 + current_temp * 3) / 4;
// 		
// 	return current;
	 	return (uint32_t)((float)(ADC_temp) / (float)(Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].AD_value) \
	                  * (float)(Global_Cal_Facter.LC_C_Facter.LC_Cal_Point[net][curdetection][curgear].Current));

	
}

void LC_Init(void)
{
	uint8_t i = 0;
	LC_Control_GPIO_Init();
	AD_CS_SET();
	AD_CS1_SET();
	CD4094_ST_CLR();
	CS5460_CS_SET();
	for(;i<NUM_OF_4094;i++)C4094_Write(i,0);
	C4094_Output();
	D16_Mcp3202_Read(0);
	D16_Mcp3202_Read(1);
	D3_Mcp3202_Read(0);
	D3_Mcp3202_Read(1);
	
	HAL_CS5460Init();//功率器件CS5460
	
	LC_Relay_Control(GB9706_1,1,1);
}
